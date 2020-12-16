#include "rkaiq_ccm.h"

RKAiqToolCCM::RKAiqToolCCM(rk_aiq_sys_ctx_t* ctx) : ctx_(ctx) {}

RKAiqToolCCM::~RKAiqToolCCM() {}

int RKAiqToolCCM::SetAttrib(rk_aiq_ccm_attrib_t attr) {
    return rk_aiq_user_api_accm_SetAttrib(ctx_, attr);
}

int RKAiqToolCCM::GetAttrib(rk_aiq_ccm_attrib_t* attr) {
    return rk_aiq_user_api_accm_GetAttrib(ctx_, attr);
}

int RKAiqToolCCM::QueryCCMInfo(rk_aiq_ccm_querry_info_t* attr) {
    return rk_aiq_user_api_accm_QueryCcmInfo(ctx_, attr);
}