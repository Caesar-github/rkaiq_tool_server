#ifndef _TOOL_RKAIQ_API_MANAGER_H_
#define _TOOL_RKAIQ_API_MANAGER_H_

#include "rkaiq_cmdid.h"
#include "rkaiq_ae.h"
#include "rkaiq_anr.h"
#include "rkaiq_engine.h"
#include "rkaiq_imgproc.h"
#include <memory>
#include <thread>

#include "logger/log.h"

class RKAiqToolManager {
public:
  RKAiqToolManager();
  virtual ~RKAiqToolManager();

private:
  rk_aiq_sys_ctx_t *ctx_;
  std::thread *rkaiq_engine_thread_;
  static int thread_quit_;
  std::unique_ptr<RKAiqEngine> engine_;
  std::unique_ptr<RKAiqToolImgProc> imgproc_;
  std::unique_ptr<RKAiqToolAE> ae_;
  std::unique_ptr<RKAiqToolANR> anr_;
};

#endif // _TOOL_RKAIQ_API_MANAGER_H_
