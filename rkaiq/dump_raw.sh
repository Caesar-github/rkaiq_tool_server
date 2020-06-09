#!/bin/sh

#media-ctl -d /dev/media0 --set-v4l2 '"m00_b_os04a10 2-0036":0[fmt:SBGGR10/2688x1520]'
media-ctl -l '"rkisp-isp-subdev":2->"rkisp-bridge-ispp":0[0]'
media-ctl -l '"rkisp-isp-subdev":2->"rkisp_mainpath":0[1]'
media-ctl -d /dev/media0 --set-v4l2 '"rkisp-isp-subdev":0[crop:(0,0)/2688x1520]'
media-ctl -d /dev/media0 --set-v4l2 '"rkisp-isp-subdev":0[fmt:SBGGR12/2688x1520]'
media-ctl -d /dev/media0 --set-v4l2 '"rkisp-isp-subdev":2[crop:(0,0)/2688x1520]'
media-ctl -d /dev/media0 --set-v4l2 '"rkisp-isp-subdev":2[fmt:SBGGR12/2688x1520]'
#v4l2-ctl -d /dev/video0  --set-fmt-video=width=2688,height=1520,pixelformat=BG12 --stream-mmap=4 --stream-to=/data/output.raw --stream-count=1 --stream-skip=5 --stream-poll --verbose