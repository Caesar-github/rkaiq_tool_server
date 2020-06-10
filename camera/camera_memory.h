#ifndef _CAMERA_MEMORY_H_
#define _CAMERA_MEMORY_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h> /* getopt_long() */

#include <errno.h>
#include <fcntl.h> /* low-level i/o */
#include <linux/v4l2-controls.h>
#include <linux/v4l2-subdev.h>
#include <linux/videodev2.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "logger/log.h"

#define FMT_NUM_PLANES 1
#define PORT 8080
#define CLEAR(x) memset(&(x), 0, sizeof(x))

enum io_method {
  IO_METHOD_READ,
  IO_METHOD_MMAP,
  IO_METHOD_USERPTR,
};

struct buffer {
  void *start;
  size_t length;
  int socket;
  unsigned short checksum;
};

struct capture_info {
  const char *dev_name;
  int dev_fd;
  enum io_method io;
  struct buffer *buffers;
  unsigned int n_buffers;
  int format;
  int width;
  int height;
  enum v4l2_buf_type capture_buf_type;
  int frame_count;
  char out_file[255];
  FILE *out_fp;
};

void errno_debug(const char *s);
int xioctl(int fh, int request, void *arg);
void init_read(struct capture_info *cap_info, unsigned int buffer_size);
void init_mmap(struct capture_info *cap_info);
void init_userp(struct capture_info *cap_info, unsigned int buffer_size);

int check_io_method(enum io_method io, unsigned int capabilities);
int init_io_method(struct capture_info *cap_info, unsigned int size);

#endif
