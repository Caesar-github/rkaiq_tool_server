#include "rkaiq_media.h"

#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "aiqtool"

extern int g_width;
extern int g_height;

std::string RKAiqMedia::GetSensorName(struct media_device* device) {
  std::string sensor_name;
  media_entity* entity = NULL;
  const struct media_entity_desc* entity_info;
  uint32_t nents = media_get_entities_count(device);
  for (uint32_t j = 0; j < nents; ++j) {
    entity = media_get_entity(device, j);
    entity_info = media_entity_get_info(entity);
    if ((NULL != entity_info) && (entity_info->type == MEDIA_ENT_T_V4L2_SUBDEV_SENSOR)) {
      sensor_name = entity_info->name;
      break;
    }
  }
  return sensor_name;
}

int RKAiqMedia::IsLinkSensor(struct media_device* device) {
  int link_sensor = 0;
  media_entity* entity = NULL;
  const struct media_entity_desc* entity_info;
  uint32_t nents = media_get_entities_count(device);
  for (uint32_t j = 0; j < nents; ++j) {
    entity = media_get_entity(device, j);
    entity_info = media_entity_get_info(entity);
    if ((NULL != entity_info) && (entity_info->type == MEDIA_ENT_T_V4L2_SUBDEV_SENSOR)) {
      link_sensor = 1;
      break;
    }
  }

  return link_sensor;
}

std::string RKAiqMedia::GetLinkSensorSubDev(struct media_device* device) {
  std::string subdev = "";
  media_entity* entity = NULL;
  const struct media_entity_desc* entity_info;
  uint32_t nents = media_get_entities_count(device);
  for (uint32_t j = 0; j < nents; ++j) {
    entity = media_get_entity(device, j);
    entity_info = media_entity_get_info(entity);
    if ((NULL != entity_info) && (entity_info->type == MEDIA_ENT_T_V4L2_SUBDEV_SENSOR)) {
      subdev = media_entity_get_devname(entity);
      break;
    }
  }
  return subdev;
}

void RKAiqMedia::GetIsppSubDevs(int id, struct media_device* device, const char* devpath) {
  media_entity* entity = NULL;
  const char* entity_name = NULL;
  int index = 0;
  isp_info_t* isp0_info = NULL;
  isp_info_t* isp1_info = NULL;
  ispp_info_t* ispp_info = NULL;

  isp0_info = &media_info[0].isp;
  isp1_info = &media_info[1].isp;

  for (int i = 0; i < MAX_CAM_NUM; i++) {
    if (!isp0_info->media_dev_path.empty() && !isp1_info->media_dev_path.empty() &&
        isp0_info->model_idx > isp1_info->model_idx) {
      if (i == 0) {
        index = 1;
      } else if (i == 1) {
        index = 0;
      } else
        index = i;
    }
    ispp_info = &media_info[index].ispp;

    if (ispp_info->media_dev_path.empty()) {
      break;
    }
    if (ispp_info->media_dev_path.compare(devpath) == 0) {
      LOG_ERROR("ispp info of path %s exists!", devpath);
      return;
    }
  }

  LOG_ERROR("ispp media index %d, media info array id  %d\n", id, index);
  if (index >= MAX_CAM_NUM) {
    return;
  }

  ispp_info->model_idx = id;
  ispp_info->media_dev_path = devpath;

  entity = media_get_entity_by_name(device, "rkispp_input_image");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      ispp_info->pp_input_image_path = entity_name;
    }
  }
  entity = media_get_entity_by_name(device, "rkispp_m_bypass");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      ispp_info->pp_m_bypass_path = entity_name;
    }
  }
  entity = media_get_entity_by_name(device, "rkispp_scale0");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      ispp_info->pp_scale0_path = entity_name;
    }
  }
  entity = media_get_entity_by_name(device, "rkispp_scale1");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      ispp_info->pp_scale1_path = entity_name;
    }
  }
  entity = media_get_entity_by_name(device, "rkispp_scale2");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      ispp_info->pp_scale2_path = entity_name;
    }
  }
  entity = media_get_entity_by_name(device, "rkispp_input_params");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      ispp_info->pp_input_params_path = entity_name;
    }
  }
  entity = media_get_entity_by_name(device, "rkispp-stats");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      ispp_info->pp_stats_path = entity_name;
    }
  }
  entity = media_get_entity_by_name(device, "rkispp-subdev");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      ispp_info->pp_dev_path = entity_name;
    }
  }

  LOG_ERROR("model(%s): ispp_info(%d): ispp-subdev entity name: %s\n", device->info.model, index,
            ispp_info->pp_dev_path.c_str());
}

void RKAiqMedia::GetIspSubDevs(int id, struct media_device* device, const char* devpath) {
  media_entity* entity = NULL;
  const char* entity_name = NULL;
  int index = 0;
  cif_info_t* cif_info = NULL;
  isp_info_t* isp_info = NULL;

  for (index = 0; index < MAX_CAM_NUM; index++) {
    cif_info = &media_info[index].cif;
    isp_info = &media_info[index].isp;
    if (isp_info->media_dev_path.empty()) {
      if (IsLinkSensor(device) && !cif_info->media_dev_path.empty()) {
        continue;
      }
      break;
    } else if (isp_info->media_dev_path.compare(devpath) == 0) {
      LOG_ERROR("isp info of path %s exists!", devpath);
      return;
    }
  }

  LOG_ERROR("isp media index %d, media info array id  %d\n", id, index);
  if (index >= MAX_CAM_NUM) {
    return;
  }

  isp_info->linked_sensor = IsLinkSensor(device);
  if (isp_info->linked_sensor) {
    isp_info->sensor_name = GetSensorName(device);
    isp_info->sensor_subdev_path = GetLinkSensorSubDev(device);
  }
  isp_info->model_idx = id;
  isp_info->media_dev_path = devpath;

  entity = media_get_entity_by_name(device, "rkisp-isp-subdev");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      isp_info->isp_dev_path = entity_name;
    }

    struct v4l2_mbus_framefmt format;
    media_pad* src_pad = (media_pad*)media_entity_get_pad(entity, 2);
    int ret = v4l2_subdev_get_format(entity, &format, src_pad->index, V4L2_SUBDEV_FORMAT_ACTIVE);
    if (ret != 0) {
      LOG_ERROR("v4l2_subdev_get_format failed!\n");
    } else {
      if (g_width > format.width || g_height > format.height) {
        g_width = format.width;
        g_height = format.height;
        LOG_ERROR("fixup width %d height %d\n", g_width, g_height);
      }
    }
  }

  entity = media_get_entity_by_name(device, "rkisp-csi-subdev");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      isp_info->csi_dev_path = entity_name;
    }
  }
  entity = media_get_entity_by_name(device, "rkisp-mpfbc-subdev");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      isp_info->mpfbc_dev_path = entity_name;
    }
  }
  entity = media_get_entity_by_name(device, "rkisp_mainpath");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      isp_info->main_path = entity_name;
    }
  }
  entity = media_get_entity_by_name(device, "rkisp_selfpath");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      isp_info->self_path = entity_name;
    }
  }
  entity = media_get_entity_by_name(device, "rkisp_rawwr0");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      isp_info->rawwr0_path = entity_name;
    }
  }
  entity = media_get_entity_by_name(device, "rkisp_rawwr1");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      isp_info->rawwr1_path = entity_name;
    }
  }
  entity = media_get_entity_by_name(device, "rkisp_rawwr2");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      isp_info->rawwr2_path = entity_name;
    }
  }
  entity = media_get_entity_by_name(device, "rkisp_rawwr3");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      isp_info->rawwr3_path = entity_name;
    }
  }
  entity = media_get_entity_by_name(device, "rkisp_dmapath");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      isp_info->dma_path = entity_name;
    }
  }
  entity = media_get_entity_by_name(device, "rkisp_rawrd0_m");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      isp_info->rawrd0_m_path = entity_name;
    }
  }
  entity = media_get_entity_by_name(device, "rkisp_rawrd1_l");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      isp_info->rawrd1_l_path = entity_name;
    }
  }
  entity = media_get_entity_by_name(device, "rkisp_rawrd2_s");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      isp_info->rawrd2_s_path = entity_name;
    }
  }
  entity = media_get_entity_by_name(device, "rkisp-statistics");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      isp_info->stats_path = entity_name;
    }
  }
  entity = media_get_entity_by_name(device, "rkisp-input-params");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      isp_info->input_params_path = entity_name;
    }
  }
  entity = media_get_entity_by_name(device, "rkisp-mipi-luma");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      isp_info->mipi_luma_path = entity_name;
    }
  }
  entity = media_get_entity_by_name(device, "rockchip-mipi-dphy-rx");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      isp_info->mipi_dphy_rx_path = entity_name;
    }
  }

  LOG_ERROR("model(%s): isp_info(%d): isp-subdev entity name: %s\n", device->info.model, index,
            isp_info->isp_dev_path.c_str());
}

void RKAiqMedia::GetCifSubDevs(int id, struct media_device* device, const char* devpath) {
  media_entity* entity = NULL;
  const struct media_entity_desc* entity_info = NULL;
  const char* entity_name = NULL;
  int index = 0;
  cif_info_t* cif_info = nullptr;

  int link_sensor = IsLinkSensor(device);
  if (!link_sensor) {
    return;
  }

  for (index = 0; index < MAX_CAM_NUM; index++) {
    cif_info = &media_info[index].cif;
    if (cif_info->media_dev_path.empty()) {
      break;
    }
    if (cif_info->media_dev_path.compare(devpath) == 0) {
      LOG_ERROR("isp info of path %s exists!", devpath);
      return;
    }
  }

  LOG_ERROR("cif media index %d, media info array id  %d\n", id, index);
  if (index >= MAX_CAM_NUM) {
    return;
  }

  cif_info->model_idx = id;
  cif_info->linked_sensor = link_sensor;
  if (cif_info->linked_sensor) {
    cif_info->sensor_name = GetSensorName(device);
    cif_info->sensor_subdev_path = GetLinkSensorSubDev(device);
  }
  cif_info->media_dev_path = devpath;

  entity = media_get_entity_by_name(device, "stream_cif_mipi_id0");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      cif_info->mipi_id0 = entity_name;
    }
  }

  entity = media_get_entity_by_name(device, "stream_cif_mipi_id1");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      cif_info->mipi_id1 = entity_name;
    }
  }

  entity = media_get_entity_by_name(device, "stream_cif_mipi_id2");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      cif_info->mipi_id2 = entity_name;
    }
  }

  entity = media_get_entity_by_name(device, "stream_cif_mipi_id3");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      cif_info->mipi_id3 = entity_name;
    }
  }

  entity = media_get_entity_by_name(device, "rkcif-mipi-luma");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      cif_info->mipi_luma_path = entity_name;
    }
  }

  entity = media_get_entity_by_name(device, "rockchip-mipi-csi2");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      cif_info->mipi_csi2_sd_path = entity_name;
    }
  }

  entity = media_get_entity_by_name(device, "rkcif-lvds-subdev");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      cif_info->lvds_sd_path = entity_name;
    }
  }

  entity = media_get_entity_by_name(device, "rkcif-lite-lvds-subdev");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      cif_info->lvds_sd_path = entity_name;
    }
  }

  entity = media_get_entity_by_name(device, "rockchip-mipi-dphy-rx");
  if (entity) {
    entity_name = media_entity_get_devname(entity);
    if (entity_name) {
      cif_info->mipi_dphy_rx_path = entity_name;
    }
  }
}

void RKAiqMedia::GetLensSubDevs(int id, struct media_device* device, const char* devpath, int count) {
  media_entity* entity = NULL;
  const struct media_entity_desc* entity_info = NULL;
  const char* entity_name = NULL;
  uint32_t k;
  cif_info_t* cif_info = NULL;
  isp_info_t* isp_info = NULL;
  lens_info_t* lens_info = nullptr;
  int index = 0;

  for (index = 0; index < MAX_CAM_NUM; index++) {
    cif_info = &media_info[index].cif;
    isp_info = &media_info[index].isp;
    lens_info = &media_info[index].lens;
    if (!IsLinkSensor(device)) {
      continue;
    }
    if (!isp_info->media_dev_path.empty() && isp_info->linked_sensor) {
      lens_info->sensor_name = isp_info->sensor_name;
      lens_info->sensor_subdev_path = isp_info->sensor_subdev_path;
      break;
    } else if (!cif_info->media_dev_path.empty() && cif_info->linked_sensor) {
      lens_info->sensor_name = cif_info->sensor_name;
      lens_info->sensor_subdev_path = cif_info->sensor_subdev_path;
      break;
    } else {
      return;
    }
  }

  LOG_ERROR("isp media index %d, media info array id  %d\n", id, index);
  if (index >= MAX_CAM_NUM) {
    return;
  }

  for (int index = 0; index < MAX_CAM_NUM; index++) {
    if (lens_info->media_dev_path.empty()) {
      break;
    }
    if (lens_info->media_dev_path.compare(devpath) == 0) {
      LOG_ERROR("lens info of path %s exists!", devpath);
      return;
    }
  }
  lens_info->model_idx = id;
  lens_info->media_dev_path = devpath;

  for (k = 0; k < count; ++k) {
    entity = media_get_entity(device, k);
    entity_info = media_entity_get_info(entity);
    if ((NULL != entity_info) && (entity_info->type == MEDIA_ENT_T_V4L2_SUBDEV_LENS)) {
      if ((entity_info->name[0] == 'm') && (strncmp(entity_info->name, lens_info->sensor_name.c_str(), 3) == 0)) {
        if (entity_info->flags == 1)
          lens_info->module_ircut_dev_name = std::string(media_entity_get_devname(entity));
        else
          lens_info->module_lens_dev_name = std::string(media_entity_get_devname(entity));
      }
    } else if ((NULL != entity_info) && (entity_info->type == MEDIA_ENT_T_V4L2_SUBDEV_FLASH)) {
      if ((entity_info->name[0] == 'm') && (strncmp(entity_info->name, lens_info->sensor_name.c_str(), 3) == 0)) {
        if (strstr(entity_info->name, "-ir") != NULL) {
          lens_info->module_flash_ir_dev_name[lens_info->flash_ir_num++] =
              std::string(media_entity_get_devname(entity));
        } else
          lens_info->module_flash_dev_name[lens_info->flash_num++] = std::string(media_entity_get_devname(entity));
      }
    }
  }
}

int RKAiqMedia::LinkToIsp(bool enable) {
  int ret;
  int index = 0;
  char sys_path[64];
  media_device* device = NULL;
  media_entity* entity = NULL;
  media_pad *src_pad = NULL, *src_raw2_s = NULL, *src_raw1_l = NULL, *src_raw0_m = NULL;
  media_pad *sink_pad = NULL, *sink_pad_bridge = NULL, *sink_pad_mp = NULL;

  LOG_ERROR("############## LinkToIsp\n");
  system(VICAP_COMPACT_TEST_ON);
  system(VICAP2_COMPACT_TEST_ON);

  while (index < 10) {
    snprintf(sys_path, 64, "/dev/media%d", index++);
    if (access(sys_path, F_OK)) {
      continue;
    }
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
    const struct media_device_info* info = media_get_info(device);
    LOG_INFO("%s: model %s\n", sys_path, info->model);
    if (strcmp(info->model, "rkisp0") != 0 && strcmp(info->model, "rkisp1") != 0 && strcmp(info->model, "rkisp") != 0) {
      media_device_unref(device);
      continue;
    }
    LOG_INFO("%s: setup link to isp enable %d\n", sys_path, enable);
    entity = media_get_entity_by_name(device, "rkisp-isp-subdev");
    if (entity) {
      const struct media_entity_desc* info = media_entity_get_info(entity);
      src_pad = (media_pad*)media_entity_get_pad(entity, 2);
      if (!src_pad) {
        LOG_ERROR("get rkisp-isp-subdev source pad failed!\n");
      }
      if (enable) {
        struct v4l2_mbus_framefmt format;
        ret = v4l2_subdev_get_format(src_pad->entity, &format, src_pad->index, V4L2_SUBDEV_FORMAT_ACTIVE);
        if (ret != 0) {
          LOG_ERROR("v4l2_subdev_get_format failed!\n");
        }
        char set_fmt[128];
        sprintf(set_fmt, "\"%s\":%d [fmt:%s/%dx%d field:none]", info->name, src_pad->index, "YUYV8_2X8", format.width,
                format.height);
        ret = v4l2_subdev_parse_setup_formats(device, set_fmt);
        if (ret) {
          LOG_ERROR("Unable to setup formats: %s (%d)\n", strerror(-ret), -ret);
        }
      }
    }

    entity = media_get_entity_by_name(device, "rkisp-bridge-ispp");
    if (entity) {
      sink_pad_bridge = (media_pad*)media_entity_get_pad(entity, 0);
      if (!sink_pad_bridge) {
        LOG_ERROR("get rkisp-bridge-ispp sink pad failed!\n");
      }
    }

    entity = media_get_entity_by_name(device, "rkisp_mainpath");
    if (entity) {
      sink_pad_mp = (media_pad*)media_entity_get_pad(entity, 0);
      if (!sink_pad_mp) {
        LOG_ERROR("get rkisp_mainpath sink pad failed!\n");
      }
    }

    entity = media_get_entity_by_name(device, "rkisp-isp-subdev");
    if (entity) {
      sink_pad = (media_pad*)media_entity_get_pad(entity, 0);
      if (!sink_pad) {
        LOG_DEBUG("get rkisp-isp-subdev source pad failed!\n");
      }
    }

    entity = media_get_entity_by_name(device, "rkisp_rawrd0_m");
    if (entity) {
      src_raw0_m = (media_pad*)media_entity_get_pad(entity, 0);
      if (!src_raw0_m) {
        LOG_DEBUG("get rkisp_rawrd0_m sink pad failed!\n");
      }
    }

    entity = media_get_entity_by_name(device, "rkisp_rawrd1_l");
    if (entity) {
      src_raw1_l = (media_pad*)media_entity_get_pad(entity, 0);
      if (!src_raw1_l) {
        LOG_DEBUG("get rkisp_rawrd1_l sink pad failed!\n");
      }
    }

    entity = media_get_entity_by_name(device, "rkisp_rawrd2_s");
    if (entity) {
      src_raw2_s = (media_pad*)media_entity_get_pad(entity, 0);
      if (!src_raw2_s) {
        LOG_DEBUG("get rkisp_rawrd2_s sink pad failed!\n");
      }
    }

    if (enable) {
      if (src_pad && sink_pad_bridge && sink_pad_mp) {
        ret = media_setup_link(device, src_pad, sink_pad_mp, 0);
        if (ret) {
          LOG_ERROR("media_setup_link sink_pad_bridge FAILED: %d\n", ret);
        }
        ret = media_setup_link(device, src_pad, sink_pad_bridge, MEDIA_LNK_FL_ENABLED);
        if (ret) {
          LOG_ERROR("media_setup_link sink_pad_bridge FAILED: %d\n", ret);
        }
        LOG_ERROR("media_setup_link isp SUCCESS\n");
      } else {
        LOG_ERROR("media_setup_link isp FAILED\n");
      }
    } else {
      if (src_pad && sink_pad_bridge && sink_pad_mp && src_raw2_s && src_raw1_l && src_raw0_m) {
        ret = media_setup_link(device, src_raw0_m, sink_pad, 0);
        if (ret) {
          LOG_ERROR("media_setup_link src_raw0_m sink_pad FAILED: %d\n", ret);
        }
        ret = media_setup_link(device, src_raw1_l, sink_pad, 0);
        if (ret) {
          LOG_ERROR("media_setup_link src_raw1_l sink_pad FAILED: %d\n", ret);
        }
        ret = media_setup_link(device, src_raw2_s, sink_pad, 0);
        if (ret) {
          LOG_ERROR("media_setup_link src_raw2_s sink_pad FAILED: %d\n", ret);
        }
        ret = media_setup_link(device, src_pad, sink_pad_bridge, 0);
        if (ret) LOG_ERROR("media_setup_link src_pad sink_pad_bridge FAILED: %d\n", ret);
        ret = media_setup_link(device, src_pad, sink_pad_mp, MEDIA_LNK_FL_ENABLED);
        if (ret) {
          LOG_ERROR("media_setup_link src_pad src_pad FAILED: %d\n", ret);
        }
        LOG_ERROR("media_setup_link unlink isp SUCCESS\n");
      } else {
        LOG_ERROR("media_setup_link unlink isp FAILED\n");
      }
    }

    // ret = v4l2_subdev_parse_setup_formats(device, "crop:(0,0)/2688x1520");
    media_device_unref(device);
  }
  return ret;
}

int RKAiqMedia::GetIspVer() {
  struct v4l2_capability cap;
  int ret = -1;

  int fd = open(media_info[0].isp.stats_path.c_str(), O_RDWR);
  if (fd < 0) {
    LOG_ERROR("Failed to open dev %s", media_info[0].isp.stats_path.c_str());
    return ret;
  }

  ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
  if (ret < 0) {
    LOG_ERROR("Failed to query cap from %s", media_info[0].isp.stats_path.c_str());
    goto out;
  }

  char* p;
  p = strrchr((char*)cap.driver, '_');
  if (p == NULL) {
    goto out;
  }

  if (*(p + 1) != 'v') {
    goto out;
  }

  ret = atoi(p + 2);

out:
  if (fd >= 0) close(fd);

  return ret;
}

int RKAiqMedia::GetMediaInfo() {
  struct media_device* device = NULL;
  const struct media_entity_desc* entity_info = NULL;
  struct media_entity* entity = NULL;
  int32_t nents = 0;
  int ret;
  char sys_path[64];
  unsigned int index = 0, id, i;
  bool link_cif = false;

  while (index < MAX_MEDIA_NUM) {
    id = index;
    snprintf(sys_path, 64, "/dev/media%d", index++);
    if (access(sys_path, F_OK)) {
      continue;
    }

    LOG_ERROR("access %s\n", sys_path);

    device = media_device_new(sys_path);
    if (!device) {
      return -ENOMEM;
    }

    ret = media_device_enumerate(device);
    if (ret) {
      media_device_unref(device);
      return ret;
    }

    if (strcmp(device->info.model, "rkispp0") == 0 || strcmp(device->info.model, "rkispp1") == 0 ||
        strcmp(device->info.model, "rkispp") == 0) {
      GetIsppSubDevs(id, device, sys_path);
      goto media_unref;
    } else if (strcmp(device->info.model, "rkisp0") == 0 || strcmp(device->info.model, "rkisp1") == 0 ||
               strcmp(device->info.model, "rkisp") == 0) {
      GetIspSubDevs(id, device, sys_path);
    } else if (strcmp(device->info.model, "rkcif") == 0 || strcmp(device->info.model, "rkcif_mipi_lvds") == 0 ||
               strcmp(device->info.model, "rkcif_lite_mipi_lvds") == 0) {
      GetCifSubDevs(id, device, sys_path);
    } else {
      goto media_unref;
    }

    nents = media_get_entities_count(device);
    for (int j = 0; j < nents; ++j) {
      entity = media_get_entity(device, j);
      entity_info = media_entity_get_info(entity);
      if ((NULL != entity_info) && (entity_info->type == MEDIA_ENT_T_V4L2_SUBDEV_SENSOR)) {
        GetLensSubDevs(id, device, sys_path, nents);
      }
    }

  media_unref:
    media_device_unref(device);
  }

  return ret;
}

int RKAiqMedia::DumpMediaInfo() {
  LOG_DEBUG("DumpMediaInfo:\n");

  for (int i = 0; i < MAX_CAM_NUM; i++) {
    cif_info_t* cif = &media_info[i].cif;
    isp_info_t* isp = &media_info[i].isp;
    ispp_info_t* ispp = &media_info[i].ispp;
    LOG_DEBUG("##### Camera index: %d\n", i);
    if (isp->linked_sensor) {
      LOG_DEBUG("\t sensor_name :    %s\n", isp->sensor_name.c_str());
    } else if (cif->linked_sensor) {
      LOG_DEBUG("\t sensor_name :    %s\n", cif->sensor_name.c_str());
    }
    if (cif->model_idx >= 0) {
      LOG_DEBUG("#### cif:\n");
      LOG_DEBUG("\t model_idx :         %d\n", cif->model_idx);
      LOG_DEBUG("\t linked_sensor :     %d\n", cif->linked_sensor);
      LOG_DEBUG("\t media_dev_path :    %s\n", cif->media_dev_path.c_str());
      LOG_DEBUG("\t mipi_id0 : 		  %s\n", cif->mipi_id0.c_str());
      LOG_DEBUG("\t mipi_id1 : 		  %s\n", cif->mipi_id1.c_str());
      LOG_DEBUG("\t mipi_id2 : 		  %s\n", cif->mipi_id2.c_str());
      LOG_DEBUG("\t mipi_id3 : 		  %s\n", cif->mipi_id3.c_str());
      LOG_DEBUG("\t mipi_dphy_rx_path : %s\n", cif->mipi_dphy_rx_path.c_str());
      LOG_DEBUG("\t mipi_csi2_sd_path : %s\n", cif->mipi_csi2_sd_path.c_str());
      LOG_DEBUG("\t lvds_sd_path :      %s\n", cif->lvds_sd_path.c_str());
      LOG_DEBUG("\t mipi_luma_path :    %s\n", cif->mipi_luma_path.c_str());
    }
    if (isp->model_idx >= 0) {
      LOG_DEBUG("#### isp:\n");
      LOG_DEBUG("\t model_idx :         %d\n", isp->model_idx);
      LOG_DEBUG("\t linked_sensor :     %d\n", isp->linked_sensor);
      LOG_DEBUG("\t media_dev_path :    %s\n", isp->media_dev_path.c_str());
      LOG_DEBUG("\t isp_dev_path :      %s\n", isp->isp_dev_path.c_str());
      LOG_DEBUG("\t csi_dev_path :      %s\n", isp->csi_dev_path.c_str());
      LOG_DEBUG("\t mpfbc_dev_path :    %s\n", isp->mpfbc_dev_path.c_str());
      LOG_DEBUG("\t main_path :         %s\n", isp->main_path.c_str());
      LOG_DEBUG("\t self_path :         %s\n", isp->self_path.c_str());
      LOG_DEBUG("\t rawwr0_path :       %s\n", isp->rawwr0_path.c_str());
      LOG_DEBUG("\t rawwr1_path :       %s\n", isp->rawwr1_path.c_str());
      LOG_DEBUG("\t rawwr2_path :       %s\n", isp->rawwr2_path.c_str());
      LOG_DEBUG("\t rawwr3_path :       %s\n", isp->rawwr3_path.c_str());
      LOG_DEBUG("\t dma_path :          %s\n", isp->dma_path.c_str());
      LOG_DEBUG("\t rawrd0_m_path :     %s\n", isp->rawrd0_m_path.c_str());
      LOG_DEBUG("\t rawrd1_l_path :     %s\n", isp->rawrd1_l_path.c_str());
      LOG_DEBUG("\t rawrd2_s_path :     %s\n", isp->rawrd2_s_path.c_str());
      LOG_DEBUG("\t stats_path :        %s\n", isp->stats_path.c_str());
      LOG_DEBUG("\t input_params_path : %s\n", isp->input_params_path.c_str());
      LOG_DEBUG("\t mipi_luma_path :    %s\n", isp->mipi_luma_path.c_str());
      LOG_DEBUG("\t mipi_dphy_rx_path : %s\n", isp->mipi_dphy_rx_path.c_str());
    }
    if (ispp->model_idx >= 0) {
      LOG_DEBUG("#### ispp:\n");
      LOG_DEBUG("\t model_idx :            %d\n", ispp->model_idx);
      LOG_DEBUG("\t media_dev_path :       %s\n", ispp->media_dev_path.c_str());
      LOG_DEBUG("\t pp_input_image_path :  %s\n", ispp->pp_input_image_path.c_str());
      LOG_DEBUG("\t pp_m_bypass_path :     %s\n", ispp->pp_m_bypass_path.c_str());
      LOG_DEBUG("\t pp_scale0_path :       %s\n", ispp->pp_scale0_path.c_str());
      LOG_DEBUG("\t pp_scale1_path :       %s\n", ispp->pp_scale1_path.c_str());
      LOG_DEBUG("\t pp_scale2_path :       %s\n", ispp->pp_scale2_path.c_str());
      LOG_DEBUG("\t pp_input_params_path : %s\n", ispp->pp_input_params_path.c_str());
      LOG_DEBUG("\t pp_stats_path :        %s\n", ispp->pp_stats_path.c_str());
      LOG_DEBUG("\t pp_dev_path :          %s\n", ispp->pp_dev_path.c_str());
    }
  }
  return 0;
}

RKAiqMedia::RKAiqMedia() {
  for (int i = 0; i < MAX_CAM_NUM; i++) {
    media_info[i].cif.model_idx = -1;
    media_info[i].isp.model_idx = -1;
    media_info[i].ispp.model_idx = -1;
    media_info[i].cif.linked_sensor = 0;
    media_info[i].isp.linked_sensor = 0;
  }
}
