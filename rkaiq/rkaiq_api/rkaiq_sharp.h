#ifndef _TOOL_RKAIQ_API_SHARP_H_
#define _TOOL_RKAIQ_API_SHARP_H_

#include "rk_aiq_user_api_asharp.h"

class RKAiqToolSharp {
public:
  RKAiqToolSharp(rk_aiq_sys_ctx_t *ctx);
  virtual ~RKAiqToolSharp();

  int SetAttrib(rk_aiq_sharp_attrib_t *attr);
  int GetAttrib(rk_aiq_sharp_attrib_t *attr);
  int SetIQPara(rk_aiq_sharp_IQpara_t *para);
  int GetIQPara(rk_aiq_sharp_IQpara_t *para);
  int SetStrength(float fPercent);
  int GetStrength(float *pPercent);

private:
  rk_aiq_sys_ctx_t *ctx_;
};

#endif // _TOOL_RKAIQ_API_SHARP_H_
