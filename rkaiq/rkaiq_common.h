#ifndef _RKAIQ_COMMON_H__
#define _RKAIQ_COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logger/log.h"
#include "rkaiq_cmdid.h"

#include "camera_capture.h"
#include "camera_infohw.h"
#include "rkaiq_manager.h"
#ifdef ENABLE_RSTP_SERVER
#include "rtsp_server.h"
#endif
#include "tcp_server.h"

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

typedef enum { PC_TO_DEVICE = 0x00, DEVICE_TO_PC } cmdType;

#define RKAIQ_TOOL_VERSION "v0.0.1"

#define STOP_RKLUNCH_CMD "sh /oem/RkLunch-stop.sh"

extern std::shared_ptr<RKAiqToolManager> rkaiq_manager;

#define RKID_ISP_ON "ISP_ON"
#define RKID_ISP_OFF "ISP_OFF"
#define RKID_CHECK "IGNORE"

#pragma pack(1)
typedef struct CommandData_s {
  uint8_t RKID[8];
  uint16_t cmdType;
  uint16_t cmdID;
  uint8_t version[8];
  uint16_t datLen;
  uint8_t dat[48];
  uint16_t checkSum;
} CommandData_t;
#pragma pack()

typedef enum {
  RKISP_CMD_UAPI_SET = 0x00,
  RKISP_CMD_UAPI_GET = 0x01,
  RKISP_CMD_CAPTURE = 0x02,
  RKISP_CMD_DUMP_RAW = 0x03,
  RKISP_CMD_STATUS = 0x80,
} FuncType_e;

typedef enum {
  ENUM_ID_CAPTURE_STATUS = 0x0001,
  ENUM_ID_CAPTURE_RAW_CAPTURE = 0x0002,
  ENUM_ID_CAPTURE_YUV_CAPTURE = 0x0006,
} RkispCmdCaptureID_e;

typedef enum {
  PROC_ID_CAPTURE_RAW_STATUS = 0x00,
  PROC_ID_CAPTURE_RAW_GET_PARAM = 0x01,
  PROC_ID_CAPTURE_RAW_SET_PARAM = 0x02,
  PROC_ID_CAPTURE_RAW_START = 0x03,
  PROC_ID_CAPTURE_RAW_CHECKSUM = 0x04,
} RkispCmdRawCaptureProcID_e;

typedef enum {
  PROC_ID_CAPTURE_YUV_STATUS = 0x00,
  PROC_ID_CAPTURE_YUV_GET_PARAM = 0x01,
  PROC_ID_CAPTURE_YUV_SET_PARAM = 0x02,
  PROC_ID_CAPTURE_YUV_START = 0x03,
  PROC_ID_CAPTURE_YUV_CHECKSUM = 0x04,
} RkispCmdYuvCaptureProcID_e;

typedef enum {
  ENUM_ID_GET_STATUS = 0x0100,
} RkispCmdStatusID_e;

#if 0
#pragma pack(1)
typedef struct SensorParams_s
{
	uint8_t  isValid;
	uint32_t FPS;
	uint32_t HTS;
	uint32_t VTS;
	uint32_t Bits;
	uint8_t  Endianness;
}SensorParams_t;
#pragma pack()

#pragma pack(1)
typedef struct QuerySensorParam_s
{
	uint16_t ResW;
	uint16_t ResH;
}QuerySensorParam_t;
#pragma pack()

#pragma pack(1)
typedef struct CaptureParams_s
{
	uint32_t GAIN;
	uint32_t TIME;
	uint8_t  ConversionGainMode;
	uint8_t  Bits;
	uint8_t  FrameNum;
	uint8_t  MultiFrame;
}CaptureParams_t;
#pragma pack()

typedef enum
{
	LITTLE_ENDIAN = 0,
	BIG_ENDIAN,
}endiannessType_e;

typedef enum
{
	RES_FAILED = 0x00,
	RES_SUCCESS = 0x01,
	KNOCK_KNOCK = 0x80,
	READY = 0x81,
	BUSY = 0x82,
}cmdStatus_e;

typedef enum
{
	VIDEO_APP_OFF = 0x80,
	VIDEO_APP_ON
}videoAppStatus_e;

typedef enum
{
	LCG_MODE,
	HCG_MODE,
	NORMAL_MODE,
}conversionGainMode_e;

typedef enum
{
	LOW_LIGHT,
	HIGH_LIGHT,
	NORMAL_LIGHT
}lightMode_e;

typedef enum
{
	MULTI_FRAME_OFF,
	MULTI_FRAME_ON
}multiFrameMode_e;
#endif

#endif
