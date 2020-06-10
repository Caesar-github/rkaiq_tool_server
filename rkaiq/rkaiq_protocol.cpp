#include "rkaiq_protocol.h"
#include "tcp_server.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "rkaiq_protocol.cpp"

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

#pragma pack(1)
typedef struct Sensor_Params_s {
  uint8_t status;
  uint32_t fps;
  uint32_t hts;
  uint32_t vts;
} Sensor_Params_t;
#pragma pack()

#pragma pack(1)
typedef struct Capture_Params_s {
  uint16_t gain;
  uint16_t time;
  uint8_t lhcg;
  uint8_t bits;
  uint8_t framenumber;
  uint8_t multiframe;
} Capture_Params_t;
#pragma pack()

#pragma pack(1)
typedef struct Capture_Reso_s {
  uint16_t width;
  uint16_t height;
} Capture_Reso_t;
#pragma pack()

int capture_status = READY;
int capture_mode = CAPTURE_NORMAL;
struct capture_info cap_info;
static vector<uint16_t> cap_check_sums;

static int RunCmd(const char *cmd) {
  char buffer[1024];
  FILE *read_fp;
  int chars_read;
  int ret;

  memset(buffer, 0, 1024);
  read_fp = popen(cmd, "r");
  if (read_fp != NULL) {
    chars_read = fread(buffer, sizeof(char), BUFSIZ - 1, read_fp);
    if (chars_read > 0) {
      ret = 0;
    } else {
      ret = -1;
    }
    pclose(read_fp);
  } else {
    ret = -1;
  }

  return ret;
}

static void InitCommandPingAns(Common_Cmd_t *cmd, int ret_status) {
  strncpy((char *)cmd->RKID, TAG_DEVICE_TO_PC, sizeof(cmd->RKID));
  cmd->cmdType = DEVICE_TO_PC;
  cmd->cmdID = CHECK_DEVICE_STATUS;
  cmd->datLen = 1;
  memset(cmd->dat, 0, sizeof(cmd->dat));
  cmd->dat[0] = ret_status;
  cmd->checkSum = 0;
  for (int i = 0; i < cmd->datLen; i++)
    cmd->checkSum += cmd->dat[i];
}

static void InitCommandRawCapAns(Common_Cmd_t *cmd, int ret_status) {
  strncpy((char *)cmd->RKID, TAG_DEVICE_TO_PC, sizeof(cmd->RKID));
  cmd->cmdType = DEVICE_TO_PC;
  cmd->cmdID = RAW_CAPTURE;
  cmd->datLen = 2;
  memset(cmd->dat, 0, sizeof(cmd->dat));
  cmd->dat[0] = 0x00; // ProcessID
  cmd->dat[1] = ret_status;
  cmd->checkSum = 0;
  for (int i = 0; i < cmd->datLen; i++)
    cmd->checkSum += cmd->dat[i];
}

static void RawCaptureinit(Common_Cmd_t *cmd) {
  char *buf = (char *)(cmd->dat);
  Capture_Reso_t *Reso = (Capture_Reso_t *)(cmd->dat + 1);
  cap_info.dev_fd = -1;
  cap_info.dev_name = VIDEO_RAW0;
  cap_info.io = IO_METHOD_MMAP;
  cap_info.height = Reso->height;
  cap_info.width = Reso->width;
  cap_info.format = v4l2_fourcc('B', 'G', '1', '2');
  LOG_INFO("get ResW: %d  ResH: %d\n", cap_info.width, cap_info.height);
}

static void GetSensorPara(Common_Cmd_t *cmd, int ret_status) {
  struct v4l2_queryctrl ctrl;
  struct v4l2_subdev_frame_interval finterval;
  struct v4l2_subdev_format fmt;
  struct v4l2_format format;

  memset(cmd, 0, sizeof(Common_Cmd_s));

  Sensor_Params_t *sensorParam = (Sensor_Params_t *)(&cmd->dat[1]);
  int hblank, vblank;
  int vts, hts;
  float fps;

  cap_info.dev_fd = device_open(cap_info.dev_name);
  int subdev_fd = device_open("/dev/v4l-subdev4");

  memset(&ctrl, 0, sizeof(ctrl));
  ctrl.id = V4L2_CID_HBLANK;
  if (device_getblank(cap_info.dev_fd, &ctrl) < 0) {
    // todo
    sensorParam->status = RES_FAILED;
    goto end;
  }
  hblank = ctrl.minimum;
  LOG_INFO("get hblank: %d\n", hblank);

  memset(&ctrl, 0, sizeof(ctrl));
  ctrl.id = V4L2_CID_VBLANK;
  if (device_getblank(cap_info.dev_fd, &ctrl) < 0) {
    // todo
    sensorParam->status = RES_FAILED;
    goto end;
  }
  vblank = ctrl.minimum;
  LOG_INFO("get vblank: %d\n", vblank);

  memset(&fmt, 0, sizeof(fmt));
  fmt.pad = 0;
  fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
  if (device_getsubdevformat(subdev_fd, &fmt) < 0) {
    sensorParam->status = RES_FAILED;
    goto end;
  }
  vts = vblank + fmt.format.height;
  hts = hblank + fmt.format.width;
  LOG_INFO("get hts: %d  vts: %d\n", hts, vts);

  memset(&finterval, 0, sizeof(finterval));
  finterval.pad = 0;
  if (device_getsensorfps(subdev_fd, &finterval) < 0) {
    sensorParam->status = RES_FAILED;
    goto end;
  }
  fps = (float)(finterval.interval.denominator) / finterval.interval.numerator;
  LOG_INFO("get fps: %f\n", fps);

  strncpy((char *)cmd->RKID, TAG_DEVICE_TO_PC, sizeof(cmd->RKID));
  cmd->cmdType = DEVICE_TO_PC;
  cmd->cmdID = RAW_CAPTURE;
  cmd->datLen = 14;
  memset(cmd->dat, 0, sizeof(cmd->dat));
  cmd->dat[0] = 0x01;
  sensorParam->status = ret_status;
  sensorParam->fps = fps;
  sensorParam->hts = hts;
  sensorParam->vts = vts;

  cmd->checkSum = 0;
  for (int i = 0; i < cmd->datLen; i++) {
    cmd->checkSum += cmd->dat[i];
  }
  LOG_INFO("cmd->checkSum %d\n", cmd->checkSum);

  if (subdev_fd > 0) {
    device_close(subdev_fd);
    subdev_fd = -1;
  }
  return;

end:
  strncpy((char *)cmd->RKID, TAG_DEVICE_TO_PC, sizeof(cmd->RKID));
  cmd->cmdType = PC_TO_DEVICE;
  cmd->cmdID = RAW_CAPTURE;
  cmd->datLen = 14;
  cmd->dat[0] = 0x01;
  cmd->checkSum = 0;
  for (int i = 0; i < cmd->datLen; i++)
    cmd->checkSum += cmd->dat[i];
  if (subdev_fd > 0) {
    device_close(subdev_fd);
    subdev_fd = -1;
  }
}

static void SetCapConf(Common_Cmd_t *recv_cmd, Common_Cmd_t *cmd,
                       int ret_status) {
  memset(cmd, 0, sizeof(Common_Cmd_s));
  Capture_Params_t *CapParam = (Capture_Params_t *)(recv_cmd->dat + 1);

  for (int i = 0; i < recv_cmd->datLen; i++) {
    LOG_INFO("data[%d]: 0x%x\n", i, recv_cmd->dat[i]);
  }

  LOG_INFO(" set gain        : %d\n", CapParam->gain);
  LOG_INFO(" set exposure    : %d\n", CapParam->time);
  LOG_INFO(" set lhcg        : %d\n", CapParam->lhcg);
  LOG_INFO(" set bits        : %d\n", CapParam->bits);
  LOG_INFO(" set framenumber : %d\n", CapParam->framenumber);
  LOG_INFO(" set multiframe  : %d\n", CapParam->multiframe);

  struct v4l2_control exp;
  exp.id = V4L2_CID_EXPOSURE;
  exp.value = CapParam->time;
  struct v4l2_control gain;
  gain.id = V4L2_CID_ANALOGUE_GAIN;
  gain.value = CapParam->gain;

  if (device_setctrl(cap_info.dev_fd, &exp) < 0) {
    LOG_ERROR(" set exposure result failed to device\n");
  }
  if (device_setctrl(cap_info.dev_fd, &gain) < 0) {
    LOG_ERROR(" set gain result failed to device\n");
  }

  strncpy((char *)cmd->RKID, TAG_DEVICE_TO_PC, sizeof(cmd->RKID));
  cmd->cmdType = DEVICE_TO_PC;
  cmd->cmdID = RAW_CAPTURE;
  cmd->datLen = 2;
  memset(cmd->dat, 0, sizeof(cmd->dat));
  cmd->dat[0] = 0x02;
  cmd->dat[1] = ret_status;
  for (int i = 0; i < cmd->datLen; i++) {
    LOG_INFO("data[%d]: 0x%x\n", i, cmd->dat[i]);
  }
  cmd->checkSum = 0;
  for (int i = 0; i < cmd->datLen; i++) {
    cmd->checkSum += cmd->dat[i];
  }
  LOG_INFO("cmd->checkSum %d\n", cmd->checkSum);

  if (cap_info.dev_fd > 0) {
    device_close(cap_info.dev_fd);
    cap_info.dev_fd = -1;
  }
}

static void DoCaptureCallBack(int socket, void *buffer, int size) {
  LOG_INFO(" DoCaptureCallBack\n");
  char *buf = NULL;
  int total = size;
  int packet_len = MAXPACKETSIZE;
  int send_size = 0;
  int ret_val;
  uint16_t check_sum = 0;
  buf = (char *)buffer;
  while (total > 0) {
    if (total < packet_len) {
      send_size = total;
    } else {
      send_size = packet_len;
    }
    ret_val = send(socket, buf, send_size, 0);
    total -= send_size;
    buf += ret_val;
  }

  buf = (char *)buffer;
  for (int i = 0; i < size; i++) {
    check_sum += buf[i];
  }
  cap_check_sums.push_back(check_sum);
}

static void DoCapture(int socket) {
  LOG_INFO("DoCapture entry!!!!!\n");
  read_frame(socket, &cap_info, DoCaptureCallBack);
  LOG_INFO("DoCapture exit!!!!!\n");
}

static void DumpCapinfo() {
  LOG_DEBUG("DumpCapinfo: \n");
  LOG_DEBUG("    dev_name ------------- %s\n", cap_info.dev_name);
  LOG_DEBUG("    dev_fd --------------- %d\n", cap_info.dev_fd);
  LOG_DEBUG("    io ------------------- %d\n", cap_info.io);
  LOG_DEBUG("    width ---------------- %d\n", cap_info.width);
  LOG_DEBUG("    height --------------- %d\n", cap_info.height);
  LOG_DEBUG("    format --------------- %d\n", cap_info.format);
  LOG_DEBUG("    capture_buf_type ----- %d\n", cap_info.capture_buf_type);
  LOG_DEBUG("    out_file ------------- %s\n", cap_info.out_file);
  LOG_DEBUG("    frame_count ---------- %d\n", cap_info.frame_count);
}

static void RawCaputure(Common_Cmd_t *cmd, int socket) {
  LOG_INFO("Raw_Capture enter!!!!!!!\n");
  init_device(&cap_info);
  DumpCapinfo();
  start_capturing(&cap_info);
  DoCapture(socket);
  stop_capturing(&cap_info);
  uninit_device(&cap_info);
  LOG_INFO("Raw_Capture exit!!!!!!!\n");
}

static void SendRawDataResult(Common_Cmd_t *cmd, Common_Cmd_t *recv_cmd) {
  unsigned short *checksum;
  checksum = (unsigned short *)&recv_cmd->dat[1];
  strncpy((char *)cmd->RKID, TAG_DEVICE_TO_PC, sizeof(cmd->RKID));
  cmd->cmdType = DEVICE_TO_PC;
  cmd->cmdID = RAW_CAPTURE;
  cmd->datLen = 2;
  memset(cmd->dat, 0, sizeof(cmd->dat));
  cmd->dat[0] = 0x04;
  cmd->dat[1] = RES_FAILED;

  LOG_INFO("cap_check_sums size %d\n", cap_check_sums.size());
  for (auto &iter : cap_check_sums) {
    LOG_INFO("cap_check_sums %d, recieve %d\n", iter, *checksum);
  }

  for (auto iter = cap_check_sums.begin(); iter != cap_check_sums.end();) {
    if (*iter == *checksum) {
      cmd->dat[1] = RES_SUCCESS;
      iter = cap_check_sums.erase(iter);
      break;
    } else {
      iter++;
    }
  }
  cmd->checkSum = 0;
  for (int i = 0; i < cmd->datLen; i++)
    cmd->checkSum += cmd->dat[i];
}

static void SetAppStatus(Common_Cmd_t *cmd, Common_Cmd_t *recv_cmd) {
  int ret_status = RES_SUCCESS;
  unsigned short *AppStatus;
  AppStatus = (unsigned short *)&recv_cmd->dat[0];
  LOG_INFO("recv_cmd->dat[0] = %p>>\n", AppStatus);
  if (*AppStatus == VIDEO_APP_OFF) {
    LOG_INFO("kill app start\n");
    int ret = RunCmd(STOP_RKLUNCH_CMD);
    usleep(2000000);
    if (ret < 0) {
      LOG_ERROR("kill app failed\n");
      ret_status = RES_FAILED;
    }
  } else if (*AppStatus == VIDEO_APP_ON) {
    LOG_INFO("run app start\n");
    int ret = RunCmd(START_RKLUNCH_CMD);
    usleep(2000000);
    if (ret < 0) {
      LOG_ERROR("run app failed\n");
      ret_status = RES_FAILED;
    }
  }
  strncpy((char *)cmd->RKID, TAG_DEVICE_TO_PC, sizeof(cmd->RKID));
  cmd->cmdType = DEVICE_TO_PC;
  cmd->cmdID = VIDEO_APP_STATUS_SET;
  cmd->datLen = 1;
  memset(cmd->dat, 0, sizeof(cmd->dat));
  cmd->dat[0] = ret_status;
  cmd->checkSum = 0;
  for (int i = 0; i < cmd->datLen; i++) {
    cmd->checkSum += cmd->dat[i];
    LOG_INFO("Set_App_Status cmd->dat[%d] = %d\n", i, cmd->dat[i]);
  }
}

static void ReqAppStatus(Common_Cmd_t *cmd) {
  memset(cmd->dat, 0, sizeof(cmd->dat));
  if (device_open(cap_info.dev_name) < 0) {
    cmd->dat[0] = VIDEO_APP_ON;
    LOG_INFO("app status is ON\n");
  } else {
    cmd->dat[0] = VIDEO_APP_OFF;
    LOG_INFO("app status is OFF\n");
  }
  strncpy((char *)cmd->RKID, TAG_DEVICE_TO_PC, sizeof(cmd->RKID));
  cmd->cmdType = DEVICE_TO_PC;
  cmd->cmdID = VIDEO_APP_STATUS_REQ;
  cmd->datLen = 1;
  cmd->checkSum = 0;
  for (int i = 0; i < cmd->datLen; i++)
    cmd->checkSum += cmd->dat[i];
}

void RKAiqProtocol::HandlerTCPMessage(int sockfd, char *buffer, int size) {
  Common_Cmd_t *common_cmd = (Common_Cmd_t *)buffer;
  Common_Cmd_t send_cmd;
  char send_data[MAXPACKETSIZE];
  int ret_val;

  LOG_INFO("HandlerTCPMessage:\n");

  for (int i = 0; i < common_cmd->datLen; i++) {
    LOG_INFO("DATA[%d]: 0x%x\n", i, common_cmd->dat[i]);
  }

  if (strcmp((char *)common_cmd->RKID, TAG_PC_TO_DEVICE) == 0) {
    LOG_INFO("RKID: %s\n", common_cmd->RKID);
  } else {
    LOG_INFO("RKID: Unknow\n");
    return;
  }

  LOG_INFO("cmdID: %d\n", common_cmd->cmdID);

  switch (common_cmd->cmdID) {
  case CHECK_DEVICE_STATUS:
    LOG_INFO("CmdID CHECK_DEVICE_STATUS in\n");
    if (common_cmd->dat[0] == KNOCK_KNOCK) {
      InitCommandPingAns(&send_cmd, READY);
      LOG_INFO("Device is READY\n");
    } else {
      LOG_ERROR("Unknow CHECK_DEVICE_STATUS message\n");
    }
    memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
    send(sockfd, send_data, sizeof(Common_Cmd_s), 0);
    LOG_INFO("cmdID CHECK_DEVICE_STATUS out\n\n");
    break;
  case RAW_CAPTURE: {
    LOG_INFO("CmdID RAW_CAPTURE in\n");
    char *datBuf = (char *)(common_cmd->dat);
    int ret = system("dump_raw.sh");
    if (ret < 0) {
      LOG_ERROR("cmdID RAW_CAPTURE set isp mode failed\n");
    }

    switch (datBuf[0]) {
    case RAW_CAPTURE_GET_DEVICE_STATUS:
      LOG_INFO("ProcID RAW_CAPTURE_GET_DEVICE_STATUS in\n");
      cap_check_sums.clear();
      if (common_cmd->dat[1] == KNOCK_KNOCK) {
        if (capture_status == RAW_CAP) {
          LOG_INFO("capture_status BUSY\n");
          InitCommandRawCapAns(&send_cmd, BUSY);
        } else {
          LOG_INFO("capture_status READY\n");
          InitCommandRawCapAns(&send_cmd, READY);
        }
      } else {
        LOG_ERROR("Unknow RAW_CAPTURE_GET_DEVICE_STATUS message\n");
      }
      memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
      send(sockfd, send_data, sizeof(Common_Cmd_s), 0);
      LOG_INFO("ProcID RAW_CAPTURE_GET_DEVICE_STATUS out\n");
      break;
    case RAW_CAPTURE_GET_PCLK_HTS_VTS:
      LOG_INFO("ProcID RAW_CAPTURE_GET_PCLK_HTS_VTS in\n");
      RawCaptureinit(common_cmd);
      GetSensorPara(&send_cmd, RES_SUCCESS);
      LOG_INFO("send_cmd.checkSum %d\n", send_cmd.checkSum);
      memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
      ret_val = send(sockfd, send_data, sizeof(Common_Cmd_s), 0);
      LOG_INFO("ProcID RAW_CAPTURE_GET_PCLK_HTS_VTS out\n");
      break;
    case RAW_CAPTURE_SET_PARAMS:
      LOG_INFO("ProcID RAW_CAPTURE_SET_PARAMS in\n");
      SetCapConf(common_cmd, &send_cmd, READY);
      memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
      send(sockfd, send_data, sizeof(Common_Cmd_s), 0);
      LOG_INFO("ProcID RAW_CAPTURE_SET_PARAMS out\n");
      break;
    case RAW_CAPTURE_DO_CAPTURE: {
      LOG_INFO("ProcID RAW_CAPTURE_DO_CAPTURE in\n");
      capture_status = RAW_CAP;
      RawCaputure(&send_cmd, sockfd);
      capture_status = AVALIABLE;
      LOG_INFO("ProcID RAW_CAPTURE_DO_CAPTURE out\n");
      break;
    }
    case RAW_CAPTURE_COMPARE_CHECKSUM:
      LOG_INFO("ProcID RAW_CAPTURE_COMPARE_CHECKSUM in\n");
      SendRawDataResult(&send_cmd, common_cmd);
      memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
      ret_val = send(sockfd, send_data, sizeof(Common_Cmd_s), 0);
      LOG_INFO("ProcID RAW_CAPTURE_COMPARE_CHECKSUM out\n");
      break;
    default:
      break;
    }
    LOG_INFO("CmdID RAW_CAPTURE out\n\n");
    break;
  }
  case VIDEO_APP_STATUS_REQ:
    LOG_INFO("CmdID VIDEO_APP_STATUS_REQ in\n");
    if (common_cmd->dat[0] == KNOCK_KNOCK) {
      ReqAppStatus(&send_cmd);
      LOG_INFO("VIDEO_APP_STATUS_REQ is DONE\n");
    } else {
      LOG_ERROR("Unknow VIDEO_APP_STATUS_REQ message\n");
    }
    memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
    ret_val = send(sockfd, send_data, sizeof(Common_Cmd_s), 0);
    LOG_INFO("CmdID RAW_CAPTURE out\n\n");
    break;
  case VIDEO_APP_STATUS_SET:
    LOG_INFO("VIDEO_APP_STATUS_SET start\n");
    system("dump_yuv.sh");
    if (common_cmd->dat[0] == VIDEO_APP_OFF ||
        common_cmd->dat[0] == VIDEO_APP_ON) {
      LOG_INFO("VIDEO_APP_STATUS is on(0x80) or off(0x81) : 0x%x\n",
               common_cmd->dat[0]);
      SetAppStatus(&send_cmd, common_cmd);
      LOG_INFO("VIDEO_APP_STATUS_SET is DONE\n");
    } else {
      LOG_ERROR("Unknow VIDEO_APP_STATUS_SET message\n");
    }
    memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
    send(sockfd, send_data, sizeof(Common_Cmd_s), 0);
    LOG_INFO("VIDEO_APP_STATUS_SET end\n\n");
    break;
  default:
    break;
  }
}
