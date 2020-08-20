#ifndef _RKAIQ_PROTOCOL_H__
#define _RKAIQ_PROTOCOL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>

#include "camera_capture.h"
#include "camera_infohw.h"
#include "rkaiq_manager.h"
#ifdef ENABLE_RSTP_SERVER
#include "rtsp_server.h"
#endif
#include "logger/log.h"
#include "rkaiq_cmdid.h"

typedef enum { PC_TO_DEVICE = 0x00, DEVICE_TO_PC } cmdType;

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

typedef enum {
  KNOCK_KNOCK = 0x80,
  READY,
  BUSY,
  RES_FAILED = 0x00,
  RES_SUCCESS
} cmdStatus;

typedef enum {
  PACKET_TYPE_SET = 0x00,
  PACKET_TYPE_GET = 0x01,
  PACKET_TYPE_STATUS = 0x80
} packeType;

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

#define RKAIQ_TOOL_VERSION "v0.0.1"

#define VIDEO_RAW0 "/dev/video0"
#define SAVE_RAW0_PATH "/data/output.raw"

#define TAG_PC_TO_DEVICE "RKISP-AK"
#define TAG_DEVICE_TO_PC "RKISP-AS"

#define TAG_OL_PC_TO_DEVICE "AIQ-REQ"
#define TAG_OL_DEVICE_TO_PC "AIQ-ANS"

#define STOP_RKLUNCH_CMD "sh /oem/RkLunch-stop.sh"

#define START_DBSERVER_CMD "dbserver &"
#define START_ISPSERVER_CMD "ispserver -no-sync-db &"

int StopProcess(const char *process, const char *str);
int WaitProcessExit(const char *process, int sec);

class RKAiqProtocol {
public:
  RKAiqProtocol() = default;
  virtual ~RKAiqProtocol() = default;
  static int DoChangeAppMode(appRunStatus mode);
  static void HandlerTCPMessage(int sockfd, char *buffer, int size);
  static void HandlerOnLineMessage(int sockfd, char *buffer, int size);
  static void HandlerRawCapMessage(int sockfd, char *buffer, int size);
  static std::shared_ptr<RKAiqToolManager> rkaiq_manager_;
};

#endif
