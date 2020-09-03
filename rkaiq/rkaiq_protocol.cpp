#include "rkaiq_protocol.h"
#include "multiframe_process.h"
#include "tcp_server.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "rkaiq_protocol.cpp"

extern int app_run_mode;
extern int g_width;
extern int g_height;
extern std::shared_ptr<RKAiqToolManager> rkaiq_manager;

static int capture_status = READY;
static int capture_mode = CAPTURE_NORMAL;
static int capture_frames = 0;
static int capture_frames_index = 0;
static uint16_t capture_check_sum;
static struct capture_info cap_info;
static uint32_t *averge_frame0;
static uint16_t *averge_frame1;

void RunCmd(const char *cmd, char *result) {
  FILE *fp;
  fp = popen(cmd, "r");
  if (!fp) {
    LOG_INFO("[%s] fail\n", cmd);
    return;
  }
  fgets(result, 1024, fp);
  LOG_INFO("fgets result: %s\n", result);
  fclose(fp);
}

static int SetLHcg(int mode) {
  char cmd[1024] = {0};
  char result[1024] = {0};
  RunCmd("media-ctl -p -d /dev/media1 | grep sensor -B 1 -i | "
         "head -1 | cut -d ' ' -f 5",
         result);
  snprintf(cmd, sizeof(cmd), "eval find "
                             "/sys/devices/platform/ -name %s",
           result);
  RunCmd(cmd, result);
  int length = strlen(result) - 1;
  result[length] = '\0';
  snprintf(cmd, sizeof(cmd), "echo %d > %s/cam_s_cg", mode, result);
  RunCmd(cmd, result);
  LOG_INFO(" set lhcg cmd %s\n", cmd);
  return 0;
}

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
  int ret = initCamHwInfos(&cap_info);
  if (cap_info.link == link_to_isp) {
    ret = setupLink(&cap_info, true);

    if (ret < 0)
      LOG_INFO(">>>>>>>>>setup link to isp output raw failed\n");
  }
  cap_info.dev_fd = -1;
  cap_info.subdev_fd = -1;
  if (cap_info.link == link_to_isp) {
    cap_info.dev_name = cap_info.vd_path.isp_main_path;
  } else {
    cap_info.dev_name = cap_info.cif_path.cif_video_path;
  }

  cap_info.io = IO_METHOD_MMAP;
  cap_info.height = Reso->height;
  cap_info.width = Reso->width;
  // cap_info.format = v4l2_fourcc('B', 'G', '1', '2');
  LOG_INFO("get ResW: %d  ResH: %d\n", cap_info.width, cap_info.height);
}

static void RawCaptureDeinit() {
  if (cap_info.subdev_fd > 0) {
    device_close(cap_info.subdev_fd);
    cap_info.subdev_fd = -1;
    LOG_ERROR("device_close(cap_info.subdev_fd)\n");
  }
  if (cap_info.dev_fd > 0) {
    device_close(cap_info.dev_fd);
    cap_info.dev_fd = -1;
    LOG_ERROR("device_close(cap_info.dev_fd)\n");
  }
}

static void GetSensorPara(Common_Cmd_t *cmd, int ret_status) {
  struct v4l2_queryctrl ctrl;
  struct v4l2_subdev_frame_interval finterval;
  struct v4l2_subdev_format fmt;
  struct v4l2_format format;

  memset(cmd, 0, sizeof(Common_Cmd_s));

  Sensor_Params_t *sensorParam = (Sensor_Params_t *)(&cmd->dat[1]);
  int hblank, vblank;
  int vts, hts, ret;
  float fps;
  int endianness = 0;

  //cap_info.dev_fd = device_open(cap_info.dev_name);
  cap_info.subdev_fd = device_open(cap_info.sd_path.device_name);

  LOG_INFO("sensor subdev path: %s\n", cap_info.sd_path.device_name);

  memset(&ctrl, 0, sizeof(ctrl));
  ctrl.id = V4L2_CID_HBLANK;
  if (device_getblank(cap_info.subdev_fd, &ctrl) < 0) {
    // todo
    sensorParam->status = RES_FAILED;
    goto end;
  }
  hblank = ctrl.minimum;
  LOG_INFO("get hblank: %d\n", hblank);

  memset(&ctrl, 0, sizeof(ctrl));
  ctrl.id = V4L2_CID_VBLANK;
  if (device_getblank(cap_info.subdev_fd, &ctrl) < 0) {
    // todo
    sensorParam->status = RES_FAILED;
    goto end;
  }
  vblank = ctrl.minimum;
  LOG_INFO("get vblank: %d\n", vblank);

  memset(&fmt, 0, sizeof(fmt));
  fmt.pad = 0;
  fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
  if (device_getsubdevformat(cap_info.subdev_fd, &fmt) < 0) {
    sensorParam->status = RES_FAILED;
    goto end;
  }
  vts = vblank + fmt.format.height;
  hts = hblank + fmt.format.width;
  LOG_INFO("get hts: %d  vts: %d\n", hts, vts);
  cap_info.format = convert_to_v4l2fmt(&cap_info, fmt.format.code);
  cap_info.sd_path.sen_fmt = fmt.format.code;
  cap_info.sd_path.width = fmt.format.width;
  cap_info.sd_path.height = fmt.format.height;

  LOG_INFO("get sensor code: %d  bits: %d, cap_info.format:  %d\n", cap_info.sd_path.sen_fmt,
           cap_info.sd_path.bits, cap_info.format);

  /* set isp subdev fmt to bayer raw*/
  if (cap_info.link == link_to_isp) {
    ret = rkisp_set_ispsd_fmt(&cap_info, fmt.format.width, fmt.format.height,
                              fmt.format.code, cap_info.width, cap_info.height,
                              fmt.format.code);
    endianness = 1;
    LOG_INFO("rkisp_set_ispsd_fmt: %d endianness = %d\n", ret, endianness);

    if (ret) {
      LOG_ERROR("subdev choose the best fit fmt: %dx%d, 0x%08x\n",
                fmt.format.width, fmt.format.height, fmt.format.code);
      sensorParam->status = RES_FAILED;
      goto end;
    }
  } else if (cap_info.link == link_to_vicap) {
    ret = system(VICAP_COMPACT_TEST);
    LOG_INFO("VICAP_COMPACT_TEST: %d  change to no compact\n", ret);
  }


  memset(&finterval, 0, sizeof(finterval));
  finterval.pad = 0;
  if (device_getsensorfps(cap_info.subdev_fd, &finterval) < 0) {
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
  sensorParam->bits = cap_info.sd_path.bits;
  sensorParam->endianness = endianness;
  LOG_INFO("sensorParam->endianness: %d\n", endianness);

  cmd->checkSum = 0;
  for (int i = 0; i < cmd->datLen; i++) {
    cmd->checkSum += cmd->dat[i];
  }
  LOG_INFO("cmd->checkSum %d\n", cmd->checkSum);

  if (cap_info.subdev_fd > 0) {
    device_close(cap_info.subdev_fd);
    cap_info.subdev_fd = -1;
  }
  if (cap_info.dev_fd > 0) {
    device_close(cap_info.dev_fd);
    cap_info.dev_fd = -1;
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
  if (cap_info.subdev_fd > 0) {
    device_close(cap_info.subdev_fd);
    cap_info.subdev_fd = -1;
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
  cap_info.subdev_fd = device_open(cap_info.sd_path.device_name);

  capture_frames = CapParam->framenumber;
  capture_frames_index = 0;
  cap_info.frame_count = CapParam->framenumber;
  cap_info.lhcg = CapParam->lhcg;
  capture_mode = CapParam->multiframe;
  capture_check_sum = 0;

  struct v4l2_control exp;
  exp.id = V4L2_CID_EXPOSURE;
  exp.value = CapParam->time;
  struct v4l2_control gain;
  gain.id = V4L2_CID_ANALOGUE_GAIN;
  gain.value = CapParam->gain;

  if (device_setctrl(cap_info.subdev_fd, &exp) < 0) {
    LOG_ERROR(" set exposure result failed to device\n");
    ret_status = RES_FAILED;
  }
  if (device_setctrl(cap_info.subdev_fd, &gain) < 0) {
    LOG_ERROR(" set gain result failed to device\n");
    ret_status = RES_FAILED;
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

  if (cap_info.subdev_fd > 0) {
    device_close(cap_info.subdev_fd);
    cap_info.subdev_fd = -1;
  }
}

static void SendRawData(int socket, int index, void *buffer, int size) {
  LOG_INFO(" SendRawData\n");
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

  LOG_INFO("capture raw index %d, check_sum %d capture_check_sum %d\n", index,
           check_sum, capture_check_sum);
  capture_check_sum = check_sum;
}

static void DoCaptureCallBack(int socket, int index, void *buffer, int size) {
  LOG_INFO(" DoCaptureCallBack size %d\n", size);
  int width = cap_info.width;
  int height = cap_info.height;
  if (size > (width * height * 2)) {
    LOG_ERROR(" DoMultiFrameCallBack size error\n");
    return;
  }
  SendRawData(socket, index, buffer, size);
}

static void DoCapture(int socket) {
  LOG_INFO("DoCapture entry!!!!!\n");
  AutoDuration ad;
  int skip_frame = 5;

  if (capture_frames_index == 0) {
    for (int i = 0; i < skip_frame; i++) {
      if (i == 0 && cap_info.lhcg != 2)
        SetLHcg(cap_info.lhcg);
      read_frame(socket, i, &cap_info, nullptr);
      LOG_INFO("DoCapture skip frame %d ...\n", i);
    }
  }

  read_frame(socket, capture_frames_index, &cap_info, DoCaptureCallBack);
  capture_frames_index++;

  LOG_INFO("DoCapture %lld ms %lld us\n", ad.Get() / 1000, ad.Get() % 1000);
  LOG_INFO("DoCapture exit!!!!!\n");
}

static void DoMultiFrameCallBack(int socket, int index, void *buffer,
                                 int size) {
  LOG_INFO(" DoMultiFrameCallBack size %d\n", size);
  AutoDuration ad;
  int width = cap_info.width;
  int height = cap_info.height;

  if (size > (width * height * 2)) {
    LOG_ERROR(" DoMultiFrameCallBack size error\n");
    return;
  }

  DumpRawData((uint16_t *)buffer, size, 2);
  if (cap_info.link == link_to_vicap)
    MultiFrameAddition(averge_frame0, (uint16_t *)buffer, width, height, false);
  else
    MultiFrameAddition(averge_frame0, (uint16_t *)buffer, width, height);
  DumpRawData32(averge_frame0, size, 2);
  LOG_INFO("index %d MultiFrameAddition %lld ms %lld us\n", index,
           ad.Get() / 1000, ad.Get() % 1000);
  ad.Reset();
  if (index == (capture_frames - 1)) {
    MultiFrameAverage(averge_frame0, averge_frame1, width, height,
                      capture_frames);
    DumpRawData32(averge_frame0, size, 2);
    DumpRawData(averge_frame1, size, 2);
    LOG_INFO("index %d MultiFrameAverage %lld ms %lld us\n", index,
             ad.Get() / 1000, ad.Get() % 1000);
    ad.Reset();
    SendRawData(socket, index, averge_frame1, size);
    LOG_INFO("index %d SendRawData %lld ms %lld us\n", index, ad.Get() / 1000,
             ad.Get() % 1000);
  } else if (index == ((capture_frames >> 1) - 1)) {
    SendRawData(socket, index, buffer, size);
    LOG_INFO("index %d SendRawData %lld ms %lld us\n", index, ad.Get() / 1000,
             ad.Get() % 1000);
  }
}

static int InitMultiFrame() {
  uint32_t one_frame_size = cap_info.width * cap_info.height * sizeof(uint32_t);
  averge_frame0 = (uint32_t *)malloc(one_frame_size);
  one_frame_size = one_frame_size >> 1;
  averge_frame1 = (uint16_t *)malloc(one_frame_size);
  memset(averge_frame0, 0, one_frame_size);
  memset(averge_frame1, 0, one_frame_size);
  return 0;
}

static int deInitMultiFrame() {
  if (averge_frame0 != nullptr) {
    free(averge_frame0);
  }
  if (averge_frame1 != nullptr) {
    free(averge_frame1);
  }
  averge_frame0 = nullptr;
  averge_frame1 = nullptr;
  return 0;
}

static void DoMultiFrameCapture(int socket) {
  LOG_INFO("DoMultiFrameCapture entry!!!!!\n");
  AutoDuration ad;

  int skip_frame = 5;
  if (capture_frames_index == 0) {
    for (int i = 0; i < skip_frame; i++) {
      if (i == 0 && cap_info.lhcg != 2)
        SetLHcg(cap_info.lhcg);
      read_frame(socket, i, &cap_info, nullptr);
      LOG_INFO("DoCapture skip frame %d ...\n", i);
    }
  }

  if (capture_frames_index == 0) {
    for (int i = 0; i < (capture_frames >> 1); i++) {
      read_frame(socket, i, &cap_info, DoMultiFrameCallBack);
      capture_frames_index++;
    }
  } else if (capture_frames_index == (capture_frames >> 1)) {
    for (int i = (capture_frames >> 1); i < capture_frames; i++) {
      read_frame(socket, i, &cap_info, DoMultiFrameCallBack);
      capture_frames_index++;
    }
  }

  LOG_INFO("DoMultiFrameCapture %lld ms %lld us\n", ad.Get() / 1000,
           ad.Get() % 1000);
  LOG_INFO("DoMultiFrameCapture exit!!!!!\n");
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

static int StartCapture() {
  LOG_INFO(" enter\n");
  init_device(&cap_info);
  DumpCapinfo();
  start_capturing(&cap_info);
  if (capture_mode != CAPTURE_NORMAL)
    InitMultiFrame();
  LOG_INFO(" exit\n");
  return 0;
}

static int StopCapture() {
  LOG_INFO(" enter\n");
  stop_capturing(&cap_info);
  uninit_device(&cap_info);
  RawCaptureDeinit();
  if (capture_mode != CAPTURE_NORMAL)
    deInitMultiFrame();
  LOG_INFO(" exit\n");
  return 0;
}

static void RawCaputure(Common_Cmd_t *cmd, int socket) {
  LOG_INFO(" enter\n");
  LOG_INFO("capture_frames %d capture_frames_index %d\n", capture_frames,
           capture_frames_index);
  if (capture_frames_index == 0) {
    StartCapture();
  }

  if (capture_mode == CAPTURE_NORMAL)
    DoCapture(socket);
  else
    DoMultiFrameCapture(socket);

  if (capture_frames_index == capture_frames) {
    StopCapture();
  }

  LOG_INFO(" exit\n");
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
  LOG_INFO("capture_check_sum %d, recieve %d\n", capture_check_sum, *checksum);
  if (capture_check_sum == *checksum) {
    cmd->dat[1] = RES_SUCCESS;
  } else {
    cmd->dat[1] = RES_FAILED;
    StopCapture();
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
    int ret = system(STOP_RKLUNCH_CMD);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    if (ret < 0) {
      LOG_ERROR("kill app failed\n");
      ret_status = RES_FAILED;
    }
  } else if (*AppStatus == VIDEO_APP_ON) {
    LOG_INFO("run app start\n");
    if (cap_info.link == link_to_isp)
      setupLink(&cap_info, false);
    /* set isp subdev fmt to bayer raw*/
    if (rkisp_set_ispsd_fmt(&cap_info, cap_info.sd_path.width,
                            cap_info.sd_path.height, cap_info.sd_path.sen_fmt,
                            cap_info.width, cap_info.height,
                            MEDIA_BUS_FMT_YUYV8_2X8) < 0)
      LOG_ERROR("set isp subdev fmt to YUYV8_2X8 FAILED\n");
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
  int dev_fd = device_open(cap_info.dev_name);
  if (dev_fd < 0) {
    cmd->dat[0] = VIDEO_APP_ON;
    LOG_INFO("app status is ON\n");
  } else {
    cmd->dat[0] = VIDEO_APP_OFF;
    LOG_INFO("app status is OFF\n");
    device_close(dev_fd);
  }
  strncpy((char *)cmd->RKID, TAG_DEVICE_TO_PC, sizeof(cmd->RKID));
  cmd->cmdType = DEVICE_TO_PC;
  cmd->cmdID = VIDEO_APP_STATUS_REQ;
  cmd->datLen = 1;
  cmd->checkSum = 0;
  for (int i = 0; i < cmd->datLen; i++)
    cmd->checkSum += cmd->dat[i];
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
    HandlerRawCapMessage(sockfd, buffer, size);
  } else if (strcmp((char *)common_cmd->RKID, TAG_OL_PC_TO_DEVICE) == 0) {
    DoChangeAppMode(APP_RUN_STATUS_TUNRING);
    HandlerOnLineMessage(sockfd, buffer, size);
  }
}

void RKAiqProtocol::HandlerRawCapMessage(int sockfd, char *buffer, int size) {
  Common_Cmd_t *common_cmd = (Common_Cmd_t *)buffer;
  Common_Cmd_t send_cmd;
  char send_data[MAXPACKETSIZE];
  int ret_val, ret;

  LOG_INFO("HandlerRawCapMessage:\n");

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

    switch (datBuf[0]) {
    case RAW_CAPTURE_GET_DEVICE_STATUS:
      LOG_INFO("ProcID RAW_CAPTURE_GET_DEVICE_STATUS in\n");
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

void RKAiqProtocol::HandlerOnLineMessage(int sockfd, char *buffer, int size) {
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
