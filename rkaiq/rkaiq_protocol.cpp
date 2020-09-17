#include "rkaiq_protocol.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "rkaiq_protocol.cpp"

extern int app_run_mode;
extern int g_width;
extern int g_height;

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
    rkaiq_manager = std::make_shared<RKAiqToolManager>();
    init_rtsp(g_width, g_height);
  }
  app_run_mode = mode;
  return 0;
}

void RKAiqProtocol::HandlerTCPMessage(int sockfd, char *buffer, int size) {
  Common_Cmd_t *common_cmd = (Common_Cmd_t *)buffer;
  LOG_INFO("HandlerTCPMessage:\n");
  LOG_INFO("HandlerTCPMessage Common_Cmd_t: 0x%x\n", sizeof(Common_Cmd_t));
  LOG_INFO("HandlerTCPMessage RKID: %s\n", (char *)common_cmd->RKID);

  // TODO Check APP Mode

  if (strcmp((char *)common_cmd->RKID, TAG_PC_TO_DEVICE) == 0) {
    DoChangeAppMode(APP_RUN_STATUS_CAPTURE);
    RKAiqRawProtocol::HandlerRawCapMessage(sockfd, buffer, size);
  } else if (strcmp((char *)common_cmd->RKID, TAG_OL_PC_TO_DEVICE) == 0) {
    DoChangeAppMode(APP_RUN_STATUS_TUNRING);
    RKAiqOLProtocol::HandlerOnLineMessage(sockfd, buffer, size);
  }
}