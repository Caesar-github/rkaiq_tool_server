#ifndef _TOOL_RKAIQ_API_AE_H_
#define _TOOL_RKAIQ_API_AE_H_

#include "rk_aiq_user_api_ae.h"

class RKAiqToolAE {
public:
  RKAiqToolAE(rk_aiq_sys_ctx_t *ctx);
  virtual ~RKAiqToolAE();

  int setExpSwAttr(const Uapi_ExpSwAttr_t expSwAttr);
  int getExpSwAttr(Uapi_ExpSwAttr_t *pExpSwAttr);
  int setLinAeRouteAttr(const Uapi_LinAeRouteAttr_t linAeRouteAttr);
  int getLinAeRouteAttr(Uapi_LinAeRouteAttr_t *pLinAeRouteAttr);
  int setHdrAeRouteAttr(const Uapi_HdrAeRouteAttr_t hdrAeRouteAttr);
  int getHdrAeRouteAttr(Uapi_HdrAeRouteAttr_t *pHdrAeRouteAttr);
  int queryExpResInfo(Uapi_ExpQueryInfo_t *pExpResInfo);
  int setLinExpAttr(const Uapi_LinExpAttr_t linExpAttr);
  int getLinExpAttr(Uapi_LinExpAttr_t *pLinExpAttr);
  int setHdrExpAttr(const Uapi_HdrExpAttr_t hdrExpAttr);
  int getHdrExpAttr(Uapi_HdrExpAttr_t *pHdrExpAttr);

private:
  rk_aiq_sys_ctx_t *ctx_;
};

#endif // _TOOL_RKAIQ_API_AE_H_
