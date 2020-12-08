#include "rkaiq_adpcc.h"

RKAiqToolADPCC::RKAiqToolADPCC(rk_aiq_sys_ctx_t* ctx) : ctx_(ctx) {}

RKAiqToolADPCC::~RKAiqToolADPCC() {}

int RKAiqToolADPCC::SetAttrib(rk_aiq_dpcc_attrib_t* attr) {
    return rk_aiq_user_api_adpcc_SetAttrib(ctx_, attr);
}

int RKAiqToolADPCC::GetAttrib(rk_aiq_dpcc_attrib_t* attr) {
    return rk_aiq_user_api_adpcc_GetAttrib(ctx_, attr);
}