#include "rkaiq_awb.h"

RKAiqToolAWB::RKAiqToolAWB(rk_aiq_sys_ctx_t* ctx) : ctx_(ctx) {}

RKAiqToolAWB::~RKAiqToolAWB() {}

int RKAiqToolAWB::SetAttrib(rk_aiq_wb_attrib_t attr) {
    return rk_aiq_user_api_awb_SetAttrib(ctx_, attr);
}

int RKAiqToolAWB::GetAttrib(rk_aiq_wb_attrib_t* attr) {
    return rk_aiq_user_api_awb_GetAttrib(ctx_, attr);
}

int RKAiqToolAWB::GetCCT(rk_aiq_wb_cct_t* attr) {
    return rk_aiq_user_api_awb_GetCCT(ctx_, attr);
}

int RKAiqToolAWB::QueryWBInfo(rk_aiq_wb_querry_info_t* attr) {
    return rk_aiq_user_api_awb_QueryWBInfo(ctx_, attr);
}