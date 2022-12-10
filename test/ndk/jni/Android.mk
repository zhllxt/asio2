
LOCAL_PATH := $(call my-dir)

ASIO2_ROOT_DIR    := $(LOCAL_PATH)/../../../
ASIO2_EXAMPLE_DIR := $(ASIO2_ROOT_DIR)/example
ASIO2_TEST_DIR    := $(ASIO2_ROOT_DIR)/test

ASIO2_INCLUDE := $(ASIO2_ROOT_DIR)/3rd
ASIO2_INCLUDE += $(ASIO2_ROOT_DIR)/3rd/openssl/include
ASIO2_INCLUDE += $(ASIO2_ROOT_DIR)/include

# $(info "TARGET_ARCH_ABI = $(TARGET_ARCH_ABI)")
# $(info "ASIO2_ROOT_DIR = $(ASIO2_ROOT_DIR)")

ifeq      ($(TARGET_ARCH_ABI), arm64-v8a)

else ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)

else ifeq ($(TARGET_ARCH_ABI), x86)

else ifeq ($(TARGET_ARCH_ABI), x86_64)
 
else
 $(error "Not supported TARGET_ARCH_ABI: $(TARGET_ARCH_ABI)")
endif

###############################################################################
## ssl librarys link

include $(CLEAR_VARS)
LOCAL_MODULE := libssl
LOCAL_SRC_FILES := $(ASIO2_ROOT_DIR)/3rd/openssl/prebuilt/android/$(TARGET_ARCH_ABI)/libssl.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libcrypto
LOCAL_SRC_FILES := $(ASIO2_ROOT_DIR)/3rd/openssl/prebuilt/android/$(TARGET_ARCH_ABI)/libcrypto.a
include $(PREBUILT_STATIC_LIBRARY)

###############################################################################
## projects

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := asio2_rpc_qps_client.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/bench/rpc/asio2_rpc_qps_client/asio2_rpc_qps_client.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := asio2_rpc_qps_server.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/bench/rpc/asio2_rpc_qps_server/asio2_rpc_qps_server.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := asio_tcp_tps_client.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/bench/tcp/asio_tcp_tps_client/asio_tcp_tps_client.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := asio_tcp_tps_server.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/bench/tcp/asio_tcp_tps_server/asio_tcp_tps_server.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := asio2_tcp_tps_client.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/bench/tcp/asio2_tcp_tps_client/asio2_tcp_tps_client.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := asio2_tcp_tps_server.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/bench/tcp/asio2_tcp_tps_server/asio2_tcp_tps_server.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := aes.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/aes.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := base64.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/base64.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := des.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/des.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := event_dispatcher.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/event_dispatcher.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := http.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/http.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := https.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/https.cpp
LOCAL_STATIC_LIBRARIES += libssl
LOCAL_STATIC_LIBRARIES += libcrypto
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := ini.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/ini.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := md5.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/md5.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := mqtt.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/mqtt.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := rdc.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/rdc.cpp
LOCAL_STATIC_LIBRARIES += libssl
LOCAL_STATIC_LIBRARIES += libcrypto
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := reflection.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/reflection.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := rpc.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/rpc.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := sha1.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/sha1.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := shared_iopool.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/shared_iopool.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := tcp_dgram.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/tcp_dgram.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := tcp_general.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/tcp_general.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := thread_pool.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/thread_pool.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := timer.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/timer.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := timer_enable_error.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/timer_enable_error.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := udp.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/udp.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := uuid.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/uuid.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := websocket.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/websocket.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := zlib.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/zlib.cpp
include $(BUILD_EXECUTABLE)
