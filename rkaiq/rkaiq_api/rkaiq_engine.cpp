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

  entity = media_get_entity_by_name(device, name);
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

int RKAiqEngine::LinkToIsp() {
  int ret;
  int index = 0;
  char sys_path[64];
  media_device *device = NULL;
  media_entity *entity = NULL;
  media_pad *src_pad = NULL, *sink_pad_bridge = NULL, *sink_pad_mp = NULL;
  while (index < 10) {
    snprintf(sys_path, 64, "/dev/media%d", index++);
    if (access(sys_path, F_OK))
      continue;
    device = media_device_new(sys_path);
    if (device == NULL) {
      LOG_ERROR("Failed to create media %s\n", sys_path);
      continue;
    }
    ret = media_device_enumerate(device);
    if (ret < 0) {
      LOG_ERROR("Failed to enumerate %s (%d)\n", sys_path, ret);
      media_device_unref(device);
      continue;
    }
    const struct media_device_info *info = media_get_info(device);
    LOG_INFO("%s: model %s\n", sys_path, info->model);
    if (strcmp(info->model, "rkisp0") != 0 &&
        strcmp(info->model, "rkisp1") != 0 &&
        strcmp(info->model, "rkisp") != 0) {
      media_device_unref(device);
      continue;
    }
    LOG_INFO("%s: setup link to isp\n", sys_path);
    entity = media_get_entity_by_name(device, "rkisp-isp-subdev");
    if (entity) {
      const struct media_entity_desc *info = media_entity_get_info(entity);
      src_pad = (media_pad *)media_entity_get_pad(entity, 2);
      if (!src_pad) {
        LOG_ERROR("get rkisp-isp-subdev source pad failed!\n");
      }

      struct v4l2_mbus_framefmt format;
      ret = v4l2_subdev_get_format(src_pad->entity, &format, src_pad->index,
                                   V4L2_SUBDEV_FORMAT_ACTIVE);
      if (ret != 0)
        LOG_ERROR("v4l2_subdev_get_format failed!\n");
      char set_fmt[128];
      sprintf(set_fmt, "\"%s\":%d [fmt:%s/%dx%d field:none]", info->name,
              src_pad->index, "YUYV8_2X8", format.width, format.height);
      ret = v4l2_subdev_parse_setup_formats(device, set_fmt);
      if (ret)
        LOG_ERROR("Unable to setup formats: %s (%d)\n", strerror(-ret), -ret);
    }

    entity = media_get_entity_by_name(device, "rkisp-bridge-ispp");
    if (entity) {
      sink_pad_bridge = (media_pad *)media_entity_get_pad(entity, 0);
      if (!sink_pad_bridge) {
        LOG_ERROR("get rkisp-bridge-ispp sink pad failed!\n");
      }
    }
    entity = media_get_entity_by_name(device, "rkisp_mainpath");
    if (entity) {
      sink_pad_mp = (media_pad *)media_entity_get_pad(entity, 0);
      if (!sink_pad_mp) {
        LOG_ERROR("get rkisp_mainpath sink pad failed!\n");
      }
    }
    if (src_pad && sink_pad_bridge && sink_pad_mp) {
      ret = media_setup_link(device, src_pad, sink_pad_mp, 0);
      if (ret)
        LOG_ERROR("media_setup_link sink_pad_bridge FAILED: %d\n", ret);
      ret = media_setup_link(device, src_pad, sink_pad_bridge,
                             MEDIA_LNK_FL_ENABLED);
      if (ret)
        LOG_ERROR("media_setup_link sink_pad_bridge FAILED: %d\n", ret);
    }

    // ret = v4l2_subdev_parse_setup_formats(device, "crop:(0,0)/2688x1520");

    media_device_unref(device);
  }
  return ret;
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
        sensor_entity_name_ = ef->name;
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

int RKAiqEngine::GetMediaInfo(struct rkaiq_media_info *media_info_) {
  struct media_device *device = NULL;
  int ret;
  char sys_path[64];
  unsigned int index = 0, i;
  int find_sensor = 0;
  int find_isp = 0;
  int find_ispp = 0;

  while (index < 10) {
    snprintf(sys_path, 64, "/dev/media%d", index++);
    if (access(sys_path, F_OK))
      continue;

    LOG_ERROR("access %s\n", sys_path);

    device = media_device_new(sys_path);
    if (!device)
      return -ENOMEM;

    ret = media_device_enumerate(device);
    if (ret) {
      media_device_unref(device);
      return ret;
    }

    /* Try Sensor */
    if (!find_sensor && !ret) {
      unsigned int nents = media_get_entities_count(device);
      for (i = 0; i < nents; ++i) {
        struct media_entity *entity = media_get_entity(device, i);
        const struct media_entity_desc *info = media_entity_get_info(entity);
        unsigned int type = info->type;
        if (MEDIA_ENT_T_V4L2_SUBDEV == (type & MEDIA_ENT_TYPE_MASK)) {
          unsigned int subtype = type & MEDIA_ENT_SUBTYPE_MASK;
          if (subtype == 1) {
            ret = EnumrateModules(device, media_info_);
            find_sensor = 1;
          }
        }
      }
    }

    /* Try rkisp */
    if (!find_isp && !ret) {
      ret = GetDevName(device, "rkisp-isp-subdev", media_info_->sd_isp_path);
      ret |=
          GetDevName(device, "rkisp-input-params", media_info_->vd_params_path);
      ret |= GetDevName(device, "rkisp-statistics", media_info_->vd_stats_path);
      if (ret == 0) {
        find_isp = 1;
      }
    }

    /* Try rkispp */
    if (!find_ispp && !ret) {
      ret = GetDevName(device, "rkispp-subdev", media_info_->sd_ispp_path);
      if (ret == 0) {
        find_ispp = 1;
        struct media_entity *entity =
            media_get_entity_by_name(device, "rkispp-subdev");
        if (entity) {
          const struct media_pad *pad = media_entity_get_pad(entity, 0);
          if (entity) {
            struct v4l2_mbus_framefmt format;
            ret = v4l2_subdev_get_format(pad->entity, &format, pad->index,
                                         V4L2_SUBDEV_FORMAT_ACTIVE);
            if (ret) {
              LOG_ERROR("Cann't Get camera resolution\n");
            } else {
              width_ = format.width;
              height_ = format.height;
              LOG_ERROR("Get camera resolution %dx%d\n", width_, height_);
            }
          }
        }
      }
    }

    media_device_unref(device);
  }

  LOG_ERROR("find_sensor %d find_isp %d find_ispp %d\n", find_sensor, find_isp,
            find_ispp);

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
  LOG_ERROR("iqfile path %s\n", iqfiles_path_.c_str());
  LOG_ERROR("mode_       %x\n", mode_);
  ctx_ = rk_aiq_uapi_sysctl_init(sensor_entity_name_.c_str(),
                                 iqfiles_path_.c_str(), NULL, NULL);
  if (rk_aiq_uapi_sysctl_prepare(ctx_, width_, height_, mode_)) {
    LOG_DEBUG("rkaiq engine prepare failed !\n");
    return -1;
  }
  return 0;
}

int RKAiqEngine::InitEngine(int mode) {
  if (mode == 0) {
    mode_ = RK_AIQ_WORKING_MODE_NORMAL;
  } else if (mode == 1) {
    mode_ = RK_AIQ_WORKING_MODE_ISP_HDR2;
  } else if (mode == 2) {
    mode_ = RK_AIQ_WORKING_MODE_ISP_HDR3;
  }
  InitEngine();
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

  while (!thread_quit_) {
    isp_fd = open(rkaiq_engine->media_info_.sd_ispp_path, O_RDWR);
    if (isp_fd < 0) {
      LOG_ERROR("open %s failed %s\n", rkaiq_engine->media_info_.sd_ispp_path,
                strerror(errno));
      return;
    }
    LOG_DEBUG("subcrible stream event yes ...\n");
    rkaiq_engine->SubcribleStreamEvent(isp_fd, true);
    LOG_DEBUG("init engine...\n");
    rkaiq_engine->InitEngine();
    LOG_DEBUG("wait stream start event...\n");
    rkaiq_engine->WaitStreamEvent(isp_fd, CIFISP_V4L2_EVENT_STREAM_START, -1);
    LOG_DEBUG("wait stream start event success ...\n");
    rkaiq_engine->StartEngine();
    LOG_DEBUG("wait stream stop event...\n");
    rkaiq_engine->WaitStreamEvent(isp_fd, CIFISP_V4L2_EVENT_STREAM_STOP, -1);
    LOG_DEBUG("wait stream stop event success ...\n");
    LOG_DEBUG("stop engine...\n");
    rkaiq_engine->StopEngine();
    LOG_DEBUG("deinit engine...\n");
    rkaiq_engine->DeInitEngine();
    LOG_DEBUG("subcrible stream event no ...\n");
    rkaiq_engine->SubcribleStreamEvent(isp_fd, false);
    close(isp_fd);
  }

  LOG_DEBUG("RKAiqEngineLoop exit\n\n");
}

RKAiqEngine::RKAiqEngine(std::string iqfiles_path)
    : iqfiles_path_(iqfiles_path), ctx_(nullptr), width_(1920), height_(1080) {
  mode_ = RK_AIQ_WORKING_MODE_NORMAL;
  LinkToIsp();
  if (GetMediaInfo(&media_info_)) {
    LOG_DEBUG("Bad media topology error %d, %s\n", errno, strerror(errno));
    return;
  }
  // rkaiq_engine_thread_ = new std::thread(RKAiqEngineLoop, this);
  // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

RKAiqEngine::~RKAiqEngine() {
  StopEngine();
  DeInitEngine();
  // if (rkaiq_engine_thread_) {
  //   rkaiq_engine_thread_->join();
  //   delete rkaiq_engine_thread_;
  //   rkaiq_engine_thread_ = nullptr;
  // }
}
