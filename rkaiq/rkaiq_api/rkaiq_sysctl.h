#ifndef _TOOL_RKAIQ_API_SYSCTL_H_
#define _TOOL_RKAIQ_API_SYSCTL_H_

#include "rk_aiq.h"
#include "rk_aiq_user_api_sysctl.h"

class RKAiqToolSysCtl {
public:
  RKAiqToolSysCtl(rk_aiq_sys_ctx_t *ctx);
  virtual ~RKAiqToolSysCtl();

  int SetCpsLtCfg(rk_aiq_cpsl_cfg_t *cfg);
  int GetCpsLtInfo(rk_aiq_cpsl_info_t *info);
  int QueryCpsLtCap(rk_aiq_cpsl_cap_t *cap);

  int GetVersionInfo(rk_aiq_ver_info_t *vers);

private:
  rk_aiq_sys_ctx_t *ctx_;
};

#endif // _TOOL_RKAIQ_API_SYSCTL_H_
