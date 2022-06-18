
LOCAL_PATH := $(call my-dir)

ASIO2_INCLUDE := $(LOCAL_PATH)/../../../3rd
ASIO2_INCLUDE += $(LOCAL_PATH)/../../../include

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := aes.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../unit/aes.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := base64.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../unit/base64.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := des.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../unit/des.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := event_dispatcher.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../unit/event_dispatcher.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := http.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../unit/http.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := ini.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../unit/ini.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := md5.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../unit/md5.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := reflection.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../unit/reflection.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := rpc.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../unit/rpc.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := sha1.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../unit/sha1.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := shared_iopool.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../unit/shared_iopool.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := tcp_general.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../unit/tcp_general.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := thread_pool.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../unit/thread_pool.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := timer.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../unit/timer.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := timer_enable_error.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../unit/timer_enable_error.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := uuid.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../unit/uuid.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := websocket.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../unit/websocket.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := zlib.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../unit/zlib.cpp
include $(BUILD_EXECUTABLE)
