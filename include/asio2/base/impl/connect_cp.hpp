/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_CONNECT_COMPONENT_HPP__
#define __ASIO2_CONNECT_COMPONENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <future>
#include <utility>
#include <string_view>

#include <asio2/base/iopool.hpp>
#include <asio2/base/listener.hpp>
#include <asio2/base/detail/ecs.hpp>

#include <asio2/base/impl/event_queue_cp.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t, bool IsSession>
	class connect_cp_member_variables;

	template<class derived_t, class args_t>
	class connect_cp_member_variables<derived_t, args_t, true>
	{
	};

	template<class derived_t, class args_t>
	class connect_cp_member_variables<derived_t, args_t, false>
	{
	public:
		/**
		 * @brief Set the host of the server.
		 * If connect failed, and you want to use a different ip when reconnect,
		 * then you can do it like this:
		 *  client.bind_connect([&client]()
		 *	{
		 *		if (asio2::get_last_error()) // has some error, means connect failed
		 *			client.set_host("192.168.0.99");
		 *	});
		 * when reconnecting, the ip "192.168.0.99" will be used as the server's ip.
		 */
		template<typename String>
		inline derived_t& set_host(String&& host)
		{
			this->host_ = detail::to_string(std::forward<String>(host));
			return (static_cast<derived_t&>(*this));
		}

		/**
		 * @brief Set the port of the server.
		 */
		template<typename StrOrInt>
		inline derived_t& set_port(StrOrInt&& port)
		{
			this->port_ = detail::to_string(std::forward<StrOrInt>(port));
			return (static_cast<derived_t&>(*this));
		}

	protected:
		/// Save the host and port of the server
		std::string                     host_, port_;
	};

	/*
	 * can't use "derived_t::is_session()" as the third template parameter of connect_cp_member_variables,
	 * must use "args_t::is_session", beacuse "tcp_session" is derived from "connect_cp", when the 
	 * "connect_cp" is contructed, the "tcp_session" has't contructed yet, then it will can't find the 
	 * "derived_t::is_session()" function, and compile failure.
	 */
	template<class derived_t, class args_t>
	class connect_cp : public connect_cp_member_variables<derived_t, args_t, args_t::is_session>
	{
	public:
		using socket_t           = typename args_t::socket_t;
		using decay_socket_t     = typename std::remove_cv_t<std::remove_reference_t<socket_t>>;
		using lowest_layer_t     = typename decay_socket_t::lowest_layer_type;
		using resolver_type      = typename asio::ip::basic_resolver<typename lowest_layer_t::protocol_type>;
		using endpoints_type     = typename resolver_type::results_type;
		using endpoints_iterator = typename endpoints_type::iterator;

		using self               = connect_cp<derived_t, args_t>;

	public:
		/**
		 * @brief constructor
		 */
		connect_cp() noexcept {}

		/**
		 * @destructor
		 */
		~connect_cp() = default;

	protected:
		template<bool IsAsync, typename C, typename DeferEvent, bool IsSession = args_t::is_session>
		inline typename std::enable_if_t<!IsSession, void>
		_start_connect(std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(derive.io().running_in_this_thread());

			clear_last_error();

		#if defined(_DEBUG) || defined(DEBUG)
			derive.is_stop_reconnect_timer_called_ = false;
			derive.is_stop_connect_timeout_timer_called_ = false;
			derive.is_disconnect_called_ = false;
		#endif

			state_t expected = state_t::starting;
			if (!derive.state_.compare_exchange_strong(expected, state_t::starting))
			{
				ASIO2_ASSERT(false);

				derive._handle_connect(asio::error::operation_aborted,
					std::move(this_ptr), std::move(ecs), std::move(chain));

				return;
			}

			derive._make_reconnect_timer(this_ptr, ecs);

			// start the timeout timer
			derive._make_connect_timeout_timer(this_ptr, derive.get_connect_timeout());

			derive._post_resolve(std::move(this_ptr), std::move(ecs), std::move(chain));
		}

		template<typename C, typename DeferEvent, bool IsSession = args_t::is_session>
		inline typename std::enable_if_t<!IsSession, void>
		_post_resolve(std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			std::string_view h, p;

			if constexpr (ecs_helper::has_socks5<C>())
			{
				auto sock5 = ecs->get_component().socks5_option(std::in_place);

				h = sock5->host();
				p = sock5->port();
			}
			else
			{
				h = this->host_;
				p = this->port_;
			}

			// resolve the server address.
			std::unique_ptr<resolver_type> resolver_ptr = std::make_unique<resolver_type>(
				derive.io().context());

			resolver_type* resolver_rptr = resolver_ptr.get();

			// Before async_resolve execution is complete, we must hold the resolver object.
			// so we captured the resolver_ptr into the lambda callback function.
			resolver_rptr->async_resolve(h, p,
			[&derive, this_ptr = std::move(this_ptr), ecs = std::move(ecs),
				resolver_ptr = std::move(resolver_ptr), chain = std::move(chain)]
			(error_code ec, endpoints_type endpoints) mutable
			{
				// if the connect timeout timer is timedout and called already, it means 
				// connect timed out already.
				if (!derive.connect_timeout_timer_)
				{
					ec = asio::error::timed_out;
				}

				std::unique_ptr<endpoints_type> eps = std::make_unique<endpoints_type>(std::move(endpoints));

				endpoints_type* p = eps.get();

				if (ec)
					derive._handle_connect(ec,
						std::move(this_ptr), std::move(ecs), std::move(chain));
				else
					derive._post_connect(ec, std::move(eps), p->begin(),
						std::move(this_ptr), std::move(ecs), std::move(chain));
			});
		}

		template<typename C, typename DeferEvent, bool IsSession = args_t::is_session>
		typename std::enable_if_t<!IsSession, void>
		inline _post_connect(
			error_code ec, std::unique_ptr<endpoints_type> eps, endpoints_iterator iter,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(derive.io().running_in_this_thread());

			state_t expected = state_t::starting;
			if (!derive.state_.compare_exchange_strong(expected, state_t::starting))
			{
				// There are no more endpoints. Shut down the client.
				derive._handle_connect(asio::error::operation_aborted,
					std::move(this_ptr), std::move(ecs), std::move(chain));

				return;
			}

			if (iter == eps->end())
			{
				// There are no more endpoints. Shut down the client.
				derive._handle_connect(ec ? ec : asio::error::host_unreachable,
					std::move(this_ptr), std::move(ecs), std::move(chain));

				return;
			}

			auto& socket = derive.socket();

			// the socket.open(...) must after async_resolve, beacuse only after async_resolve,
			// we can get the remote endpoint is ipv4 or ipv6, then we can open the socket
			// with ipv4 or ipv6 correctly.

			error_code ec_ignore{};

			// if the socket's binded endpoint is ipv4, and the finded endpoint is ipv6, then
			// the async_connect will be failed, so we need reopen the socket with ipv6.
			if (socket.is_open())
			{
				auto oldep = socket.local_endpoint(ec_ignore);

				if (ec_ignore || oldep.protocol() != iter->endpoint().protocol())
				{
					socket.cancel(ec_ignore);
					socket.close(ec_ignore);
				}
			}

			if (!socket.is_open())
			{
				socket.open(iter->endpoint().protocol(), ec);

				if (ec)
				{
					derive._handle_connect(ec, std::move(this_ptr), std::move(ecs), std::move(chain));

					return;
				}

				// set port reuse
				socket.set_option(asio::socket_base::reuse_address(true), ec_ignore);

				// open succeeded. set the keeplive values
				if constexpr (std::is_same_v<typename lowest_layer_t::protocol_type, asio::ip::tcp>)
				{
					derive.set_keep_alive_options();
				}
				else
				{
					std::ignore = true;
				}

				// We don't call the bind function, beacuse it will be called internally in asio
				//socket.bind(endpoint);

				// And you can call the bind function youself in the bind_init() callback.
				// eg:
				// 
				// asio::ip::tcp::endpoint ep(asio::ip::tcp::v4(), 1234);
				// asio::ip::udp::endpoint ep(asio::ip::udp::v6(), 9876);
				// asio::ip::tcp::endpoint ep(asio::ip::make_address("0.0.0.0"), 1234);
				// 
				// client.socket().bind(ep);

				// maybe there has multi endpoints, and connect the first endpoint failed, then the
				// next endpoint is connected, at this time, the ec and get_last_error maybe not 0,
				// so we need clear the last error, otherwise if use call get_last_error in the
				// fire init function, the get_last_error will be not 0.
				clear_last_error();

				derive._fire_init();
			}
			else
			{
				ASIO2_LOG(spdlog::level::err, "The client socket is opened already.");
			}

			// Start the asynchronous connect operation.
			socket.async_connect(iter->endpoint(), make_allocator(derive.rallocator(),
			[&derive, this_ptr = std::move(this_ptr), ecs = std::move(ecs), chain = std::move(chain),
				eps = std::move(eps), iter]
			(const error_code & ec) mutable
			{
				if (ec && ec != asio::error::operation_aborted)
					derive._post_connect(ec, std::move(eps), ++iter,
						std::move(this_ptr), std::move(ecs), std::move(chain));
				else
					derive._post_proxy(ec,
						std::move(this_ptr), std::move(ecs), std::move(chain));
			}));
		}

		template<typename C, typename DeferEvent>
		inline void _post_proxy(
			const error_code& ec,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			set_last_error(ec);

			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(derive.io().running_in_this_thread());

			state_t expected = state_t::starting;
			if (!derive.state_.compare_exchange_strong(expected, state_t::starting))
			{
				derive._handle_connect(asio::error::operation_aborted,
					std::move(this_ptr), std::move(ecs), std::move(chain));

				return;
			}

			if constexpr (std::is_base_of_v<component_tag, detail::remove_cvref_t<C>>)
			{
				if (ec)
				{
					return derive._handle_proxy(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
				}

				//// Traverse each component in order, and if it is a proxy component, start it
				//detail::for_each_tuple(ecs->get_component().values(), [&](auto& component) mutable
				//{
				//	using type = detail::remove_cvref_t<decltype(component)>;

				//	if constexpr (std::is_base_of_v<asio2::socks5::option_base, type>)
				//		derive._socks5_start(std::move(this_ptr), std::move(ecs));
				//	else
				//	{
				//		std::ignore = true;
				//	}
				//});

				if constexpr (C::has_socks5())
					derive._socks5_start(std::move(this_ptr), std::move(ecs), std::move(chain));
				else
					derive._handle_proxy(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
			}
			else
			{
				derive._handle_proxy(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
			}
		}

		template<typename C, typename DeferEvent>
		inline void _handle_proxy(
			const error_code& ec,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			set_last_error(ec);

			derived_t& derive = static_cast<derived_t&>(*this);

			derive._handle_connect(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
		}

		template<typename C, typename DeferEvent>
		inline void _handle_connect(
			const error_code& ec,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			set_last_error(ec);

			derived_t& derive = static_cast<derived_t&>(*this);

			if constexpr (args_t::is_session)
			{
				ASIO2_ASSERT(derive.sessions().io().running_in_this_thread());
			}
			else
			{
				ASIO2_ASSERT(derive.io().running_in_this_thread());
			}

			derive._done_connect(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
		}

		template<typename C, typename DeferEvent>
		inline void _done_connect(
			error_code ec, std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// code run to here, the state must not be stopped( stopping is possible but stopped is impossible )
			//ASIO2_ASSERT(derive.state() != state_t::stopped);

			if constexpr (args_t::is_session)
			{
				ASIO2_ASSERT(derive.sessions().io().running_in_this_thread());

				// if socket is invalid, it means that the connect is timeout and the socket has
				// been closed by the connect timeout timer, so reset the error to timed_out.
				if (!derive.socket().is_open())
				{
					ec = asio::error::timed_out;
				}
			}
			else
			{
				ASIO2_ASSERT(derive.io().running_in_this_thread());

				// if connect_timeout_timer_ is empty, it means that the connect timeout timer is
				// timeout and the callback has called already, so reset the error to timed_out.
				// note : when the async_resolve is failed, the socket is invalid to.
				if (!derive.connect_timeout_timer_)
				{
					ec = asio::error::timed_out;
				}
			}

			state_t expected;

			// Set the state to started before fire_connect because the user may send data in
			// fire_connect and fail if the state is not set to started.
			if (!ec)
			{
				expected = state_t::starting;
				if (!derive.state().compare_exchange_strong(expected, state_t::started))
					ec = asio::error::operation_aborted;
			}

			// set last error before fire connect
			set_last_error(ec);

			// Is session : Only call fire_connect notification when the connection is succeed.
			if constexpr (args_t::is_session)
			{
				ASIO2_ASSERT(derive.sessions().io().running_in_this_thread());

				if (!ec)
				{
					expected = state_t::started;
					if (derive.state().compare_exchange_strong(expected, state_t::started))
					{
						derive._fire_connect(this_ptr, ecs);
					}
				}
			}
			// Is client : Whether the connection succeeds or fails, always call fire_connect notification
			else
			{
				ASIO2_ASSERT(derive.io().running_in_this_thread());

				// if state is not stopped, call _fire_connect
				expected = state_t::stopped;
				if (!derive.state().compare_exchange_strong(expected, state_t::stopped))
				{
					derive._fire_connect(this_ptr, ecs);
				}
			}

			if (!ec)
			{
				expected = state_t::started;
				if (!derive.state().compare_exchange_strong(expected, state_t::started))
					ec = asio::error::operation_aborted;
			}

			// Whatever of connection success or failure or timeout, cancel the timeout timer.
			derive._stop_connect_timeout_timer();

			// must set last error again, beacuse in the _fire_connect some errors maybe occured.
			set_last_error(ec);

			if (ec)
			{
				if constexpr (args_t::is_session)
				{
					derive._do_disconnect(ec, std::move(this_ptr), std::move(chain));
				}
				else
				{
					// can't pass the whole chain to the _do_disconnect, it will cause the auto
					// reconnect has no effect.
					derive._do_disconnect(ec, std::move(this_ptr), defer_event(chain.move_guard()));
				}

				return;
			}

			derive._do_start(std::move(this_ptr), std::move(ecs), std::move(chain));
		}
	};
}

#endif // !__ASIO2_CONNECT_COMPONENT_HPP__
