#ifndef _TOOL_RKAIQ_API_CCM_H_
#define _TOOL_RKAIQ_API_CCM_H_

#include "rk_aiq_user_api_accm.h"

class RKAiqToolCCM {
    public:
        RKAiqToolCCM(rk_aiq_sys_ctx_t* ctx);
        virtual ~RKAiqToolCCM();

        int SetAttrib(rk_aiq_ccm_attrib_t attr);
        int GetAttrib(rk_aiq_ccm_attrib_t* attr);
        int QueryCCMInfo(rk_aiq_ccm_querry_info_t* attr);

    private:
        rk_aiq_sys_ctx_t* ctx_;
};

#endif // _TOOL_RKAIQ_API_CCM_H_
