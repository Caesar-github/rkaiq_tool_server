#include "rkaiq_protocol.h"
#include "tcp_server.h"

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
  uint8_t isValid;
  uint32_t FPS;
  uint32_t HTS;
  uint32_t VTS;
} Sensor_Params_t;
#pragma pack()

#pragma pack(1)
typedef struct Capture_Params_s {
  uint16_t GAIN;
  uint16_t TIME;
  uint8_t Bits;
} Capture_Params_t;
#pragma pack()

#pragma pack(1)
typedef struct Capture_Reso_s {
  uint16_t Width;
  uint16_t Height;
  ;
} Capture_Reso_t;
#pragma pack()

int RKAiqProtocol::RunStatus = READY;
struct capture_info RKAiqProtocol::cap_info;
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
  capture_info *cap_info = &RKAiqProtocol::cap_info;
  char *buf = (char *)(cmd->dat);
  Capture_Reso_t *Reso = (Capture_Reso_t *)(cmd->dat + 1);
  cap_info->frame_count = 1;
  cap_info->dev_name = VIDEO_RAW0;
  cap_info->io = IO_METHOD_MMAP;
  cap_info->height = Reso->Height;
  cap_info->width = Reso->Width;
  cap_info->format = v4l2_fourcc('B', 'G', '1', '2');
  fprintf(stderr, "get ResW: %d  ResH: %d\n", cap_info->width,
          cap_info->height);
}

static void GetSensorPara(Common_Cmd_t *cmd, struct capture_info *cap_info,
                          int ret_status) {
  struct v4l2_queryctrl ctrl;
  struct v4l2_subdev_frame_interval finterval;
  struct v4l2_subdev_format fmt;
  struct v4l2_format format;

  memset(cmd, 0, sizeof(Common_Cmd_s));

  Sensor_Params_t *sensorParam = (Sensor_Params_t *)(&cmd->dat[1]);
  int horzBlank, vertBlank;
  int VTS, HTS;
  float fps;

  cap_info->dev_fd = device_open(cap_info->dev_name);
  int subdev_fd = device_open("/dev/v4l-subdev4");

  memset(&ctrl, 0, sizeof(ctrl));
  ctrl.id = V4L2_CID_HBLANK;
  if (device_getblank(cap_info->dev_fd, &ctrl) < 0) {
    // todo
    sensorParam->isValid = RES_FAILED;
    goto end;
  }
  horzBlank = ctrl.minimum;

  fprintf(stderr, "get hblank: %d\n", horzBlank);

  memset(&ctrl, 0, sizeof(ctrl));
  ctrl.id = V4L2_CID_VBLANK;
  if (device_getblank(cap_info->dev_fd, &ctrl) < 0) {
    // todo
    sensorParam->isValid = RES_FAILED;
    goto end;
  }
  vertBlank = ctrl.minimum;

  fprintf(stderr, "get vblank: %d\n", vertBlank);

#if 1
  memset(&fmt, 0, sizeof(fmt));
  fmt.pad = 0;
  fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
  if (device_getsubdevformat(subdev_fd, &fmt) < 0) {
    sensorParam->isValid = RES_FAILED;
    goto end;
  }

  VTS = vertBlank + fmt.format.height;
  HTS = horzBlank + fmt.format.width;

#endif
  // VTS = vertBlank + cap_info->height;
  // HTS = horzBlank + cap_info->width;

  fprintf(stderr, "get VTS: %d  HTS: %d\n", VTS, HTS);
#if 1
  memset(&finterval, 0, sizeof(finterval));
  finterval.pad = 0;

  if (device_getsensorfps(subdev_fd, &finterval) < 0) {
    sensorParam->isValid = RES_FAILED;
    goto end;
  }

  fps = (float)(finterval.interval.denominator) / finterval.interval.numerator;
  fprintf(stderr, "get fps: %f\n", fps);
#endif
  // fps = 30;
  strncpy((char *)cmd->RKID, TAG_DEVICE_TO_PC, sizeof(cmd->RKID));
  cmd->cmdType = DEVICE_TO_PC;
  cmd->cmdID = RAW_CAPTURE;
  cmd->datLen = 14;
  memset(cmd->dat, 0, sizeof(cmd->dat));
  cmd->dat[0] = 0x01;
  // Sensor_Params_t *sensorParam = (Sensor_Params_t *)(cmd->dat + 1);
  sensorParam->isValid = ret_status;
  sensorParam->FPS = fps;
  sensorParam->HTS = HTS;
  sensorParam->VTS = VTS;

  cmd->checkSum = 0;
  for (int i = 0; i < cmd->datLen; i++) {
    cmd->checkSum += cmd->dat[i];
  }
  fprintf(stderr, "cmd->checkSum %d\n", cmd->checkSum);

end:
  strncpy((char *)cmd->RKID, TAG_DEVICE_TO_PC, sizeof(cmd->RKID));
  cmd->cmdType = PC_TO_DEVICE;
  cmd->cmdID = RAW_CAPTURE;
  cmd->datLen = 14;
  cmd->dat[0] = 0x01;
  cmd->checkSum = 0;
  for (int i = 0; i < cmd->datLen; i++)
    cmd->checkSum += cmd->dat[i];
}

static void SetCapConf(Common_Cmd_t *recv_cmd, Common_Cmd_t *cmd,
                       struct capture_info *cap_info, int ret_status) {
  memset(cmd, 0, sizeof(Common_Cmd_s));
  Capture_Params_t *CapParam = (Capture_Params_t *)(recv_cmd->dat + 1);

  for (int i = 0; i < recv_cmd->datLen; i++) {
    fprintf(stderr, "data[%d]: 0x%x\n", i, recv_cmd->dat[i]);
  }

#if 0
	struct v4l2_ext_control exp_gain[2];
    struct v4l2_ext_controls ctrls;
	//cap_info->dev_fd = device_open(cap_info->dev_name);
	//int subdev_fd = device_open("/dev/v4l-subdev4");

	exp_gain[0].id = V4L2_CID_EXPOSURE;
	exp_gain[0].value = 1238;//CapParam->TIME;
	exp_gain[1].id = V4L2_CID_GAIN;
	exp_gain[1].value = 100;//CapParam->GAIN;


	ctrls.count = 2;
	ctrls.ctrl_class = V4L2_CTRL_CLASS_USER;
	ctrls.controls = exp_gain;
	ctrls.reserved[0] = 0;
	ctrls.reserved[1] = 0;
	fprintf(stderr, " set exposure : %d   set gain : %d \n", exp_gain[0].value, exp_gain[1].value);

	if (device_set3aexposure(cap_info->dev_fd, &ctrls) < 0) {
		fprintf(stderr, " set exposure result failed to device\n");
	}

#else

  struct v4l2_control exp;
  exp.id = V4L2_CID_EXPOSURE;
  exp.value = CapParam->TIME;
  ;
  struct v4l2_control gain;
  gain.id = V4L2_CID_ANALOGUE_GAIN;
  gain.value = CapParam->GAIN;

  fprintf(stderr, " set gain : %d   set exposure : %d \n", gain.value,
          exp.value);

  if (device_setctrl(cap_info->dev_fd, &exp) < 0) {
    fprintf(stderr, " set exposure result failed to device\n");
  }

  if (device_setctrl(cap_info->dev_fd, &gain) < 0) {
    fprintf(stderr, " set gain result failed to device\n");
  }
#endif

  strncpy((char *)cmd->RKID, TAG_DEVICE_TO_PC, sizeof(cmd->RKID));
  cmd->cmdType = DEVICE_TO_PC;
  cmd->cmdID = RAW_CAPTURE;
  cmd->datLen = 2;
  memset(cmd->dat, 0, sizeof(cmd->dat));
  cmd->dat[0] = 0x02;
  cmd->dat[1] = ret_status;
  for (int i = 0; i < cmd->datLen; i++) {
    fprintf(stderr, "data[%d]: 0x%x\n", i, cmd->dat[i]);
  }
  cmd->checkSum = 0;
  for (int i = 0; i < cmd->datLen; i++) {
    cmd->checkSum += cmd->dat[i];
  }
  fprintf(stderr, "cmd->checkSum %d\n", cmd->checkSum);
}

static void DoCaptureCallBack(int socket, void *buffer, int size) {
  fprintf(stderr, " DoCaptureCallBack!!!!!\n\n");
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
    // fprintf(stderr, "DoCaptureCallBack send raw buffer, total remaind %d\n",
    // total);
  }

  buf = (char *)buffer;
  for (int i = 0; i < size; i++) {
    check_sum += buf[i];
  }
  cap_check_sums.push_back(check_sum);
}

static void DoCapture(int socket, struct capture_info *cap_info) {
  fprintf(stderr, "\n DoCapture entry!!!!!\n\n");
  read_frame(socket, cap_info, DoCaptureCallBack);
  fprintf(stderr, "\n DoCapture exit!!!!!\n\n");
}

static void DumpCapinfo(capture_info *cap_info) {
  fprintf(stderr, "DumpCapinfo: \n");
  fprintf(stderr, "    dev_name ------------- %s\n", cap_info->dev_name);
  fprintf(stderr, "    dev_fd --------------- %d\n", cap_info->dev_fd);
  fprintf(stderr, "    io ------------------- %d\n", cap_info->io);
  fprintf(stderr, "    width ---------------- %d\n", cap_info->width);
  fprintf(stderr, "    height --------------- %d\n", cap_info->height);
  fprintf(stderr, "    format --------------- %d\n", cap_info->format);
  fprintf(stderr, "    capture_buf_type ----- %d\n",
          cap_info->capture_buf_type);
  fprintf(stderr, "    out_file ------------- %s\n", cap_info->out_file);
  fprintf(stderr, "    frame_count ---------- %d\n", cap_info->frame_count);
}

static void RawCaputure(Common_Cmd_t *cmd, struct capture_info *cap_info,
                        int socket) {
  fprintf(stderr, "\n Raw_Capture enter!!!!!!! \n\n");
  DumpCapinfo(cap_info);
  init_device(cap_info);
  start_capturing(cap_info);
  DoCapture(socket, cap_info);
  stop_capturing(cap_info);
  uninit_device(cap_info);
  fprintf(stderr, "\n Raw_Capture exit!!!!!!! \n\n");
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

  printf("######## SendRawDataResult cap_check_sums size %d\n",
         cap_check_sums.size());
  for (auto &iter : cap_check_sums) {
    fprintf(stderr, "######## cap_check_sums %d, recieve %d\n", iter,
            *checksum);
  }

  for (auto iter = cap_check_sums.begin(); iter != cap_check_sums.end();) {
    if (*iter == 9) {
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
  fprintf(stderr, ">>>>>>>>>>>Set_App_Status recv_cmd->dat[0] = %p>>\n",
          AppStatus);
  if (*AppStatus == VIDEO_APP_OFF) {
    fprintf(stderr, ">>>>>>>>>>>close app start>>>>>>>>>>>>>>>>>>>\n");
    int ret = RunCmd(STOP_RKLUNCH_CMD);
    usleep(2000000);
    if (ret < 0) {
      fprintf(stderr, "kill app failed\n");
      ret_status = RES_FAILED;
    }
  } else if (*AppStatus == VIDEO_APP_ON) {
    fprintf(stderr, ">>>>>>>>>>>open app start>>>>>>>>>>>>>>>>>>>\n");
    int ret = RunCmd(START_RKLUNCH_CMD);
    usleep(2000000);
    if (ret < 0) {
      fprintf(stderr, "open app failed\n");
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
    fprintf(stderr, "Set_App_Status cmd->dat[%d] = %d\n", i, cmd->dat[i]);
  }
}

static void ReqAppStatus(Common_Cmd_t *cmd) {
  memset(cmd->dat, 0, sizeof(cmd->dat));
  if (device_open(RKAiqProtocol::cap_info.dev_name) < 0) {
    cmd->dat[0] = VIDEO_APP_ON;
    fprintf(stderr, "video status is ON\n");
  } else {
    cmd->dat[0] = VIDEO_APP_OFF;
    fprintf(stderr, "video status is OFF\n");
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

  fprintf(stderr, "HandlerTCPMessage:\n");

  for (int i = 0; i < common_cmd->datLen; i++) {
    fprintf(stderr, "DATA[%d]: 0x%x\n", i, common_cmd->dat[i]);
  }

  if (strcmp((char *)common_cmd->RKID, TAG_PC_TO_DEVICE) == 0) {
    fprintf(stderr, "RKID: %s\n", common_cmd->RKID);
  } else {
    fprintf(stderr, "RKID: Unknow\n");
    return;
  }

  switch (common_cmd->cmdID) {
  case CHECK_DEVICE_STATUS:
    fprintf(stderr, "CmdID CHECK_DEVICE_STATUS in\n");
    if (common_cmd->dat[0] == KNOCK_KNOCK) {
      InitCommandPingAns(&send_cmd, READY);
      fprintf(stderr, "Device is READY\n");
    } else {
      fprintf(stderr, "Unknow CHECK_DEVICE_STATUS message\n");
    }
    memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
    send(sockfd, send_data, sizeof(Common_Cmd_s), 0);
    fprintf(stderr, "cmdID CHECK_DEVICE_STATUS out\n\n");
    break;
  case RAW_CAPTURE: {
    fprintf(stderr, "CmdID RAW_CAPTURE in\n");
    char *datBuf = (char *)(common_cmd->dat);
    int ret = system("dump_raw.sh");
    if (ret < 0) {
      fprintf(stderr, "cmdID RAW_CAPTURE set isp mode failed\n");
    }

    switch (datBuf[0]) {
    case RAW_CAPTURE_GET_DEVICE_STATUS:
      fprintf(stderr, "ProcID RAW_CAPTURE_GET_DEVICE_STATUS in\n");
      if (common_cmd->dat[1] == KNOCK_KNOCK) {
        if (RunStatus == RAW_CAP)
          InitCommandRawCapAns(&send_cmd, BUSY);
        InitCommandRawCapAns(&send_cmd, READY);
      } else {
        fprintf(stderr, "Unknow RAW_CAPTURE_GET_DEVICE_STATUS message\n");
      }
      memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
      send(sockfd, send_data, sizeof(Common_Cmd_s), 0);
      fprintf(stderr, "ProcID RAW_CAPTURE_GET_DEVICE_STATUS out\n");
      break;
    case RAW_CAPTURE_GET_PCLK_HTS_VTS:
      fprintf(stderr, "ProcID RAW_CAPTURE_GET_PCLK_HTS_VTS in\n");
      RawCaptureinit(common_cmd);
      GetSensorPara(&send_cmd, &RKAiqProtocol::cap_info, RES_SUCCESS);
      fprintf(stderr, "send_cmd.checkSum %d\n", send_cmd.checkSum);
      memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
      ret_val = send(sockfd, send_data, sizeof(Common_Cmd_s), 0);
      fprintf(stderr, "ProcID RAW_CAPTURE_GET_PCLK_HTS_VTS out\n");
      break;
    case RAW_CAPTURE_SET_PARAMS:
      fprintf(stderr, "ProcID RAW_CAPTURE_SET_PARAMS in\n");
      SetCapConf(common_cmd, &send_cmd, &RKAiqProtocol::cap_info, READY);
      memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
      send(sockfd, send_data, sizeof(Common_Cmd_s), 0);
      fprintf(stderr, "ProcID RAW_CAPTURE_SET_PARAMS out\n");
      break;
    case RAW_CAPTURE_DO_CAPTURE: {
      fprintf(stderr, "ProcID RAW_CAPTURE_DO_CAPTURE in\n");
      RunStatus = RAW_CAP;
      RawCaputure(&send_cmd, &cap_info, sockfd);
      RunStatus = AVALIABLE;
      fprintf(stderr, "ProcID RAW_CAPTURE_DO_CAPTURE out\n");
      break;
    }
    case RAW_CAPTURE_SEND_CHECKSUM:
      fprintf(stderr, "ProcID RAW_CAPTURE_SEND_CHECKSUM in\n");
      SendRawDataResult(&send_cmd, common_cmd);
      memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
      ret_val = send(sockfd, send_data, sizeof(Common_Cmd_s), 0);
      fprintf(stderr, "ProcID RAW_CAPTURE_SEND_CHECKSUM out\n");
      break;
    default:
      break;
    }
    fprintf(stderr, "CmdID RAW_CAPTURE out\n\n");
    break;
  }
  case VIDEO_APP_STATUS_REQ:
    fprintf(stderr, "CmdID VIDEO_APP_STATUS_REQ in\n");
    if (common_cmd->dat[0] == KNOCK_KNOCK) {
      ReqAppStatus(&send_cmd);
      fprintf(stderr, "VIDEO_APP_STATUS_REQ is DONE\n");
    } else {
      fprintf(stderr, "Unknow VIDEO_APP_STATUS_REQ message\n");
    }
    memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
    ret_val = send(sockfd, send_data, sizeof(Common_Cmd_s), 0);
    fprintf(stderr, "CmdID RAW_CAPTURE out\n\n");
    break;
  case VIDEO_APP_STATUS_SET:
    fprintf(stderr, "VIDEO_APP_STATUS_SET start\n");
    system("dump_yuv.sh");
    if (common_cmd->dat[0] == VIDEO_APP_OFF ||
        common_cmd->dat[0] == VIDEO_APP_ON) {
      fprintf(stderr, "VIDEO_APP_STATUS is on(0x80) or off(0x81) : 0x%x\n",
              common_cmd->dat[0]);
      SetAppStatus(&send_cmd, common_cmd);
      fprintf(stderr, "VIDEO_APP_STATUS_SET is DONE\n");
    } else {
      fprintf(stderr, "Unknow VIDEO_APP_STATUS_SET message\n");
    }
    memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
    send(sockfd, send_data, sizeof(Common_Cmd_s), 0);
    fprintf(stderr, "VIDEO_APP_STATUS_SET end\n\n");
    break;
  default:
    break;
  }
}
