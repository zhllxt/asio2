/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_BASIC_STREAM_HPP__
#define __ASIO2_BASIC_STREAM_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/external/asio.hpp>
#include <asio2/external/beast.hpp>

namespace asio2
{
using rate_policy_access    = beast::rate_policy_access;
using unlimited_rate_policy = beast::unlimited_rate_policy;
using simple_rate_policy    = beast::simple_rate_policy;

template<
  class Protocol,
  class Executor = asio::any_io_executor,
  class RatePolicy = unlimited_rate_policy
>
class basic_stream : public beast::basic_stream<Protocol, Executor, RatePolicy>
{
public:
  using super = beast::basic_stream<Protocol, Executor, RatePolicy>;

  using beast::basic_stream<Protocol, Executor, RatePolicy>::basic_stream;

  using executor_type = typename super::executor_type;

  /// The protocol type.
  using protocol_type = typename super::protocol_type;

  /// The endpoint type.
  using endpoint_type = typename super::endpoint_type;

  using native_handle_type = typename super::socket_type::native_handle_type;

  using lowest_layer_type = typename super::socket_type::lowest_layer_type;

  /// Get the executor associated with the object.
  inline executor_type get_executor() noexcept
  {
    return super::get_executor();
  }

  /// Get a reference to the lowest layer.
  /**
   * This function returns a reference to the lowest layer in a stack of
   * layers. Since a basic_socket cannot contain any further layers, it simply
   * returns a reference to itself.
   *
   * @return A reference to the lowest layer in the stack of layers. Ownership
   * is not transferred to the caller.
   */
  inline lowest_layer_type& lowest_layer() noexcept
  {
    return super::socket().lowest_layer();
  }

  /// Get a const reference to the lowest layer.
  /**
   * This function returns a const reference to the lowest layer in a stack of
   * layers. Since a basic_socket cannot contain any further layers, it simply
   * returns a reference to itself.
   *
   * @return A const reference to the lowest layer in the stack of layers.
   * Ownership is not transferred to the caller.
   */
  inline const lowest_layer_type& lowest_layer() const noexcept
  {
    return super::socket().lowest_layer();
  }

  /// Open the socket using the specified protocol.
  /**
   * This function opens the socket so that it will use the specified protocol.
   *
   * @param protocol An object specifying protocol parameters to be used.
   *
   * @throws asio::system_error Thrown on failure.
   *
   * @par Example
   * @code
   * asio::ip::tcp::socket socket(my_context);
   * socket.open(asio::ip::tcp::v4());
   * @endcode
   */
  inline void open(const protocol_type& protocol = protocol_type())
  {
    return super::socket().open(protocol);
  }

  /// Open the socket using the specified protocol.
  /**
   * This function opens the socket so that it will use the specified protocol.
   *
   * @param protocol An object specifying which protocol is to be used.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @par Example
   * @code
   * asio::ip::tcp::socket socket(my_context);
   * asio::error_code ec;
   * socket.open(asio::ip::tcp::v4(), ec);
   * if (ec)
   * {
   *   // An error occurred.
   * }
   * @endcode
   */
  inline ASIO_SYNC_OP_VOID open(const protocol_type& protocol, asio::error_code& ec)
  {
    return super::socket().open(protocol, ec);
  }

  /// Assign an existing native socket to the socket.
  /*
   * This function opens the socket to hold an existing native socket.
   *
   * @param protocol An object specifying which protocol is to be used.
   *
   * @param native_socket A native socket.
   *
   * @throws asio::system_error Thrown on failure.
   */
  inline void assign(const protocol_type& protocol, const native_handle_type& native_socket)
  {
    return super::socket().assign(protocol, native_socket);
  }

  /// Assign an existing native socket to the socket.
  /*
   * This function opens the socket to hold an existing native socket.
   *
   * @param protocol An object specifying which protocol is to be used.
   *
   * @param native_socket A native socket.
   *
   * @param ec Set to indicate what error occurred, if any.
   */
  inline ASIO_SYNC_OP_VOID assign(const protocol_type& protocol,
      const native_handle_type& native_socket, asio::error_code& ec)
  {
    return super::socket().assign(protocol, native_socket, ec);
  }

  /// Determine whether the socket is open.
  inline bool is_open() const
  {
    return super::socket().is_open();
  }

  /// Close the socket.
  /**
   * This function is used to close the socket. Any asynchronous send, receive
   * or connect operations will be cancelled immediately, and will complete
   * with the asio::error::operation_aborted error.
   *
   * @throws asio::system_error Thrown on failure. Note that, even if
   * the function indicates an error, the underlying descriptor is closed.
   *
   * @note For portable behaviour with respect to graceful closure of a
   * connected socket, call shutdown() before closing the socket.
   */
  inline void close()
  {
    return super::close();
  }

  /// Close the socket.
  /**
   * This function is used to close the socket. Any asynchronous send, receive
   * or connect operations will be cancelled immediately, and will complete
   * with the asio::error::operation_aborted error.
   *
   * @param ec Set to indicate what error occurred, if any. Note that, even if
   * the function indicates an error, the underlying descriptor is closed.
   *
   * @par Example
   * @code
   * asio::ip::tcp::socket socket(my_context);
   * ...
   * asio::error_code ec;
   * socket.close(ec);
   * if (ec)
   * {
   *   // An error occurred.
   * }
   * @endcode
   *
   * @note For portable behaviour with respect to graceful closure of a
   * connected socket, call shutdown() before closing the socket.
   */
  inline ASIO_SYNC_OP_VOID close(asio::error_code& ec)
  {
    try
    {
      super::close();
    }
    catch (const system_error& e)
    {
      ec = e.code();
    }
    ASIO_SYNC_OP_VOID_RETURN(ec);
  }

  /// Release ownership of the underlying native socket.
  /**
   * This function causes all outstanding asynchronous connect, send and receive
   * operations to finish immediately, and the handlers for cancelled operations
   * will be passed the asio::error::operation_aborted error. Ownership
   * of the native socket is then transferred to the caller.
   *
   * @throws asio::system_error Thrown on failure.
   *
   * @note This function is unsupported on Windows versions prior to Windows
   * 8.1, and will fail with asio::error::operation_not_supported on
   * these platforms.
   */
//#if defined(ASIO_MSVC) && (ASIO_MSVC >= 1400) && (!defined(_WIN32_WINNT) || _WIN32_WINNT < 0x0603)
//  __declspec(deprecated("This function always fails with "
//        "operation_not_supported when used on Windows versions "
//        "prior to Windows 8.1."))
//#endif
  inline native_handle_type release()
  {
    return super::socket().release();
  }

  /// Release ownership of the underlying native socket.
  /**
   * This function causes all outstanding asynchronous connect, send and receive
   * operations to finish immediately, and the handlers for cancelled operations
   * will be passed the asio::error::operation_aborted error. Ownership
   * of the native socket is then transferred to the caller.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @note This function is unsupported on Windows versions prior to Windows
   * 8.1, and will fail with asio::error::operation_not_supported on
   * these platforms.
   */
//#if defined(ASIO_MSVC) && (ASIO_MSVC >= 1400) && (!defined(_WIN32_WINNT) || _WIN32_WINNT < 0x0603)
//  __declspec(deprecated("This function always fails with "
//        "operation_not_supported when used on Windows versions "
//        "prior to Windows 8.1."))
//#endif
  inline native_handle_type release(asio::error_code& ec)
  {
    return super::socket().release(ec);
  }

  /// Get the native socket representation.
  /**
   * This function may be used to obtain the underlying representation of the
   * socket. This is intended to allow access to native socket functionality
   * that is not otherwise provided.
   */
  inline native_handle_type native_handle()
  {
    return super::socket().native_handle();
  }

  /// Cancel all asynchronous operations associated with the socket.
  /**
   * This function causes all outstanding asynchronous connect, send and receive
   * operations to finish immediately, and the handlers for cancelled operations
   * will be passed the asio::error::operation_aborted error.
   *
   * @throws asio::system_error Thrown on failure.
   *
   * @note Calls to cancel() will always fail with
   * asio::error::operation_not_supported when run on Windows XP, Windows
   * Server 2003, and earlier versions of Windows, unless
   * ASIO_ENABLE_CANCELIO is defined. However, the CancelIo function has
   * two issues that should be considered before enabling its use:
   *
   * @li It will only cancel asynchronous operations that were initiated in the
   * current thread.
   *
   * @li It can appear to complete without error, but the request to cancel the
   * unfinished operations may be silently ignored by the operating system.
   * Whether it works or not seems to depend on the drivers that are installed.
   *
   * For portable cancellation, consider using one of the following
   * alternatives:
   *
   * @li Disable asio's I/O completion port backend by defining
   * ASIO_DISABLE_IOCP.
   *
   * @li Use the close() function to simultaneously cancel the outstanding
   * operations and close the socket.
   *
   * When running on Windows Vista, Windows Server 2008, and later, the
   * CancelIoEx function is always used. This function does not have the
   * problems described above.
   */
//#if defined(ASIO_MSVC) && (ASIO_MSVC >= 1400) && (!defined(_WIN32_WINNT) || _WIN32_WINNT < 0x0600) && !defined(ASIO_ENABLE_CANCELIO)
//  __declspec(deprecated("By default, this function always fails with "
//        "operation_not_supported when used on Windows XP, Windows Server 2003, "
//        "or earlier. Consult documentation for details."))
//#endif
  inline void cancel()
  {
    return super::cancel();
  }

  /// Cancel all asynchronous operations associated with the socket.
  /**
   * This function causes all outstanding asynchronous connect, send and receive
   * operations to finish immediately, and the handlers for cancelled operations
   * will be passed the asio::error::operation_aborted error.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @note Calls to cancel() will always fail with
   * asio::error::operation_not_supported when run on Windows XP, Windows
   * Server 2003, and earlier versions of Windows, unless
   * ASIO_ENABLE_CANCELIO is defined. However, the CancelIo function has
   * two issues that should be considered before enabling its use:
   *
   * @li It will only cancel asynchronous operations that were initiated in the
   * current thread.
   *
   * @li It can appear to complete without error, but the request to cancel the
   * unfinished operations may be silently ignored by the operating system.
   * Whether it works or not seems to depend on the drivers that are installed.
   *
   * For portable cancellation, consider using one of the following
   * alternatives:
   *
   * @li Disable asio's I/O completion port backend by defining
   * ASIO_DISABLE_IOCP.
   *
   * @li Use the close() function to simultaneously cancel the outstanding
   * operations and close the socket.
   *
   * When running on Windows Vista, Windows Server 2008, and later, the
   * CancelIoEx function is always used. This function does not have the
   * problems described above.
   */
//#if defined(ASIO_MSVC) && (ASIO_MSVC >= 1400) && (!defined(_WIN32_WINNT) || _WIN32_WINNT < 0x0600) && !defined(ASIO_ENABLE_CANCELIO)
//  __declspec(deprecated("By default, this function always fails with "
//        "operation_not_supported when used on Windows XP, Windows Server 2003, "
//        "or earlier. Consult documentation for details."))
//#endif
  inline ASIO_SYNC_OP_VOID cancel(asio::error_code& ec)
  {
    try
    {
      super::cancel();
    }
    catch (const system_error& e)
    {
      ec = e.code();
    }
    ASIO_SYNC_OP_VOID_RETURN(ec);
  }

  /// Determine whether the socket is at the out-of-band data mark.
  /**
   * This function is used to check whether the socket input is currently
   * positioned at the out-of-band data mark.
   *
   * @return A bool indicating whether the socket is at the out-of-band data
   * mark.
   *
   * @throws asio::system_error Thrown on failure.
   */
  inline bool at_mark() const
  {
    return super::socket().at_mark();
  }

  /// Determine whether the socket is at the out-of-band data mark.
  /**
   * This function is used to check whether the socket input is currently
   * positioned at the out-of-band data mark.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @return A bool indicating whether the socket is at the out-of-band data
   * mark.
   */
  inline bool at_mark(asio::error_code& ec) const
  {
    return super::socket().at_mark(ec);
  }

  /// Determine the number of bytes available for reading.
  /**
   * This function is used to determine the number of bytes that may be read
   * without blocking.
   *
   * @return The number of bytes that may be read without blocking, or 0 if an
   * error occurs.
   *
   * @throws asio::system_error Thrown on failure.
   */
  inline std::size_t available() const
  {
    return super::socket().available();
  }

  /// Determine the number of bytes available for reading.
  /**
   * This function is used to determine the number of bytes that may be read
   * without blocking.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @return The number of bytes that may be read without blocking, or 0 if an
   * error occurs.
   */
  inline std::size_t available(asio::error_code& ec) const
  {
    return super::socket().available(ec);
  }

  /// Bind the socket to the given local endpoint.
  /**
   * This function binds the socket to the specified endpoint on the local
   * machine.
   *
   * @param endpoint An endpoint on the local machine to which the socket will
   * be bound.
   *
   * @throws asio::system_error Thrown on failure.
   *
   * @par Example
   * @code
   * asio::ip::tcp::socket socket(my_context);
   * socket.open(asio::ip::tcp::v4());
   * socket.bind(asio::ip::tcp::endpoint(
   *       asio::ip::tcp::v4(), 12345));
   * @endcode
   */
  inline void bind(const endpoint_type& endpoint)
  {
    return super::socket().bind(endpoint);
  }

  /// Bind the socket to the given local endpoint.
  /**
   * This function binds the socket to the specified endpoint on the local
   * machine.
   *
   * @param endpoint An endpoint on the local machine to which the socket will
   * be bound.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @par Example
   * @code
   * asio::ip::tcp::socket socket(my_context);
   * socket.open(asio::ip::tcp::v4());
   * asio::error_code ec;
   * socket.bind(asio::ip::tcp::endpoint(
   *       asio::ip::tcp::v4(), 12345), ec);
   * if (ec)
   * {
   *   // An error occurred.
   * }
   * @endcode
   */
  inline ASIO_SYNC_OP_VOID bind(const endpoint_type& endpoint,
      asio::error_code& ec)
  {
    return super::socket().bind(endpoint, ec);
  }

  /// Connect the socket to the specified endpoint.
  /**
   * This function is used to connect a socket to the specified remote endpoint.
   * The function call will block until the connection is successfully made or
   * an error occurs.
   *
   * The socket is automatically opened if it is not already open. If the
   * connect fails, and the socket was automatically opened, the socket is
   * not returned to the closed state.
   *
   * @param peer_endpoint The remote endpoint to which the socket will be
   * connected.
   *
   * @throws asio::system_error Thrown on failure.
   *
   * @par Example
   * @code
   * asio::ip::tcp::socket socket(my_context);
   * asio::ip::tcp::endpoint endpoint(
   *     asio::ip::address::from_string("1.2.3.4"), 12345);
   * socket.connect(endpoint);
   * @endcode
   */
  //inline void connect(const endpoint_type& peer_endpoint)
  //{
  //  return super::connect(peer_endpoint);
  //}

  /// Connect the socket to the specified endpoint.
  /**
   * This function is used to connect a socket to the specified remote endpoint.
   * The function call will block until the connection is successfully made or
   * an error occurs.
   *
   * The socket is automatically opened if it is not already open. If the
   * connect fails, and the socket was automatically opened, the socket is
   * not returned to the closed state.
   *
   * @param peer_endpoint The remote endpoint to which the socket will be
   * connected.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @par Example
   * @code
   * asio::ip::tcp::socket socket(my_context);
   * asio::ip::tcp::endpoint endpoint(
   *     asio::ip::address::from_string("1.2.3.4"), 12345);
   * asio::error_code ec;
   * socket.connect(endpoint, ec);
   * if (ec)
   * {
   *   // An error occurred.
   * }
   * @endcode
   */
  //inline ASIO_SYNC_OP_VOID connect(const endpoint_type& peer_endpoint,
  //    asio::error_code& ec)
  //{
  //  return super::connect(peer_endpoint, ec);
  //}

  /// Start an asynchronous connect.
  /**
   * This function is used to asynchronously connect a socket to the specified
   * remote endpoint. It is an initiating function for an @ref
   * asynchronous_operation, and always returns immediately.
   *
   * The socket is automatically opened if it is not already open. If the
   * connect fails, and the socket was automatically opened, the socket is
   * not returned to the closed state.
   *
   * @param peer_endpoint The remote endpoint to which the socket will be
   * connected. Copies will be made of the endpoint object as required.
   *
   * @param token The @ref completion_token that will be used to produce a
   * completion handler, which will be called when the connect completes.
   * Potential completion tokens include @ref use_future, @ref use_awaitable,
   * @ref yield_context, or a function object with the correct completion
   * signature. The function signature of the completion handler must be:
   * @code void handler(
   *   const asio::error_code& error // Result of operation.
   * ); @endcode
   * Regardless of whether the asynchronous operation completes immediately or
   * not, the completion handler will not be invoked from within this function.
   * On immediate completion, invocation of the handler will be performed in a
   * manner equivalent to using asio::post().
   *
   * @par Completion Signature
   * @code void(asio::error_code) @endcode
   *
   * @par Example
   * @code
   * void connect_handler(const asio::error_code& error)
   * {
   *   if (!error)
   *   {
   *     // Connect succeeded.
   *   }
   * }
   *
   * ...
   *
   * asio::ip::tcp::socket socket(my_context);
   * asio::ip::tcp::endpoint endpoint(
   *     asio::ip::address::from_string("1.2.3.4"), 12345);
   * socket.async_connect(endpoint, connect_handler);
   * @endcode
   *
   * @par Per-Operation Cancellation
   * On POSIX or Windows operating systems, this asynchronous operation supports
   * cancellation for the following asio::cancellation_type values:
   *
   * @li @c cancellation_type::terminal
   *
   * @li @c cancellation_type::partial
   *
   * @li @c cancellation_type::total
   */
  //template <
  //    ASIO_COMPLETION_TOKEN_FOR(void (asio::error_code))
  //      ConnectToken ASIO_DEFAULT_COMPLETION_TOKEN_TYPE(executor_type)>
  //ASIO_INITFN_AUTO_RESULT_TYPE_PREFIX(ConnectToken,
  //    void (asio::error_code))
  //async_connect(const endpoint_type& peer_endpoint,
  //    ASIO_MOVE_ARG(ConnectToken) token
  //      ASIO_DEFAULT_COMPLETION_TOKEN(executor_type))
  //  ASIO_INITFN_AUTO_RESULT_TYPE_SUFFIX((
  //    async_initiate<ConnectToken, void (asio::error_code)>(
  //        declval<initiate_async_connect>(), token,
  //        peer_endpoint, declval<asio::error_code&>())))
  //{
  //  return super::async_connect(peer_endpoint, std::forward<ConnectToken>(token));
  //}

  /// Set an option on the socket.
  /**
   * This function is used to set an option on the socket.
   *
   * @param option The new option value to be set on the socket.
   *
   * @throws asio::system_error Thrown on failure.
   *
   * @sa SettableSocketOption @n
   * asio::socket_base::broadcast @n
   * asio::socket_base::do_not_route @n
   * asio::socket_base::keep_alive @n
   * asio::socket_base::linger @n
   * asio::socket_base::receive_buffer_size @n
   * asio::socket_base::receive_low_watermark @n
   * asio::socket_base::reuse_address @n
   * asio::socket_base::send_buffer_size @n
   * asio::socket_base::send_low_watermark @n
   * asio::ip::multicast::join_group @n
   * asio::ip::multicast::leave_group @n
   * asio::ip::multicast::enable_loopback @n
   * asio::ip::multicast::outbound_interface @n
   * asio::ip::multicast::hops @n
   * asio::ip::tcp::no_delay
   *
   * @par Example
   * Setting the IPPROTO_TCP/TCP_NODELAY option:
   * @code
   * asio::ip::tcp::socket socket(my_context);
   * ...
   * asio::ip::tcp::no_delay option(true);
   * socket.set_option(option);
   * @endcode
   */
  template <typename SettableSocketOption>
  inline void set_option(const SettableSocketOption& option)
  {
    return super::socket().set_option(option);
  }

  /// Set an option on the socket.
  /**
   * This function is used to set an option on the socket.
   *
   * @param option The new option value to be set on the socket.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @sa SettableSocketOption @n
   * asio::socket_base::broadcast @n
   * asio::socket_base::do_not_route @n
   * asio::socket_base::keep_alive @n
   * asio::socket_base::linger @n
   * asio::socket_base::receive_buffer_size @n
   * asio::socket_base::receive_low_watermark @n
   * asio::socket_base::reuse_address @n
   * asio::socket_base::send_buffer_size @n
   * asio::socket_base::send_low_watermark @n
   * asio::ip::multicast::join_group @n
   * asio::ip::multicast::leave_group @n
   * asio::ip::multicast::enable_loopback @n
   * asio::ip::multicast::outbound_interface @n
   * asio::ip::multicast::hops @n
   * asio::ip::tcp::no_delay
   *
   * @par Example
   * Setting the IPPROTO_TCP/TCP_NODELAY option:
   * @code
   * asio::ip::tcp::socket socket(my_context);
   * ...
   * asio::ip::tcp::no_delay option(true);
   * asio::error_code ec;
   * socket.set_option(option, ec);
   * if (ec)
   * {
   *   // An error occurred.
   * }
   * @endcode
   */
  template <typename SettableSocketOption>
  inline ASIO_SYNC_OP_VOID set_option(const SettableSocketOption& option,
      asio::error_code& ec)
  {
    return super::socket().set_option(option, ec);
  }

  /// Get an option from the socket.
  /**
   * This function is used to get the current value of an option on the socket.
   *
   * @param option The option value to be obtained from the socket.
   *
   * @throws asio::system_error Thrown on failure.
   *
   * @sa GettableSocketOption @n
   * asio::socket_base::broadcast @n
   * asio::socket_base::do_not_route @n
   * asio::socket_base::keep_alive @n
   * asio::socket_base::linger @n
   * asio::socket_base::receive_buffer_size @n
   * asio::socket_base::receive_low_watermark @n
   * asio::socket_base::reuse_address @n
   * asio::socket_base::send_buffer_size @n
   * asio::socket_base::send_low_watermark @n
   * asio::ip::multicast::join_group @n
   * asio::ip::multicast::leave_group @n
   * asio::ip::multicast::enable_loopback @n
   * asio::ip::multicast::outbound_interface @n
   * asio::ip::multicast::hops @n
   * asio::ip::tcp::no_delay
   *
   * @par Example
   * Getting the value of the SOL_SOCKET/SO_KEEPALIVE option:
   * @code
   * asio::ip::tcp::socket socket(my_context);
   * ...
   * asio::ip::tcp::socket::keep_alive option;
   * socket.get_option(option);
   * bool is_set = option.value();
   * @endcode
   */
  template <typename GettableSocketOption>
  inline void get_option(GettableSocketOption& option) const
  {
    return super::socket().get_option(option);
  }

  /// Get an option from the socket.
  /**
   * This function is used to get the current value of an option on the socket.
   *
   * @param option The option value to be obtained from the socket.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @sa GettableSocketOption @n
   * asio::socket_base::broadcast @n
   * asio::socket_base::do_not_route @n
   * asio::socket_base::keep_alive @n
   * asio::socket_base::linger @n
   * asio::socket_base::receive_buffer_size @n
   * asio::socket_base::receive_low_watermark @n
   * asio::socket_base::reuse_address @n
   * asio::socket_base::send_buffer_size @n
   * asio::socket_base::send_low_watermark @n
   * asio::ip::multicast::join_group @n
   * asio::ip::multicast::leave_group @n
   * asio::ip::multicast::enable_loopback @n
   * asio::ip::multicast::outbound_interface @n
   * asio::ip::multicast::hops @n
   * asio::ip::tcp::no_delay
   *
   * @par Example
   * Getting the value of the SOL_SOCKET/SO_KEEPALIVE option:
   * @code
   * asio::ip::tcp::socket socket(my_context);
   * ...
   * asio::ip::tcp::socket::keep_alive option;
   * asio::error_code ec;
   * socket.get_option(option, ec);
   * if (ec)
   * {
   *   // An error occurred.
   * }
   * bool is_set = option.value();
   * @endcode
   */
  template <typename GettableSocketOption>
  inline ASIO_SYNC_OP_VOID get_option(GettableSocketOption& option,
      asio::error_code& ec) const
  {
    return super::socket().get_option(option, ec);
  }

  /// Perform an IO control command on the socket.
  /**
   * This function is used to execute an IO control command on the socket.
   *
   * @param command The IO control command to be performed on the socket.
   *
   * @throws asio::system_error Thrown on failure.
   *
   * @sa IoControlCommand @n
   * asio::socket_base::bytes_readable @n
   * asio::socket_base::non_blocking_io
   *
   * @par Example
   * Getting the number of bytes ready to read:
   * @code
   * asio::ip::tcp::socket socket(my_context);
   * ...
   * asio::ip::tcp::socket::bytes_readable command;
   * socket.io_control(command);
   * std::size_t bytes_readable = command.get();
   * @endcode
   */
  template <typename IoControlCommand>
  inline void io_control(IoControlCommand& command)
  {
    return super::socket().io_control(command);
  }

  /// Perform an IO control command on the socket.
  /**
   * This function is used to execute an IO control command on the socket.
   *
   * @param command The IO control command to be performed on the socket.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @sa IoControlCommand @n
   * asio::socket_base::bytes_readable @n
   * asio::socket_base::non_blocking_io
   *
   * @par Example
   * Getting the number of bytes ready to read:
   * @code
   * asio::ip::tcp::socket socket(my_context);
   * ...
   * asio::ip::tcp::socket::bytes_readable command;
   * asio::error_code ec;
   * socket.io_control(command, ec);
   * if (ec)
   * {
   *   // An error occurred.
   * }
   * std::size_t bytes_readable = command.get();
   * @endcode
   */
  template <typename IoControlCommand>
  inline ASIO_SYNC_OP_VOID io_control(IoControlCommand& command,
      asio::error_code& ec)
  {
    return super::socket().io_control(command, ec);
  }

  /// Gets the non-blocking mode of the socket.
  /**
   * @returns @c true if the socket's synchronous operations will fail with
   * asio::error::would_block if they are unable to perform the requested
   * operation immediately. If @c false, synchronous operations will block
   * until complete.
   *
   * @note The non-blocking mode has no effect on the behaviour of asynchronous
   * operations. Asynchronous operations will never fail with the error
   * asio::error::would_block.
   */
  inline bool non_blocking() const
  {
    return super::socket().non_blocking();
  }

  /// Sets the non-blocking mode of the socket.
  /**
   * @param mode If @c true, the socket's synchronous operations will fail with
   * asio::error::would_block if they are unable to perform the requested
   * operation immediately. If @c false, synchronous operations will block
   * until complete.
   *
   * @throws asio::system_error Thrown on failure.
   *
   * @note The non-blocking mode has no effect on the behaviour of asynchronous
   * operations. Asynchronous operations will never fail with the error
   * asio::error::would_block.
   */
  inline void non_blocking(bool mode)
  {
    return super::socket().non_blocking(mode);
  }

  /// Sets the non-blocking mode of the socket.
  /**
   * @param mode If @c true, the socket's synchronous operations will fail with
   * asio::error::would_block if they are unable to perform the requested
   * operation immediately. If @c false, synchronous operations will block
   * until complete.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @note The non-blocking mode has no effect on the behaviour of asynchronous
   * operations. Asynchronous operations will never fail with the error
   * asio::error::would_block.
   */
  inline ASIO_SYNC_OP_VOID non_blocking(
      bool mode, asio::error_code& ec)
  {
    return super::socket().non_blocking(mode, ec);
  }

  /// Gets the non-blocking mode of the native socket implementation.
  /**
   * This function is used to retrieve the non-blocking mode of the underlying
   * native socket. This mode has no effect on the behaviour of the socket
   * object's synchronous operations.
   *
   * @returns @c true if the underlying socket is in non-blocking mode and
   * direct system calls may fail with asio::error::would_block (or the
   * equivalent system error).
   *
   * @note The current non-blocking mode is cached by the socket object.
   * Consequently, the return value may be incorrect if the non-blocking mode
   * was set directly on the native socket.
   *
   * @par Example
   * This function is intended to allow the encapsulation of arbitrary
   * non-blocking system calls as asynchronous operations, in a way that is
   * transparent to the user of the socket object. The following example
   * illustrates how Linux's @c sendfile system call might be encapsulated:
   * @code template <typename Handler>
   * struct sendfile_op
   * {
   *   tcp::socket& sock_;
   *   int fd_;
   *   Handler handler_;
   *   off_t offset_;
   *   std::size_t total_bytes_transferred_;
   *
   *   // Function call operator meeting WriteHandler requirements.
   *   // Used as the handler for the async_write_some operation.
   *   void operator()(asio::error_code ec, std::size_t)
   *   {
   *     // Put the underlying socket into non-blocking mode.
   *     if (!ec)
   *       if (!sock_.native_non_blocking())
   *         sock_.native_non_blocking(true, ec);
   *
   *     if (!ec)
   *     {
   *       for (;;)
   *       {
   *         // Try the system call.
   *         errno = 0;
   *         int n = ::sendfile(sock_.native_handle(), fd_, &offset_, 65536);
   *         ec = asio::error_code(n < 0 ? errno : 0,
   *             asio::error::get_system_category());
   *         total_bytes_transferred_ += ec ? 0 : n;
   *
   *         // Retry operation immediately if interrupted by signal.
   *         if (ec == asio::error::interrupted)
   *           continue;
   *
   *         // Check if we need to run the operation again.
   *         if (ec == asio::error::would_block
   *             || ec == asio::error::try_again)
   *         {
   *           // We have to wait for the socket to become ready again.
   *           sock_.async_wait(tcp::socket::wait_write, *this);
   *           return;
   *         }
   *
   *         if (ec || n == 0)
   *         {
   *           // An error occurred, or we have reached the end of the file.
   *           // Either way we must exit the loop so we can call the handler.
   *           break;
   *         }
   *
   *         // Loop around to try calling sendfile again.
   *       }
   *     }
   *
   *     // Pass result back to user's handler.
   *     handler_(ec, total_bytes_transferred_);
   *   }
   * };
   *
   * template <typename Handler>
   * void async_sendfile(tcp::socket& sock, int fd, Handler h)
   * {
   *   sendfile_op<Handler> op = { sock, fd, h, 0, 0 };
   *   sock.async_wait(tcp::socket::wait_write, op);
   * } @endcode
   */
  inline bool native_non_blocking() const
  {
    return super::socket().native_non_blocking();
  }

  /// Sets the non-blocking mode of the native socket implementation.
  /**
   * This function is used to modify the non-blocking mode of the underlying
   * native socket. It has no effect on the behaviour of the socket object's
   * synchronous operations.
   *
   * @param mode If @c true, the underlying socket is put into non-blocking
   * mode and direct system calls may fail with asio::error::would_block
   * (or the equivalent system error).
   *
   * @throws asio::system_error Thrown on failure. If the @c mode is
   * @c false, but the current value of @c non_blocking() is @c true, this
   * function fails with asio::error::invalid_argument, as the
   * combination does not make sense.
   *
   * @par Example
   * This function is intended to allow the encapsulation of arbitrary
   * non-blocking system calls as asynchronous operations, in a way that is
   * transparent to the user of the socket object. The following example
   * illustrates how Linux's @c sendfile system call might be encapsulated:
   * @code template <typename Handler>
   * struct sendfile_op
   * {
   *   tcp::socket& sock_;
   *   int fd_;
   *   Handler handler_;
   *   off_t offset_;
   *   std::size_t total_bytes_transferred_;
   *
   *   // Function call operator meeting WriteHandler requirements.
   *   // Used as the handler for the async_write_some operation.
   *   void operator()(asio::error_code ec, std::size_t)
   *   {
   *     // Put the underlying socket into non-blocking mode.
   *     if (!ec)
   *       if (!sock_.native_non_blocking())
   *         sock_.native_non_blocking(true, ec);
   *
   *     if (!ec)
   *     {
   *       for (;;)
   *       {
   *         // Try the system call.
   *         errno = 0;
   *         int n = ::sendfile(sock_.native_handle(), fd_, &offset_, 65536);
   *         ec = asio::error_code(n < 0 ? errno : 0,
   *             asio::error::get_system_category());
   *         total_bytes_transferred_ += ec ? 0 : n;
   *
   *         // Retry operation immediately if interrupted by signal.
   *         if (ec == asio::error::interrupted)
   *           continue;
   *
   *         // Check if we need to run the operation again.
   *         if (ec == asio::error::would_block
   *             || ec == asio::error::try_again)
   *         {
   *           // We have to wait for the socket to become ready again.
   *           sock_.async_wait(tcp::socket::wait_write, *this);
   *           return;
   *         }
   *
   *         if (ec || n == 0)
   *         {
   *           // An error occurred, or we have reached the end of the file.
   *           // Either way we must exit the loop so we can call the handler.
   *           break;
   *         }
   *
   *         // Loop around to try calling sendfile again.
   *       }
   *     }
   *
   *     // Pass result back to user's handler.
   *     handler_(ec, total_bytes_transferred_);
   *   }
   * };
   *
   * template <typename Handler>
   * void async_sendfile(tcp::socket& sock, int fd, Handler h)
   * {
   *   sendfile_op<Handler> op = { sock, fd, h, 0, 0 };
   *   sock.async_wait(tcp::socket::wait_write, op);
   * } @endcode
   */
  inline void native_non_blocking(bool mode)
  {
    return super::socket().native_non_blocking(mode);
  }

  /// Sets the non-blocking mode of the native socket implementation.
  /**
   * This function is used to modify the non-blocking mode of the underlying
   * native socket. It has no effect on the behaviour of the socket object's
   * synchronous operations.
   *
   * @param mode If @c true, the underlying socket is put into non-blocking
   * mode and direct system calls may fail with asio::error::would_block
   * (or the equivalent system error).
   *
   * @param ec Set to indicate what error occurred, if any. If the @c mode is
   * @c false, but the current value of @c non_blocking() is @c true, this
   * function fails with asio::error::invalid_argument, as the
   * combination does not make sense.
   *
   * @par Example
   * This function is intended to allow the encapsulation of arbitrary
   * non-blocking system calls as asynchronous operations, in a way that is
   * transparent to the user of the socket object. The following example
   * illustrates how Linux's @c sendfile system call might be encapsulated:
   * @code template <typename Handler>
   * struct sendfile_op
   * {
   *   tcp::socket& sock_;
   *   int fd_;
   *   Handler handler_;
   *   off_t offset_;
   *   std::size_t total_bytes_transferred_;
   *
   *   // Function call operator meeting WriteHandler requirements.
   *   // Used as the handler for the async_write_some operation.
   *   void operator()(asio::error_code ec, std::size_t)
   *   {
   *     // Put the underlying socket into non-blocking mode.
   *     if (!ec)
   *       if (!sock_.native_non_blocking())
   *         sock_.native_non_blocking(true, ec);
   *
   *     if (!ec)
   *     {
   *       for (;;)
   *       {
   *         // Try the system call.
   *         errno = 0;
   *         int n = ::sendfile(sock_.native_handle(), fd_, &offset_, 65536);
   *         ec = asio::error_code(n < 0 ? errno : 0,
   *             asio::error::get_system_category());
   *         total_bytes_transferred_ += ec ? 0 : n;
   *
   *         // Retry operation immediately if interrupted by signal.
   *         if (ec == asio::error::interrupted)
   *           continue;
   *
   *         // Check if we need to run the operation again.
   *         if (ec == asio::error::would_block
   *             || ec == asio::error::try_again)
   *         {
   *           // We have to wait for the socket to become ready again.
   *           sock_.async_wait(tcp::socket::wait_write, *this);
   *           return;
   *         }
   *
   *         if (ec || n == 0)
   *         {
   *           // An error occurred, or we have reached the end of the file.
   *           // Either way we must exit the loop so we can call the handler.
   *           break;
   *         }
   *
   *         // Loop around to try calling sendfile again.
   *       }
   *     }
   *
   *     // Pass result back to user's handler.
   *     handler_(ec, total_bytes_transferred_);
   *   }
   * };
   *
   * template <typename Handler>
   * void async_sendfile(tcp::socket& sock, int fd, Handler h)
   * {
   *   sendfile_op<Handler> op = { sock, fd, h, 0, 0 };
   *   sock.async_wait(tcp::socket::wait_write, op);
   * } @endcode
   */
  inline ASIO_SYNC_OP_VOID native_non_blocking(
      bool mode, asio::error_code& ec)
  {
    return super::socket().native_non_blocking(mode, ec);
  }

  /// Get the local endpoint of the socket.
  /**
   * This function is used to obtain the locally bound endpoint of the socket.
   *
   * @returns An object that represents the local endpoint of the socket.
   *
   * @throws asio::system_error Thrown on failure.
   *
   * @par Example
   * @code
   * asio::ip::tcp::socket socket(my_context);
   * ...
   * asio::ip::tcp::endpoint endpoint = socket.local_endpoint();
   * @endcode
   */
  inline endpoint_type local_endpoint() const
  {
    return super::socket().local_endpoint();
  }

  /// Get the local endpoint of the socket.
  /**
   * This function is used to obtain the locally bound endpoint of the socket.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @returns An object that represents the local endpoint of the socket.
   * Returns a default-constructed endpoint object if an error occurred.
   *
   * @par Example
   * @code
   * asio::ip::tcp::socket socket(my_context);
   * ...
   * asio::error_code ec;
   * asio::ip::tcp::endpoint endpoint = socket.local_endpoint(ec);
   * if (ec)
   * {
   *   // An error occurred.
   * }
   * @endcode
   */
  inline endpoint_type local_endpoint(asio::error_code& ec) const
  {
    return super::socket().local_endpoint(ec);
  }

  /// Get the remote endpoint of the socket.
  /**
   * This function is used to obtain the remote endpoint of the socket.
   *
   * @returns An object that represents the remote endpoint of the socket.
   *
   * @throws asio::system_error Thrown on failure.
   *
   * @par Example
   * @code
   * asio::ip::tcp::socket socket(my_context);
   * ...
   * asio::ip::tcp::endpoint endpoint = socket.remote_endpoint();
   * @endcode
   */
  inline endpoint_type remote_endpoint() const
  {
    return super::socket().remote_endpoint();
  }

  /// Get the remote endpoint of the socket.
  /**
   * This function is used to obtain the remote endpoint of the socket.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @returns An object that represents the remote endpoint of the socket.
   * Returns a default-constructed endpoint object if an error occurred.
   *
   * @par Example
   * @code
   * asio::ip::tcp::socket socket(my_context);
   * ...
   * asio::error_code ec;
   * asio::ip::tcp::endpoint endpoint = socket.remote_endpoint(ec);
   * if (ec)
   * {
   *   // An error occurred.
   * }
   * @endcode
   */
  inline endpoint_type remote_endpoint(asio::error_code& ec) const
  {
    return super::socket().remote_endpoint(ec);
  }

  /// Disable sends or receives on the socket.
  /**
   * This function is used to disable send operations, receive operations, or
   * both.
   *
   * @param what Determines what types of operation will no longer be allowed.
   *
   * @throws asio::system_error Thrown on failure.
   *
   * @par Example
   * Shutting down the send side of the socket:
   * @code
   * asio::ip::tcp::socket socket(my_context);
   * ...
   * socket.shutdown(asio::ip::tcp::socket::shutdown_send);
   * @endcode
   */
  inline void shutdown(asio::socket_base::shutdown_type what)
  {
    return super::socket().shutdown(what);
  }

  /// Disable sends or receives on the socket.
  /**
   * This function is used to disable send operations, receive operations, or
   * both.
   *
   * @param what Determines what types of operation will no longer be allowed.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @par Example
   * Shutting down the send side of the socket:
   * @code
   * asio::ip::tcp::socket socket(my_context);
   * ...
   * asio::error_code ec;
   * socket.shutdown(asio::ip::tcp::socket::shutdown_send, ec);
   * if (ec)
   * {
   *   // An error occurred.
   * }
   * @endcode
   */
  inline ASIO_SYNC_OP_VOID shutdown(asio::socket_base::shutdown_type what,
      asio::error_code& ec)
  {
    return super::socket().shutdown(what, ec);
  }

  /// Wait for the socket to become ready to read, ready to write, or to have
  /// pending error conditions.
  /**
   * This function is used to perform a blocking wait for a socket to enter
   * a ready to read, write or error condition state.
   *
   * @param w Specifies the desired socket state.
   *
   * @par Example
   * Waiting for a socket to become readable.
   * @code
   * asio::ip::tcp::socket socket(my_context);
   * ...
   * socket.wait(asio::ip::tcp::socket::wait_read);
   * @endcode
   */
  inline void wait(asio::socket_base::wait_type w)
  {
    return super::socket().wait(w);
  }

  /// Wait for the socket to become ready to read, ready to write, or to have
  /// pending error conditions.
  /**
   * This function is used to perform a blocking wait for a socket to enter
   * a ready to read, write or error condition state.
   *
   * @param w Specifies the desired socket state.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @par Example
   * Waiting for a socket to become readable.
   * @code
   * asio::ip::tcp::socket socket(my_context);
   * ...
   * asio::error_code ec;
   * socket.wait(asio::ip::tcp::socket::wait_read, ec);
   * @endcode
   */
  inline ASIO_SYNC_OP_VOID wait(asio::socket_base::wait_type w, asio::error_code& ec)
  {
    return super::socket().wait(w, ec);
  }

  /// Asynchronously wait for the socket to become ready to read, ready to
  /// write, or to have pending error conditions.
  /**
   * This function is used to perform an asynchronous wait for a socket to enter
   * a ready to read, write or error condition state. It is an initiating
   * function for an @ref asynchronous_operation, and always returns
   * immediately.
   *
   * @param w Specifies the desired socket state.
   *
   * @param token The @ref completion_token that will be used to produce a
   * completion handler, which will be called when the wait completes. Potential
   * completion tokens include @ref use_future, @ref use_awaitable, @ref
   * yield_context, or a function object with the correct completion signature.
   * The function signature of the completion handler must be:
   * @code void handler(
   *   const asio::error_code& error // Result of operation.
   * ); @endcode
   * Regardless of whether the asynchronous operation completes immediately or
   * not, the completion handler will not be invoked from within this function.
   * On immediate completion, invocation of the handler will be performed in a
   * manner equivalent to using asio::post().
   *
   * @par Completion Signature
   * @code void(asio::error_code) @endcode
   *
   * @par Example
   * @code
   * void wait_handler(const asio::error_code& error)
   * {
   *   if (!error)
   *   {
   *     // Wait succeeded.
   *   }
   * }
   *
   * ...
   *
   * asio::ip::tcp::socket socket(my_context);
   * ...
   * socket.async_wait(asio::ip::tcp::socket::wait_read, wait_handler);
   * @endcode
   *
   * @par Per-Operation Cancellation
   * On POSIX or Windows operating systems, this asynchronous operation supports
   * cancellation for the following asio::cancellation_type values:
   *
   * @li @c cancellation_type::terminal
   *
   * @li @c cancellation_type::partial
   *
   * @li @c cancellation_type::total
   */
  template <class... Args>
  inline decltype(auto) async_wait(Args&&... args)
  {
    return super::socket().async_wait(std::forward<Args>(args)...);
  }
};
}

#ifdef ASIO2_HEADER_ONLY
namespace bho::beast::websocket
#else
namespace boost::beast::websocket
#endif
{
  template<
    class Protocol,
    class Executor = asio::any_io_executor,
    class RatePolicy = asio2::unlimited_rate_policy
  >
  void
  teardown(
      beast::role_type role,
      asio2::basic_stream<Protocol, Executor, RatePolicy>& s,
      beast::error_code& ec)
  {
  /*
      If you are trying to use OpenSSL and this goes off, you need to
      add an include for <asio2/bho/beast/websocket/ssl.hpp>.
  
      If you are creating an instance of beast::websocket::stream with your
      own user defined type, you must provide an overload of teardown with
      the corresponding signature (including the role_type).
  */
	  teardown(role, s.socket(), ec);
  }
  
  /** Start tearing down a connection.
  
      This begins tearing down a connection asynchronously.
      The implementation will call the overload of this function
      based on the `Socket` parameter used to consruct the socket.
      When `Stream` is a user defined type, and not a
      `net::ip::tcp::socket` or any `net::ssl::stream`,
      callers are responsible for providing a suitable overload
      of this function.
  
      @param role The role of the local endpoint
  
      @param socket The socket to tear down.
  
      @param handler The completion handler to invoke when the operation
      completes. The implementation takes ownership of the handler by
      performing a decay-copy. The equivalent function signature of
      the handler must be:
      @code
      void handler(
          beast::error_code const& error // result of operation
      );
      @endcode
      Regardless of whether the asynchronous operation completes
      immediately or not, the handler will not be invoked from within
      this function. Invocation of the handler will be performed in a
      manner equivalent to using `net::post`.
  
  */
  template<
    class TeardownHandler,
    class Protocol,
    class Executor = asio::any_io_executor,
    class RatePolicy = asio2::unlimited_rate_policy
  >
  void
  async_teardown(
      beast::role_type role,
      asio2::basic_stream<Protocol, Executor, RatePolicy>& s,
      TeardownHandler&& handler)
  {
  /*
      If you are trying to use OpenSSL and this goes off, you need to
      add an include for <asio2/bho/beast/websocket/ssl.hpp>.
  
      If you are creating an instance of beast::websocket::stream with your
      own user defined type, you must provide an overload of teardown with
      the corresponding signature (including the role_type).
  */
	  async_teardown(role, s.socket(), std::forward<TeardownHandler>(handler));
  }
}

#endif // !__ASIO2_BASIC_STREAM_HPP__
