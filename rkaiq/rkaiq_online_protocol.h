#ifndef _RKAIQ_ONLINE_PROTOCOL_H__
#define _RKAIQ_ONLINE_PROTOCOL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rkaiq_common.h"

#include "logger/log.h"
#include "rkaiq_cmdid.h"

#pragma pack(1)
typedef struct Common_OL_Cmd_s {
  uint8_t RKID[8];
  uint16_t cmdType;
  uint16_t cmdID;
  uint8_t version[8];
  uint16_t datLen;
  uint8_t dat[4];
  uint16_t checkSum;
} Common_OL_Cmd_t;
#pragma pack()

#define TAG_OL_PC_TO_DEVICE "AIQ-REQ"
#define TAG_OL_DEVICE_TO_PC "AIQ-ANS"

class RKAiqOLProtocol {
public:
  RKAiqOLProtocol() = default;
  virtual ~RKAiqOLProtocol() = default;
  static void HandlerOnLineMessage(int sockfd, char *buffer, int size);
};

#endif
