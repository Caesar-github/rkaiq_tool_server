#ifndef _TOOL_RKAIQ_API_IMGPROC_H_
#define _TOOL_RKAIQ_API_IMGPROC_H_

#include "rk_aiq_user_api_imgproc.h"

class RKAiqToolImgProc {
public:
  RKAiqToolImgProc(rk_aiq_sys_ctx_t *ctx);
  virtual ~RKAiqToolImgProc();

  int setExpMode(opMode_t mode);
  int getExpMode(opMode_t *mode);
  int setAeMode(aeMode_t mode);
  int getAeMode(aeMode_t *mode);
  int setExpGainRange(paRange_t *gain);
  int getExpGainRange(paRange_t *gain);
  int setExpTimeRange(paRange_t *time);
  int getExpTimeRange(paRange_t *time);
  int setBLCMode(bool on, paRect_t *rect);
  int setHLCMode(bool on, paRect_t *rect);
  int setLExpMode(opMode_t mode);
  int getLExpMode(opMode_t *mode);
  int setMLExp(unsigned int ratio);
  int getMLExp(unsigned int *ratio);
  int setAntiFlickerMode(antiFlickerMode_t mode);
  int getAntiFlickerMode(antiFlickerMode_t *mode);
  int setExpPwrLineFreqMode(expPwrLineFreq_t freq);
  int getExpPwrLineFreqMode(expPwrLineFreq_t *freq);
  int setDayNSwMode(opMode_t mode);
  int getDayNSwMode(opMode_t *mode);
  int setMDNScene(dayNightScene_t scene);
  int getMDNScene(dayNightScene_t *scene);
  int setADNSens(unsigned int level);
  int getADNSens(unsigned int *level);
  int setFLightMode(opMode_t mode);
  int getFLightMode(opMode_t *mode);
  int setMFLight(bool on);
  int getMFLight(bool *on);
  int setWBMode(opMode_t mode);
  int getWBMode(opMode_t *mode);
  int lockAWB();
  int unlockAWB();
  int setAWBRange(awbRange_t range);
  int getAWBRange(awbRange_t *range);
  int setMWBScene(rk_aiq_wb_scene_t scene);
  int getMWBScene(rk_aiq_wb_scene_t *scene);
  int setMWBGain(rk_aiq_wb_gain_t *gain);
  int getMWBGain(rk_aiq_wb_gain_t *gain);
  int setMWBCT(unsigned int ct);
  int getMWBCT(unsigned int *ct);
  int setCrSuppsn(unsigned int level);
  int getCrSuppsn(unsigned int *level);
  int setFocusMode(opMode_t mode);
  int getFocusMode(opMode_t *mode);
  int setFocusWin(paRect_t *rect);
  int getFocusWin(paRect_t *rect);
  int setFixedModeCode(unsigned short code);
  int getFixedModeCode(unsigned short *code);
  int setMinFocusDis(unsigned int distance);
  int getMinFocusDis(unsigned int *distance);
  int setOpZoomRange(paRange_t *range);
  int getOpZoomRange(paRange_t *range);
  int setOpZoomSpeed(unsigned int level);
  int getOpZoomSpeed(unsigned int *level);
  int setHDRMode(opMode_t mode);
  int getHDRMode(opMode_t *mode);
  int setMHDRStrth(bool on, unsigned int level);
  int getMHDRStrth(bool *on, unsigned int *level);
  int setNRMode(opMode_t mode);
  int getNRMode(opMode_t *mode);
  int setANRStrth(unsigned int level);
  int getANRStrth(unsigned int *level);
  int setMSpaNRStrth(bool on, unsigned int level);
  int getMSpaNRStrth(bool *on, unsigned int *level);
  int setMTNRStrth(bool on, unsigned int level);
  int getMTNRStrth(bool *on, unsigned int *level);
  int setDhzMode(opMode_t mode);
  int getDhzMode(opMode_t *mode);
  int setMDhzStrth(bool on, unsigned int level);
  int getMDhzStrth(bool *on, unsigned int *level);
  int setContrast(unsigned int level);
  int getContrast(unsigned int *level);
  int setBrightness(unsigned int level);
  int getBrightness(unsigned int *level);
  int setSaturation(unsigned int level);
  int getSaturation(unsigned int *level);
  int setSharpness(unsigned int level);
  int getSharpness(unsigned int *level);
  int setGammaCoef(unsigned int level);

private:
  rk_aiq_sys_ctx_t *ctx_;
};

#endif // _TOOL_RKAIQ_API_IMGPROC_H_
