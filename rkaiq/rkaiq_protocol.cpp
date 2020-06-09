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
buffer *RKAiqProtocol::raw_buf = new buffer();

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
  strcpy(cap_info->out_file, SAVE_RAW0_PATH);
  if ((cap_info->out_fp = fopen(cap_info->out_file, "w")) == NULL) {
    perror("Creat file failed");
    exit(0);
  }
  printf("get ResW: %d  ResH: %d\n", cap_info->width, cap_info->height);
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

  printf("get hblank: %d\n", horzBlank);

  memset(&ctrl, 0, sizeof(ctrl));
  ctrl.id = V4L2_CID_VBLANK;
  if (device_getblank(cap_info->dev_fd, &ctrl) < 0) {
    // todo
    sensorParam->isValid = RES_FAILED;
    goto end;
  }
  vertBlank = ctrl.minimum;

  printf("get vblank: %d\n", vertBlank);

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

  printf("get VTS: %d  HTS: %d\n", VTS, HTS);
#if 1
  memset(&finterval, 0, sizeof(finterval));
  finterval.pad = 0;

  if (device_getsensorfps(subdev_fd, &finterval) < 0) {
    sensorParam->isValid = RES_FAILED;
    goto end;
  }

  fps = (float)(finterval.interval.denominator) / finterval.interval.numerator;
  printf("get fps: %f\n", fps);
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
  printf("cmd->checkSum %d\n", cmd->checkSum);

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
    printf("data[%d]: 0x%x\n", i, recv_cmd->dat[i]);
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
	printf(" set exposure : %d   set gain : %d \n", exp_gain[0].value, exp_gain[1].value);

	if (device_set3aexposure(cap_info->dev_fd, &ctrls) < 0) {
		printf(" set exposure result failed to device\n");
	}

#else

  struct v4l2_control exp;
  exp.id = V4L2_CID_EXPOSURE;
  exp.value = CapParam->TIME;
  ;
  struct v4l2_control gain;
  gain.id = V4L2_CID_ANALOGUE_GAIN;
  gain.value = CapParam->GAIN;

  printf(" set gain : %d   set exposure : %d \n", gain.value, exp.value);

  if (device_setctrl(cap_info->dev_fd, &exp) < 0) {
    printf(" set exposure result failed to device\n");
  }

  if (device_setctrl(cap_info->dev_fd, &gain) < 0) {
    printf(" set gain result failed to device\n");
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
    printf("data[%d]: 0x%x\n", i, cmd->dat[i]);
  }
  cmd->checkSum = 0;
  for (int i = 0; i < cmd->datLen; i++) {
    cmd->checkSum += cmd->dat[i];
  }
  printf("cmd->checkSum %d\n", cmd->checkSum);
}

static void DoCaptureCallBack(int socket, void *buffer, int size) {
  printf(" DoCaptureCallBack!!!!!\n\n");
  char *buf = NULL;
  int total = size;
  int packet_len = MAXPACKETSIZE;
  int send_size = 0;
  int ret_val;
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
    printf("DoCaptureCallBack send raw buffer, total remaind %d\n", total);
  }
}

static void DoCapture(int socket, struct capture_info *cap_info,
                      buffer *raw_buf) {
  printf("\n DoCapture entry!!!!!\n\n");
  read_frame(socket, cap_info, DoCaptureCallBack);
  printf("\n DoCapture exit!!!!!\n\n");
}

static void DumpCapinfo(capture_info *cap_info) {
  printf("DumpCapinfo: \n");
  printf("    dev_name ------------- %s\n", cap_info->dev_name);
  printf("    dev_fd --------------- %d\n", cap_info->dev_fd);
  printf("    io ------------------- %d\n", cap_info->io);
  printf("    width ---------------- %d\n", cap_info->width);
  printf("    height --------------- %d\n", cap_info->height);
  printf("    format --------------- %d\n", cap_info->format);
  printf("    capture_buf_type ----- %d\n", cap_info->capture_buf_type);
  printf("    out_file ------------- %s\n", cap_info->out_file);
  printf("    frame_count ---------- %d\n", cap_info->frame_count);
}

static void RawCaputure(Common_Cmd_t *cmd, struct capture_info *cap_info,
                        int socket, buffer *raw_buf) {
  printf("\n Raw_Capture enter!!!!!!! \n\n");
  DumpCapinfo(cap_info);
  init_device(cap_info);
  start_capturing(cap_info);
  DoCapture(socket, cap_info, raw_buf);
  stop_capturing(cap_info);
  uninit_device(cap_info);
  fclose(cap_info->out_fp);
  printf("\n Raw_Capture exit!!!!!!! \n\n");
}

void SendRawDataResult(Common_Cmd_t *cmd, Common_Cmd_t *recv_cmd,
                       buffer *raw_buf) {
  unsigned short *checksum;
  checksum = (unsigned short *)&recv_cmd->dat[1];
  strncpy((char *)cmd->RKID, TAG_DEVICE_TO_PC, sizeof(cmd->RKID));
  cmd->cmdType = DEVICE_TO_PC;
  cmd->cmdID = RAW_CAPTURE;
  cmd->datLen = 2;
  memset(cmd->dat, 0, sizeof(cmd->dat));
  cmd->dat[0] = 0x04;
  printf("raw_buf->checksum = %d,  recieve checksum = %d\n", raw_buf->checksum,
         *checksum);
  if (*checksum == raw_buf->checksum)
    cmd->dat[1] = RES_SUCCESS;
  else
    cmd->dat[1] = RES_FAILED;
  cmd->checkSum = 0;
  for (int i = 0; i < cmd->datLen; i++)
    cmd->checkSum += cmd->dat[i];
}

static void SetAppStatus(Common_Cmd_t *cmd, Common_Cmd_t *recv_cmd) {
  int ret_status = RES_SUCCESS;
  unsigned short *AppStatus;
  AppStatus = (unsigned short *)&recv_cmd->dat[0];
  printf(">>>>>>>>>>>Set_App_Status recv_cmd->dat[0] = %p>>\n", AppStatus);
  if (*AppStatus == VIDEO_APP_OFF) {
    printf(">>>>>>>>>>>close app start>>>>>>>>>>>>>>>>>>>\n");
    int ret = RunCmd(STOP_RKLUNCH_CMD);
    usleep(2000000);
    if (ret < 0) {
      printf("kill app failed\n");
      ret_status = RES_FAILED;
    }
  } else if (*AppStatus == VIDEO_APP_ON) {
    printf(">>>>>>>>>>>open app start>>>>>>>>>>>>>>>>>>>\n");
    int ret = RunCmd(START_RKLUNCH_CMD);
    usleep(2000000);
    if (ret < 0) {
      printf("open app failed\n");
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
    printf("Set_App_Status cmd->dat[%d] = %d\n", i, cmd->dat[i]);
  }
}

static void ReqAppStatus(Common_Cmd_t *cmd) {
  memset(cmd->dat, 0, sizeof(cmd->dat));
  if (device_open(RKAiqProtocol::cap_info.dev_name) < 0) {
    cmd->dat[0] = VIDEO_APP_ON;
    printf("video status is ON\n");
  } else {
    cmd->dat[0] = VIDEO_APP_OFF;
    printf("video status is OFF\n");
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

  for (int i = 0; i < common_cmd->datLen; i++) {
    fprintf(stderr, "data[%d]: 0x%x\n", i, common_cmd->dat[i]);
  }

  if (strcmp((char *)common_cmd->RKID, TAG_PC_TO_DEVICE) == 0) {
    fprintf(stderr, "common_cmd RKID: %s\n", common_cmd->RKID);
  } else {
    fprintf(stderr, "Unknow command RKID\n");
    return;
  }

  switch (common_cmd->cmdID) {
  case CHECK_DEVICE_STATUS:
    fprintf(stderr, "Check device status\n");
    if (common_cmd->dat[0] == KNOCK_KNOCK) {
      InitCommandPingAns(&send_cmd, READY);
      fprintf(stderr, "Device is READY\n");
    } else {
      fprintf(stderr, "Unknow command message\n");
    }
    memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
    ret_val = send(sockfd, send_data, sizeof(Common_Cmd_s), 0);
    printf("send length %d\n", ret_val);
    break;
  case RAW_CAPTURE: {

    fprintf(stderr, "RAW CAPTURE\n");
    char *datBuf = (char *)(common_cmd->dat);
    int ret = system("./dump_raw.sh &");
    if (ret < 0)
      fprintf(stderr, ">>>>>>>>>set isp output raw failed\n");

    switch (datBuf[0]) {
    case RAW_CAPTURE_GET_DEVICE_STATUS:
      fprintf(stderr, "RAW_CAPTURE_GET_DEVICE_STATUS\n");
      if (common_cmd->dat[1] == KNOCK_KNOCK) {
        if (RunStatus == RAW_CAP)
          InitCommandRawCapAns(&send_cmd, BUSY);
        InitCommandRawCapAns(&send_cmd, READY);
      } else {
        fprintf(stderr, "Unknow command message\n");
      }
      memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
      ret_val = send(sockfd, send_data, sizeof(Common_Cmd_s), 0);
      printf("raw capture case0: check status send length %d\n", ret_val);
      break;
    case RAW_CAPTURE_GET_PCLK_HTS_VTS:
      fprintf(stderr, "RAW_CAPTURE_GET_PCLK_HTS_VTS\n");
      RawCaptureinit(common_cmd);
      GetSensorPara(&send_cmd, &RKAiqProtocol::cap_info, RES_SUCCESS);
      fprintf(stderr, "send_cmd.checkSum %d\n", send_cmd.checkSum);
      memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
      ret_val = send(sockfd, send_data, sizeof(Common_Cmd_s), 0);
      printf("raw capture case1: get sensor parameters send length %d\n",
             ret_val);
      break;
    case RAW_CAPTURE_SET_PARAMS:
      SetCapConf(common_cmd, &send_cmd, &RKAiqProtocol::cap_info, READY);
      printf("send_cmd.checkSum %d\n", send_cmd.checkSum);
      memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
      ret_val = send(sockfd, send_data, sizeof(Common_Cmd_s), 0);
      printf("raw capture case2: set sensor conf send length %d\n", ret_val);
      break;
    case RAW_CAPTURE_DO_CAPTURE: {
      printf("case 3 RAW CAPTURE\n");
      memset(RKAiqProtocol::raw_buf, 0, sizeof(buffer));
      RunStatus = RAW_CAP;
      RawCaputure(&send_cmd, &cap_info, sockfd, RKAiqProtocol::raw_buf);
      RunStatus = AVALIABLE;
      printf("\ncase 3 RAW CAPTURE exit\n\n");
      break;
    }
    case RAW_CAPTURE_SEND_CHECKSUM:
      printf("case 4 send raw data result\n");
      SendRawDataResult(&send_cmd, common_cmd, RKAiqProtocol::raw_buf);
      memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
      printf("send_cmd.checkSum %d\n", send_cmd.checkSum);
      ret_val = send(sockfd, send_data, sizeof(Common_Cmd_s), 0);
      printf("raw capture case4:result send length %d\n", ret_val);
      break;
    default:
      break;
    }
    break;
  }
  case VIDEO_APP_STATUS_REQ:
    printf("Req_App_Status start\n");
    if (common_cmd->dat[0] == KNOCK_KNOCK) {
      ReqAppStatus(&send_cmd);
      printf("Req_App_Status is DONE\n");
    } else {
      printf("Unknow command message\n");
    }

    memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
    ret_val = send(sockfd, send_data, sizeof(Common_Cmd_s), 0);
    printf("send length %d\n", ret_val);
    printf("Req_App_Status end\n");
    break;
  case VIDEO_APP_STATUS_SET:
    printf("VIDEO_APP_STATUS_SET start\n");
    system("./dump_yuv.sh &");
    if (common_cmd->dat[0] == VIDEO_APP_OFF ||
        common_cmd->dat[0] == VIDEO_APP_ON) {
      printf("SetAppStatus is on(128) or off(129) :%d\n", common_cmd->dat[0]);
      SetAppStatus(&send_cmd, common_cmd);
      printf("SetAppStatus is DONE\n");
    } else {
      printf("Unknow command message\n");
    }
    memcpy(send_data, &send_cmd, sizeof(Common_Cmd_s));
    ret_val = send(sockfd, send_data, sizeof(Common_Cmd_s), 0);
    printf("send length %d\n", ret_val);
    printf("VIDEO_APP_STATUS_SET end\n");
    break;
  default:
    break;
  }
}
