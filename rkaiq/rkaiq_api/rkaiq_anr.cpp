#include "rkaiq_anr.h"

RKAiqToolANR::RKAiqToolANR(rk_aiq_sys_ctx_t *ctx) : ctx_(ctx) {}

RKAiqToolANR::~RKAiqToolANR() {}

int RKAiqToolANR::SetAttrib(rk_aiq_nr_attrib_t *attr) {
  return rk_aiq_user_api_anr_SetAttrib(ctx_, attr);
}

int RKAiqToolANR::GetAttrib(rk_aiq_nr_attrib_t *attr) {
  return rk_aiq_user_api_anr_GetAttrib(ctx_, attr);
}

int RKAiqToolANR::SetIQPara(rk_aiq_nr_IQPara_t *attr) {
  return rk_aiq_user_api_anr_SetIQPara(ctx_, attr);
}

int RKAiqToolANR::GetIQPara(rk_aiq_nr_IQPara_t *attr) {
  return rk_aiq_user_api_anr_GetIQPara(ctx_, attr);
}

int RKAiqToolANR::SetLumaSFStrength(float fPercnt) {
  return rk_aiq_user_api_anr_SetLumaSFStrength(ctx_, fPercnt);
}

int RKAiqToolANR::SetLumaTFStrength(float fPercnt) {
  return rk_aiq_user_api_anr_SetLumaTFStrength(ctx_, fPercnt);
}

int RKAiqToolANR::GetLumaSFStrength(float *pPercnt) {
  return rk_aiq_user_api_anr_GetLumaSFStrength(ctx_, pPercnt);
}

int RKAiqToolANR::GetLumaTFStrength(float *pPercnt) {
  return rk_aiq_user_api_anr_GetLumaTFStrength(ctx_, pPercnt);
}

int RKAiqToolANR::SetChromaSFStrength(float fPercnt) {
  return rk_aiq_user_api_anr_SetChromaSFStrength(ctx_, fPercnt);
}

int RKAiqToolANR::SetChromaTFStrength(float fPercnt) {
  return rk_aiq_user_api_anr_SetChromaTFStrength(ctx_, fPercnt);
}

int RKAiqToolANR::GetChromaSFStrength(float *pPercnt) {
  return rk_aiq_user_api_anr_GetChromaSFStrength(ctx_, pPercnt);
}

int RKAiqToolANR::GetChromaTFStrength(float *pPercnt) {
  return rk_aiq_user_api_anr_GetChromaTFStrength(ctx_, pPercnt);
}

int RKAiqToolANR::SetRawnrSFStrength(float fPercnt) {
  return rk_aiq_user_api_anr_SetRawnrSFStrength(ctx_, fPercnt);
}

int RKAiqToolANR::GetRawnrSFStrength(float *pPercnt) {
  return rk_aiq_user_api_anr_GetRawnrSFStrength(ctx_, pPercnt);
}
