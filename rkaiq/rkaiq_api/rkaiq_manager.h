#ifndef _TOOL_RKAIQ_API_MANAGER_H_
#define _TOOL_RKAIQ_API_MANAGER_H_

#include "rkaiq_ae.h"
#include "rkaiq_anr.h"
#include "rkaiq_cmdid.h"
#include "rkaiq_engine.h"
#include "rkaiq_imgproc.h"
#include "rkaiq_sharp.h"
#include <memory>
#include <thread>

#include "logger/log.h"

class RKAiqToolManager {
public:
  RKAiqToolManager();
  virtual ~RKAiqToolManager();
  int AeIoCtrl(int id, void *data, int size);
  int ImgProcIoCtrl(int id, void *data, int size);
  int AnrIoCtrl(int id, void *data, int size);
  int SharpIoCtrl(int id, void *data, int size);
  int IoCtrl(int id, void *data, int size);
  void SaveExit();

private:
  rk_aiq_sys_ctx_t *ctx_;
  std::thread *rkaiq_engine_thread_;
  static int thread_quit_;
  std::unique_ptr<RKAiqEngine> engine_;
  std::unique_ptr<RKAiqToolImgProc> imgproc_;
  std::unique_ptr<RKAiqToolAE> ae_;
  std::unique_ptr<RKAiqToolANR> anr_;
  std::unique_ptr<RKAiqToolSharp> asharp_;
};

#endif // _TOOL_RKAIQ_API_MANAGER_H_
