#include "rkaiq_ahdr.h"

RKAiqToolAHDR::RKAiqToolAHDR(rk_aiq_sys_ctx_t *ctx) : ctx_(ctx) {}

RKAiqToolAHDR::~RKAiqToolAHDR() {}

int RKAiqToolAHDR::SetAttrib(const ahdr_attrib_t attr) {
  return rk_aiq_user_api_ahdr_SetAttrib(ctx_, attr);
}

int RKAiqToolAHDR::GetAttrib(ahdr_attrib_t *attr) {
  return rk_aiq_user_api_ahdr_GetAttrib(ctx_, attr);
}