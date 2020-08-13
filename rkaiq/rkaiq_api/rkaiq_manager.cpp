#include "rkaiq_manager.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "rkaiq_manager.cpp"

RKAiqToolManager::RKAiqToolManager() : ctx_(nullptr) {
  engine_.reset(new RKAiqEngine());
  ctx_ = engine_->GetContext();
  imgproc_.reset(new RKAiqToolImgProc(ctx_));
  ae_.reset(new RKAiqToolAE(ctx_));
  anr_.reset(new RKAiqToolANR(ctx_));
  asharp_.reset(new RKAiqToolSharp(ctx_));
}

RKAiqToolManager::~RKAiqToolManager() {
  anr_.reset(nullptr);
  ae_.reset(nullptr);
  imgproc_.reset(nullptr);
  engine_.reset(nullptr);
}

void RKAiqToolManager::SaveExit() { RKAiqEngine::thread_quit_ = 1; }

int RKAiqToolManager::IoCtrl(int id, void *data, int size) {
  LOG_INFO("IoCtrl id: 0x%x\n", id);
  if (id > ENUM_ID_AE_START && id < ENUM_ID_AE_END) {
    AeIoCtrl(id, data, size);
  } else if (id > ENUM_ID_IMGPROC_START && id < ENUM_ID_IMGPROC_END) {
    ImgProcIoCtrl(id, data, size);
  } else if (id > ENUM_ID_ANR_START && id < ENUM_ID_ANR_END) {
    AnrIoCtrl(id, data, size);
  }
  return 0;
}

int RKAiqToolManager::AeIoCtrl(int id, void *data, int size) {
  LOG_INFO("AeIoCtrl id: 0x%x\n", id);
  switch (id) {
  case ENUM_ID_AE_SETEXPSWATTR:
    ae_->setExpSwAttr(*(Uapi_ExpSwAttr_t *)data);
    break;
  case ENUM_ID_AE_GETEXPSWATTR:
    ae_->getExpSwAttr((Uapi_ExpSwAttr_t *)data);
    break;
  case ENUM_ID_AE_SETLINAEDAYROUTEATTR:
    ae_->setLinAeDayRouteAttr(*(Uapi_LinAeRouteAttr_t *)data);
    break;
  case ENUM_ID_AE_GETLINAEDAYROUTEATTR:
    ae_->getLinAeDayRouteAttr((Uapi_LinAeRouteAttr_t *)data);
    break;
  case ENUM_ID_AE_SETLINAENIGHTROUTEATTR:
    ae_->setLinAeNightRouteAttr(*(Uapi_LinAeRouteAttr_t *)data);
    break;
  case ENUM_ID_AE_GETLINAENIGHTROUTEATTR:
    ae_->getLinAeNightRouteAttr((Uapi_LinAeRouteAttr_t *)data);
    break;
  case ENUM_ID_AE_SETHDRAEDAYROUTEATTR:
    ae_->setHdrAeDayRouteAttr(*(Uapi_HdrAeRouteAttr_t *)data);
    break;
  case ENUM_ID_AE_GETHDRAEDAYROUTEATTR:
    ae_->getHdrAeDayRouteAttr((Uapi_HdrAeRouteAttr_t *)data);
    break;
  case ENUM_ID_AE_SETHDRAENIGHTROUTEATTR:
    ae_->setHdrAeNightRouteAttr(*(Uapi_HdrAeRouteAttr_t *)data);
    break;
  case ENUM_ID_AE_GETHDRAENIGHTROUTEATTR:
    ae_->getHdrAeNightRouteAttr((Uapi_HdrAeRouteAttr_t *)data);
    break;
  case ENUM_ID_AE_QUERYEXPRESINFO:
    ae_->queryExpResInfo((Uapi_ExpQueryInfo_t *)data);
    break;
  case ENUM_ID_AE_SETLINEXPATTR:
    ae_->setLinExpAttr(*(Uapi_LinExpAttr_t *)data);
    break;
  case ENUM_ID_AE_GETLINEXPATTR:
    ae_->getLinExpAttr((Uapi_LinExpAttr_t *)data);
    break;
  case ENUM_ID_AE_SETHDREXPATTR:
    ae_->setHdrExpAttr(*(Uapi_HdrExpAttr_t *)data);
    break;
  case ENUM_ID_AE_GETHDREXPATTR:
    ae_->getHdrExpAttr((Uapi_HdrExpAttr_t *)data);
    break;
  default:
    LOG_INFO("cmdID: Unknow\n");
    break;
  }
  return 0;
}

int RKAiqToolManager::ImgProcIoCtrl(int id, void *data, int size) {
  LOG_INFO("ImgProcIoCtrl id: 0x%x\n", id);
  switch (id) {
  case ENUM_ID_IMGPROC_SETEXPMODE:
    imgproc_->setExpMode(*(opMode_t *)data);
    break;
  case ENUM_ID_IMGPROC_GETEXPMODE:
    imgproc_->getExpMode((opMode_t *)data);
    break;
  case ENUM_ID_IMGPROC_SETAEMODE:
    imgproc_->setAeMode(*(aeMode_t *)data);
    break;
  case ENUM_ID_IMGPROC_GETAEMODE:
    imgproc_->getAeMode((aeMode_t *)data);
    break;
  case ENUM_ID_IMGPROC_SETEXPGAINRANGE:
    imgproc_->setExpGainRange((paRange_t *)data);
    break;
  case ENUM_ID_IMGPROC_GETEXPGAINRANGE:
    imgproc_->getExpGainRange((paRange_t *)data);
    break;
  case ENUM_ID_IMGPROC_SETEXPTIMERANGE:
    imgproc_->setExpTimeRange((paRange_t *)data);
    break;
  case ENUM_ID_IMGPROC_GETEXPTIMERANGE:
    imgproc_->getExpTimeRange((paRange_t *)data);
    break;
  case ENUM_ID_IMGPROC_SETBLCMODE:
    break;
  case ENUM_ID_IMGPROC_SETHLCMODE:
    break;
  case ENUM_ID_IMGPROC_SETLEXPMODE:
    break;
  case ENUM_ID_IMGPROC_GETLEXPMODE:
    break;
  case ENUM_ID_IMGPROC_SETMLEXP:
    break;
  case ENUM_ID_IMGPROC_GETMLEXP:
    break;
  case ENUM_ID_IMGPROC_SETANTIFLICKERMODE:
    break;
  case ENUM_ID_IMGPROC_GETANTIFLICKERMODE:
    break;
  case ENUM_ID_IMGPROC_SETEXPPWRLINEFREQMODE:
    break;
  case ENUM_ID_IMGPROC_GETEXPPWRLINEFREQMODE:
    break;
  case ENUM_ID_IMGPROC_SETDAYNSWMODE:
    break;
  case ENUM_ID_IMGPROC_GETDAYNSWMODE:
    break;
  case ENUM_ID_IMGPROC_SETMDNSCENE:
    break;
  case ENUM_ID_IMGPROC_GETMDNSCENE:
    break;
  case ENUM_ID_IMGPROC_SETADNSENS:
    break;
  case ENUM_ID_IMGPROC_GETADNSENS:
    break;
  case ENUM_ID_IMGPROC_SETFLIGHTMODE:
    break;
  case ENUM_ID_IMGPROC_GETFLIGHTMODE:
    break;
  case ENUM_ID_IMGPROC_SETMFLIGHT:
    break;
  case ENUM_ID_IMGPROC_GETMFLIGHT:
    break;
  case ENUM_ID_IMGPROC_SETWBMODE:
    break;
  case ENUM_ID_IMGPROC_GETWBMODE:
    break;
  case ENUM_ID_IMGPROC_LOCKAWB:
    break;
  case ENUM_ID_IMGPROC_UNLOCKAWB:
    break;
  case ENUM_ID_IMGPROC_SETAWBRANGE:
    break;
  case ENUM_ID_IMGPROC_GETAWBRANGE:
    break;
  case ENUM_ID_IMGPROC_SETMWBSCENE:
    break;
  case ENUM_ID_IMGPROC_GETMWBSCENE:
    break;
  case ENUM_ID_IMGPROC_SETMWBGAIN:
    break;
  case ENUM_ID_IMGPROC_GETMWBGAIN:
    break;
  case ENUM_ID_IMGPROC_SETMWBCT:
    break;
  case ENUM_ID_IMGPROC_GETMWBCT:
    break;
  case ENUM_ID_IMGPROC_SETCRSUPPSN:
    break;
  case ENUM_ID_IMGPROC_GETCRSUPPSN:
    break;
  case ENUM_ID_IMGPROC_SETFOCUSMODE:
    break;
  case ENUM_ID_IMGPROC_GETFOCUSMODE:
    break;
  case ENUM_ID_IMGPROC_SETFOCUSWIN:
    break;
  case ENUM_ID_IMGPROC_GETFOCUSWIN:
    break;
  case ENUM_ID_IMGPROC_SETFIXEDMODECODE:
    break;
  case ENUM_ID_IMGPROC_GETFIXEDMODECODE:
    break;
  case ENUM_ID_IMGPROC_SETMINFOCUSDIS:
    break;
  case ENUM_ID_IMGPROC_GETMINFOCUSDIS:
    break;
  case ENUM_ID_IMGPROC_SETOPZOOMRANGE:
    break;
  case ENUM_ID_IMGPROC_GETOPZOOMRANGE:
    break;
  case ENUM_ID_IMGPROC_SETOPZOOMSPEED:
    break;
  case ENUM_ID_IMGPROC_GETOPZOOMSPEED:
    break;
  case ENUM_ID_IMGPROC_SETHDRMODE:
    break;
  case ENUM_ID_IMGPROC_GETHDRMODE:
    break;
  case ENUM_ID_IMGPROC_SETMHDRSTRTH:
    break;
  case ENUM_ID_IMGPROC_GETMHDRSTRTH:
    break;
  case ENUM_ID_IMGPROC_SETNRMODE:
    break;
  case ENUM_ID_IMGPROC_GETNRMODE:
    break;
  case ENUM_ID_IMGPROC_SETANRSTRTH:
    break;
  case ENUM_ID_IMGPROC_GETANRSTRTH:
    break;
  case ENUM_ID_IMGPROC_SETMSPANRSTRTH:
    break;
  case ENUM_ID_IMGPROC_GETMSPANRSTRTH:
    break;
  case ENUM_ID_IMGPROC_SETMTNRSTRTH:
    break;
  case ENUM_ID_IMGPROC_GETMTNRSTRTH:
    break;
  case ENUM_ID_IMGPROC_SETDHZMODE:
    break;
  case ENUM_ID_IMGPROC_GETDHZMODE:
    break;
  case ENUM_ID_IMGPROC_SETMDHZSTRTH:
    break;
  case ENUM_ID_IMGPROC_GETMDHZSTRTH:
    break;
  case ENUM_ID_IMGPROC_SETCONTRAST:
    break;
  case ENUM_ID_IMGPROC_GETCONTRAST:
    break;
  case ENUM_ID_IMGPROC_SETBRIGHTNESS:
    break;
  case ENUM_ID_IMGPROC_GETBRIGHTNESS:
    break;
  case ENUM_ID_IMGPROC_SETSATURATION:
    break;
  case ENUM_ID_IMGPROC_GETSATURATION:
    break;
  case ENUM_ID_IMGPROC_SETSHARPNESS:
    break;
  case ENUM_ID_IMGPROC_GETSHARPNESS:
    break;
  case ENUM_ID_IMGPROC_SETGAMMACOEF:
    break;
  default:
    LOG_INFO("cmdID: Unknow\n");
    break;
  }
  return 0;
}

int RKAiqToolManager::AnrIoCtrl(int id, void *data, int size) {
  LOG_INFO("AnrIoCtrl id: 0x%x\n", id);
  switch (id) {
  case ENUM_ID_ANR_SETBAYERNRATTR: {
    rk_aiq_nr_IQPara_t param;
    param.module_bits = ANR_MODULE_BAYERNR;
    param.stBayernrPara = *(CalibDb_BayerNr_t *)data;
    anr_->SetIQPara(&param);
  } break;
  case ENUM_ID_ANR_SETMFNRATTR: {
    rk_aiq_nr_IQPara_t param;
    param.module_bits = ANR_MODULE_MFNR;
    param.stMfnrPara = *(CalibDb_MFNR_t *)data;
    anr_->SetIQPara(&param);
  } break;
  case ENUM_ID_ANR_SETUVNRATTR: {
    rk_aiq_nr_IQPara_t param;
    param.module_bits = ANR_MODULE_UVNR;
    param.stUvnrPara = *(CalibDb_UVNR_t *)data;
    anr_->SetIQPara(&param);
  } break;
  case ENUM_ID_ANR_SETYNRATTR: {
    rk_aiq_nr_IQPara_t param;
    param.module_bits = ANR_MODULE_YNR;
    param.stYnrPara = *(CalibDb_YNR_t *)data;
    anr_->SetIQPara(&param);
  } break;
  case ENUM_ID_ANR_GETBAYERNRATTR: {
    rk_aiq_nr_IQPara_t param;
    param.module_bits = ANR_MODULE_BAYERNR;
    anr_->GetIQPara(&param);
    memcpy(data, &param.stBayernrPara, sizeof(CalibDb_BayerNr_t));
  } break;
  case ENUM_ID_ANR_GETMFNRATTR: {
    rk_aiq_nr_IQPara_t param;
    param.module_bits = ANR_MODULE_MFNR;
    anr_->GetIQPara(&param);
    memcpy(data, &param.stMfnrPara, sizeof(CalibDb_MFNR_t));
  } break;
  case ENUM_ID_ANR_GETUVNRATTR: {
    rk_aiq_nr_IQPara_t param;
    param.module_bits = ANR_MODULE_UVNR;
    anr_->GetIQPara(&param);
    memcpy(data, &param.stUvnrPara, sizeof(CalibDb_UVNR_t));
  } break;
  case ENUM_ID_ANR_GETYNRATTR: {
    rk_aiq_nr_IQPara_t param;
    param.module_bits = ANR_MODULE_YNR;
    anr_->GetIQPara(&param);
    memcpy(data, &param.stYnrPara, sizeof(CalibDb_YNR_t));
  } break;
  case ENUM_ID_ANR_SETATTRIB:
    anr_->SetAttrib((rk_aiq_nr_attrib_t *)data);
    break;
  case ENUM_ID_ANR_GETATTRIB:
    anr_->GetAttrib((rk_aiq_nr_attrib_t *)data);
    break;
  case ENUM_ID_ANR_SETLUMASFSTRENGTH:
    anr_->SetLumaSFStrength(*(float *)data);
    break;
  case ENUM_ID_ANR_SETLUMATFSTRENGTH:
    anr_->SetLumaTFStrength(*(float *)data);
    break;
  case ENUM_ID_ANR_GETLUMASFSTRENGTH:
    anr_->GetLumaSFStrength((float *)data);
    break;
  case ENUM_ID_ANR_GETLUMATFSTRENGTH:
    anr_->GetLumaTFStrength((float *)data);
    break;
  case ENUM_ID_ANR_SETCHROMASFSTRENGTH:
    anr_->SetChromaSFStrength(*(float *)data);
    break;
  case ENUM_ID_ANR_SETCHROMATFSTRENGTH:
    anr_->SetChromaTFStrength(*(float *)data);
    break;
  case ENUM_ID_ANR_GETCHROMASFSTRENGTH:
    anr_->GetChromaSFStrength((float *)data);
    break;
  case ENUM_ID_ANR_GETCHROMATFSTRENGTH:
    anr_->GetChromaTFStrength((float *)data);
    break;
  case ENUM_ID_ANR_SETRAWNRSFSTRENGTH:
    anr_->SetRawnrSFStrength(*(float *)data);
    break;
  case ENUM_ID_ANR_GETRAWNRSFSTRENGTH:
    anr_->GetRawnrSFStrength((float *)data);
    break;
  default:
    LOG_INFO("cmdID: Unknow\n");
    break;
  }
  return 0;
}

int RKAiqToolManager::SharpIoCtrl(int id, void *data, int size) {
  LOG_INFO("SharpIoCtrl id: 0x%x\n", id);
  switch (id) {
  case ENUM_ID_SHARP_SET_ATTR:
    asharp_->SetAttrib((rk_aiq_sharp_attrib_t *)data);
    break;
  case ENUM_ID_SHARP_GET_ATTR:
    asharp_->GetAttrib((rk_aiq_sharp_attrib_t *)data);
    break;
  case ENUM_ID_SHARP_SET_IQPARA:
    asharp_->SetIQPara((rk_aiq_sharp_IQpara_t *)data);
    break;
  case ENUM_ID_SHARP_GET_IQPARA:
    asharp_->GetIQPara((rk_aiq_sharp_IQpara_t *)data);
    break;
  case ENUM_ID_SHARP_SET_STRENGTH:
    asharp_->SetStrength(*(float *)data);
    break;
  case ENUM_ID_SHARP_GET_STRENGTH:
    asharp_->GetStrength((float *)data);
    break;
  default:
    LOG_INFO("cmdID: Unknow\n");
    break;
  }
  return 0;
}