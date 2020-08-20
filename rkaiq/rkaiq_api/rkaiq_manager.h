#ifndef _TOOL_RKAIQ_API_MANAGER_H_
#define _TOOL_RKAIQ_API_MANAGER_H_

#include "rkaiq_ae.h"
#include "rkaiq_anr.h"
#include "rkaiq_cmdid.h"
#include "rkaiq_engine.h"
#include "rkaiq_imgproc.h"
#include "rkaiq_sharp.h"
#include "rkaiq_sysctl.h"
#include <memory>
#include <thread>

#include "logger/log.h"

#define MATCH_RKAIQ_VERSION "v1.0.5" 
#define MATCH_IQ_PARSER_VERSION "v1.1.8"

class RKAiqToolManager {
public:
  RKAiqToolManager();
  virtual ~RKAiqToolManager();
  int AeIoCtrl(int id, void *data, int size);
  int ImgProcIoCtrl(int id, void *data, int size);
  int AnrIoCtrl(int id, void *data, int size);
  int SharpIoCtrl(int id, void *data, int size);
  int SysCtlIoCtrl(int id, void *data, int size);
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
  std::unique_ptr<RKAiqToolSysCtl> sysctl_;
};

#endif // _TOOL_RKAIQ_API_MANAGER_H_
