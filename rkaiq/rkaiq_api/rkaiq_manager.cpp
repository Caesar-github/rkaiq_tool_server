#include "rkaiq_manager.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "rkaiq_manager.cpp"

RKAiqToolManager::RKAiqToolManager() : ctx_(nullptr) {
  engine_.reset(new RKAiqEngine());
  ctx_ = engine_->GetContext();
  imgproc_.reset(new RKAiqToolImgProc(ctx_));
}

RKAiqToolManager::~RKAiqToolManager() {
  imgproc_.reset(nullptr);
  engine_.reset(nullptr);
}
