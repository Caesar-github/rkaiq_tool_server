#include "camera_infohw.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "camera_infohw.cpp"

__u32 convert_to_v4l2fmt(int code) {
  __u32 v4l2_fmt = 0;
  switch (code) {
  /* 8bit raw bayer */
  case MEDIA_BUS_FMT_SBGGR8_1X8:
    v4l2_fmt = V4L2_PIX_FMT_SRGGB8;
    break;
  case MEDIA_BUS_FMT_SGBRG8_1X8:
    v4l2_fmt = V4L2_PIX_FMT_SGBRG8;
    break;
  case MEDIA_BUS_FMT_SGRBG8_1X8:
    v4l2_fmt = V4L2_PIX_FMT_SGRBG8;
    break;
  case MEDIA_BUS_FMT_SRGGB8_1X8:
    v4l2_fmt = V4L2_PIX_FMT_SRGGB8;
    break;
  case MEDIA_BUS_FMT_Y8_1X8:
    v4l2_fmt = V4L2_PIX_FMT_GREY;
    break;
  /* 10bit raw bayer */
  case MEDIA_BUS_FMT_SBGGR10_1X10:
    v4l2_fmt = V4L2_PIX_FMT_SRGGB10;
    break;
  case MEDIA_BUS_FMT_SGBRG10_1X10:
    v4l2_fmt = V4L2_PIX_FMT_SGBRG10;
    break;
  case MEDIA_BUS_FMT_SGRBG10_1X10:
    v4l2_fmt = V4L2_PIX_FMT_SGRBG10;
    break;
  case MEDIA_BUS_FMT_SRGGB10_1X10:
    v4l2_fmt = V4L2_PIX_FMT_SRGGB10;
    break;
  case MEDIA_BUS_FMT_Y10_1X10:
    v4l2_fmt = V4L2_PIX_FMT_Y10;
    break;
  /* 12bit raw bayer */
  case MEDIA_BUS_FMT_SBGGR12_1X12:
    v4l2_fmt = V4L2_PIX_FMT_SRGGB12;
    break;
  case MEDIA_BUS_FMT_SGBRG12_1X12:
    v4l2_fmt = V4L2_PIX_FMT_SGBRG12;
    break;
  case MEDIA_BUS_FMT_SGRBG12_1X12:
    v4l2_fmt = V4L2_PIX_FMT_SGRBG12;
    break;
  case MEDIA_BUS_FMT_SRGGB12_1X12:
    v4l2_fmt = V4L2_PIX_FMT_SRGGB12;
    break;
  case MEDIA_BUS_FMT_Y12_1X12:
    v4l2_fmt = V4L2_PIX_FMT_Y12;
    break;
  default:
    LOG_ERROR("nonsupport raw bayer formats, please check sensor output fmt\n");
    break;
  }
  return v4l2_fmt;
}

int get_isp_subdevs(struct media_device *device, const char *devpath,
                    isp_t *isp_info) {
  media_entity *entity = NULL;
  const char *entity_name = NULL;

  if (!device || !isp_info || !devpath)
    return -1;

  strncpy(isp_info->media_dev_path, (char *)devpath,
          sizeof(isp_info->media_dev_path));
  LOG_INFO("get isp subdev: %s \n", isp_info->media_dev_path);
  entity = media_get_entity_by_name(device, "rkisp_mainpath",
                                    strlen("rkisp_mainpath"));
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      strncpy(isp_info->isp_main_path, (char *)entity_name,
              sizeof(isp_info->isp_main_path));
    }
  }
  entity = media_get_entity_by_name(device, "rkisp-isp-subdev",
                                    strlen("rkisp-isp-subdev"));
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      strncpy(isp_info->isp_sd_path, (char *)entity_name,
              sizeof(isp_info->isp_sd_path));
      LOG_INFO("isp subdev path: %s\n", isp_info->isp_sd_path);
    }
  }
  return 0;
}

int get_sensor_info(struct media_device *device, const char *devpath,
                    sensor_t *cam_info) {
  uint32_t nents, j = 0;
  const struct media_entity_desc *entity_info = NULL;
  struct media_entity *entity = NULL;

  device = media_device_new(devpath);

  /* Enumerate entities, pads and links. */
  media_device_enumerate(device);
  nents = media_get_entities_count(device);
  for (j = 0; j < nents; ++j) {
    entity = media_get_entity(device, j);
    entity_info = media_entity_get_info(entity);
    if ((NULL != entity_info) &&
        (entity_info->type == MEDIA_ENT_T_V4L2_SUBDEV_SENSOR))
      strncpy(cam_info->device_name, (char *)media_entity_get_devname(entity),
              sizeof(cam_info->device_name));
    strncpy(cam_info->sensor_name, entity_info->name,
            sizeof(cam_info->sensor_name));
    LOG_INFO("sensor subdev path: %s\n", cam_info->device_name);
  }
  media_device_unref(device);
  return 0;
}

static int rkisp_sd_set_crop(const char *ispsd, int fd, int pad, int *w,
                             int *h) {
  struct v4l2_subdev_selection sel;
  int ret;

  memset(&sel, 0, sizeof(sel));
  sel.which = V4L2_SUBDEV_FORMAT_ACTIVE;
  sel.pad = pad;
  sel.r.width = *w;
  sel.r.height = *h;
  sel.r.left = 0;
  sel.r.top = 0;
  sel.target = V4L2_SEL_TGT_CROP;
  sel.flags = V4L2_SEL_FLAG_LE;
  ret = device_setsubdevcrop(fd, &sel);
  if (ret) {
    LOG_ERROR("subdev %s pad %d crop failed, ret = %d\n", ispsd, sel.pad, ret);
    return ret;
  }

  *w = sel.r.width;
  *h = sel.r.height;

  return 0;
}

int rkisp_sd_set_fmt(const char *ispsd, int pad, int *w, int *h, int code) {
  struct v4l2_subdev_format fmt;
  int ret, fd;

  fd = device_open(ispsd);
  if (fd < 0) {
    LOG_ERROR("Open isp subdev %s failed, %s\n", ispsd, strerror(errno));
    return fd;
  }

  if (pad == 2) { /* Source pad */
    ret = rkisp_sd_set_crop(ispsd, fd, pad, w, h);
    if (ret)
      goto close;
  }

  /* Get fmt and only update what we want */
  memset(&fmt, 0, sizeof(fmt));
  fmt.pad = pad;
  fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
  ret = device_getsubdevformat(fd, &fmt);
  if (ret < 0) {
    LOG_ERROR("subdev %s get pad: %d fmt failed %s.\n", ispsd, fmt.pad,
              strerror(errno));
    goto close;
  }

  fmt.format.height = *h;
  fmt.format.width = *w;
  fmt.format.code = code;

  ret = device_setsubdevformat(fd, &fmt);
  if (ret < 0) {
    LOG_ERROR("subdev %s set pad: %d fmt failed %s.\n", ispsd, fmt.pad,
              strerror(errno));
    goto close;
  }

  if (fmt.format.width != *w || fmt.format.height != *h ||
      fmt.format.code != code) {
    LOG_INFO("subdev %s pad %d choose the best fit fmt: %dx%d, 0x%08x\n", ispsd,
             pad, fmt.format.width, fmt.format.height, fmt.format.code);
  }

  *w = fmt.format.width;
  *h = fmt.format.height;
  if (pad == 0) {
    ret = rkisp_sd_set_crop(ispsd, fd, pad, w, h);
    if (ret)
      goto close;
  }

close:
  close(fd);

  return ret;
}

int rkisp_set_ispsd_fmt(struct capture_info *media_info, int in_w, int in_h,
                        int in_code, int out_w, int out_h, int out_code) {
  const char *ispsd;
  int ret;

  ispsd = media_info->vd_path.isp_sd_path;
  // TODO: check source and sink pad
  ret = rkisp_sd_set_fmt(ispsd, 0, &in_w, &in_h, in_code);
  ret |= rkisp_sd_set_fmt(ispsd, 2, &out_w, &out_h, out_code);

  return ret;
}

int setupLink(struct capture_info *media_info, bool raw_mode) {
  media_device *device = NULL;
  media_entity *entity = NULL;
  media_pad *src_pad = NULL, *sink_pad_bridge = NULL, *sink_pad_mp = NULL;
  int ret;

  device = media_device_new(media_info->vd_path.media_dev_path);

  LOG_INFO("%s: setup link for raw or yuv: %d\n",
           media_info->vd_path.media_dev_path, raw_mode);

  /* Enumerate entities, pads and links. */
  media_device_enumerate(device);
  entity = media_get_entity_by_name(device, "rkisp-isp-subdev",
                                    strlen("rkisp-isp-subdev"));
  if (entity) {
    src_pad = (media_pad *)media_entity_get_pad(entity, 2);
    if (!src_pad) {
      LOG_DEBUG("get rkisp-isp-subdev source pad failed!\n");
      goto FAIL;
    }
  }

  entity = media_get_entity_by_name(device, "rkisp-bridge-ispp",
                                    strlen("rkisp-bridge-ispp"));
  if (entity) {
    sink_pad_bridge = (media_pad *)media_entity_get_pad(entity, 0);
    if (!sink_pad_bridge) {
      LOG_DEBUG("get rkisp-bridge-ispp sink pad failed!\n");
      goto FAIL;
    }
  }

  entity = media_get_entity_by_name(device, "rkisp_mainpath",
                                    strlen("rkisp_mainpath"));
  if (entity) {
    sink_pad_mp = (media_pad *)media_entity_get_pad(entity, 0);
    if (!sink_pad_mp) {
      LOG_DEBUG("get rkisp_mainpath sink pad failed!\n");
      goto FAIL;
    }
  }

  if (raw_mode) {
    ret = media_setup_link(device, src_pad, sink_pad_bridge, 0);
    if (ret)
      LOG_ERROR("media_setup_link sink_pad_bridge FAILED: %d\n", ret);
    ret = media_setup_link(device, src_pad, sink_pad_mp, MEDIA_LNK_FL_ENABLED);
    if (ret)
      LOG_ERROR("media_setup_link sink_pad_bridge FAILED: %d\n", ret);
  } else {
    ret = media_setup_link(device, src_pad, sink_pad_mp, 0);
    if (ret)
      LOG_ERROR("media_setup_link sink_pad_bridge FAILED: %d\n", ret);
    ret = media_setup_link(device, src_pad, sink_pad_bridge,
                           MEDIA_LNK_FL_ENABLED);
    if (ret)
      LOG_ERROR("media_setup_link sink_pad_bridge FAILED: %d\n", ret);
  }
  media_device_unref(device);
  return 0;
FAIL:
  media_device_unref(device);
  return -1;
}

int initCamHwInfos(struct capture_info *media_info) {
  // TODO
  // (1) all sensor info
  // (2) all pipeline entity infos belonged to
  //     the sensor

  char sys_path[64], devpath[32];
  FILE *fp = NULL;
  struct media_device *device = NULL;
  uint32_t nents, j = 0, i = 0;
  const struct media_entity_desc *entity_info = NULL;
  struct media_entity *entity = NULL;

  LOG_INFO("init start!!!!!!\n");
  while (i < MAX_MEDIA_INDEX) {
    snprintf(sys_path, 64, "/dev/media%d", i++);
    fp = fopen(sys_path, "r");
    if (!fp)
      continue;
    fclose(fp);
    device = media_device_new(sys_path);

    /* Enumerate entities, pads and links. */
    media_device_enumerate(device);

    if (strcmp(device->info.model, "rkisp") == 0) {
      int ret = get_isp_subdevs(device, sys_path, &media_info->vd_path);
      if (ret)
        return ret;
      ret = get_sensor_info(device, sys_path, &media_info->sd_path);
      if (ret)
        return ret;
    } else {
      goto media_unref;
    }
  media_unref:
    media_device_unref(device);
  }
  return 0;
}
