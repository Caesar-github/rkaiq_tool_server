#ifndef _TOOL_RKAIQ_API_MANAGER_H_
#define _TOOL_RKAIQ_API_MANAGER_H_

#include "rkaiq_adpcc.h"
#include "rkaiq_ae.h"
#include "rkaiq_agamma.h"
#include "rkaiq_ahdr.h"
#include "rkaiq_anr.h"
#include "rkaiq_cmdid.h"
#include "rkaiq_dehaze.h"
#include "rkaiq_engine.h"
#include "rkaiq_imgproc.h"
#include "rkaiq_sharp.h"
#include "rkaiq_sysctl.h"
#include <memory>
#include <thread>

#include "logger/log.h"

#define MATCH_RKAIQ_VERSION_1 "v1.0.5"
#define MATCH_RKAIQ_VERSION_2 "v1.0.6"
#define MATCH_RKAIQ_VERSION_3 "v1.0.7"
#define MATCH_RKAIQ_VERSION_4 "v1.0.8"
#define MATCH_RKAIQ_VERSION_5 "v1.0.9"
#define MATCH_RKAIQ_VERSION_6 "v1.2.0"
#define MATCH_RKAIQ_VERSION_7 "v1.2.1"
#define MATCH_RKAIQ_VERSION_8 "v1.2.2"

#define MATCH_IQ_PARSER_VERSION "unknow"

class RKAiqToolManager {
public:
  RKAiqToolManager() = delete;
  RKAiqToolManager(std::string iqfiles_path, std::string sensor_name);
  virtual ~RKAiqToolManager();
  int AeIoCtrl(int id, void *data, int size);
  int ImgProcIoCtrl(int id, void *data, int size);
  int AnrIoCtrl(int id, void *data, int size);
  int SharpIoCtrl(int id, void *data, int size);
  int SysCtlIoCtrl(int id, void *data, int size);
  int AHDRIoCtrl(int id, void *data, int size);
  int AGamamIoCtrl(int id, void *data, int size);
  int ADPCCIoCtrl(int id, void *data, int size);
  int DEHAZEIoCtrl(int id, void *data, int size);
  int IoCtrl(int id, void *data, int size);
  void SaveExit();

private:
  std::string iqfiles_path_;
  std::string sensor_name_;
  rk_aiq_sys_ctx_t *ctx_;
  std::thread *rkaiq_engine_thread_;
  static int thread_quit_;
  std::unique_ptr<RKAiqEngine> engine_;
  std::unique_ptr<RKAiqToolImgProc> imgproc_;
  std::unique_ptr<RKAiqToolAE> ae_;
  std::unique_ptr<RKAiqToolANR> anr_;
  std::unique_ptr<RKAiqToolSharp> asharp_;
  std::unique_ptr<RKAiqToolSysCtl> sysctl_;
  std::unique_ptr<RKAiqToolAHDR> ahdr_;
  std::unique_ptr<RKAiqToolADPCC> dpcc_;
  std::unique_ptr<RKAiqToolAGamma> gamma_;
  std::unique_ptr<RKAiqToolDehaze> dehaze_;
};

#endif // _TOOL_RKAIQ_API_MANAGER_H_
