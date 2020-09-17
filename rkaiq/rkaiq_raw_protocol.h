#ifndef _RKAIQ_RAW_PROTOCOL_H__
#define _RKAIQ_RAW_PROTOCOL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logger/log.h"
#include "rkaiq_cmdid.h"

#include "rkaiq_common.h"

typedef enum {
  CHECK_DEVICE_STATUS = 0x01,
  RAW_CAPTURE,
  VIDEO_APP_STATUS_REQ,
  VIDEO_APP_STATUS_SET,
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
  APP_RUN_STATUS_TUNRING = 0,
  APP_RUN_STATUS_CAPTURE = 1,
} appRunStatus;

typedef enum { RAW_CAP = 0x00, AVALIABLE } runStaus;

#pragma pack(1)
typedef struct Common_Cmd_s {
  uint8_t RKID[8];
  uint16_t cmdType;
  uint16_t cmdID;
  uint16_t datLen;
  uint8_t dat[48]; // defined by command
  uint16_t checkSum;
} Common_Cmd_t;
#pragma pack()

#pragma pack(1)
typedef struct Sensor_Params_s {
  uint8_t status;
  uint32_t fps;
  uint32_t hts;
  uint32_t vts;
  uint32_t bits;
  uint8_t endianness;
} Sensor_Params_t;
#pragma pack()

#pragma pack(1)
typedef struct Capture_Params_s {
  uint32_t gain;
  uint32_t time;
  uint8_t lhcg;
  uint8_t bits;
  uint8_t framenumber;
  uint8_t multiframe;
} Capture_Params_t;
#pragma pack()

#pragma pack(1)
typedef struct Capture_Reso_s {
  uint16_t width;
  uint16_t height;
} Capture_Reso_t;
#pragma pack()

#define VIDEO_RAW0 "/dev/video0"
#define SAVE_RAW0_PATH "/data/output.raw"

#define TAG_PC_TO_DEVICE "RKISP-AK"
#define TAG_DEVICE_TO_PC "RKISP-AS"

#define VICAP_COMPACT_TEST                                                     \
  "echo 0 > /sys/devices/platform/rkcif_mipi_lvds/compact_test"

int StopProcess(const char *process, const char *str);
int WaitProcessExit(const char *process, int sec);

class RKAiqRawProtocol {
public:
  RKAiqRawProtocol() = default;
  virtual ~RKAiqRawProtocol() = default;
  static void HandlerRawCapMessage(int sockfd, char *buffer, int size);
};

#endif
