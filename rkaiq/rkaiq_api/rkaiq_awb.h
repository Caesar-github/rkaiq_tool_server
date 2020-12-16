#ifndef _TOOL_RKAIQ_API_AWB_H_
#define _TOOL_RKAIQ_API_AWB_H_

#include "rk_aiq_user_api_awb.h"

class RKAiqToolAWB {
    public:
        RKAiqToolAWB(rk_aiq_sys_ctx_t* ctx);
        virtual ~RKAiqToolAWB();

        int SetAttrib(rk_aiq_wb_attrib_t attr);
        int GetAttrib(rk_aiq_wb_attrib_t* attr);
        int GetCCT(rk_aiq_wb_cct_t* attr);
        int QueryWBInfo(rk_aiq_wb_querry_info_t* attr);

    private:
        rk_aiq_sys_ctx_t* ctx_;
};

#endif // _TOOL_RKAIQ_API_AWB_H_
