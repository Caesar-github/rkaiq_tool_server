#ifndef _CAMERA_DEVICE_H_
#define _CAMERA_DEVICE_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h> /* getopt_long() */

#include <errno.h>
#include <fcntl.h> /* low-level i/o */
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/videodev2.h>

#include "camera_memory.h"

void device_close(int dev_fd);
int device_open(const char *dev_name);
int device_querycap(int dev_fd, struct v4l2_capability *cap);
int device_cropcap(int dev_fd, struct v4l2_cropcap *cropcap,
                   struct v4l2_crop *crop);
int device_setformat(int dev_fd, struct v4l2_format *fmt);
int device_getformat(int dev_fd, struct v4l2_format *fmt);
int device_getsubdevformat(int dev_fd, struct v4l2_subdev_format *fmt);
int device_getblank(int dev_fd, struct v4l2_queryctrl *ctrl);
int device_set3aexposure(int dev_fd, struct v4l2_ext_controls *ctrls);
int device_setctrl(int dev_fd, struct v4l2_control *ctrl);
int device_getsensorfps(int dev_fd,
                        struct v4l2_subdev_frame_interval *finterval);
int device_streamon(int dev_fd, enum v4l2_buf_type *type);
int device_streamoff(int dev_fd, enum v4l2_buf_type *type);
int device_qbuf(int dev_fd, struct v4l2_buffer *buf);
int device_dqbuf(int dev_fd, struct v4l2_buffer *buf);

#endif