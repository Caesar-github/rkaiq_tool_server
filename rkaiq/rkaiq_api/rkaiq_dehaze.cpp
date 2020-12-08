#include "rkaiq_dehaze.h"

RKAiqToolDehaze::RKAiqToolDehaze(rk_aiq_sys_ctx_t* ctx) : ctx_(ctx) {}

RKAiqToolDehaze::~RKAiqToolDehaze() {}

int RKAiqToolDehaze::SetAttrib(adehaze_sw_t attr) {
    return rk_aiq_user_api_adehaze_setSwAttrib(ctx_, attr);
}

int RKAiqToolDehaze::GetAttrib(adehaze_sw_t* attr) {
    return rk_aiq_user_api_adehaze_getSwAttrib(ctx_, attr);
}