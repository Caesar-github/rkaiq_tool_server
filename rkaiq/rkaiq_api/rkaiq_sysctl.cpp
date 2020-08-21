#include "rkaiq_sysctl.h"

RKAiqToolSysCtl::RKAiqToolSysCtl(rk_aiq_sys_ctx_t *ctx) : ctx_(ctx) {}

RKAiqToolSysCtl::~RKAiqToolSysCtl() {}

int RKAiqToolSysCtl::SetCpsLtCfg(rk_aiq_cpsl_cfg_t *cfg) {
  return rk_aiq_uapi_sysctl_setCpsLtCfg(ctx_, cfg);
}

int RKAiqToolSysCtl::GetCpsLtInfo(rk_aiq_cpsl_info_t *info) {
  return rk_aiq_uapi_sysctl_getCpsLtInfo(ctx_, info);
}

int RKAiqToolSysCtl::QueryCpsLtCap(rk_aiq_cpsl_cap_t *cap) {
  return rk_aiq_uapi_sysctl_queryCpsLtCap(ctx_, cap);
}

int RKAiqToolSysCtl::GetVersionInfo(rk_aiq_ver_info_t *vers) {
  rk_aiq_uapi_get_version_info(vers);
  return 0;
}