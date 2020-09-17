#include "rkaiq_online_protocol.h"

static void DoAnswer(int sockfd, Common_OL_Cmd_t *cmd, int cmd_id,
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

  memcpy(send_data, cmd, sizeof(Common_OL_Cmd_s));
  send(sockfd, send_data, sizeof(Common_OL_Cmd_s), 0);
  LOG_INFO("exit\n");
}

static void DoAnswer2(int sockfd, Common_OL_Cmd_t *cmd, int cmd_id,
                      uint16_t check_sum, uint32_t result) {
  char send_data[MAXPACKETSIZE];
  LOG_INFO("enter\n");
  strncpy((char *)cmd->RKID, TAG_OL_DEVICE_TO_PC, sizeof(cmd->RKID));
  cmd->cmdType = DEVICE_TO_PC;
  cmd->cmdID = cmd_id;
  strncpy((char *)cmd->version, RKAIQ_TOOL_VERSION, sizeof(cmd->version));
  cmd->datLen = 4;
  memset(cmd->dat, 0, sizeof(cmd->dat));
  cmd->dat[0] = result;
  cmd->dat[1] = check_sum & 0xFF;
  cmd->dat[2] = (check_sum >> 8) & 0xFF;
  cmd->checkSum = 0;
  for (int i = 0; i < cmd->datLen; i++)
    cmd->checkSum += cmd->dat[i];

  memcpy(send_data, cmd, sizeof(Common_OL_Cmd_s));
  send(sockfd, send_data, sizeof(Common_OL_Cmd_s), 0);
  LOG_INFO("exit\n");
}

static int DoCheckSum(int sockfd, uint16_t check_sum) {
  char recv_data[MAXPACKETSIZE];
  int recv_size = 0;
  int param_size = sizeof(Common_OL_Cmd_s);
  int remain_size = param_size;
  int try_count = 3;
  LOG_INFO("enter\n");

  struct timeval interval = {3, 0};
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&interval,
             sizeof(struct timeval));

  while (remain_size > 0) {
    int offset = param_size - remain_size;
    recv_size = recv(sockfd, recv_data + offset, remain_size, 0);
    if (recv_size < 0) {
      if (errno == EAGAIN) {
        LOG_INFO("recv size %d, do try again, count %d\n", recv_size,
                 try_count);
        try_count--;
        if (try_count < 0)
          break;
        continue;
      } else {
        LOG_ERROR("Error socket recv failed %d\n", errno);
      }
    }
    remain_size = remain_size - recv_size;
  }
  LOG_INFO("recv_size: 0x%x expect 0x%x\n", recv_size, sizeof(Common_OL_Cmd_s));

  Common_OL_Cmd_t *cmd = (Common_OL_Cmd_t *)recv_data;
  uint16_t recv_check_sum = 0;
  recv_check_sum += cmd->dat[0] & 0xff;
  recv_check_sum += (cmd->dat[1] & 0xff) << 8;
  LOG_INFO("check_sum local: 0x%x pc: 0x%x\n", check_sum, recv_check_sum);

  if (check_sum != recv_check_sum) {
    LOG_INFO("check_sum fail!\n");
    return -1;
  }

  LOG_INFO("exit\n");
  return 0;
}

static void OnLineSet(int sockfd, Common_OL_Cmd_t *cmd, uint16_t &check_sum,
                      uint32_t &result) {
  int recv_size = 0;
  int param_size = *(int *)cmd->dat;
  int remain_size = param_size;

  LOG_INFO("enter\n");
  LOG_INFO("expect recv param_size 0x%x\n", param_size);
  char *param = (char *)malloc(param_size);
  while (remain_size > 0) {
    int offset = param_size - remain_size;
    recv_size = recv(sockfd, param + offset, remain_size, 0);
    remain_size = remain_size - recv_size;
  }

  LOG_INFO("recv ready\n");

  for (int i = 0; i < param_size; i++)
    check_sum += param[i];

  LOG_INFO("DO Sycn Setting, CmdId: 0x%x, expect ParamSize 0x%x\n", cmd->cmdID,
           param_size);
  if (rkaiq_manager)
    result = rkaiq_manager->IoCtrl(cmd->cmdID, param, param_size);

  free(param);
  LOG_INFO("exit\n");
}

static int OnLineGet(int sockfd, Common_OL_Cmd_t *cmd) {
  int ret = 0;
  int send_size = 0;
  int param_size = *(int *)cmd->dat;
  int remain_size = param_size;
  LOG_INFO("enter\n");
  LOG_INFO("ParamSize: 0x%x\n", param_size);

  uint8_t *param = (uint8_t *)malloc(param_size);

  LOG_INFO("DO Get Setting, CmdId: 0x%x, expect ParamSize 0x%x\n", cmd->cmdID,
           param_size);
  if (rkaiq_manager)
    rkaiq_manager->IoCtrl(cmd->cmdID, param, param_size);

  while (remain_size > 0) {
    int offset = param_size - remain_size;
    send_size = send(sockfd, param + offset, remain_size, 0);
    remain_size = param_size - send_size;
  }

  uint16_t check_sum = 0;
  for (int i = 0; i < param_size; i++)
    check_sum += param[i];

  ret = DoCheckSum(sockfd, check_sum);

  free(param);
  LOG_INFO("exit\n");
  return ret;
}

void RKAiqOLProtocol::HandlerOnLineMessage(int sockfd, char *buffer, int size) {
  Common_OL_Cmd_t *common_cmd = (Common_OL_Cmd_t *)buffer;
  Common_OL_Cmd_t send_cmd;
  int ret_val, ret;

  LOG_INFO("HandlerOnLineMessage:\n");
  LOG_INFO("DATA datLen: 0x%x\n", common_cmd->datLen);

  if (strcmp((char *)common_cmd->RKID, TAG_OL_PC_TO_DEVICE) == 0) {
    LOG_INFO("RKID: %s\n", common_cmd->RKID);
  } else {
    LOG_INFO("RKID: Unknow\n");
    return;
  }

  LOG_INFO("cmdID: 0x%x cmdType: 0x%x\n", common_cmd->cmdID,
           common_cmd->cmdType);

  switch (common_cmd->cmdType) {
  case PACKET_TYPE_STATUS:
    DoAnswer(sockfd, &send_cmd, common_cmd->cmdID, READY);
    break;
  case PACKET_TYPE_SET: {
    uint16_t check_sum;
    uint32_t result;
    DoAnswer(sockfd, &send_cmd, common_cmd->cmdID, READY);
    OnLineSet(sockfd, common_cmd, check_sum, result);
    DoAnswer2(sockfd, &send_cmd, common_cmd->cmdID, check_sum,
              result ? RES_FAILED : RES_SUCCESS);
  } break;
  case PACKET_TYPE_GET:
    ret = OnLineGet(sockfd, common_cmd);
    if (ret == 0)
      DoAnswer(sockfd, &send_cmd, common_cmd->cmdID, RES_SUCCESS);
    else
      DoAnswer(sockfd, &send_cmd, common_cmd->cmdID, RES_FAILED);
    break;
  default:
    LOG_INFO("cmdID: Unknow\n");
    break;
  }
}
