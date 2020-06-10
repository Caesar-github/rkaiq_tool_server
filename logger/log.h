// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_LOGGER_H_
#define _RK_LOGGER_H_

extern int log_level;

#define LOG_LEVEL_ERROR 0
#define LOG_LEVEL_WARN 1
#define LOG_LEVEL_INFO 2
#define LOG_LEVEL_DEBUG 3

#ifndef LOG_TAG
#define LOG_TAG ""
#endif // LOG_TAG

#define LOG_INFO(format, ...)                                                  \
  do {                                                                         \
    if (log_level < LOG_LEVEL_INFO)                                            \
      break;                                                                   \
    fprintf(stderr, "[%s][%s]:" format, LOG_TAG, __FUNCTION__, ##__VA_ARGS__); \
  } while (0)

#define LOG_WARN(format, ...)                                                  \
  do {                                                                         \
    if (log_level < LOG_LEVEL_WARN)                                            \
      break;                                                                   \
    fprintf(stderr, "[%s][%s]:" format, LOG_TAG, __FUNCTION__, ##__VA_ARGS__); \
  } while (0)

#define LOG_ERROR(format, ...)                                                 \
  do {                                                                         \
    if (log_level < LOG_LEVEL_ERROR)                                           \
      break;                                                                   \
    fprintf(stderr, "[%s][%s]:" format, LOG_TAG, __FUNCTION__, ##__VA_ARGS__); \
  } while (0)

#define LOG_DEBUG(format, ...)                                                 \
  do {                                                                         \
    if (log_level < LOG_LEVEL_DEBUG)                                           \
      break;                                                                   \
    fprintf(stderr, "[%s][%s]:" format, LOG_TAG, __FUNCTION__, ##__VA_ARGS__); \
  } while (0)

#endif
