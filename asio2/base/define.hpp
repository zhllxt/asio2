/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_DEFINE_HPP__
#define __ASIO2_DEFINE_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#define ASIO2_CLASS_DECLARE_BASE(KEYWORD)                                           \
	template <class, class>                      KEYWORD alive_time_cp;             \
	template <class, class>                      KEYWORD async_event_cp;            \
	template <class, class>                      KEYWORD connect_cp;                \
	template <class, class>                      KEYWORD connect_time_cp;           \
	template <class, class>                      KEYWORD connect_timeout_cp;        \
	template <class, class>                      KEYWORD data_persistence_cp;       \
	template <class, class>                      KEYWORD disconnect_cp;             \
	template <class, class>                      KEYWORD event_queue_cp;            \
	template <class, class>                      KEYWORD event_queue_guard;         \
	template <class, class>                      KEYWORD local_endpoint_cp;         \
	template <class, class>                      KEYWORD post_cp;                   \
	template <class, class>                      KEYWORD rdc_call_cp;               \
	template <class, class>                      KEYWORD rdc_call_cp_impl;          \
	template <class, class>                      KEYWORD reconnect_timer_cp;        \
	template <class, class>                      KEYWORD send_cp;                   \
	template <class, class>                      KEYWORD silence_timer_cp;          \
	template <class, class>                      KEYWORD socket_cp;                 \
	template <class, class>                      KEYWORD user_data_cp;              \
	template <class, class>                      KEYWORD user_timer_cp;             \
	template <class       >                      KEYWORD session_mgr_t;             \


#define ASIO2_CLASS_DECLARE_TCP_BASE(KEYWORD)                                       \
	template <class, bool >                      KEYWORD ssl_context_cp;            \
	template <class, class>                      KEYWORD ssl_stream_cp;             \
	template <class, class>                      KEYWORD tcp_keepalive_cp;          \
	template <class, class>                      KEYWORD tcp_recv_op;               \
	template <class, class>                      KEYWORD tcp_send_op;               \
	template <class, class>                      KEYWORD http_send_cp;              \
	template <class, class>                      KEYWORD ws_stream_cp;              \
	template <class, class>                      KEYWORD http_recv_op;              \
	template <class, class>                      KEYWORD http_send_op;              \
	template <class, class>                      KEYWORD ws_send_op;                \
	template <class, class>                      KEYWORD rpc_call_cp;               \
	template <class, class>                      KEYWORD rpc_recv_op;               \
	template <class       >                      KEYWORD http_router_t;             \
	template <class       >                      KEYWORD rpc_invoker_t;             \


#define ASIO2_CLASS_DECLARE_TCP_CLIENT(KEYWORD)                                     \
	template <class, class>                      KEYWORD client_impl_t;             \
	template <class, class>                      KEYWORD tcp_client_impl_t;         \
	template <class, class>                      KEYWORD tcps_client_impl_t;        \
	template <class, class>                      KEYWORD http_client_impl_t;        \
	template <class, class>                      KEYWORD https_client_impl_t;       \
	template <class, class>                      KEYWORD ws_client_impl_t;          \
	template <class, class>                      KEYWORD wss_client_impl_t;         \
	template <class, class>                      KEYWORD rpc_client_impl_t;         \


#define ASIO2_CLASS_DECLARE_TCP_SERVER(KEYWORD)                                     \
	template <class, class>                      KEYWORD server_impl_t;             \
	template <class, class>                      KEYWORD tcp_server_impl_t;         \
	template <class, class>                      KEYWORD tcps_server_impl_t;        \
	template <class, class>                      KEYWORD http_server_impl_t;        \
	template <class, class>                      KEYWORD https_server_impl_t;       \
	template <class, class>                      KEYWORD ws_server_impl_t;          \
	template <class, class>                      KEYWORD wss_server_impl_t;         \
	template <class, class>                      KEYWORD rpc_server_impl_t;         \


#define ASIO2_CLASS_DECLARE_TCP_SESSION(KEYWORD)                                    \
	template <class, class>                      KEYWORD session_impl_t;            \
	template <class, class>                      KEYWORD tcp_session_impl_t;        \
	template <class, class>                      KEYWORD tcps_session_impl_t;       \
	template <class, class>                      KEYWORD http_session_impl_t;       \
	template <class, class>                      KEYWORD https_session_impl_t;      \
	template <class, class>                      KEYWORD ws_session_impl_t;         \
	template <class, class>                      KEYWORD wss_session_impl_t;        \
	template <class, class>                      KEYWORD rpc_session_impl_t;        \


#define ASIO2_CLASS_DECLARE_UDP_BASE(KEYWORD)                                       \
	template <class, class>                      KEYWORD kcp_stream_cp;             \
	template <class, class>                      KEYWORD udp_send_cp;               \
	template <class, class>                      KEYWORD udp_send_op;               \


#define ASIO2_CLASS_DECLARE_UDP_CLIENT(KEYWORD)                                     \
	template <class, class>                      KEYWORD client_impl_t;             \
	template <class, class>                      KEYWORD udp_client_impl_t;         \


#define ASIO2_CLASS_DECLARE_UDP_SERVER(KEYWORD)                                     \
	template <class, class>                      KEYWORD server_impl_t;             \
	template <class, class>                      KEYWORD udp_server_impl_t;         \


#define ASIO2_CLASS_DECLARE_UDP_SESSION(KEYWORD)                                    \
	template <class, class>                      KEYWORD session_impl_t;            \
	template <class, class>                      KEYWORD udp_session_impl_t;        \


//-------------------------------------------------------------------------------------------------

#define ASIO2_CLASS_FORWARD_DECLARE_BASE        ASIO2_CLASS_DECLARE_BASE   (class)
#define ASIO2_CLASS_FRIEND_DECLARE_BASE         ASIO2_CLASS_DECLARE_BASE   (friend class)

//-------------------------------------------------------------------------------------------------

#define ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE    ASIO2_CLASS_DECLARE_TCP_BASE   (class)
#define ASIO2_CLASS_FORWARD_DECLARE_TCP_CLIENT  ASIO2_CLASS_DECLARE_TCP_CLIENT (class)
#define ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER  ASIO2_CLASS_DECLARE_TCP_SERVER (class)
#define ASIO2_CLASS_FORWARD_DECLARE_TCP_SESSION ASIO2_CLASS_DECLARE_TCP_SESSION(class)

#define ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE     ASIO2_CLASS_DECLARE_TCP_BASE   (friend class)
#define ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT   ASIO2_CLASS_DECLARE_TCP_CLIENT (friend class)
#define ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER   ASIO2_CLASS_DECLARE_TCP_SERVER (friend class)
#define ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION  ASIO2_CLASS_DECLARE_TCP_SESSION(friend class)

//-------------------------------------------------------------------------------------------------

#define ASIO2_CLASS_FORWARD_DECLARE_UDP_BASE    ASIO2_CLASS_DECLARE_UDP_BASE   (class)
#define ASIO2_CLASS_FORWARD_DECLARE_UDP_CLIENT  ASIO2_CLASS_DECLARE_UDP_CLIENT (class)
#define ASIO2_CLASS_FORWARD_DECLARE_UDP_SERVER  ASIO2_CLASS_DECLARE_UDP_SERVER (class)
#define ASIO2_CLASS_FORWARD_DECLARE_UDP_SESSION ASIO2_CLASS_DECLARE_UDP_SESSION(class)

#define ASIO2_CLASS_FRIEND_DECLARE_UDP_BASE     ASIO2_CLASS_DECLARE_UDP_BASE   (friend class)
#define ASIO2_CLASS_FRIEND_DECLARE_UDP_CLIENT   ASIO2_CLASS_DECLARE_UDP_CLIENT (friend class)
#define ASIO2_CLASS_FRIEND_DECLARE_UDP_SERVER   ASIO2_CLASS_DECLARE_UDP_SERVER (friend class)
#define ASIO2_CLASS_FRIEND_DECLARE_UDP_SESSION  ASIO2_CLASS_DECLARE_UDP_SESSION(friend class)

#endif // !__ASIO2_DEFINE_HPP__
