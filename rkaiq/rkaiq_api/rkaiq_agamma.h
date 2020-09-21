#ifndef _TOOL_RKAIQ_API_AGAMMA_H_
#define _TOOL_RKAIQ_API_AGAMMA_H_

#include "rk_aiq.h"
#include "rk_aiq_user_api_agamma.h"

class RKAiqToolAGamma {
public:
  RKAiqToolAGamma(rk_aiq_sys_ctx_t *ctx);
  virtual ~RKAiqToolAGamma();

  int SetAttrib(const rk_aiq_gamma_attrib_t attr);
  int GetAttrib(rk_aiq_gamma_attrib_t *attr);

private:
  rk_aiq_sys_ctx_t *ctx_;
};

#endif // _TOOL_RKAIQ_API_AGAMMA_H_
