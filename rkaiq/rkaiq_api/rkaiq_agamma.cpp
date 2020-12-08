#include "rkaiq_agamma.h"

RKAiqToolAGamma::RKAiqToolAGamma(rk_aiq_sys_ctx_t* ctx) : ctx_(ctx) {}

RKAiqToolAGamma::~RKAiqToolAGamma() {}

int RKAiqToolAGamma::SetAttrib(const rk_aiq_gamma_attrib_t attr) {
    return rk_aiq_user_api_agamma_SetAttrib(ctx_, attr);
}

int RKAiqToolAGamma::GetAttrib(rk_aiq_gamma_attrib_t* attr) {
    return rk_aiq_user_api_agamma_GetAttrib(ctx_, attr);
}