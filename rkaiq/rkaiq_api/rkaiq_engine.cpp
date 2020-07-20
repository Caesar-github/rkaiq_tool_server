#include "rkaiq_engine.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "rkaiq_engine.cpp"

#define CIFISP_V4L2_EVENT_STREAM_START (V4L2_EVENT_PRIVATE_START + 1)
#define CIFISP_V4L2_EVENT_STREAM_STOP (V4L2_EVENT_PRIVATE_START + 2)

int RKAiqEngine::thread_quit_ = 0;

int RKAiqEngine::GetDevName(struct media_device *device, const char *name,
                            char *dev_name) {
  const char *devname;
  struct media_entity *entity = NULL;

  entity = media_get_entity_by_name(device, name, strlen(name));
  if (!entity)
    return -1;

  devname = media_entity_get_devname(entity);

  if (!devname) {
    LOG_ERROR("can't find %s device path!", name);
    return -1;
  }

  strncpy(dev_name, devname, RKAIQ_FILE_PATH_LEN);

  LOG_DEBUG("get %s devname: %s\n", name, dev_name);

  return 0;
}

int RKAiqEngine::EnumrateModules(struct media_device *device,
                                 struct rkaiq_media_info *media_info) {
  uint32_t nents, i;
  const char *dev_name = NULL;
  int active_sensor = -1;

  nents = media_get_entities_count(device);
  for (i = 0; i < nents; ++i) {
    int module_idx = -1;
    struct media_entity *e;
    const struct media_entity_desc *ef;
    const struct media_link *link;

    e = media_get_entity(device, i);
    ef = media_entity_get_info(e);

    if (ef->type != MEDIA_ENT_T_V4L2_SUBDEV_SENSOR &&
        ef->type != MEDIA_ENT_T_V4L2_SUBDEV_FLASH &&
        ef->type != MEDIA_ENT_T_V4L2_SUBDEV_LENS)
      continue;

    if (ef->name[0] != 'm' && ef->name[3] != '_') {
      LOG_ERROR("sensor/lens/flash entity name format is incorrect,"
                "pls check driver version !\n");
      return -1;
    }

    /* Retrive the sensor index from sensor name,
      * which is indicated by two characters after 'm',
      *  e.g.  m00_b_ov13850 1-0010
      *         ^^, 00 is the module index
      */
    module_idx = atoi(ef->name + 1);
    if (module_idx >= RKAIQ_CAMS_NUM_MAX) {
      LOG_ERROR("sensors more than two not supported, %s\n", ef->name);
      continue;
    }

    dev_name = media_entity_get_devname(e);

    switch (ef->type) {
    case MEDIA_ENT_T_V4L2_SUBDEV_SENSOR:
      strncpy(media_info->cams[module_idx].sd_sensor_path, dev_name,
              RKAIQ_FILE_PATH_LEN);

      link = media_entity_get_link(e, 0);
      if (link && (link->flags & MEDIA_LNK_FL_ENABLED)) {
        media_info->cams[module_idx].link_enabled = true;
        active_sensor = module_idx;
        strcpy(media_info->cams[module_idx].sensor_entity_name, ef->name);
      }
      break;
    case MEDIA_ENT_T_V4L2_SUBDEV_FLASH:
      // TODO, support multiple flashes attached to one module
      strncpy(media_info->cams[module_idx].sd_flash_path[0], dev_name,
              RKAIQ_FILE_PATH_LEN);
      break;
    case MEDIA_ENT_T_V4L2_SUBDEV_LENS:
      strncpy(media_info->cams[module_idx].sd_lens_path, dev_name,
              RKAIQ_FILE_PATH_LEN);
      break;
    default:
      break;
    }
  }

  if (active_sensor < 0) {
    LOG_ERROR("Not sensor link is enabled, does sensor probe correctly?\n");
    return -1;
  }

  return 0;
}

int RKAiqEngine::GetMedia0Info(struct rkaiq_media_info *media_info_) {
  struct media_device *device = NULL;
  int ret;

  device = media_device_new("/dev/media0");
  if (!device)
    return -ENOMEM;
  /* Enumerate entities, pads and links. */
  ret = media_device_enumerate(device);
  if (ret)
    return ret;
  if (!ret) {
    /* Try rkisp */
    ret = GetDevName(device, "rkisp-isp-subdev", media_info_->sd_isp_path);
    ret |=
        GetDevName(device, "rkisp-input-params", media_info_->vd_params_path);
    ret |= GetDevName(device, "rkisp-statistics", media_info_->vd_stats_path);
  }
  if (ret) {
    media_device_unref(device);
    return ret;
  }

  ret = EnumrateModules(device, media_info_);
  media_device_unref(device);

  return ret;
}

int RKAiqEngine::GetMedia1Info(struct rkaiq_media_info *media_info_) {
  struct media_device *device = NULL;
  int ret;

  device = media_device_new("/dev/media1");
  if (!device)
    return -ENOMEM;
  /* Enumerate entities, pads and links. */
  ret = media_device_enumerate(device);
  if (ret)
    return ret;
  if (!ret) {
    /* Try rkisp */
    ret = GetDevName(device, "rkispp_input_params", media_info_->sd_ispp_path);
  }
  if (ret) {
    media_device_unref(device);
    return ret;
  }

  media_device_unref(device);

  return ret;
}

int RKAiqEngine::WaitStreamEvent(int fd, unsigned int event_type,
                                 int time_out_ms) {
  int ret;
  struct v4l2_event event;

  memset(&event, 0, sizeof(event));

  do {
    /*
     * xioctl instead of poll.
     * Since poll() cannot wait for input before stream on,
     * it will return an error directly. So, use ioctl to
     * dequeue event and block until sucess.
     */
    ret = xioctl(fd, VIDIOC_DQEVENT, &event);
    if (ret == 0 && event.type == event_type) {
      return 0;
    }
  } while (true);

  return -1;
}

int RKAiqEngine::SubcribleStreamEvent(int fd, bool subs) {
  struct v4l2_event_subscription sub;
  int ret = 0;

  memset(&sub, 0, sizeof(sub));
  sub.type = CIFISP_V4L2_EVENT_STREAM_START;
  ret = xioctl(fd, subs ? VIDIOC_SUBSCRIBE_EVENT : VIDIOC_UNSUBSCRIBE_EVENT,
               &sub);
  if (ret) {
    LOG_ERROR("can't subscribe %s start event!\n", media_info_.sd_ispp_path);
    exit(EXIT_FAILURE);
  }

  memset(&sub, 0, sizeof(sub));
  sub.type = CIFISP_V4L2_EVENT_STREAM_STOP;
  ret = xioctl(fd, subs ? VIDIOC_SUBSCRIBE_EVENT : VIDIOC_UNSUBSCRIBE_EVENT,
               &sub);
  if (ret) {
    LOG_ERROR("can't subscribe %s stop event!\n", media_info_.vd_params_path);
  }

  LOG_ERROR("subscribe events from %s success !\n", media_info_.vd_params_path);

  return 0;
}

int RKAiqEngine::InitEngine() {
  int index;

  char *hdr_mode = getenv("HDR_MODE");
  if (hdr_mode) {
    LOG_DEBUG("hdr mode: %s \n", hdr_mode);
    if (0 == atoi(hdr_mode))
      mode_ = RK_AIQ_WORKING_MODE_NORMAL;
    else if (1 == atoi(hdr_mode))
      mode_ = RK_AIQ_WORKING_MODE_ISP_HDR2;
    else if (2 == atoi(hdr_mode))
      mode_ = RK_AIQ_WORKING_MODE_ISP_HDR3;
  }

  std::string sensor_entity_name;
  std::string iq_file_dir;
  int width, height;

  ctx_ = rk_aiq_uapi_sysctl_init(sensor_entity_name.c_str(),
                                 iq_file_dir.c_str(), NULL, NULL);

  if (rk_aiq_uapi_sysctl_prepare(ctx_, width, height, mode_)) {
    LOG_DEBUG("rkaiq engine prepare failed !\n");
    return -1;
  }
  return 0;
}

int RKAiqEngine::StartEngine() {
  rk_aiq_uapi_sysctl_start(ctx_);
  if (ctx_ == NULL) {
    LOG_DEBUG("rkisp_init engine failed\n");
    return -1;
  } else {
    LOG_DEBUG("rkisp_init engine succeed\n");
  }
  return 0;
}

int RKAiqEngine::StopEngine() {
  rk_aiq_uapi_sysctl_stop(ctx_);
  return 0;
}

int RKAiqEngine::DeInitEngine() {
  rk_aiq_uapi_sysctl_deinit(ctx_);
  return 0;
}

void RKAiqEngine::RKAiqEngineLoop(void *arg) {
  int ret = 0;
  int isp_fd;
  unsigned int stream_event = -1;
  RKAiqEngine *rkaiq_engine = (RKAiqEngine *)arg;

  LOG_DEBUG("RKAiqEngineLoop entry\n");

  isp_fd = open(rkaiq_engine->media_info_.sd_ispp_path, O_RDWR);
  if (isp_fd < 0) {
    LOG_ERROR("open %s failed %s\n", rkaiq_engine->media_info_.sd_ispp_path,
              strerror(errno));
    return;
  }
  rkaiq_engine->SubcribleStreamEvent(isp_fd, true);
  rkaiq_engine->InitEngine();

  while (!thread_quit_) {
    LOG_DEBUG("wait stream start event...\n");
    rkaiq_engine->WaitStreamEvent(isp_fd, CIFISP_V4L2_EVENT_STREAM_START, -1);
    LOG_DEBUG("wait stream start event success ...\n");
    rkaiq_engine->StartEngine();
    LOG_DEBUG("wait stream stop event...\n");
    rkaiq_engine->WaitStreamEvent(isp_fd, CIFISP_V4L2_EVENT_STREAM_STOP, -1);
    LOG_DEBUG("wait stream stop event success ...\n");
    LOG_DEBUG("stop engine...\n");
    rkaiq_engine->StopEngine();
  }

  rkaiq_engine->DeInitEngine();
  rkaiq_engine->SubcribleStreamEvent(isp_fd, false);
  close(isp_fd);
  LOG_DEBUG("RKAiqEngineLoop exit\n\n");
}

RKAiqEngine::RKAiqEngine() : ctx_(nullptr) {
  if (GetMedia0Info(&media_info_)) {
    LOG_DEBUG("Bad media0 topology error %d, %s\n", errno, strerror(errno));
    return;
  }
  if (GetMedia1Info(&media_info_)) {
    LOG_DEBUG("Bad media1 topology error %d, %s\n", errno, strerror(errno));
    return;
  }
  rkaiq_engine_thread_ = new std::thread(RKAiqEngineLoop, this);
}

RKAiqEngine::~RKAiqEngine() {
  if (rkaiq_engine_thread_) {
    rkaiq_engine_thread_->join();
    delete rkaiq_engine_thread_;
    rkaiq_engine_thread_ = nullptr;
  }
}
