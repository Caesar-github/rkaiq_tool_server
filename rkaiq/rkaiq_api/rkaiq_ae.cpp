#include "rkaiq_ae.h"

RKAiqToolAE::RKAiqToolAE(rk_aiq_sys_ctx_t *ctx) {}

RKAiqToolAE::~RKAiqToolAE() {}

int RKAiqToolAE::setExpSwAttr(const Uapi_ExpSwAttr_t expSwAttr) {
  return rk_aiq_user_api_ae_setExpSwAttr(ctx_, expSwAttr);
}

int RKAiqToolAE::getExpSwAttr(Uapi_ExpSwAttr_t *pExpSwAttr) {
  return rk_aiq_user_api_ae_getExpSwAttr(ctx_, pExpSwAttr);
}

int RKAiqToolAE::setLinAeRouteAttr(const Uapi_LinAeRouteAttr_t linAeRouteAttr) {
  return rk_aiq_user_api_ae_setLinAeRouteAttr(ctx_, linAeRouteAttr);
}

int RKAiqToolAE::getLinAeRouteAttr(Uapi_LinAeRouteAttr_t *pLinAeRouteAttr) {
  return rk_aiq_user_api_ae_getLinAeRouteAttr(ctx_, pLinAeRouteAttr);
}

int RKAiqToolAE::setHdrAeRouteAttr(const Uapi_HdrAeRouteAttr_t hdrAeRouteAttr) {
  return rk_aiq_user_api_ae_setHdrAeRouteAttr(ctx_, hdrAeRouteAttr);
}

int RKAiqToolAE::getHdrAeRouteAttr(Uapi_HdrAeRouteAttr_t *pHdrAeRouteAttr) {
  return rk_aiq_user_api_ae_getHdrAeRouteAttr(ctx_, pHdrAeRouteAttr);
}

int RKAiqToolAE::queryExpResInfo(Uapi_ExpQueryInfo_t *pExpResInfo) {
  return rk_aiq_user_api_ae_queryExpResInfo(ctx_, pExpResInfo);
}

int RKAiqToolAE::setLinExpAttr(const Uapi_LinExpAttr_t linExpAttr) {
  return rk_aiq_user_api_ae_setLinExpAttr(ctx_, linExpAttr);
}

int RKAiqToolAE::getLinExpAttr(Uapi_LinExpAttr_t *pLinExpAttr) {
  return rk_aiq_user_api_ae_getLinExpAttr(ctx_, pLinExpAttr);
}

int RKAiqToolAE::setHdrExpAttr(const Uapi_HdrExpAttr_t hdrExpAttr) {
  return rk_aiq_user_api_ae_setHdrExpAttr(ctx_, hdrExpAttr);
}

int RKAiqToolAE::getHdrExpAttr(Uapi_HdrExpAttr_t *pHdrExpAttr) {
  return rk_aiq_user_api_ae_getHdrExpAttr(ctx_, pHdrExpAttr);
}
