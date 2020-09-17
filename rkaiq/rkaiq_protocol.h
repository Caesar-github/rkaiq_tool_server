#ifndef _RKAIQ_PROTOCOL_H__
#define _RKAIQ_PROTOCOL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>

#include "logger/log.h"
#include "rkaiq_cmdid.h"

#include "rkaiq_online_protocol.h"
#include "rkaiq_raw_protocol.h"

int StopProcess(const char *process, const char *str);
int WaitProcessExit(const char *process, int sec);

class RKAiqProtocol {
public:
  RKAiqProtocol() = default;
  virtual ~RKAiqProtocol() = default;
  static int DoChangeAppMode(appRunStatus mode);
  static void HandlerTCPMessage(int sockfd, char *buffer, int size);
};

#endif
