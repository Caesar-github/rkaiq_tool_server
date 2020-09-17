#ifndef _RKAIQ_ONLINE_PROTOCOL_H__
#define _RKAIQ_ONLINE_PROTOCOL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rkaiq_common.h"

#include "logger/log.h"
#include "rkaiq_cmdid.h"

#define TAG_OL_PC_TO_DEVICE RKID_ISP_ON
#define TAG_OL_DEVICE_TO_PC RKID_ISP_ON

class RKAiqOLProtocol {
public:
  RKAiqOLProtocol() = default;
  virtual ~RKAiqOLProtocol() = default;
  static void HandlerOnLineMessage(int sockfd, char *buffer, int size);
};

#endif
