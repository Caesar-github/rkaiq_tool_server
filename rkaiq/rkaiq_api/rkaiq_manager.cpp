#include "rkaiq_manager.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "rkaiq_manager.cpp"

extern int g_mode;

RKAiqToolManager::RKAiqToolManager(std::string iqfiles_path,
                                   std::string sensor_name)
    : iqfiles_path_(iqfiles_path), sensor_name_(sensor_name), ctx_(nullptr) {
  engine_.reset(new RKAiqEngine(iqfiles_path_, sensor_name_));
  engine_->InitEngine(g_mode);
  engine_->StartEngine();
  ctx_ = engine_->GetContext();
  LOG_ERROR("ctx_ %p\n", ctx_);
  imgproc_.reset(new RKAiqToolImgProc(ctx_));
  ae_.reset(new RKAiqToolAE(ctx_));
  anr_.reset(new RKAiqToolANR(ctx_));
  asharp_.reset(new RKAiqToolSharp(ctx_));
  sysctl_.reset(new RKAiqToolSysCtl(ctx_));

  rk_aiq_ver_info_t vers;
  sysctl_->GetVersionInfo(&vers);
  LOG_ERROR("vers aiq_ver %s iq_parser_ver %s\n", vers.aiq_ver,
            vers.iq_parser_ver);
  if (strcmp(MATCH_RKAIQ_VERSION_1, vers.aiq_ver) &&
      strcmp(MATCH_RKAIQ_VERSION_2, vers.aiq_ver) &&
      strcmp(MATCH_RKAIQ_VERSION_3, vers.aiq_ver) &&
      strcmp(MATCH_RKAIQ_VERSION_4, vers.aiq_ver) &&
      strcmp(MATCH_RKAIQ_VERSION_5, vers.aiq_ver) &&
      strcmp(MATCH_RKAIQ_VERSION_6, vers.aiq_ver)) {
    LOG_ERROR("version: aiq_ver %s iq_parser_ver %s should be match \n"
              "   aiq_ver %s \n"
              "or aiq_ver %s \n"
              "or aiq_ver %s \n"
              "or aiq_ver %s \n"
              "or aiq_ver %s \n"
              "or aiq_ver %s \n",
              vers.aiq_ver, vers.iq_parser_ver, MATCH_RKAIQ_VERSION_1,
              MATCH_RKAIQ_VERSION_2, MATCH_RKAIQ_VERSION_3,
              MATCH_RKAIQ_VERSION_4,
              MATCH_RKAIQ_VERSION_5 MATCH_RKAIQ_VERSION_6);
    exit(-1);
  }
}

RKAiqToolManager::~RKAiqToolManager() {
  anr_.reset(nullptr);
  ae_.reset(nullptr);
  imgproc_.reset(nullptr);
  engine_.reset(nullptr);
}

void RKAiqToolManager::SaveExit() {}

int RKAiqToolManager::IoCtrl(int id, void *data, int size) {
  LOG_INFO("IoCtrl id: 0x%x\n", id);
  if (id > ENUM_ID_AE_START && id < ENUM_ID_AE_END) {
    AeIoCtrl(id, data, size);
  } else if (id > ENUM_ID_IMGPROC_START && id < ENUM_ID_IMGPROC_END) {
    ImgProcIoCtrl(id, data, size);
  } else if (id > ENUM_ID_ANR_START && id < ENUM_ID_ANR_END) {
    AnrIoCtrl(id, data, size);
  } else if (id > ENUM_ID_SHARP_START && id < ENUM_ID_SHARP_END) {
    SharpIoCtrl(id, data, size);
  } else if (id > ENUM_ID_SYSCTL_START && id < ENUM_ID_SYSCTL_END) {
    SysCtlIoCtrl(id, data, size);
  }
  LOG_INFO("IoCtrl id: 0x%x exit\n", id);
  return 0;
}

int RKAiqToolManager::AeIoCtrl(int id, void *data, int size) {
  LOG_INFO("AeIoCtrl id: 0x%x\n", id);
  switch (id) {
  case ENUM_ID_AE_SETEXPSWATTR:
    CHECK_PARAM_SIZE(Uapi_ExpSwAttr_t, size);
    ae_->setExpSwAttr(*(Uapi_ExpSwAttr_t *)data);
    break;
  case ENUM_ID_AE_GETEXPSWATTR:
    CHECK_PARAM_SIZE(Uapi_ExpSwAttr_t, size);
    ae_->getExpSwAttr((Uapi_ExpSwAttr_t *)data);
    break;
  case ENUM_ID_AE_SETLINAEDAYROUTEATTR:
    CHECK_PARAM_SIZE(Uapi_LinAeRouteAttr_t, size);
    ae_->setLinAeDayRouteAttr(*(Uapi_LinAeRouteAttr_t *)data);
    break;
  case ENUM_ID_AE_GETLINAEDAYROUTEATTR:
    CHECK_PARAM_SIZE(Uapi_LinAeRouteAttr_t, size);
    ae_->getLinAeDayRouteAttr((Uapi_LinAeRouteAttr_t *)data);
    break;
  case ENUM_ID_AE_SETLINAENIGHTROUTEATTR:
    CHECK_PARAM_SIZE(Uapi_LinAeRouteAttr_t, size);
    ae_->setLinAeNightRouteAttr(*(Uapi_LinAeRouteAttr_t *)data);
    break;
  case ENUM_ID_AE_GETLINAENIGHTROUTEATTR:
    CHECK_PARAM_SIZE(Uapi_LinAeRouteAttr_t, size);
    ae_->getLinAeNightRouteAttr((Uapi_LinAeRouteAttr_t *)data);
    break;
  case ENUM_ID_AE_SETHDRAEDAYROUTEATTR:
    CHECK_PARAM_SIZE(Uapi_HdrAeRouteAttr_t, size);
    ae_->setHdrAeDayRouteAttr(*(Uapi_HdrAeRouteAttr_t *)data);
    break;
  case ENUM_ID_AE_GETHDRAEDAYROUTEATTR:
    CHECK_PARAM_SIZE(Uapi_HdrAeRouteAttr_t, size);
    ae_->getHdrAeDayRouteAttr((Uapi_HdrAeRouteAttr_t *)data);
    break;
  case ENUM_ID_AE_SETHDRAENIGHTROUTEATTR:
    CHECK_PARAM_SIZE(Uapi_HdrAeRouteAttr_t, size);
    ae_->setHdrAeNightRouteAttr(*(Uapi_HdrAeRouteAttr_t *)data);
    break;
  case ENUM_ID_AE_GETHDRAENIGHTROUTEATTR:
    CHECK_PARAM_SIZE(Uapi_HdrAeRouteAttr_t, size);
    ae_->getHdrAeNightRouteAttr((Uapi_HdrAeRouteAttr_t *)data);
    break;
  case ENUM_ID_AE_QUERYEXPRESINFO:
    CHECK_PARAM_SIZE(Uapi_ExpQueryInfo_t, size);
    ae_->queryExpResInfo((Uapi_ExpQueryInfo_t *)data);
    break;
  case ENUM_ID_AE_SETLINEXPATTR:
    CHECK_PARAM_SIZE(Uapi_LinExpAttr_t, size);
    ae_->setLinExpAttr(*(Uapi_LinExpAttr_t *)data);
    break;
  case ENUM_ID_AE_GETLINEXPATTR:
    CHECK_PARAM_SIZE(Uapi_LinExpAttr_t, size);
    ae_->getLinExpAttr((Uapi_LinExpAttr_t *)data);
    break;
  case ENUM_ID_AE_SETHDREXPATTR:
    CHECK_PARAM_SIZE(Uapi_HdrExpAttr_t, size);
    ae_->setHdrExpAttr(*(Uapi_HdrExpAttr_t *)data);
    break;
  case ENUM_ID_AE_GETHDREXPATTR:
    CHECK_PARAM_SIZE(Uapi_HdrExpAttr_t, size);
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
    param.module_bits = 1 << ANR_MODULE_BAYERNR;
    param.stBayernrPara = *(CalibDb_BayerNr_t *)data;
    CHECK_PARAM_SIZE(CalibDb_BayerNr_t, size);
    anr_->SetIQPara(&param);
  } break;
  case ENUM_ID_ANR_SETMFNRATTR: {
    rk_aiq_nr_IQPara_t param;
    param.module_bits = 1 << ANR_MODULE_MFNR;
    param.stMfnrPara = *(CalibDb_MFNR_t *)data;
    CHECK_PARAM_SIZE(CalibDb_MFNR_t, size);
    anr_->SetIQPara(&param);
  } break;
  case ENUM_ID_ANR_SETUVNRATTR: {
    rk_aiq_nr_IQPara_t param;
    param.module_bits = 1 << ANR_MODULE_UVNR;
    param.stUvnrPara = *(CalibDb_UVNR_t *)data;
    CHECK_PARAM_SIZE(CalibDb_UVNR_t, size);
    anr_->SetIQPara(&param);
  } break;
  case ENUM_ID_ANR_SETYNRATTR: {
    rk_aiq_nr_IQPara_t param;
    param.module_bits = 1 << ANR_MODULE_YNR;
    param.stYnrPara = *(CalibDb_YNR_t *)data;
    CHECK_PARAM_SIZE(CalibDb_YNR_t, size);
    anr_->SetIQPara(&param);
  } break;
  case ENUM_ID_ANR_GETBAYERNRATTR: {
    rk_aiq_nr_IQPara_t param;
    param.module_bits = 1 << ANR_MODULE_BAYERNR;
    CHECK_PARAM_SIZE(CalibDb_BayerNr_t, size);
    anr_->GetIQPara(&param);
    memcpy(data, &param.stBayernrPara, sizeof(CalibDb_BayerNr_t));
  } break;
  case ENUM_ID_ANR_GETMFNRATTR: {
    rk_aiq_nr_IQPara_t param;
    param.module_bits = 1 << ANR_MODULE_MFNR;
    CHECK_PARAM_SIZE(CalibDb_MFNR_t, size);
    anr_->GetIQPara(&param);
    memcpy(data, &param.stMfnrPara, sizeof(CalibDb_MFNR_t));
  } break;
  case ENUM_ID_ANR_GETUVNRATTR: {
    rk_aiq_nr_IQPara_t param;
    param.module_bits = 1 << ANR_MODULE_UVNR;
    CHECK_PARAM_SIZE(CalibDb_UVNR_t, size);
    anr_->GetIQPara(&param);
    memcpy(data, &param.stUvnrPara, sizeof(CalibDb_UVNR_t));
  } break;
  case ENUM_ID_ANR_GETYNRATTR: {
    rk_aiq_nr_IQPara_t param;
    param.module_bits = 1 << ANR_MODULE_YNR;
    CHECK_PARAM_SIZE(CalibDb_YNR_t, size);
    anr_->GetIQPara(&param);
    memcpy(data, &param.stYnrPara, sizeof(CalibDb_YNR_t));
  } break;
  case ENUM_ID_ANR_SETATTRIB:
    CHECK_PARAM_SIZE(rk_aiq_nr_attrib_t, size);
    anr_->SetAttrib((rk_aiq_nr_attrib_t *)data);
    break;
  case ENUM_ID_ANR_GETATTRIB:
    CHECK_PARAM_SIZE(rk_aiq_nr_attrib_t, size);
    anr_->GetAttrib((rk_aiq_nr_attrib_t *)data);
    break;
  case ENUM_ID_ANR_SETLUMASFSTRENGTH:
    CHECK_PARAM_SIZE(float, size);
    anr_->SetLumaSFStrength(*(float *)data);
    break;
  case ENUM_ID_ANR_SETLUMATFSTRENGTH:
    CHECK_PARAM_SIZE(float, size);
    anr_->SetLumaTFStrength(*(float *)data);
    break;
  case ENUM_ID_ANR_GETLUMASFSTRENGTH:
    CHECK_PARAM_SIZE(float, size);
    anr_->GetLumaSFStrength((float *)data);
    break;
  case ENUM_ID_ANR_GETLUMATFSTRENGTH:
    CHECK_PARAM_SIZE(float, size);
    anr_->GetLumaTFStrength((float *)data);
    break;
  case ENUM_ID_ANR_SETCHROMASFSTRENGTH:
    CHECK_PARAM_SIZE(float, size);
    anr_->SetChromaSFStrength(*(float *)data);
    break;
  case ENUM_ID_ANR_SETCHROMATFSTRENGTH:
    CHECK_PARAM_SIZE(float, size);
    anr_->SetChromaTFStrength(*(float *)data);
    break;
  case ENUM_ID_ANR_GETCHROMASFSTRENGTH:
    CHECK_PARAM_SIZE(float, size);
    anr_->GetChromaSFStrength((float *)data);
    break;
  case ENUM_ID_ANR_GETCHROMATFSTRENGTH:
    CHECK_PARAM_SIZE(float, size);
    anr_->GetChromaTFStrength((float *)data);
    break;
  case ENUM_ID_ANR_SETRAWNRSFSTRENGTH:
    CHECK_PARAM_SIZE(float, size);
    anr_->SetRawnrSFStrength(*(float *)data);
    break;
  case ENUM_ID_ANR_GETRAWNRSFSTRENGTH:
    CHECK_PARAM_SIZE(float, size);
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
    CHECK_PARAM_SIZE(rk_aiq_sharp_attrib_t, size);
    asharp_->SetAttrib((rk_aiq_sharp_attrib_t *)data);
    break;
  case ENUM_ID_SHARP_GET_ATTR:
    CHECK_PARAM_SIZE(rk_aiq_sharp_attrib_t, size);
    asharp_->GetAttrib((rk_aiq_sharp_attrib_t *)data);
    break;
  case ENUM_ID_SHARP_SET_IQPARA: {
    rk_aiq_sharp_IQpara_t param;
    param.module_bits = 1 << ASHARP_MODULE_SHARP;
    param.stSharpPara = *(CalibDb_Sharp_t *)data;
    CHECK_PARAM_SIZE(CalibDb_Sharp_t, size);
    asharp_->SetIQPara(&param);
  } break;
  case ENUM_ID_SHARP_GET_IQPARA: {
    rk_aiq_sharp_IQpara_t param;
    param.module_bits = 1 << ASHARP_MODULE_SHARP;
    CHECK_PARAM_SIZE(CalibDb_Sharp_t, size);
    asharp_->GetIQPara(&param);
    memcpy(data, &param.stSharpPara, sizeof(CalibDb_Sharp_t));
  } break;
  case ENUM_ID_SHARP_SET_STRENGTH:
    CHECK_PARAM_SIZE(float, size);
    asharp_->SetStrength(*(float *)data);
    break;
  case ENUM_ID_SHARP_GET_STRENGTH:
    CHECK_PARAM_SIZE(float, size);
    asharp_->GetStrength((float *)data);
    break;
  default:
    LOG_INFO("cmdID: Unknow\n");
    break;
  }
  return 0;
}

int RKAiqToolManager::SysCtlIoCtrl(int id, void *data, int size) {
  LOG_INFO("SysCtlIoCtrl id: 0x%x\n", id);
  switch (id) {
  case ENUM_ID_SYSCTL_SETCPSLTCFG:
    sysctl_->SetCpsLtCfg((rk_aiq_cpsl_cfg_t *)data);
    break;
  case ENUM_ID_SYSCTL_GETCPSLTINFO:
    sysctl_->GetCpsLtInfo((rk_aiq_cpsl_info_t *)data);
    break;
  case ENUM_ID_SYSCTL_QUERYCPSLTCAP:
    sysctl_->QueryCpsLtCap((rk_aiq_cpsl_cap_t *)data);
    break;
  default:
    LOG_INFO("cmdID: Unknow\n");
    break;
  }
  return 0;
}
