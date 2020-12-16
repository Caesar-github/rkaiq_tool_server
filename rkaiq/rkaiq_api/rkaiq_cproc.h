#ifndef _TOOL_RKAIQ_API_CPROC_H_
#define _TOOL_RKAIQ_API_CPROC_H_

#include "rk_aiq_user_api_acp.h"

class RKAiqToolCPROC {
    public:
        RKAiqToolCPROC(rk_aiq_sys_ctx_t* ctx);
        virtual ~RKAiqToolCPROC();

        int SetAttrib(acp_attrib_t attr);
        int GetAttrib(acp_attrib_t* attr);

    private:
        rk_aiq_sys_ctx_t* ctx_;
};

#endif // _TOOL_RKAIQ_API_CPROC_H_
