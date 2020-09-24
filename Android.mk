LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_CLANG := true
#LOCAL_CFLAGS := $(bootctrl_common_cflags)
#LOCAL_LDFLAGS := $(bootctrl_common_ldflags)
LOCAL_SHARED_LIBRARIES := \
    libbase \
    libcutils \
    liblog
#LOCAL_STATIC_LIBRARIES := \

#LOCAL_CPP_EXTENSION :=  .cpp  .cc

ALLINCLUDES = \
              -I$(LOCAL_PATH) \
              -I$(LOCAL_PATH)/camera \
              -I$(LOCAL_PATH)/common \
              -I$(LOCAL_PATH)/logger \
              -I$(LOCAL_PATH)/mediactl \
              -I$(LOCAL_PATH)/mediactl/linux \
              -I$(LOCAL_PATH)/netserver \
              -I$(LOCAL_PATH)/rkaiq \
              -I$(LOCAL_PATH)/rkaiq/rkaiq_api \
              -I$(LOCAL_PATH)/rkmedia \
              -I$(LOCAL_PATH)/rkmedia/include \
              -I$(LOCAL_PATH)/rkmedia/librkmedia/include/ \
              -I$(LOCAL_PATH)/rkmedia/librkmedia/include/easymedia/ \

LOCAL_CPPFLAGS += $(ALLINCLUDES)
LOCAL_CPPFLAGS += -Wno-error -Wno-non-pod-varargs -DANDROID
#LOCAL_HEADER_LIBRARIES := libhardware_headers libsystem_headers
LOCAL_SRC_FILES := rkaiq_tool_server.cpp \
                camera/camera_capture.cpp \
                camera/camera_device.cpp \
                camera/camera_infohw.cpp \
                camera/camera_memory.cpp \
                logger/log.cpp \
                netserver/tcp_server.cpp \
                netserver/tcp_client.cpp \
                rkaiq/multiframe_process.cpp \
                rkaiq/rkaiq_protocol.cpp \
                rkaiq/rkaiq_raw_protocol.cpp \
                rkaiq/rkaiq_api/rkaiq_adpcc.cpp \
                rkaiq/rkaiq_api/rkaiq_ae.cpp \
                rkaiq/rkaiq_api/rkaiq_anr.cpp \
                rkaiq/rkaiq_api/rkaiq_ahdr.cpp \
                rkaiq/rkaiq_api/rkaiq_agamma.cpp \
                rkaiq/rkaiq_api/rkaiq_engine.cpp \
                rkaiq/rkaiq_api/rkaiq_imgproc.cpp \
                rkaiq/rkaiq_api/rkaiq_manager.cpp \
                rkaiq/rkaiq_api/rkaiq_media.cpp \
                rkaiq/rkaiq_api/rkaiq_sharp.cpp \
                rkaiq/rkaiq_api/rkaiq_sysctl.cpp \
                mediactl/libmediactl.c \
                mediactl/libv4l2subdev.c \
                mediactl/media_info.c \
                mediactl/options.c

LOCAL_SHARED_LIBRARIES += librkaiq
LOCAL_MODULE := aiqserver
LOCAL_MODULE_OWNER := rockchip
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_EXECUTABLE)


# include $(CLEAR_VARS)
# LOCAL_CLANG := true
# LOCAL_SHARED_LIBRARIES := \
#     libbase \
#     libcutils \
#     liblog

# ALLINCLUDES = \
#               -I$(LOCAL_PATH) \
#               -I$(LOCAL_PATH)/rkmedia \
#               -I$(LOCAL_PATH)/rkmedia/include \
#               -I$(LOCAL_PATH)/rkmedia/include/BasicUsageEnvironment \
#               -I$(LOCAL_PATH)/rkmedia/include/groupsock \
#               -I$(LOCAL_PATH)/rkmedia/include/liveMedia \
#               -I$(LOCAL_PATH)/rkmedia/include/UsageEnvironment \
#               -I$(LOCAL_PATH)/rkmedia/librkmedia/include \
#               -I$(LOCAL_PATH)/rkmedia/librkmedia/include/easymedia
# LOCAL_CPP_EXTENSION := .cc
# LOCAL_CPPFLAGS += $(ALLINCLUDES)
# LOCAL_CPPFLAGS += -Wno-non-pod-varargs -Wno-error \
#             -DLIVE555=ON -DLIVE555_SERVER=ON -DLIVE555_SERVER_H264=ON \
#             -DLIVE555_SERVER_H265=ON -DBIONIC_IOCTL_NO_SIGNEDNESS_OVERLOAD \
#             -DENABLE_RSTP_SERVER
# LOCAL_SRC_FILES := rkmedia/sink_flow.cc \
#                 rkmedia/source_flow.cc \
#                 rkmedia/librkmedia/flow/output_stream_flow.cc \
#                 rkmedia/librkmedia/flow/source_stream_flow.cc \
#                 rkmedia/librkmedia/flow/video_encoder_flow.cc \
#                 rkmedia/librkmedia/live555//server/aac_server_media_subsession.cc \
#                 rkmedia/librkmedia/live555//server/h264_server_media_subsession.cc \
#                 rkmedia/librkmedia/live555//server/h265_server_media_subsession.cc \
#                 rkmedia/librkmedia/live555//server/live555_media_input.cc \
#                 rkmedia/librkmedia/live555//server/live555_server.cc \
#                 rkmedia/librkmedia/live555//server/mjpeg_server_media_subsession.cc \
#                 rkmedia/librkmedia/live555//server/mjpeg_video_source.cc \
#                 rkmedia/librkmedia/live555//server/mp2_server_media_subsession.cc \
#                 rkmedia/librkmedia/live555//server/rtsp_server.cc \
#                 rkmedia/librkmedia/live555//server/simple_server_media_subsession.cc \
#                 rkmedia/librkmedia/rkmpp/mpp_decoder.cc \
#                 rkmedia/librkmedia/rkmpp/mpp_encoder.cc \
#                 rkmedia/librkmedia/rkmpp/mpp_final_encoder.cc \
#                 rkmedia/librkmedia/rkmpp/mpp_inc.cc \
#                 rkmedia/librkmedia/stream/camera/v4l2_capture_stream.cc \
#                 rkmedia/librkmedia/stream/camera/v4l2_stream.cc \
#                 rkmedia/librkmedia/stream/camera/v4l2_utils.cc \
#                 rkmedia/librkmedia/buffer.cc \
#                 rkmedia/librkmedia/decoder.cc \
#                 rkmedia/librkmedia/demuxer.cc \
#                 rkmedia/librkmedia/encoder.cc \
#                 rkmedia/librkmedia/filter.cc \
#                 rkmedia/librkmedia/flow.cc \
#                 rkmedia/librkmedia/image.cc \
#                 rkmedia/librkmedia/lock.cc \
#                 rkmedia/librkmedia/media_config.cc \
#                 rkmedia/librkmedia/media_type.cc \
#                 rkmedia/librkmedia/message.cc \
#                 rkmedia/librkmedia/muxer.cc \
#                 rkmedia/librkmedia/sound.cc \
#                 rkmedia/librkmedia/stream.cc \
#                 rkmedia/librkmedia/utils.cc
# LOCAL_MODULE_RELATIVE_PATH := hw
# LOCAL_MODULE := librkmedia
# LOCAL_MODULE_OWNER := rockchip
# LOCAL_PROPRIETARY_MODULE := true
# LOCAL_SHARED_LIBRARIES += libmpp librkaiq
# LOCAL_HEADER_LIBRARIES += libhwjpeg_headers
# include $(BUILD_SHARED_LIBRARY)
