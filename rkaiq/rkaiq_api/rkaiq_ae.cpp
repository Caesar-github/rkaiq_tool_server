#include "rkaiq_ae.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "rkaiq_ae.cpp"

RKAiqToolAE::RKAiqToolAE(rk_aiq_sys_ctx_t *ctx) : ctx_(ctx) {}

RKAiqToolAE::~RKAiqToolAE() {}

int RKAiqToolAE::setExpSwAttr(const Uapi_ExpSwAttr_t expSwAttr) {
  return rk_aiq_user_api_ae_setExpSwAttr(ctx_, expSwAttr);
}

int RKAiqToolAE::getExpSwAttr(Uapi_ExpSwAttr_t *pExpSwAttr) {
  return rk_aiq_user_api_ae_getExpSwAttr(ctx_, pExpSwAttr);
}

int RKAiqToolAE::setLinAeDayRouteAttr(
    const Uapi_LinAeRouteAttr_t linAeRouteAttr) {
  return rk_aiq_user_api_ae_setLinAeDayRouteAttr(ctx_, linAeRouteAttr);
}

int RKAiqToolAE::getLinAeDayRouteAttr(Uapi_LinAeRouteAttr_t *pLinAeRouteAttr) {
  return rk_aiq_user_api_ae_getLinAeDayRouteAttr(ctx_, pLinAeRouteAttr);
}

int RKAiqToolAE::setLinAeNightRouteAttr(
    const Uapi_LinAeRouteAttr_t linAeRouteAttr) {
  return rk_aiq_user_api_ae_setLinAeNightRouteAttr(ctx_, linAeRouteAttr);
}

int RKAiqToolAE::getLinAeNightRouteAttr(
    Uapi_LinAeRouteAttr_t *pLinAeRouteAttr) {
  return rk_aiq_user_api_ae_getLinAeNightRouteAttr(ctx_, pLinAeRouteAttr);
}

int RKAiqToolAE::setHdrAeDayRouteAttr(
    const Uapi_HdrAeRouteAttr_t hdrAeRouteAttr) {
  return rk_aiq_user_api_ae_setHdrAeDayRouteAttr(ctx_, hdrAeRouteAttr);
}

int RKAiqToolAE::getHdrAeDayRouteAttr(Uapi_HdrAeRouteAttr_t *pHdrAeRouteAttr) {
  return rk_aiq_user_api_ae_getHdrAeDayRouteAttr(ctx_, pHdrAeRouteAttr);
}

int RKAiqToolAE::setHdrAeNightRouteAttr(
    const Uapi_HdrAeRouteAttr_t hdrAeRouteAttr) {
  return rk_aiq_user_api_ae_setHdrAeNightRouteAttr(ctx_, hdrAeRouteAttr);
}

int RKAiqToolAE::getHdrAeNightRouteAttr(
    Uapi_HdrAeRouteAttr_t *pHdrAeRouteAttr) {
  return rk_aiq_user_api_ae_getHdrAeNightRouteAttr(ctx_, pHdrAeRouteAttr);
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
