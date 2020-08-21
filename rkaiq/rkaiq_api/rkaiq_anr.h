#ifndef _TOOL_RKAIQ_API_BAYERNR_H_
#define _TOOL_RKAIQ_API_BAYERNR_H_

#include "rk_aiq_user_api_anr.h"

#include "logger/log.h"

class RKAiqToolANR {
public:
  RKAiqToolANR(rk_aiq_sys_ctx_t *ctx);
  virtual ~RKAiqToolANR();

  int SetAttrib(rk_aiq_nr_attrib_t *attr);
  int GetAttrib(rk_aiq_nr_attrib_t *attr);
  int SetIQPara(rk_aiq_nr_IQPara_t *attr);
  int GetIQPara(rk_aiq_nr_IQPara_t *attr);
  int SetLumaSFStrength(float fPercnt);
  int SetLumaTFStrength(float fPercnt);
  int GetLumaSFStrength(float *pPercnt);
  int GetLumaTFStrength(float *pPercnt);
  int SetChromaSFStrength(float fPercnt);
  int SetChromaTFStrength(float fPercnt);
  int GetChromaSFStrength(float *pPercnt);
  int GetChromaTFStrength(float *pPercnt);
  int SetRawnrSFStrength(float fPercnt);
  int GetRawnrSFStrength(float *pPercnt);

private:
  rk_aiq_sys_ctx_t *ctx_;
};

#endif // _TOOL_RKAIQ_API_BAYERNR_H_
