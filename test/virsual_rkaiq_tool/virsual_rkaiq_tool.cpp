#include "rkaiq_protocol.h"
#include "tcp_client.h"

#include <signal.h>

#define SERVER_PORT 5543
#define MAX_BUFFER_SIZE 8192

#pragma pack(1)
typedef struct Common_Cmd_s {
  uint8_t RKID[8];
  uint16_t cmdType;
  uint16_t cmdID;
  uint16_t datLen;
  uint8_t dat[48]; // defined by command
  uint16_t checkSum;
} Common_Cmd_t;
#pragma pack()

void sig_exit(int s) { exit(0); }

static void InitCommandCheck(Common_Cmd_t *cmd) {
  strncpy((char *)cmd->RKID, TAG_PC_TO_DEVICE, sizeof(cmd->RKID));
  cmd->cmdType = PC_TO_DEVICE;
  cmd->cmdID = CHECK_DEVICE_STATUS;
  cmd->datLen = 1;
  memset(cmd->dat, 0, sizeof(cmd->dat));
  cmd->dat[0] = 0x80;
  cmd->checkSum = 0;
  for (int i = 0; i < cmd->datLen; i++)
    cmd->checkSum += cmd->dat[i];
}

static void InitCommandRawCap0(Common_Cmd_t *cmd) {
  strncpy((char *)cmd->RKID, TAG_PC_TO_DEVICE, sizeof(cmd->RKID));
  cmd->cmdType = PC_TO_DEVICE;
  cmd->cmdID = RAW_CAPTURE;
  cmd->datLen = 2;
  memset(cmd->dat, 0, sizeof(cmd->dat));
  cmd->dat[0] = 0x00;
  cmd->dat[1] = 0x80;
  cmd->checkSum = 0;
  for (int i = 0; i < cmd->datLen; i++)
    cmd->checkSum += cmd->dat[i];
}

static void InitCommandRawCap1(Common_Cmd_t *cmd) {
  strncpy((char *)cmd->RKID, TAG_PC_TO_DEVICE, sizeof(cmd->RKID));
  cmd->cmdType = PC_TO_DEVICE;
  cmd->cmdID = RAW_CAPTURE;
  cmd->datLen = 2;
  memset(cmd->dat, 0, sizeof(cmd->dat));
  cmd->dat[0] = 0x01;
  cmd->dat[1] = 0x00;
  cmd->checkSum = 0;
  for (int i = 0; i < cmd->datLen; i++)
    cmd->checkSum += cmd->dat[i];
}

static void InitCommandRawCap2(Common_Cmd_t *cmd) {
  strncpy((char *)cmd->RKID, TAG_PC_TO_DEVICE, sizeof(cmd->RKID));
  cmd->cmdType = PC_TO_DEVICE;
  cmd->cmdID = RAW_CAPTURE;
  cmd->datLen = 2;
  memset(cmd->dat, 0, sizeof(cmd->dat));
  cmd->dat[0] = 0x02;
  cmd->dat[1] = 0x00;
  cmd->checkSum = 0;
  for (int i = 0; i < cmd->datLen; i++)
    cmd->checkSum += cmd->dat[i];
}

static void InitCommandRawCap3(Common_Cmd_t *cmd) {
  strncpy((char *)cmd->RKID, TAG_PC_TO_DEVICE, sizeof(cmd->RKID));
  cmd->cmdType = PC_TO_DEVICE;
  cmd->cmdID = RAW_CAPTURE;
  cmd->datLen = 2;
  memset(cmd->dat, 0, sizeof(cmd->dat));
  cmd->dat[0] = 0x03;
  cmd->dat[1] = 0x80;
  cmd->checkSum = 0;
  for (int i = 0; i < cmd->datLen; i++)
    cmd->checkSum += cmd->dat[i];
}

static void InitCommandRawCap4(Common_Cmd_t *cmd) {
  strncpy((char *)cmd->RKID, TAG_PC_TO_DEVICE, sizeof(cmd->RKID));
  cmd->cmdType = PC_TO_DEVICE;
  cmd->cmdID = RAW_CAPTURE;
  cmd->datLen = 2;
  memset(cmd->dat, 0, sizeof(cmd->dat));
  cmd->dat[0] = 0x04;
  cmd->dat[1] = 0x00;
  cmd->checkSum = 0;
  for (int i = 0; i < cmd->datLen; i++)
    cmd->checkSum += cmd->dat[i];
}

static void InitCommandReqAppStatus(Common_Cmd_t *cmd) {
  strncpy((char *)cmd->RKID, TAG_PC_TO_DEVICE, sizeof(cmd->RKID));
  cmd->cmdType = PC_TO_DEVICE;
  cmd->cmdID = VIDEO_APP_STATUS_REQ;
  cmd->datLen = 1;
  memset(cmd->dat, 0, sizeof(cmd->dat));
  cmd->dat[0] = 0x80;
  cmd->checkSum = 0;
  for (int i = 0; i < cmd->datLen; i++)
    cmd->checkSum += cmd->dat[i];
}

static void InitCommandSetAppStatus(Common_Cmd_t *cmd) {
  strncpy((char *)cmd->RKID, TAG_PC_TO_DEVICE, sizeof(cmd->RKID));
  cmd->cmdType = PC_TO_DEVICE;
  cmd->cmdID = VIDEO_APP_STATUS_SET;
  cmd->datLen = 1;
  memset(cmd->dat, 0, sizeof(cmd->dat));
  cmd->dat[0] = 0x80;
  cmd->checkSum = 0;
  for (int i = 0; i < cmd->datLen; i++)
    cmd->checkSum += cmd->dat[i];
}

void DumpCommand(char *buff) {
  Common_Cmd_t *common_cmd = (Common_Cmd_t *)buff;
  for (int i = 0; i < common_cmd->datLen; i++) {
    fprintf(stderr, "data[%d]: 0x%x\n", i, common_cmd->dat[i]);
  }

  char rkid[9];
  memcpy(rkid, common_cmd->RKID, 8);
  rkid[8] = '\0';
  if (strcmp("RKISP-AS", rkid) == 0) {
    fprintf(stderr, "common_cmd RKID: %s\n", common_cmd->RKID);
  } else {
    fprintf(stderr, "Unknow command RKID\n");
  }
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "Usage: ./%s ip msg_id\n", argv[0]);
    return 0;
  }
  signal(SIGINT, sig_exit);

  int msg_id = 0;
  int proc_id = 0;
  TCPClient tcp;
  tcp.Setup(argv[1], SERVER_PORT);
  msg_id = atoi(argv[2]);
  if (argc == 4)
    proc_id = atoi(argv[3]);

  int ret_val;
  Common_Cmd_t send_cmd;
  char send_data[MAX_BUFFER_SIZE];

  switch (msg_id) {
  case CHECK_DEVICE_STATUS:
    fprintf(stderr, "Check device status\n");
    InitCommandCheck(&send_cmd);
    memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
    ret_val = tcp.Send(send_data, sizeof(Common_Cmd_s));
    printf("send length %d\n", ret_val);
    tcp.Receive(send_data, MAX_BUFFER_SIZE);
    DumpCommand(send_data);
    break;
  case RAW_CAPTURE:
    switch (proc_id) {
    case RAW_CAPTURE_GET_DEVICE_STATUS:
      fprintf(stderr, "raw capture case0 Check device status\n");
      InitCommandRawCap0(&send_cmd);
      memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
      ret_val = tcp.Send(send_data, sizeof(Common_Cmd_s));
      printf("send length %d\n", ret_val);
      tcp.Receive(send_data, MAX_BUFFER_SIZE);
      DumpCommand(send_data);
      break;
    case RAW_CAPTURE_GET_PCLK_HTS_VTS:
      fprintf(stderr, "raw capture case1 GET_PCLK_HTS_VTS\n");
      InitCommandRawCap1(&send_cmd);
      memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
      ret_val = tcp.Send(send_data, sizeof(Common_Cmd_s));
      printf("send length %d\n", ret_val);
      tcp.Receive(send_data, MAX_BUFFER_SIZE);
      DumpCommand(send_data);
      break;
    case RAW_CAPTURE_SET_PARAMS:
      fprintf(stderr, "raw capture case2 SET_PARAMS\n");
      InitCommandRawCap2(&send_cmd);
      memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
      ret_val = tcp.Send(send_data, sizeof(Common_Cmd_s));
      printf("send length %d\n", ret_val);
      tcp.Receive(send_data, MAX_BUFFER_SIZE);
      DumpCommand(send_data);
      break;
    case RAW_CAPTURE_DO_CAPTURE:
      fprintf(stderr, "raw capture case3 CAPTURE\n");
      InitCommandRawCap3(&send_cmd);
      memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
      ret_val = tcp.Send(send_data, sizeof(Common_Cmd_s));
      printf("send length %d\n", ret_val);
      ret_val = tcp.Receive(send_data, MAX_BUFFER_SIZE);
      if (sizeof(Common_Cmd_s) == ret_val)
        DumpCommand(send_data);
      break;
    case RAW_CAPTURE_SEND_CHECKSUM:
      fprintf(stderr, "raw capture case3 SEND CHECKSUM\n");
      InitCommandRawCap4(&send_cmd);
      memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
      ret_val = tcp.Send(send_data, sizeof(Common_Cmd_s));
      printf("send length %d\n", ret_val);
      tcp.Receive(send_data, MAX_BUFFER_SIZE);
      DumpCommand(send_data);
      break;
    default:
      break;
    }
    break;
  case VIDEO_APP_STATUS_REQ:
    fprintf(stderr, "req app status\n");
    InitCommandReqAppStatus(&send_cmd);
    memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
    ret_val = tcp.Send(send_data, sizeof(Common_Cmd_s));
    printf("send length %d\n", ret_val);
    tcp.Receive(send_data, MAX_BUFFER_SIZE);
    DumpCommand(send_data);
    break;
  case VIDEO_APP_STATUS_SET:
    fprintf(stderr, "set app status\n");
    InitCommandSetAppStatus(&send_cmd);
    memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
    ret_val = tcp.Send(send_data, sizeof(Common_Cmd_s));
    printf("send length %d\n", ret_val);
    tcp.Receive(send_data, MAX_BUFFER_SIZE);
    DumpCommand(send_data);
    break;
  default:
    break;
  }

  return 0;
}
