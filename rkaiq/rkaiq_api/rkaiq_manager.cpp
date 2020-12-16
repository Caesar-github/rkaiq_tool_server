#include "rkaiq_manager.h"
#include "media_config.h"
#include "rtsp_server.h"
#include "image.h"
#include "rkaiq_media.h"

#ifdef LOG_TAG
    #undef LOG_TAG
#endif
#define LOG_TAG "rkaiq_manager.cpp"

extern int g_device_id;
extern int g_mode;
extern int g_width;
extern int g_height;
extern int g_maxResolutionWidth;
extern int g_maxResolutionHeight;
extern int g_rtsp_en;
extern std::shared_ptr<easymedia::Flow> g_video_rtsp_flow;
extern std::shared_ptr<easymedia::Flow> g_video_enc_flow;
extern std::shared_ptr<RKAiqMedia> rkaiq_media;
extern std::shared_ptr<RKAiqToolManager> rkaiq_manager;
extern std::string iqfile;
extern std::string g_sensor_name;

static char m_encoder_name[20] = "H264";
static int m_resolution_width = 1920;
static int m_resolution_height = 1080;
static int m_bps_target = 16384;
static char m_bps_mode[20] = "Dynamic_BPS"; //FIX_BPS
static int m_gop = 30;
static int m_qp_init = 28;
static int m_qp_step = 2;
static int m_qp_min = 8;
static int m_qp_max = 48;
static int m_qp_min_i = 8;
static int m_qp_max_i = 48;

#pragma pack(1)
typedef struct RkMedia_User_Params_s {
    char encoder_name[20];
    int resolution_width;
    int resolution_height;
    int bps_target;
    char bps_mode[20];
    int gop;
    int qp_init;
    int qp_step;
    int qp_min;
    int qp_max;
    int qp_min_i;
    int qp_max_i;
} RkMedia_User_Params_t;
#pragma pack()

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
    ahdr_.reset(new RKAiqToolAHDR(ctx_));
    dpcc_.reset(new RKAiqToolADPCC(ctx_));
    gamma_.reset(new RKAiqToolAGamma(ctx_));
    dehaze_.reset(new RKAiqToolDehaze(ctx_));
    ccm_.reset(new RKAiqToolCCM(ctx_));
    awb_.reset(new RKAiqToolAWB(ctx_));
    cproc_.reset(new RKAiqToolCPROC(ctx_));

    rk_aiq_ver_info_t vers;
    sysctl_->GetVersionInfo(&vers);
    LOG_ERROR("vers aiq_ver %s iq_parser_ver %s\n", vers.aiq_ver,
              vers.iq_parser_ver);
}

RKAiqToolManager::~RKAiqToolManager() {
    ae_.reset(nullptr);
    anr_.reset(nullptr);
    asharp_.reset(nullptr);
    sysctl_.reset(nullptr);
    ahdr_.reset(nullptr);
    dpcc_.reset(nullptr);
    gamma_.reset(nullptr);
    dehaze_.reset(nullptr);
    ccm_.reset(nullptr);
    awb_.reset(nullptr);
    cproc_.reset(nullptr);
    imgproc_.reset(nullptr);
    imgproc_.reset(nullptr);
    engine_.reset(nullptr);
}

void RKAiqToolManager::SaveExit() {}

int RKAiqToolManager::IoCtrl(int id, void* data, int size) {
    LOG_INFO("IoCtrl id: 0x%x\n", id);
    if(id > ENUM_ID_AE_START && id < ENUM_ID_AE_END) {
        AeIoCtrl(id, data, size);
    } else if(id > ENUM_ID_IMGPROC_START && id < ENUM_ID_IMGPROC_END) {
        ImgProcIoCtrl(id, data, size);
    } else if(id > ENUM_ID_ANR_START && id < ENUM_ID_ANR_END) {
        AnrIoCtrl(id, data, size);
    } else if(id > ENUM_ID_SHARP_START && id < ENUM_ID_SHARP_END) {
        SharpIoCtrl(id, data, size);
    } else if(id > ENUM_ID_SYSCTL_START && id < ENUM_ID_SYSCTL_END) {
        SysCtlIoCtrl(id, data, size);
    } else if(id > ENUM_ID_AHDR_START && id < ENUM_ID_AHDR_END) {
        AHDRIoCtrl(id, data, size);
    } else if(id > ENUM_ID_AGAMMA_START && id < ENUM_ID_AGAMMA_END) {
        AGamamIoCtrl(id, data, size);
    } else if(id > ENUM_ID_ADPCC_START && id < ENUM_ID_ADPCC_END) {
        ADPCCIoCtrl(id, data, size);
    } else if(id > ENUM_ID_DEHAZE_START && id < ENUM_ID_DEHAZE_END) {
        DEHAZEIoCtrl(id, data, size);
    } else if(id > ENUM_ID_RKMEDIA_START && id < ENUM_ID_RKMEDIA_END) {
        RkMediaCtrl(id, data, size);
    } else if(id > ENUM_ID_ACCM_START && id < ENUM_ID_ACCM_END) {
        CCMIoCtrl(id, data, size);
    } else if(id > ENUM_ID_AWB_START && id < ENUM_ID_AWB_END) {
        AWBIoCtrl(id, data, size);
    } else if(id > ENUM_ID_CPROC_START && id < ENUM_ID_CPROC_END) {
        CPROCIoCtrl(id, data, size);
    }
    LOG_INFO("IoCtrl id: 0x%x exit\n", id);
    return 0;
}

int RKAiqToolManager::AeIoCtrl(int id, void* data, int size) {
    LOG_INFO("AeIoCtrl id: 0x%x\n", id);
    switch(id) {
        case ENUM_ID_AE_SETEXPSWATTR:
            CHECK_PARAM_SIZE(Uapi_ExpSwAttr_t, size);
            ae_->setExpSwAttr(*(Uapi_ExpSwAttr_t*)data);
            break;
        case ENUM_ID_AE_GETEXPSWATTR:
            CHECK_PARAM_SIZE(Uapi_ExpSwAttr_t, size);
            ae_->getExpSwAttr((Uapi_ExpSwAttr_t*)data);
            break;
        case ENUM_ID_AE_SETLINAEDAYROUTEATTR:
            CHECK_PARAM_SIZE(Uapi_LinAeRouteAttr_t, size);
            ae_->setLinAeDayRouteAttr(*(Uapi_LinAeRouteAttr_t*)data);
            break;
        case ENUM_ID_AE_GETLINAEDAYROUTEATTR:
            CHECK_PARAM_SIZE(Uapi_LinAeRouteAttr_t, size);
            ae_->getLinAeDayRouteAttr((Uapi_LinAeRouteAttr_t*)data);
            break;
        case ENUM_ID_AE_SETLINAENIGHTROUTEATTR:
            CHECK_PARAM_SIZE(Uapi_LinAeRouteAttr_t, size);
            ae_->setLinAeNightRouteAttr(*(Uapi_LinAeRouteAttr_t*)data);
            break;
        case ENUM_ID_AE_GETLINAENIGHTROUTEATTR:
            CHECK_PARAM_SIZE(Uapi_LinAeRouteAttr_t, size);
            ae_->getLinAeNightRouteAttr((Uapi_LinAeRouteAttr_t*)data);
            break;
        case ENUM_ID_AE_SETHDRAEDAYROUTEATTR:
            CHECK_PARAM_SIZE(Uapi_HdrAeRouteAttr_t, size);
            ae_->setHdrAeDayRouteAttr(*(Uapi_HdrAeRouteAttr_t*)data);
            break;
        case ENUM_ID_AE_GETHDRAEDAYROUTEATTR:
            CHECK_PARAM_SIZE(Uapi_HdrAeRouteAttr_t, size);
            ae_->getHdrAeDayRouteAttr((Uapi_HdrAeRouteAttr_t*)data);
            break;
        case ENUM_ID_AE_SETHDRAENIGHTROUTEATTR:
            CHECK_PARAM_SIZE(Uapi_HdrAeRouteAttr_t, size);
            ae_->setHdrAeNightRouteAttr(*(Uapi_HdrAeRouteAttr_t*)data);
            break;
        case ENUM_ID_AE_GETHDRAENIGHTROUTEATTR:
            CHECK_PARAM_SIZE(Uapi_HdrAeRouteAttr_t, size);
            ae_->getHdrAeNightRouteAttr((Uapi_HdrAeRouteAttr_t*)data);
            break;
        case ENUM_ID_AE_QUERYEXPRESINFO:
            CHECK_PARAM_SIZE(Uapi_ExpQueryInfo_t, size);
            ae_->queryExpResInfo((Uapi_ExpQueryInfo_t*)data);
            break;
        case ENUM_ID_AE_SETLINEXPATTR:
            CHECK_PARAM_SIZE(Uapi_LinExpAttr_t, size);
            ae_->setLinExpAttr(*(Uapi_LinExpAttr_t*)data);
            break;
        case ENUM_ID_AE_GETLINEXPATTR:
            CHECK_PARAM_SIZE(Uapi_LinExpAttr_t, size);
            ae_->getLinExpAttr((Uapi_LinExpAttr_t*)data);
            break;
        case ENUM_ID_AE_SETHDREXPATTR:
            CHECK_PARAM_SIZE(Uapi_HdrExpAttr_t, size);
            ae_->setHdrExpAttr(*(Uapi_HdrExpAttr_t*)data);
            break;
        case ENUM_ID_AE_GETHDREXPATTR:
            CHECK_PARAM_SIZE(Uapi_HdrExpAttr_t, size);
            ae_->getHdrExpAttr((Uapi_HdrExpAttr_t*)data);
            break;
        default:
            LOG_INFO("cmdID: Unknow\n");
            break;
    }
    return 0;
}

int RKAiqToolManager::ImgProcIoCtrl(int id, void* data, int size) {
    LOG_INFO("ImgProcIoCtrl id: 0x%x\n", id);
    switch(id) {
        case ENUM_ID_IMGPROC_SETGRAYMODE:
            CHECK_PARAM_SIZE(rk_aiq_gray_mode_t, size);
            imgproc_->SetGrayMode(*(rk_aiq_gray_mode_t*)data);
            break;
        case ENUM_ID_IMGPROC_GETGRAYMODE:
            CHECK_PARAM_SIZE(rk_aiq_gray_mode_t, size);
            *(rk_aiq_gray_mode_t*)data = (rk_aiq_gray_mode_t)imgproc_->GetGrayMode();
            break;
        default:
            LOG_INFO("cmdID: Unknow\n");
            break;
    }
    return 0;
}

int RKAiqToolManager::AnrIoCtrl(int id, void* data, int size) {
    LOG_INFO("AnrIoCtrl id: 0x%x\n", id);
    switch(id) {
        case ENUM_ID_ANR_SETBAYERNRATTR: {
            rk_aiq_nr_IQPara_t param;
            param.module_bits = 1 << ANR_MODULE_BAYERNR;
            param.stBayernrPara = *(CalibDb_BayerNr_t*)data;
            CHECK_PARAM_SIZE(CalibDb_BayerNr_t, size);
            anr_->SetIQPara(&param);
        }
        break;
        case ENUM_ID_ANR_SETMFNRATTR: {
            rk_aiq_nr_IQPara_t param;
            param.module_bits = 1 << ANR_MODULE_MFNR;
            param.stMfnrPara = *(CalibDb_MFNR_t*)data;
            CHECK_PARAM_SIZE(CalibDb_MFNR_t, size);
            anr_->SetIQPara(&param);
        }
        break;
        case ENUM_ID_ANR_SETUVNRATTR: {
            rk_aiq_nr_IQPara_t param;
            param.module_bits = 1 << ANR_MODULE_UVNR;
            param.stUvnrPara = *(CalibDb_UVNR_t*)data;
            CHECK_PARAM_SIZE(CalibDb_UVNR_t, size);
            anr_->SetIQPara(&param);
        }
        break;
        case ENUM_ID_ANR_SETYNRATTR: {
            rk_aiq_nr_IQPara_t param;
            param.module_bits = 1 << ANR_MODULE_YNR;
            param.stYnrPara = *(CalibDb_YNR_t*)data;
            CHECK_PARAM_SIZE(CalibDb_YNR_t, size);
            anr_->SetIQPara(&param);
        }
        break;
        case ENUM_ID_ANR_GETBAYERNRATTR: {
            rk_aiq_nr_IQPara_t param;
            param.module_bits = 1 << ANR_MODULE_BAYERNR;
            CHECK_PARAM_SIZE(CalibDb_BayerNr_t, size);
            anr_->GetIQPara(&param);
            memcpy(data, &param.stBayernrPara, sizeof(CalibDb_BayerNr_t));
        }
        break;
        case ENUM_ID_ANR_GETMFNRATTR: {
            rk_aiq_nr_IQPara_t param;
            param.module_bits = 1 << ANR_MODULE_MFNR;
            CHECK_PARAM_SIZE(CalibDb_MFNR_t, size);
            anr_->GetIQPara(&param);
            memcpy(data, &param.stMfnrPara, sizeof(CalibDb_MFNR_t));
        }
        break;
        case ENUM_ID_ANR_GETUVNRATTR: {
            rk_aiq_nr_IQPara_t param;
            param.module_bits = 1 << ANR_MODULE_UVNR;
            CHECK_PARAM_SIZE(CalibDb_UVNR_t, size);
            anr_->GetIQPara(&param);
            memcpy(data, &param.stUvnrPara, sizeof(CalibDb_UVNR_t));
        }
        break;
        case ENUM_ID_ANR_GETYNRATTR: {
            rk_aiq_nr_IQPara_t param;
            param.module_bits = 1 << ANR_MODULE_YNR;
            CHECK_PARAM_SIZE(CalibDb_YNR_t, size);
            anr_->GetIQPara(&param);
            memcpy(data, &param.stYnrPara, sizeof(CalibDb_YNR_t));
        }
        break;
        case ENUM_ID_ANR_SETATTRIB:
            CHECK_PARAM_SIZE(rk_aiq_nr_attrib_t, size);
            anr_->SetAttrib((rk_aiq_nr_attrib_t*)data);
            break;
        case ENUM_ID_ANR_GETATTRIB:
            CHECK_PARAM_SIZE(rk_aiq_nr_attrib_t, size);
            anr_->GetAttrib((rk_aiq_nr_attrib_t*)data);
            break;
        case ENUM_ID_ANR_SETLUMASFSTRENGTH:
            CHECK_PARAM_SIZE(float, size);
            anr_->SetLumaSFStrength(*(float*)data);
            break;
        case ENUM_ID_ANR_SETLUMATFSTRENGTH:
            CHECK_PARAM_SIZE(float, size);
            anr_->SetLumaTFStrength(*(float*)data);
            break;
        case ENUM_ID_ANR_GETLUMASFSTRENGTH:
            CHECK_PARAM_SIZE(float, size);
            anr_->GetLumaSFStrength((float*)data);
            break;
        case ENUM_ID_ANR_GETLUMATFSTRENGTH:
            CHECK_PARAM_SIZE(float, size);
            anr_->GetLumaTFStrength((float*)data);
            break;
        case ENUM_ID_ANR_SETCHROMASFSTRENGTH:
            CHECK_PARAM_SIZE(float, size);
            anr_->SetChromaSFStrength(*(float*)data);
            break;
        case ENUM_ID_ANR_SETCHROMATFSTRENGTH:
            CHECK_PARAM_SIZE(float, size);
            anr_->SetChromaTFStrength(*(float*)data);
            break;
        case ENUM_ID_ANR_GETCHROMASFSTRENGTH:
            CHECK_PARAM_SIZE(float, size);
            anr_->GetChromaSFStrength((float*)data);
            break;
        case ENUM_ID_ANR_GETCHROMATFSTRENGTH:
            CHECK_PARAM_SIZE(float, size);
            anr_->GetChromaTFStrength((float*)data);
            break;
        case ENUM_ID_ANR_SETRAWNRSFSTRENGTH:
            CHECK_PARAM_SIZE(float, size);
            anr_->SetRawnrSFStrength(*(float*)data);
            break;
        case ENUM_ID_ANR_GETRAWNRSFSTRENGTH:
            CHECK_PARAM_SIZE(float, size);
            anr_->GetRawnrSFStrength((float*)data);
            break;
        default:
            LOG_INFO("cmdID: Unknow\n");
            break;
    }
    return 0;
}

int RKAiqToolManager::SharpIoCtrl(int id, void* data, int size) {
    LOG_INFO("SharpIoCtrl id: 0x%x\n", id);
    switch(id) {
        case ENUM_ID_SHARP_SET_ATTR:
            CHECK_PARAM_SIZE(rk_aiq_sharp_attrib_t, size);
            asharp_->SetAttrib((rk_aiq_sharp_attrib_t*)data);
            break;
        case ENUM_ID_SHARP_GET_ATTR:
            CHECK_PARAM_SIZE(rk_aiq_sharp_attrib_t, size);
            asharp_->GetAttrib((rk_aiq_sharp_attrib_t*)data);
            break;
        case ENUM_ID_SHARP_SET_IQPARA: {
            rk_aiq_sharp_IQpara_t param;
            param.module_bits = 1 << ASHARP_MODULE_SHARP;
            param.stSharpPara = *(CalibDb_Sharp_t*)data;
            CHECK_PARAM_SIZE(CalibDb_Sharp_t, size);
            asharp_->SetIQPara(&param);
        }
        break;
        case ENUM_ID_SHARP_GET_IQPARA: {
            rk_aiq_sharp_IQpara_t param;
            param.module_bits = 1 << ASHARP_MODULE_SHARP;
            CHECK_PARAM_SIZE(CalibDb_Sharp_t, size);
            asharp_->GetIQPara(&param);
            memcpy(data, &param.stSharpPara, sizeof(CalibDb_Sharp_t));
        }
        break;
        case ENUM_ID_SHARP_SET_EF_IQPARA: {
            rk_aiq_sharp_IQpara_t param;
            param.module_bits = 1 << ASHARP_MODULE_EDGEFILTER;
            param.stEdgeFltPara = *(CalibDb_EdgeFilter_t*)data;
            CHECK_PARAM_SIZE(CalibDb_EdgeFilter_t, size);
            asharp_->SetIQPara(&param);
        }
        break;
        case ENUM_ID_SHARP_GET_EF_IQPARA: {
            rk_aiq_sharp_IQpara_t param;
            param.module_bits = 1 << ASHARP_MODULE_EDGEFILTER;
            CHECK_PARAM_SIZE(CalibDb_EdgeFilter_t, size);
            asharp_->GetIQPara(&param);
            memcpy(data, &param.stEdgeFltPara, sizeof(CalibDb_EdgeFilter_t));

        }
        break;
        case ENUM_ID_SHARP_SET_STRENGTH:
            CHECK_PARAM_SIZE(float, size);
            asharp_->SetStrength(*(float*)data);
            break;
        case ENUM_ID_SHARP_GET_STRENGTH:
            CHECK_PARAM_SIZE(float, size);
            asharp_->GetStrength((float*)data);
            break;
        default:
            LOG_INFO("cmdID: Unknow\n");
            break;
    }
    return 0;
}

int RKAiqToolManager::SysCtlIoCtrl(int id, void* data, int size) {
    LOG_INFO("SysCtlIoCtrl id: 0x%x\n", id);
    switch(id) {
        case ENUM_ID_SYSCTL_SETCPSLTCFG:
            sysctl_->SetCpsLtCfg((rk_aiq_cpsl_cfg_t*)data);
            break;
        case ENUM_ID_SYSCTL_GETCPSLTINFO:
            sysctl_->GetCpsLtInfo((rk_aiq_cpsl_info_t*)data);
            break;
        case ENUM_ID_SYSCTL_QUERYCPSLTCAP:
            sysctl_->QueryCpsLtCap((rk_aiq_cpsl_cap_t*)data);
            break;
        case ENUM_ID_SYSCTL_SETWORKINGMODE:
            sysctl_->SetWorkingModeDyn(*(rk_aiq_working_mode_t*)data);
            break;
        default:
            LOG_INFO("cmdID: Unknow\n");
            break;
    }
    return 0;
}

int RKAiqToolManager::AHDRIoCtrl(int id, void* data, int size) {
    LOG_INFO("AHDRIoCtrl id: 0x%x\n", id);
    switch(id) {
        case ENUM_ID_AHDR_SETATTRIB: {
            CHECK_PARAM_SIZE(ahdr_attrib_t, size);
            ahdr_->SetAttrib(*(ahdr_attrib_t*)data);
        }
        break;
        case ENUM_ID_AHDR_GETATTRIB: {
            CHECK_PARAM_SIZE(ahdr_attrib_t, size);
            ahdr_->GetAttrib((ahdr_attrib_t*)data);
        }
        break;
        default:
            LOG_INFO("cmdID: Unknow\n");
            break;
    }
    return 0;
}

int RKAiqToolManager::AGamamIoCtrl(int id, void* data, int size) {
    LOG_INFO("AGamamIoCtrl id: 0x%x\n", id);
    switch(id) {
        case ENUM_ID_AGAMMA_SETATTRIB: {
            CHECK_PARAM_SIZE(rk_aiq_gamma_attrib_t, size);
            gamma_->SetAttrib(*(rk_aiq_gamma_attrib_t*)data);
        }
        break;
        case ENUM_ID_AGAMMA_GETATTRIB: {
            CHECK_PARAM_SIZE(rk_aiq_gamma_attrib_t, size);
            gamma_->GetAttrib((rk_aiq_gamma_attrib_t*)data);
        }
        break;
        default:
            LOG_INFO("cmdID: Unknow\n");
            break;
    }
    return 0;
}

int RKAiqToolManager::ADPCCIoCtrl(int id, void* data, int size) {
    LOG_INFO("ADPCCIoCtrl id: 0x%x\n", id);
    switch(id) {
        case ENUM_ID_ADPCC_SETATTRIB: {
            CHECK_PARAM_SIZE(rk_aiq_dpcc_attrib_t, size);
            dpcc_->SetAttrib((rk_aiq_dpcc_attrib_t*)data);
        }
        break;
        case ENUM_ID_ADPCC_GETATTRIB: {
            CHECK_PARAM_SIZE(rk_aiq_dpcc_attrib_t, size);
            dpcc_->GetAttrib((rk_aiq_dpcc_attrib_t*)data);
        }
        break;
        default:
            LOG_INFO("cmdID: Unknow\n");
            break;
    }
    return 0;
}

int RKAiqToolManager::DEHAZEIoCtrl(int id, void* data, int size) {
    LOG_INFO("DEHAZEIoCtrl id: 0x%x\n", id);
    switch(id) {
        case ENUM_ID_DEHAZE_SETATTRIB: {
            CHECK_PARAM_SIZE(adehaze_sw_t, size);
            dehaze_->SetAttrib(*(adehaze_sw_t*)data);
        }
        break;
        case ENUM_ID_DEHAZE_GETATTRIB: {
            CHECK_PARAM_SIZE(adehaze_sw_t, size);
            dehaze_->GetAttrib((adehaze_sw_t*)data);
        }
        break;
        default:
            LOG_INFO("cmdID: Unknow\n");
            break;
    }
    return 0;
}

int RKAiqToolManager::RkMediaCtrl(int id, void* data, int size) {
    LOG_INFO("RkMediaCtrl id: 0x%x\n", id);
    RkMedia_User_Params_s* param = (RkMedia_User_Params_s*) data;
    bool needResetRtspFlag = false;
    std::string encoderName;
    static std::string lastEncoderName = "H264";
    int resolutionWidth;
    int resolutionHeight;
    static int lastResolutionWidth = 1920;
    static int lastResolutionHeight = 1080;

    char mode[20] = {};
    int gop;
    int bpsTarget;
    media_info_t mi = rkaiq_media->GetMediaInfoT(g_device_id);
    std::string encoderConfigString;
    ImageInfo imgInfo;
    VideoEncoderCfg videoCfg;
    switch(id) {
        case ENUM_ID_RKMEDIA_SET_PARAMS:
            LOG_INFO("#### ENUM_ID_RKMEDIA_SET_PARAMS\n");
            encoderName = param->encoder_name;
            LOG_INFO("#### encoderName: %s\n", encoderName.c_str());
            if(lastEncoderName != encoderName) {
                lastEncoderName = encoderName;
                needResetRtspFlag = true;
                LOG_INFO("#### set encoder: %s\n", lastEncoderName.c_str());
                strcpy(m_encoder_name, lastEncoderName.c_str());
            }

            resolutionWidth = param->resolution_width;
            resolutionHeight = param->resolution_height;

            g_width = resolutionWidth;
            g_height = resolutionHeight;

            if(g_width > g_maxResolutionWidth || g_height > g_maxResolutionWidth) {
                g_width = g_maxResolutionWidth;
                g_height = g_maxResolutionHeight;
                LOG_ERROR("#### fixup width %d height %d\n", g_width, g_height);
            }

            if(lastResolutionWidth != resolutionWidth || lastResolutionHeight != resolutionHeight) {
                lastResolutionWidth = resolutionWidth;
                lastResolutionHeight = resolutionHeight;
                needResetRtspFlag = true;
                LOG_INFO("#### set resolution, width:%d height:%d\n", g_width, g_height);
                m_resolution_width = g_width;
                m_resolution_height = g_height;
            }

            if(needResetRtspFlag == true) {
                if(g_rtsp_en) {
                    deinit_rtsp();
                    rkaiq_media->LinkToIsp(false);
                    rkaiq_manager.reset();
                    rkaiq_manager = nullptr;
                    rkaiq_manager = std::make_shared<RKAiqToolManager>(iqfile, g_sensor_name);
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    rkaiq_media->LinkToIsp(true);
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));

                    if(encoderName == "H265") {
                        init_rtsp(mi.ispp.pp_scale0_path.c_str(), g_width, g_height, "video:h265");
                    } else {
                        init_rtsp(mi.ispp.pp_scale0_path.c_str(), g_width, g_height, "video:h264");
                    }
                }
            }

            gop = param->gop;
            video_encoder_set_gop_size(g_video_enc_flow, gop);
            LOG_INFO("#### set gop:%d\n", gop);
            m_gop = gop;

            if(strcmp(param->bps_mode, "FIX_BPS") == 0) {
                strcpy(mode, "cbr");
            } else if(strcmp(param->bps_mode, "Dynamic_BPS") == 0) {
                strcpy(mode, "vbr");
            }
            video_encoder_set_rc_mode(g_video_enc_flow, mode);
            if(strcmp(param->bps_mode, "FIX_BPS") == 0) {
                strcpy(m_bps_mode, "FIX_BPS");
            } else if(strcmp(param->bps_mode, "Dynamic_BPS") == 0) {
                strcpy(m_bps_mode, "Dynamic_BPS");
            }
            LOG_INFO("#### set bps_mode:%s\n", m_bps_mode);

            bpsTarget = param->bps_target;
            LOG_INFO("#### param->bps_target:%d\n", bpsTarget);
            if(strcmp(param->bps_mode, "FIX_BPS") == 0) {
                video_encoder_set_bps(g_video_enc_flow, bpsTarget * 1024, bpsTarget * 1024, bpsTarget * 1024);
                LOG_INFO("#### set fix bpsTarget:%d\n", bpsTarget * 1024);
                m_bps_target = bpsTarget;
            } else if(strcmp(param->bps_mode, "Dynamic_BPS") == 0) {
                video_encoder_set_bps(g_video_enc_flow, bpsTarget * 1024, bpsTarget * 1024 * 0.5, bpsTarget * 1024 * 1.5);
                LOG_INFO("#### set dynamic bpsTarget:%d\n", bpsTarget * 1024);
                m_bps_target = bpsTarget;
            }

            VideoEncoderQp qps;
            qps.qp_init = param->qp_init;
            qps.qp_step = param->qp_step;
            qps.qp_min = param->qp_min;
            qps.qp_max = param->qp_max;
            qps.qp_min_i = param->qp_min_i;
            qps.qp_max_i = param->qp_max_i;
            video_encoder_set_qp(g_video_enc_flow, qps);
            LOG_INFO("#### set qp_init:%d\n", qps.qp_init);
            LOG_INFO("#### set qp_step:%d\n", qps.qp_step);
            LOG_INFO("#### set qp_min:%d\n", qps.qp_min);
            LOG_INFO("#### set qp_max:%d\n", qps.qp_max);
            LOG_INFO("#### set qp_min_i:%d\n", qps.qp_min_i);
            LOG_INFO("#### set qp_max_i:%d\n", qps.qp_max_i);
            m_qp_init = param->qp_init;
            m_qp_step = param->qp_step;
            m_qp_min = param->qp_min;
            m_qp_max = param->qp_max;
            m_qp_min_i = param->qp_min_i;
            m_qp_max_i = param->qp_max_i;
            break;
        case ENUM_ID_RKMEDIA_GET_PARAMS:
            LOG_INFO("#### ENUM_ID_RKMEDIA_GET_PARAMS\n");
            memcpy(param->encoder_name, m_encoder_name, sizeof(m_encoder_name));
            LOG_INFO("#### m_encoder_name:%s\n", m_encoder_name);
            param->resolution_width = m_resolution_width;
            LOG_INFO("#### m_resolution_width:%d\n", m_resolution_width);
            param->resolution_height = m_resolution_height;
            LOG_INFO("#### m_resolution_height:%d\n", m_resolution_height);

            param->bps_target = m_bps_target;
            LOG_INFO("#### m_bps_target:%d\n", m_bps_target);
            // get_video_encoder_config_string(&imgInfo, &videoCfg);
            // LOG_INFO("#### bps_max:%s\n", videoCfg.max_bps);
            // param->bps_target = videoCfg.max_bps;


            memcpy(param->bps_mode, m_bps_mode, sizeof(m_bps_mode));
            LOG_INFO("#### m_bps_mode:%s\n", m_bps_mode);
            // encoderConfigString = get_video_encoder_config_string(&imgInfo, &videoCfg);
            // LOG_INFO("#### bps_mode:%s\n", videoCfg.rc_mode);

            param->gop = m_gop;
            LOG_INFO("#### m_gop:%d\n", m_gop);
            // get_video_encoder_config_string(&imgInfo, &videoCfg);
            // param->gop = videoCfg.gop;
            // LOG_INFO("#### gop:%d\n", );

            param->qp_init = m_qp_init;
            LOG_INFO("#### m_qp_init:%d\n", m_qp_init);
            param->qp_step = m_qp_step;
            LOG_INFO("#### m_qp_step:%d\n", m_qp_step);
            param->qp_min = m_qp_min;
            LOG_INFO("#### m_qp_min:%d\n", m_qp_min);
            param->qp_max = m_qp_max;
            LOG_INFO("#### m_qp_max:%d\n", m_qp_max);
            param->qp_min_i = m_qp_min_i;
            LOG_INFO("#### m_qp_min_i:%d\n", m_qp_min_i);
            param->qp_max_i = m_qp_max_i;
            LOG_INFO("#### m_qp_max_i:%d\n", m_qp_max_i);

            memcpy(data, param, sizeof(RkMedia_User_Params_s));
            break;
        default:
            LOG_INFO("cmdID: Unknow\n");
            break;
    }
    return 0;
}

int RKAiqToolManager::CCMIoCtrl(int id, void* data, int size) {
    LOG_INFO("CCMIoCtrl id: 0x%x\n", id);
    switch(id) {
        case ENUM_ID_ACCM_SETATTRIB: {
            CHECK_PARAM_SIZE(rk_aiq_ccm_attrib_t, size);
            ccm_->SetAttrib(*(rk_aiq_ccm_attrib_t*)data);
        }
        break;
        case ENUM_ID_ACCM_GETATTRIB: {
            CHECK_PARAM_SIZE(rk_aiq_ccm_attrib_t, size);
            ccm_->GetAttrib((rk_aiq_ccm_attrib_t*)data);
        }
        break;
        case ENUM_ID_ACCM_QUERYCCMINFO: {
            CHECK_PARAM_SIZE(rk_aiq_ccm_querry_info_t, size);
            ccm_->QueryCCMInfo((rk_aiq_ccm_querry_info_t*)data);
        }
        break;
        default:
            LOG_INFO("cmdID: Unknow\n");
            break;
    }
    return 0;
}

int RKAiqToolManager::AWBIoCtrl(int id, void* data, int size) {
    LOG_INFO("AWBIoCtrl id: 0x%x\n", id);
    switch(id) {
        case ENUM_ID_AWB_SETATTRIB: {
            CHECK_PARAM_SIZE(rk_aiq_wb_attrib_t, size);
            awb_->SetAttrib(*(rk_aiq_wb_attrib_t*)data);
        }
        break;
        case ENUM_ID_AWB_GETATTRIB: {
            CHECK_PARAM_SIZE(rk_aiq_wb_attrib_t, size);
            awb_->GetAttrib((rk_aiq_wb_attrib_t*)data);
        }
        break;
        case ENUM_ID_AWB_QUERYWBINFO: {
            CHECK_PARAM_SIZE(rk_aiq_wb_querry_info_t, size);
            awb_->QueryWBInfo((rk_aiq_wb_querry_info_t*)data);
        }
        break;
        default:
            LOG_INFO("cmdID: Unknow\n");
            break;
    }
    return 0;
}

int RKAiqToolManager::CPROCIoCtrl(int id, void* data, int size) {
    LOG_INFO("CPROCIoCtrl id: 0x%x\n", id);
    switch(id) {
        case ENUM_ID_CPROC_SETATTRIB: {
            CHECK_PARAM_SIZE(acp_attrib_t, size);
            cproc_->SetAttrib(*(acp_attrib_t*)data);
        }
        break;
        case ENUM_ID_CPROC_GETATTRIB: {
            CHECK_PARAM_SIZE(acp_attrib_t, size);
            cproc_->GetAttrib((acp_attrib_t*)data);
        }
        break;
        default:
            LOG_INFO("cmdID: Unknow\n");
            break;
    }
    return 0;
}