
LOCAL_PATH := $(call my-dir)

ASIO2_INCLUDE := $(LOCAL_PATH)/../../../3rd
ASIO2_INCLUDE += $(LOCAL_PATH)/../../../include

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := http_server.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../http/server/http_server.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := http_client.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../http/client/http_client.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := icmp_ping.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../icmp/icmp_ping.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := rpc_server.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../rpc/server/rpc_server.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := rpc_client.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../rpc/client/rpc_client.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := serial_port.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../serial_port/serial_port.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := tcp_server_character.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../tcp/server/character/tcp_server_character.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := tcp_server_custom.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../tcp/server/custom/tcp_server_custom.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := tcp_server_datagram.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../tcp/server/datagram/tcp_server_datagram.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := tcp_server_general.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../tcp/server/general/tcp_server_general.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := tcp_client_character.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../tcp/client/character/tcp_client_character.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := tcp_client_custom.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../tcp/client/custom/tcp_client_custom.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := tcp_client_datagram.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../tcp/client/datagram/tcp_client_datagram.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := tcp_client_general.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../tcp/client/general/tcp_client_general.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := udp_server.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../udp/server/general/udp_server.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := udp_server_kcp.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../udp/server/kcp/udp_server_kcp.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := udp_client.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../udp/client/general/udp_client.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := udp_client_kcp.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../udp/client/kcp/udp_client_kcp.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := udp_cast.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../udp/cast/udp_cast.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := websocket_server.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../websocket/server/websocket_server.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := websocket_client.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../websocket/client/websocket_client.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := mqtt_client.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../mqtt/client/mqtt_client.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(ASIO2_INCLUDE)
LOCAL_MODULE := shared_iopool.out
LOCAL_SRC_FILES := $(LOCAL_PATH)/../../shared_iopool/shared_iopool.cpp
include $(BUILD_EXECUTABLE)
