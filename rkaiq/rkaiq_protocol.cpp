#include "rkaiq_protocol.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "rkaiq_protocol.cpp"

extern int app_run_mode;
extern int g_width;
extern int g_height;
extern std::string iqfile;
extern std::string g_sensor_name;

static int ProcessExists(const char *process_name) {
  FILE *fp;
  char cmd[1024] = {0};
  char buf[1024] = {0};
  snprintf(cmd, sizeof(cmd), "ps -ef | grep %s | grep -v grep", process_name);
  fp = popen(cmd, "r");
  if (!fp) {
    LOG_INFO("popen ps | grep %s fail\n", process_name);
    return -1;
  }
  while (fgets(buf, sizeof(buf), fp)) {
    LOG_INFO("ProcessExists %s\n", buf);
    if (strstr(buf, process_name)) {
      fclose(fp);
      return 1;
    }
  }
  fclose(fp);
  return 0;
}

int StopProcess(const char *process, const char *str) {
  int count = 0;
  while (ProcessExists(process) > 0) {
    LOG_INFO("StopProcess %s... \n", process);
    system(str);
    sleep(1);
    count++;
    if (count > 3)
      return -1;
  }
  return 0;
}

int WaitProcessExit(const char *process, int sec) {
  int count = 0;
  LOG_INFO("WaitProcessExit %s... \n", process);
  while (ProcessExists(process) > 0) {
    LOG_INFO("WaitProcessExit %s... \n", process);
    sleep(1);
    count++;
    if (count > sec)
      return -1;
  }
  return 0;
}

int RKAiqProtocol::DoChangeAppMode(appRunStatus mode) {
  if (app_run_mode == mode) {
    return 0;
  }
  if (mode == APP_RUN_STATUS_CAPTURE) {
    LOG_INFO("Switch to APP_RUN_STATUS_CAPTURE\n");
    deinit_rtsp();
    rkaiq_manager.reset();
    rkaiq_manager = nullptr;
  } else {
    LOG_INFO("Switch to APP_RUN_STATUS_TUNRING\n");
    LOG_ERROR("sensor_name %s\n", g_sensor_name.c_str());
    rkaiq_manager = std::make_shared<RKAiqToolManager>(iqfile, g_sensor_name);
    init_rtsp(g_width, g_height);
  }
  app_run_mode = mode;
  return 0;
}

static void InitCommandPingAns(CommandData_t *cmd, int ret_status) {
  strncpy((char *)cmd->RKID, RKID_CHECK, sizeof(cmd->RKID));
  cmd->cmdType = DEVICE_TO_PC;
  cmd->cmdID = ENUM_ID_CAPTURE_STATUS;
  cmd->datLen = 1;
  memset(cmd->dat, 0, sizeof(cmd->dat));
  cmd->dat[0] = ret_status;
  cmd->checkSum = 0;
  for (int i = 0; i < cmd->datLen; i++)
    cmd->checkSum += cmd->dat[i];
}

static void DoAnswer(int sockfd, CommandData_t *cmd, int cmd_id,
                     int ret_status) {
  char send_data[MAXPACKETSIZE];
  LOG_INFO("enter\n");

  strncpy((char *)cmd->RKID, TAG_OL_DEVICE_TO_PC, sizeof(cmd->RKID));
  cmd->cmdType = DEVICE_TO_PC;
  cmd->cmdID = cmd_id;
  strncpy((char *)cmd->version, RKAIQ_TOOL_VERSION, sizeof(cmd->version));
  cmd->datLen = 4;
  memset(cmd->dat, 0, sizeof(cmd->dat));
  cmd->dat[0] = ret_status;
  cmd->checkSum = 0;
  for (int i = 0; i < cmd->datLen; i++)
    cmd->checkSum += cmd->dat[i];

  memcpy(send_data, cmd, sizeof(CommandData_t));
  send(sockfd, send_data, sizeof(CommandData_t), 0);
  LOG_INFO("exit\n");
}

void RKAiqProtocol::HandlerCheckDevice(int sockfd, char *buffer, int size) {
  CommandData_t *common_cmd = (CommandData_t *)buffer;
  CommandData_t send_cmd;
  char send_data[MAXPACKETSIZE];

  LOG_INFO("HandlerRawCapMessage:\n");

  // for (int i = 0; i < common_cmd->datLen; i++) {
  //   LOG_INFO("DATA[%d]: 0x%x\n", i, common_cmd->dat[i]);
  // }

  if (strcmp((char *)common_cmd->RKID, RKID_CHECK) == 0) {
    LOG_INFO("RKID: %s\n", common_cmd->RKID);
  } else {
    LOG_INFO("RKID: Unknow\n");
    return;
  }

  LOG_INFO("cmdID: %d\n", common_cmd->cmdID);

  switch (common_cmd->cmdID) {
  case ENUM_ID_CAPTURE_STATUS:
    LOG_INFO("CmdID ENUM_ID_CAPTURE_STATUS in\n");
    if (common_cmd->dat[0] == KNOCK_KNOCK) {
      InitCommandPingAns(&send_cmd, READY);
      LOG_INFO("Device is READY\n");
    } else {
      LOG_ERROR("Unknow ENUM_ID_CAPTURE_STATUS message\n");
    }
    memcpy(send_data, &send_cmd, sizeof(CommandData_t));
    send(sockfd, send_data, sizeof(CommandData_t), 0);
    LOG_INFO("cmdID ENUM_ID_CAPTURE_STATUS out\n\n");
    break;
  case ENUM_ID_GET_STATUS:
    DoAnswer(sockfd, &send_cmd, common_cmd->cmdID, READY);
    break;
  default:
    break;
  }
}

void RKAiqProtocol::HandlerTCPMessage(int sockfd, char *buffer, int size) {
  CommandData_t *common_cmd = (CommandData_t *)buffer;
  LOG_INFO("HandlerTCPMessage:\n");
  LOG_INFO("HandlerTCPMessage CommandData_t: 0x%x\n", sizeof(CommandData_t));
  LOG_INFO("HandlerTCPMessage RKID: %s\n", (char *)common_cmd->RKID);

  // TODO Check APP Mode

  if (strcmp((char *)common_cmd->RKID, TAG_PC_TO_DEVICE) == 0) {
    DoChangeAppMode(APP_RUN_STATUS_CAPTURE);
    RKAiqRawProtocol::HandlerRawCapMessage(sockfd, buffer, size);
  } else if (strcmp((char *)common_cmd->RKID, TAG_OL_PC_TO_DEVICE) == 0) {
    DoChangeAppMode(APP_RUN_STATUS_TUNRING);
    RKAiqOLProtocol::HandlerOnLineMessage(sockfd, buffer, size);
  } else if (strcmp((char *)common_cmd->RKID, RKID_CHECK) == 0) {
    HandlerCheckDevice(sockfd, buffer, size);
  }
}