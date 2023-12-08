
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
LOCAL_MODULE := codecvt.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/codecvt.cpp
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
LOCAL_MODULE := http1.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/http1.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := http2.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/http2.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := http3.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/http3.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := http4.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/http4.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := https1.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/https1.cpp
LOCAL_STATIC_LIBRARIES += libssl
LOCAL_STATIC_LIBRARIES += libcrypto
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := https2.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/https2.cpp
LOCAL_STATIC_LIBRARIES += libssl
LOCAL_STATIC_LIBRARIES += libcrypto
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := https3.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/https3.cpp
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
LOCAL_MODULE := mqtts.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/mqtts.cpp
LOCAL_STATIC_LIBRARIES += libssl
LOCAL_STATIC_LIBRARIES += libcrypto
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := mqtts_server.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/mqtts_server.cpp
LOCAL_STATIC_LIBRARIES += libssl
LOCAL_STATIC_LIBRARIES += libcrypto
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := mysql.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/mysql.cpp
LOCAL_STATIC_LIBRARIES += libssl
LOCAL_STATIC_LIBRARIES += libcrypto
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := rate_limit_http.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/rate_limit_http.cpp
LOCAL_STATIC_LIBRARIES += libssl
LOCAL_STATIC_LIBRARIES += libcrypto
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := rate_limit_rpc.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/rate_limit_rpc.cpp
LOCAL_STATIC_LIBRARIES += libssl
LOCAL_STATIC_LIBRARIES += libcrypto
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := rate_limit_tcp.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/rate_limit_tcp.cpp
LOCAL_STATIC_LIBRARIES += libssl
LOCAL_STATIC_LIBRARIES += libcrypto
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := rate_limit_ws.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/rate_limit_ws.cpp
LOCAL_STATIC_LIBRARIES += libssl
LOCAL_STATIC_LIBRARIES += libcrypto
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := rdc1.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/rdc1.cpp
LOCAL_STATIC_LIBRARIES += libssl
LOCAL_STATIC_LIBRARIES += libcrypto
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := rdc2.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/rdc2.cpp
LOCAL_STATIC_LIBRARIES += libssl
LOCAL_STATIC_LIBRARIES += libcrypto
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := rdc3.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/rdc3.cpp
LOCAL_STATIC_LIBRARIES += libssl
LOCAL_STATIC_LIBRARIES += libcrypto
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := rdc4.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/rdc4.cpp
LOCAL_STATIC_LIBRARIES += libssl
LOCAL_STATIC_LIBRARIES += libcrypto
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := rdc5.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/rdc5.cpp
LOCAL_STATIC_LIBRARIES += libssl
LOCAL_STATIC_LIBRARIES += libcrypto
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := rdc6.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/rdc6.cpp
LOCAL_STATIC_LIBRARIES += libssl
LOCAL_STATIC_LIBRARIES += libcrypto
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := rdc7.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/rdc7.cpp
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
LOCAL_MODULE := rpc1.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/rpc1.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := rpc2.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/rpc2.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := rpc3.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/rpc3.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := rpc4.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/rpc4.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := rpc5.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/rpc5.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := rpc_kcp1.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/rpc_kcp1.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := rpc_kcp2.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/rpc_kcp2.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := rpc_kcp3.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/rpc_kcp3.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := rpc_kcp4.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/rpc_kcp4.cpp
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
LOCAL_MODULE := socks5_rpc.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/socks5_rpc.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := socks5_tcp.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/socks5_tcp.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := socks5_udp.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/socks5_udp.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := start_stop.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/start_stop.cpp
LOCAL_STATIC_LIBRARIES += libssl
LOCAL_STATIC_LIBRARIES += libcrypto
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := strutil.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/strutil.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := tcp_custom.out
LOCAL_SRC_FILES := $(ASIO2_TEST_DIR)/unit/tcp_custom.cpp
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
