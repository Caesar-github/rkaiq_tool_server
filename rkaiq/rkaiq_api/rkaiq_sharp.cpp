#include "rkaiq_sharp.h"

RKAiqToolSharp::RKAiqToolSharp(rk_aiq_sys_ctx_t *ctx) : ctx_(ctx) {}

RKAiqToolSharp::~RKAiqToolSharp() {}

int RKAiqToolSharp::SetAttrib(rk_aiq_sharp_attrib_t *attr) {
  return rk_aiq_user_api_asharp_SetAttrib(ctx_, attr);
}

int RKAiqToolSharp::GetAttrib(rk_aiq_sharp_attrib_t *attr) {
  return rk_aiq_user_api_asharp_GetAttrib(ctx_, attr);
}

int RKAiqToolSharp::SetIQPara(rk_aiq_sharp_IQpara_t *para) {
  return rk_aiq_user_api_asharp_SetIQPara(ctx_, para);
}

int RKAiqToolSharp::GetIQPara(rk_aiq_sharp_IQpara_t *para) {
  return rk_aiq_user_api_asharp_GetIQPara(ctx_, para);
}

int RKAiqToolSharp::SetStrength(float fPercent) {
  return rk_aiq_user_api_asharp_SetStrength(ctx_, fPercent);
}

int RKAiqToolSharp::GetStrength(float *pPercent) {
  return rk_aiq_user_api_asharp_GetStrength(ctx_, pPercent);
}