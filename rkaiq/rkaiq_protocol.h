#ifndef _RKAIQ_PROTOCOL_H__
#define _RKAIQ_PROTOCOL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>

#include "camera_capture.h"
#include "logger/log.h"

typedef enum { PC_TO_DEVICE = 0x00, DEVICE_TO_PC } cmdType;

typedef enum {
  CHECK_DEVICE_STATUS = 0x01,
  RAW_CAPTURE,
  VIDEO_APP_STATUS_REQ,
  VIDEO_APP_STATUS_SET
} cmdID;

typedef enum {
  RAW_CAPTURE_GET_DEVICE_STATUS = 0x00,
  RAW_CAPTURE_GET_PCLK_HTS_VTS,
  RAW_CAPTURE_SET_PARAMS,
  RAW_CAPTURE_DO_CAPTURE,
  RAW_CAPTURE_COMPARE_CHECKSUM
} cmdCapRawID;

typedef enum {
  CAPTURE_NORMAL = 0,
  CAPTUER_AVERAGE,
} captureMode;

typedef enum { VIDEO_APP_OFF = 0x80, VIDEO_APP_ON } videoAppStatus;

typedef enum {
  KNOCK_KNOCK = 0x80,
  READY,
  BUSY,
  RES_FAILED = 0x00,
  RES_SUCCESS
} cmdStatus;

typedef enum { RAW_CAP = 0x00, AVALIABLE } runStaus;

#define VIDEO_RAW0 "/dev/video0"
#define SAVE_RAW0_PATH "/data/output.raw"

#define TAG_PC_TO_DEVICE "RKISP-AK"
#define TAG_DEVICE_TO_PC "RKISP-AS"

#define STOP_RKLUNCH_CMD "sh /oem/RkLunch-stop.sh &"
#define START_RKLUNCH_CMD "sh /oem/RkLunch.sh &"

class RKAiqProtocol {
public:
  RKAiqProtocol() = default;
  virtual ~RKAiqProtocol() = default;
  static void HandlerTCPMessage(int sockfd, char *buffer, int size);
};

#endif
