#ifndef _TOOL_RKAIQ_API_ENGINE_H_
#define _TOOL_RKAIQ_API_ENGINE_H_

#include "rkaiq_imgproc.h"
#include <memory>
#include <thread>

#include "camera_memory.h"
#include "mediactl-priv.h"
#include "mediactl.h"
#include "v4l2subdev.h"
#include <linux/videodev2.h>

#include "logger/log.h"

#define RKAIQ_FILE_PATH_LEN 64
#define RKAIQ_CAMS_NUM_MAX 2
#define RKAIQ_FLASH_NUM_MAX 2

struct rkaiq_media_info {
  char sd_isp_path[RKAIQ_FILE_PATH_LEN];
  char vd_params_path[RKAIQ_FILE_PATH_LEN];
  char vd_stats_path[RKAIQ_FILE_PATH_LEN];
  char sd_ispp_path[RKAIQ_FILE_PATH_LEN];

  struct {
    char sd_sensor_path[RKAIQ_FILE_PATH_LEN];
    char sd_lens_path[RKAIQ_FILE_PATH_LEN];
    char sd_flash_path[RKAIQ_FLASH_NUM_MAX][RKAIQ_FILE_PATH_LEN];
    bool link_enabled;
    char sensor_entity_name[32];
  } cams[RKAIQ_FLASH_NUM_MAX];
};

typedef enum rk_aiq_state_e {
  AIQ_STATE_INVALID = 0,
  AIQ_STATE_INITIALIZED = 1,
  AIQ_STATE_PREPARED = 2,
  AIQ_STATE_RUNNING = 3,
  AIQ_STATE_STOPPED = 4,
  AIQ_STATE_MAX
} rk_aiq_state_t;

class RKAiqEngine {
public:
  RKAiqEngine();
  virtual ~RKAiqEngine();
  static void RKAiqEngineLoop(void *arg);
  rk_aiq_sys_ctx_t *GetContext() { return ctx_; }
  int GetDevName(struct media_device *device, const char *name, char *dev_name);
  int LinkToIsp();
  int EnumrateModules(struct media_device *device,
                      struct rkaiq_media_info *media_info);
  int GetMediaInfo(struct rkaiq_media_info *media_info_);
  int WaitStreamEvent(int fd, unsigned int event_type, int time_out_ms);
  int SubcribleStreamEvent(int fd, bool subs);
  int InitEngine();
  int StartEngine();
  int StopEngine();
  int DeInitEngine();

  friend class RKAiqToolManager;

private:
  rk_aiq_sys_ctx_t *ctx_;
  rk_aiq_working_mode_t mode_;
  std::thread *rkaiq_engine_thread_;
  static int thread_quit_;
  struct rkaiq_media_info media_info_;
  std::string sensor_entity_name_;
  std::string iq_file_dir_;
};

#endif // _TOOL_RKAIQ_API_ENGINE_H_