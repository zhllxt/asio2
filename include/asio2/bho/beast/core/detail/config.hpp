//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BEAST_CORE_DETAIL_CONFIG_HPP
#define BEAST_CORE_DETAIL_CONFIG_HPP

// Available to every header
#include <asio2/bho/config.hpp>
#include <asio2/bho/version.hpp>
#include <asio2/bho/core/ignore_unused.hpp>
#include <asio2/bho/static_assert.hpp>

#if   __has_include(<asio.hpp>)
	#include <asio.hpp>
#elif __has_include(<asio/asio.hpp>)
	#include <asio/asio.hpp>
#else
	#include "asio/associated_allocator.hpp"
	#include "asio/associated_executor.hpp"
	#include "asio/async_result.hpp"
	#include "asio/awaitable.hpp"
	#include "asio/basic_datagram_socket.hpp"
	#include "asio/basic_deadline_timer.hpp"
	#include "asio/basic_io_object.hpp"
	#include "asio/basic_raw_socket.hpp"
	#include "asio/basic_seq_packet_socket.hpp"
	#include "asio/basic_serial_port.hpp"
	#include "asio/basic_signal_set.hpp"
	#include "asio/basic_socket.hpp"
	#include "asio/basic_socket_acceptor.hpp"
	#include "asio/basic_socket_iostream.hpp"
	#include "asio/basic_socket_streambuf.hpp"
	#include "asio/basic_stream_socket.hpp"
	#include "asio/basic_streambuf.hpp"
	#include "asio/basic_waitable_timer.hpp"
	#include "asio/bind_executor.hpp"
	#include "asio/buffer.hpp"
	#include "asio/buffered_read_stream_fwd.hpp"
	#include "asio/buffered_read_stream.hpp"
	#include "asio/buffered_stream_fwd.hpp"
	#include "asio/buffered_stream.hpp"
	#include "asio/buffered_write_stream_fwd.hpp"
	#include "asio/buffered_write_stream.hpp"
	#include "asio/buffers_iterator.hpp"
	#include "asio/co_spawn.hpp"
	#include "asio/completion_condition.hpp"
	#include "asio/compose.hpp"
	#include "asio/connect.hpp"
	#include "asio/coroutine.hpp"
	#include "asio/deadline_timer.hpp"
	#include "asio/defer.hpp"
	#include "asio/detached.hpp"
	#include "asio/dispatch.hpp"
	#include "asio/error.hpp"
	#include "asio/error_code.hpp"
	#include "asio/execution.hpp"
	#include "asio/execution/allocator.hpp"
	#include "asio/execution/any_executor.hpp"
	#include "asio/execution/blocking.hpp"
	#include "asio/execution/blocking_adaptation.hpp"
	#include "asio/execution/bulk_execute.hpp"
	#include "asio/execution/bulk_guarantee.hpp"
	#include "asio/execution/connect.hpp"
	#include "asio/execution/context.hpp"
	#include "asio/execution/context_as.hpp"
	#include "asio/execution/execute.hpp"
	#include "asio/execution/executor.hpp"
	#include "asio/execution/invocable_archetype.hpp"
	#include "asio/execution/mapping.hpp"
	#include "asio/execution/occupancy.hpp"
	#include "asio/execution/operation_state.hpp"
	#include "asio/execution/outstanding_work.hpp"
	#include "asio/execution/prefer_only.hpp"
	#include "asio/execution/receiver.hpp"
	#include "asio/execution/receiver_invocation_error.hpp"
	#include "asio/execution/relationship.hpp"
	#include "asio/execution/schedule.hpp"
	#include "asio/execution/scheduler.hpp"
	#include "asio/execution/sender.hpp"
	#include "asio/execution/set_done.hpp"
	#include "asio/execution/set_error.hpp"
	#include "asio/execution/set_value.hpp"
	#include "asio/execution/start.hpp"
	#include "asio/execution_context.hpp"
	#include "asio/executor.hpp"
	#include "asio/executor_work_guard.hpp"
	#include "asio/generic/basic_endpoint.hpp"
	#include "asio/generic/datagram_protocol.hpp"
	#include "asio/generic/raw_protocol.hpp"
	#include "asio/generic/seq_packet_protocol.hpp"
	#include "asio/generic/stream_protocol.hpp"
	#include "asio/handler_alloc_hook.hpp"
	#include "asio/handler_continuation_hook.hpp"
	#include "asio/handler_invoke_hook.hpp"
	#include "asio/high_resolution_timer.hpp"
	#include "asio/io_context.hpp"
	#include "asio/io_context_strand.hpp"
	#include "asio/io_service.hpp"
	#include "asio/io_service_strand.hpp"
	#include "asio/ip/address.hpp"
	#include "asio/ip/address_v4.hpp"
	#include "asio/ip/address_v4_iterator.hpp"
	#include "asio/ip/address_v4_range.hpp"
	#include "asio/ip/address_v6.hpp"
	#include "asio/ip/address_v6_iterator.hpp"
	#include "asio/ip/address_v6_range.hpp"
	#include "asio/ip/network_v4.hpp"
	#include "asio/ip/network_v6.hpp"
	#include "asio/ip/bad_address_cast.hpp"
	#include "asio/ip/basic_endpoint.hpp"
	#include "asio/ip/basic_resolver.hpp"
	#include "asio/ip/basic_resolver_entry.hpp"
	#include "asio/ip/basic_resolver_iterator.hpp"
	#include "asio/ip/basic_resolver_query.hpp"
	#include "asio/ip/host_name.hpp"
	#include "asio/ip/icmp.hpp"
	#include "asio/ip/multicast.hpp"
	#include "asio/ip/resolver_base.hpp"
	#include "asio/ip/resolver_query_base.hpp"
	#include "asio/ip/tcp.hpp"
	#include "asio/ip/udp.hpp"
	#include "asio/ip/unicast.hpp"
	#include "asio/ip/v6_only.hpp"
	#include "asio/is_applicable_property.hpp"
	#include "asio/is_executor.hpp"
	#include "asio/is_read_buffered.hpp"
	#include "asio/is_write_buffered.hpp"
	#include "asio/local/basic_endpoint.hpp"
	#include "asio/local/connect_pair.hpp"
	#include "asio/local/datagram_protocol.hpp"
	#include "asio/local/stream_protocol.hpp"
	#include "asio/multiple_exceptions.hpp"
	#include "asio/packaged_task.hpp"
	#include "asio/placeholders.hpp"
	#include "asio/posix/basic_descriptor.hpp"
	#include "asio/posix/basic_stream_descriptor.hpp"
	#include "asio/posix/descriptor.hpp"
	#include "asio/posix/descriptor_base.hpp"
	#include "asio/posix/stream_descriptor.hpp"
	#include "asio/post.hpp"
	#include "asio/prefer.hpp"
	#include "asio/query.hpp"
	#include "asio/read.hpp"
	#include "asio/read_at.hpp"
	#include "asio/read_until.hpp"
	#include "asio/redirect_error.hpp"
	#include "asio/require.hpp"
	#include "asio/require_concept.hpp"
	#include "asio/serial_port.hpp"
	#include "asio/serial_port_base.hpp"
	#include "asio/signal_set.hpp"
	#include "asio/socket_base.hpp"
	#include "asio/static_thread_pool.hpp"
	#include "asio/steady_timer.hpp"
	#include "asio/strand.hpp"
	#include "asio/streambuf.hpp"
	#include "asio/system_context.hpp"
	#include "asio/system_error.hpp"
	#include "asio/system_executor.hpp"
	#include "asio/system_timer.hpp"
	#include "asio/this_coro.hpp"
	#include "asio/thread.hpp"
	#include "asio/thread_pool.hpp"
	#include "asio/time_traits.hpp"
	#include "asio/use_awaitable.hpp"
	#include "asio/use_future.hpp"
	#include "asio/uses_executor.hpp"
	#include "asio/version.hpp"
	#include "asio/wait_traits.hpp"
	#include "asio/windows/basic_object_handle.hpp"
	#include "asio/windows/basic_overlapped_handle.hpp"
	#include "asio/windows/basic_random_access_handle.hpp"
	#include "asio/windows/basic_stream_handle.hpp"
	#include "asio/windows/object_handle.hpp"
	#include "asio/windows/overlapped_handle.hpp"
	#include "asio/windows/overlapped_ptr.hpp"
	#include "asio/windows/random_access_handle.hpp"
	#include "asio/windows/stream_handle.hpp"
	#include "asio/write.hpp"
	#include "asio/write_at.hpp"
#endif

namespace beast {

	namespace net = ::asio;

} // beast

/*
    _MSC_VER and _MSC_FULL_VER by version:

    14.0 (2015)             1900        190023026
    14.0 (2015 Update 1)    1900        190023506
    14.0 (2015 Update 2)    1900        190023918
    14.0 (2015 Update 3)    1900        190024210
*/

#if defined(BHO_MSVC)
# if BHO_MSVC_FULL_VER < 190024210
#  error Beast requires C++11: Visual Studio 2015 Update 3 or later needed
# endif

#elif defined(BHO_GCC)
# if(BHO_GCC < 40801)
#  error Beast requires C++11: gcc version 4.8 or later needed
# endif

#else
# if \
    defined(BHO_NO_CXX11_DECLTYPE) || \
    defined(BHO_NO_CXX11_HDR_TUPLE) || \
    defined(BHO_NO_CXX11_TEMPLATE_ALIASES) || \
    defined(BHO_NO_CXX11_VARIADIC_TEMPLATES)
#  error Beast requires C++11: a conforming compiler is needed
# endif

#endif

#define BEAST_DEPRECATION_STRING \
    "This is a deprecated interface, #define BEAST_ALLOW_DEPRECATED to allow it"

#ifndef BEAST_ASSUME
# ifdef BHO_GCC
#  define BEAST_ASSUME(cond) \
    do { if (!(cond)) __builtin_unreachable(); } while (0)
# else
#  define BEAST_ASSUME(cond) do { } while(0)
# endif
#endif

// Default to a header-only implementation. The user must specifically
// request separate compilation by defining BEAST_SEPARATE_COMPILATION
#ifndef BEAST_HEADER_ONLY
# ifndef BEAST_SEPARATE_COMPILATION
#   define BEAST_HEADER_ONLY 1
# endif
#endif

#if BEAST_DOXYGEN
# define BEAST_DECL
#elif defined(BEAST_HEADER_ONLY)
# define BEAST_DECL inline
#else
# define BEAST_DECL
#endif

#ifndef BEAST_ASYNC_RESULT1
#define BEAST_ASYNC_RESULT1(type) \
	ASIO_INITFN_AUTO_RESULT_TYPE(type, void(::beast::error_code))
#endif

#ifndef BEAST_ASYNC_RESULT2
#define BEAST_ASYNC_RESULT2(type) \
	ASIO_INITFN_AUTO_RESULT_TYPE(type, void(::beast::error_code, ::std::size_t))
#endif

#ifndef BEAST_ASYNC_TPARAM1
#define BEAST_ASYNC_TPARAM1 ASIO_COMPLETION_TOKEN_FOR(void(::beast::error_code))
#endif

#ifndef BEAST_ASYNC_TPARAM2
#define BEAST_ASYNC_TPARAM2 ASIO_COMPLETION_TOKEN_FOR(void(::beast::error_code, ::std::size_t))
#endif

#endif
