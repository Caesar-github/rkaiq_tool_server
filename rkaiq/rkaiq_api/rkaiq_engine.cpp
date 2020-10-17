#include "rkaiq_engine.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "rkaiq_engine.cpp"

int RKAiqEngine::InitEngine() {
  LOG_ERROR("sensor_name %s\n", sensor_entity_name_.c_str());
  LOG_ERROR("iqfile path %s\n", iqfiles_path_.c_str());
  LOG_ERROR("mode        %x\n", mode_);
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
  rk_aiq_uapi_sysctl_stop(ctx_, false);
  return 0;
}

int RKAiqEngine::DeInitEngine() {
  rk_aiq_uapi_sysctl_deinit(ctx_);
  return 0;
}

RKAiqEngine::RKAiqEngine(std::string iqfiles_path, std::string sensor_name)
    : iqfiles_path_(iqfiles_path), ctx_(nullptr), width_(1920), height_(1080),
      mode_(RK_AIQ_WORKING_MODE_NORMAL), sensor_entity_name_(sensor_name) {}

RKAiqEngine::~RKAiqEngine() {
  StopEngine();
  DeInitEngine();
}
