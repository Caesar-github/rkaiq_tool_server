#ifndef _TOOL_RKAIQ_API_AHDR_H_
#define _TOOL_RKAIQ_API_AHDR_H_

#include "rk_aiq.h"
#include "rk_aiq_user_api_ahdr.h"

class RKAiqToolAHDR {
public:
  RKAiqToolAHDR(rk_aiq_sys_ctx_t *ctx);
  virtual ~RKAiqToolAHDR();

  int SetAttrib(const ahdr_attrib_t attr);
  int GetAttrib(ahdr_attrib_t *attr);

private:
  rk_aiq_sys_ctx_t *ctx_;
};

#endif // _TOOL_RKAIQ_API_AHDR_H_
