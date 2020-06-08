#ifndef _CAMERA_CAPTURE_H_
#define _CAMERA_CAPTURE_H_

#include "camera_device.h"
#include "camera_memory.h"

int read_frame(struct capture_info *cap_info);
void stop_capturing(struct capture_info *cap_info);
void start_capturing(struct capture_info *cap_info);
void uninit_device(struct capture_info *cap_info);
int init_device(struct capture_info *cap_info);

#endif