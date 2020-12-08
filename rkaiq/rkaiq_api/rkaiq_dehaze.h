#ifndef _TOOL_RKAIQ_API_DEHAZE_H_
#define _TOOL_RKAIQ_API_DEHAZE_H_

#include "rk_aiq_user_api_adehaze.h"

class RKAiqToolDehaze {
    public:
        RKAiqToolDehaze(rk_aiq_sys_ctx_t* ctx);
        virtual ~RKAiqToolDehaze();

        int SetAttrib(adehaze_sw_t attr);
        int GetAttrib(adehaze_sw_t* attr);

    private:
        rk_aiq_sys_ctx_t* ctx_;
};

#endif // _TOOL_RKAIQ_API_DEHAZE_H_
