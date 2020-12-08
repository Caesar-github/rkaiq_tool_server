#ifndef _TOOL_RKAIQ_API_ADPCC_H_
#define _TOOL_RKAIQ_API_ADPCC_H_

#include "rk_aiq.h"
#include "rk_aiq_user_api_adpcc.h"

class RKAiqToolADPCC {
    public:
        RKAiqToolADPCC(rk_aiq_sys_ctx_t* ctx);
        virtual ~RKAiqToolADPCC();

        int SetAttrib(rk_aiq_dpcc_attrib_t* attr);
        int GetAttrib(rk_aiq_dpcc_attrib_t* attr);

    private:
        rk_aiq_sys_ctx_t* ctx_;
};

#endif // _TOOL_RKAIQ_API_ADPCC_H_
