#include "rkaiq_imgproc.h"

RKAiqToolImgProc::RKAiqToolImgProc(rk_aiq_sys_ctx_t *ctx) : ctx_(ctx) {}

RKAiqToolImgProc::~RKAiqToolImgProc() {}

int RKAiqToolImgProc::setExpMode(opMode_t mode) {
  return rk_aiq_uapi_setExpMode(ctx_, mode);
}

int RKAiqToolImgProc::getExpMode(opMode_t *mode) {
  return rk_aiq_uapi_getExpMode(ctx_, mode);
}

int RKAiqToolImgProc::setAeMode(aeMode_t mode) {
  return rk_aiq_uapi_setAeMode(ctx_, mode);
}

int RKAiqToolImgProc::getAeMode(aeMode_t *mode) {
  return rk_aiq_uapi_getAeMode(ctx_, mode);
}

int RKAiqToolImgProc::setExpGainRange(paRange_t *gain) {
  return rk_aiq_uapi_getExpGainRange(ctx_, gain);
}

int RKAiqToolImgProc::getExpGainRange(paRange_t *gain) {
  return rk_aiq_uapi_getExpGainRange(ctx_, gain);
}

int RKAiqToolImgProc::setExpTimeRange(paRange_t *time) {
  return rk_aiq_uapi_setExpTimeRange(ctx_, time);
}

int RKAiqToolImgProc::getExpTimeRange(paRange_t *time) {
  return rk_aiq_uapi_getExpTimeRange(ctx_, time);
}

int RKAiqToolImgProc::setBLCMode(bool on, paRect_t *rect) {
  return rk_aiq_uapi_setBLCMode(ctx_, on, rect);
}

int RKAiqToolImgProc::setHLCMode(bool on, paRect_t *rect) {
  return rk_aiq_uapi_setHLCMode(ctx_, on, rect);
}

int RKAiqToolImgProc::setLExpMode(opMode_t mode) {
  return rk_aiq_uapi_setLExpMode(ctx_, mode);
}

int RKAiqToolImgProc::getLExpMode(opMode_t *mode) {
  return rk_aiq_uapi_getLExpMode(ctx_, mode);
}

int RKAiqToolImgProc::setMLExp(unsigned int ratio) {
  return rk_aiq_uapi_setMLExp(ctx_, ratio);
}

int RKAiqToolImgProc::getMLExp(unsigned int *ratio) {
  return rk_aiq_uapi_getMLExp(ctx_, ratio);
}

int RKAiqToolImgProc::setAntiFlickerMode(antiFlickerMode_t mode) {
  return rk_aiq_uapi_setAntiFlickerMode(ctx_, mode);
}

int RKAiqToolImgProc::getAntiFlickerMode(antiFlickerMode_t *mode) {
  return rk_aiq_uapi_getAntiFlickerMode(ctx_, mode);
}

int RKAiqToolImgProc::setExpPwrLineFreqMode(expPwrLineFreq_t freq) {
  return rk_aiq_uapi_setExpPwrLineFreqMode(ctx_, freq);
}

int RKAiqToolImgProc::getExpPwrLineFreqMode(expPwrLineFreq_t *freq) {
  return rk_aiq_uapi_getExpPwrLineFreqMode(ctx_, freq);
}

int RKAiqToolImgProc::setDayNSwMode(opMode_t mode) {
  return rk_aiq_uapi_setDayNSwMode(ctx_, mode);
}

int RKAiqToolImgProc::getDayNSwMode(opMode_t *mode) {
  return rk_aiq_uapi_getDayNSwMode(ctx_, mode);
}

int RKAiqToolImgProc::setMDNScene(dayNightScene_t scene) {
  return rk_aiq_uapi_setMDNScene(ctx_, scene);
}

int RKAiqToolImgProc::getMDNScene(dayNightScene_t *scene) {
  return rk_aiq_uapi_getMDNScene(ctx_, scene);
}

int RKAiqToolImgProc::setADNSens(unsigned int level) {
  return rk_aiq_uapi_setADNSens(ctx_, level);
}

int RKAiqToolImgProc::getADNSens(unsigned int *level) {
  return rk_aiq_uapi_getADNSens(ctx_, level);
}

int RKAiqToolImgProc::setFLightMode(opMode_t mode) {
  return rk_aiq_uapi_setFLightMode(ctx_, mode);
}

int RKAiqToolImgProc::getFLightMode(opMode_t *mode) {
  return rk_aiq_uapi_getFLightMode(ctx_, mode);
}

int RKAiqToolImgProc::setMFLight(bool on) {
  return rk_aiq_uapi_setMFLight(ctx_, on);
}

int RKAiqToolImgProc::getMFLight(bool *on) {
  return rk_aiq_uapi_getMFLight(ctx_, on);
}

int RKAiqToolImgProc::setWBMode(opMode_t mode) {
  return rk_aiq_uapi_setWBMode(ctx_, mode);
}

int RKAiqToolImgProc::getWBMode(opMode_t *mode) {
  return rk_aiq_uapi_getWBMode(ctx_, mode);
}

int RKAiqToolImgProc::lockAWB() { return rk_aiq_uapi_lockAWB(ctx_); }

int RKAiqToolImgProc::unlockAWB() { return rk_aiq_uapi_unlockAWB(ctx_); }

int RKAiqToolImgProc::setAWBRange(awbRange_t range) {
  return rk_aiq_uapi_setAWBRange(ctx_, range);
}

int RKAiqToolImgProc::getAWBRange(awbRange_t *range) {
  return rk_aiq_uapi_getAWBRange(ctx_, range);
}

int RKAiqToolImgProc::setMWBScene(rk_aiq_wb_scene_t scene) {
  return rk_aiq_uapi_setMWBScene(ctx_, scene);
}

int RKAiqToolImgProc::getMWBScene(rk_aiq_wb_scene_t *scene) {
  return rk_aiq_uapi_getMWBScene(ctx_, scene);
}

int RKAiqToolImgProc::setMWBGain(rk_aiq_wb_gain_t *gain) {
  return rk_aiq_uapi_setMWBGain(ctx_, gain);
}

int RKAiqToolImgProc::getMWBGain(rk_aiq_wb_gain_t *gain) {
  return rk_aiq_uapi_getMWBGain(ctx_, gain);
}

int RKAiqToolImgProc::setMWBCT(unsigned int ct) {
  return rk_aiq_uapi_setMWBCT(ctx_, ct);
}

int RKAiqToolImgProc::getMWBCT(unsigned int *ct) {
  return rk_aiq_uapi_getMWBCT(ctx_, ct);
}

int RKAiqToolImgProc::setCrSuppsn(unsigned int level) {
  return rk_aiq_uapi_setCrSuppsn(ctx_, level);
}

int RKAiqToolImgProc::getCrSuppsn(unsigned int *level) {
  return rk_aiq_uapi_getCrSuppsn(ctx_, level);
}

int RKAiqToolImgProc::setFocusMode(opMode_t mode) {
  return rk_aiq_uapi_setFocusMode(ctx_, mode);
}

int RKAiqToolImgProc::getFocusMode(opMode_t *mode) {
  return rk_aiq_uapi_getFocusMode(ctx_, mode);
}

int RKAiqToolImgProc::setFocusWin(paRect_t *rect) {
  return rk_aiq_uapi_setFocusWin(ctx_, rect);
}

int RKAiqToolImgProc::getFocusWin(paRect_t *rect) {
  return rk_aiq_uapi_getFocusWin(ctx_, rect);
}

int RKAiqToolImgProc::setFixedModeCode(unsigned short code) {
  return rk_aiq_uapi_setFixedModeCode(ctx_, code);
}

int RKAiqToolImgProc::getFixedModeCode(unsigned short *code) {
  return rk_aiq_uapi_getFixedModeCode(ctx_, code);
}

int RKAiqToolImgProc::setMinFocusDis(unsigned int distance) {
  return rk_aiq_uapi_setMinFocusDis(ctx_, distance);
}

int RKAiqToolImgProc::getMinFocusDis(unsigned int *distance) {
  return rk_aiq_uapi_getMinFocusDis(ctx_, distance);
}

int RKAiqToolImgProc::setOpZoomRange(paRange_t *range) {
  return rk_aiq_uapi_setOpZoomRange(ctx_, range);
}

int RKAiqToolImgProc::getOpZoomRange(paRange_t *range) {
  return rk_aiq_uapi_setOpZoomRange(ctx_, range);
}

int RKAiqToolImgProc::setOpZoomSpeed(unsigned int level) {
  return rk_aiq_uapi_setOpZoomSpeed(ctx_, level);
}

int RKAiqToolImgProc::getOpZoomSpeed(unsigned int *level) {
  return rk_aiq_uapi_getOpZoomSpeed(ctx_, level);
}

int RKAiqToolImgProc::setHDRMode(opMode_t mode) {
  return rk_aiq_uapi_setHDRMode(ctx_, mode);
}

int RKAiqToolImgProc::getHDRMode(opMode_t *mode) {
  return rk_aiq_uapi_getHDRMode(ctx_, mode);
}

int RKAiqToolImgProc::setMHDRStrth(bool on, unsigned int level) {
  return rk_aiq_uapi_setMHDRStrth(ctx_, on, level);
}

int RKAiqToolImgProc::getMHDRStrth(bool *on, unsigned int *level) {
  return rk_aiq_uapi_getMHDRStrth(ctx_, on, level);
}

int RKAiqToolImgProc::setNRMode(opMode_t mode) {
  return rk_aiq_uapi_setNRMode(ctx_, mode);
}

int RKAiqToolImgProc::getNRMode(opMode_t *mode) {
  return rk_aiq_uapi_getNRMode(ctx_, mode);
}

int RKAiqToolImgProc::setANRStrth(unsigned int level) {
  return rk_aiq_uapi_setANRStrth(ctx_, level);
}

int RKAiqToolImgProc::getANRStrth(unsigned int *level) {
  return rk_aiq_uapi_getANRStrth(ctx_, level);
}

int RKAiqToolImgProc::setMSpaNRStrth(bool on, unsigned int level) {
  return rk_aiq_uapi_setMSpaNRStrth(ctx_, on, level);
}

int RKAiqToolImgProc::getMSpaNRStrth(bool *on, unsigned int *level) {
  return rk_aiq_uapi_getMSpaNRStrth(ctx_, on, level);
}

int RKAiqToolImgProc::setMTNRStrth(bool on, unsigned int level) {
  return rk_aiq_uapi_setMTNRStrth(ctx_, on, level);
}

int RKAiqToolImgProc::getMTNRStrth(bool *on, unsigned int *level) {
  return rk_aiq_uapi_getMTNRStrth(ctx_, on, level);
}

int RKAiqToolImgProc::setDhzMode(opMode_t mode) {
  return rk_aiq_uapi_setDhzMode(ctx_, mode);
}

int RKAiqToolImgProc::getDhzMode(opMode_t *mode) {
  return rk_aiq_uapi_getDhzMode(ctx_, mode);
}

int RKAiqToolImgProc::setMDhzStrth(bool on, unsigned int level) {
  return rk_aiq_uapi_setMDhzStrth(ctx_, on, level);
}

int RKAiqToolImgProc::getMDhzStrth(bool *on, unsigned int *level) {
  return rk_aiq_uapi_getMDhzStrth(ctx_, on, level);
}

int RKAiqToolImgProc::setContrast(unsigned int level) {
  return rk_aiq_uapi_setContrast(ctx_, level);
}

int RKAiqToolImgProc::getContrast(unsigned int *level) {
  return rk_aiq_uapi_getContrast(ctx_, level);
}

int RKAiqToolImgProc::setBrightness(unsigned int level) {
  return rk_aiq_uapi_setBrightness(ctx_, level);
}

int RKAiqToolImgProc::getBrightness(unsigned int *level) {
  return rk_aiq_uapi_getBrightness(ctx_, level);
}

int RKAiqToolImgProc::setSaturation(unsigned int level) {
  return rk_aiq_uapi_setSaturation(ctx_, level);
}

int RKAiqToolImgProc::getSaturation(unsigned int *level) {
  return rk_aiq_uapi_getSaturation(ctx_, level);
}

int RKAiqToolImgProc::setSharpness(unsigned int level) {
  return rk_aiq_uapi_setSharpness(ctx_, level);
}

int RKAiqToolImgProc::getSharpness(unsigned int *level) {
  return rk_aiq_uapi_getSharpness(ctx_, level);
}

int RKAiqToolImgProc::setGammaCoef(unsigned int level) {
  return rk_aiq_uapi_setGammaCoef(ctx_, level);
}
