
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
LOCAL_MODULE := http_client.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/http/client/http_client.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := http_server.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/http/server/http_server.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := icmp_ping.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/icmp/icmp_ping.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := inherit.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/inherit/inherit.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := mqtt_client.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/mqtt/client/mqtt_client.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := mqtt_server.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/mqtt/server/mqtt_server.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := rate_limit.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/rate_limit/rate_limit.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := rpc_client.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/rpc/client/rpc_client.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := rpc_server.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/rpc/server/rpc_server.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := serial_port.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/serial_port/serial_port.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := shared_iopool.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/shared_iopool/shared_iopool.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := tcp_client_character.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/tcp/client/character/tcp_client_character.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := tcp_client_custom.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/tcp/client/custom/tcp_client_custom.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := tcp_client_datagram.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/tcp/client/datagram/tcp_client_datagram.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := tcp_client_general.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/tcp/client/general/tcp_client_general.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := tcp_server_character.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/tcp/server/character/tcp_server_character.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := tcp_server_custom.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/tcp/server/custom/tcp_server_custom.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := tcp_server_datagram.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/tcp/server/datagram/tcp_server_datagram.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := tcp_server_general.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/tcp/server/general/tcp_server_general.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := timer.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/timer/timer.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := udp_cast.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/udp/cast/udp_cast.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := udp_client.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/udp/client/general/udp_client.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := udp_client_kcp.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/udp/client/kcp/udp_client_kcp.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := udp_server.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/udp/server/general/udp_server.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := udp_server_kcp.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/udp/server/kcp/udp_server_kcp.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := websocket_client.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/websocket/client/websocket_client.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := websocket_server.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/websocket/server/websocket_server.cpp
include $(BUILD_EXECUTABLE)

###############################################################################
## ssl projects

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := ssl_http_client.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/ssl/http/client/ssl_http_client.cpp
LOCAL_STATIC_LIBRARIES += libssl
LOCAL_STATIC_LIBRARIES += libcrypto
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := ssl_http_server.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/ssl/http/server/ssl_http_server.cpp
LOCAL_STATIC_LIBRARIES += libssl
LOCAL_STATIC_LIBRARIES += libcrypto
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := ssl_rpc_client.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/ssl/rpc/client/ssl_rpc_client.cpp
LOCAL_STATIC_LIBRARIES += libssl
LOCAL_STATIC_LIBRARIES += libcrypto
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := ssl_rpc_server.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/ssl/rpc/server/ssl_rpc_server.cpp
LOCAL_STATIC_LIBRARIES += libssl
LOCAL_STATIC_LIBRARIES += libcrypto
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := ssl_tcp_client_general.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/ssl/tcp/client/general/ssl_tcp_client_general.cpp
LOCAL_STATIC_LIBRARIES += libssl
LOCAL_STATIC_LIBRARIES += libcrypto
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := ssl_tcp_server_general.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/ssl/tcp/server/general/ssl_tcp_server_general.cpp
LOCAL_STATIC_LIBRARIES += libssl
LOCAL_STATIC_LIBRARIES += libcrypto
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := ssl_websocket_client.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/ssl/websocket/client/ssl_websocket_client.cpp
LOCAL_STATIC_LIBRARIES += libssl
LOCAL_STATIC_LIBRARIES += libcrypto
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := ssl_websocket_server.out
LOCAL_SRC_FILES := $(ASIO2_EXAMPLE_DIR)/ssl/websocket/server/ssl_websocket_server.cpp
LOCAL_STATIC_LIBRARIES += libssl
LOCAL_STATIC_LIBRARIES += libcrypto
include $(BUILD_EXECUTABLE)
