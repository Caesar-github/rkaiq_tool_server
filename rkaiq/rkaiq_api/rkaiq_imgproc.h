#ifndef _TOOL_RKAIQ_API_IMGPROC_H_
#define _TOOL_RKAIQ_API_IMGPROC_H_

#include "rk_aiq_user_api_imgproc.h"

class RKAiqToolImgProc {
public:
  RKAiqToolImgProc(rk_aiq_sys_ctx_t *ctx);
  virtual ~RKAiqToolImgProc();

private:
  rk_aiq_sys_ctx_t *ctx_;
};

#endif // _TOOL_RKAIQ_API_IMGPROC_H_
