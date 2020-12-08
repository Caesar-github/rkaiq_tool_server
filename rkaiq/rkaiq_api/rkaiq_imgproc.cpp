#include "rkaiq_imgproc.h"

RKAiqToolImgProc::RKAiqToolImgProc(rk_aiq_sys_ctx_t* ctx) : ctx_(ctx) {}

RKAiqToolImgProc::~RKAiqToolImgProc() {}

int RKAiqToolImgProc::SetGrayMode(rk_aiq_gray_mode_t mode) {
    return rk_aiq_uapi_setGrayMode(ctx_, mode);
}

int RKAiqToolImgProc::GetGrayMode() {
    return rk_aiq_uapi_getGrayMode(ctx_);
}
