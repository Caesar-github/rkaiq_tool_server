#include "rkaiq_cproc.h"

RKAiqToolCPROC::RKAiqToolCPROC(rk_aiq_sys_ctx_t* ctx) : ctx_(ctx) {}

RKAiqToolCPROC::~RKAiqToolCPROC() {}

int RKAiqToolCPROC::SetAttrib(acp_attrib_t attr) {
    return rk_aiq_user_api_acp_SetAttrib(ctx_, attr);
}

int RKAiqToolCPROC::GetAttrib(acp_attrib_t* attr) {
    return rk_aiq_user_api_acp_GetAttrib(ctx_, attr);
}