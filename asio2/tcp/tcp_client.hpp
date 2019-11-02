/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_TCP_CLIENT_HPP__
#define __ASIO2_TCP_CLIENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/client.hpp>

#include <asio2/base/detail/condition_wrap.hpp>
#include <asio2/tcp/component/tcp_keepalive_cp.hpp>
#include <asio2/tcp/impl/tcp_send_op.hpp>
#include <asio2/tcp/impl/tcp_recv_op.hpp>

namespace asio2::detail
{
	template<class derived_t, class socket_t, class buffer_t>
	class tcp_client_impl_t
		: public client_impl_t<derived_t, socket_t, buffer_t>
		, public tcp_keepalive_cp<socket_t>
		, public tcp_send_op<derived_t, false>
		, public tcp_recv_op<derived_t, false>
	{
		template <class, bool>                friend class user_timer_cp;
		template <class, bool>                friend class connect_timeout_cp;
		template <class, class>               friend class connect_cp;
		template <class, bool>                friend class send_queue_cp;
		template <class, bool>                friend class send_cp;
		template <class, bool>                friend class tcp_send_op;
		template <class, bool>                friend class tcp_recv_op;
		template <class, class, class>        friend class client_impl_t;

	public:
		using self = tcp_client_impl_t<derived_t, socket_t, buffer_t>;
		using super = client_impl_t<derived_t, socket_t, buffer_t>;
		using buffer_type = buffer_t;
		using super::send;

	public:
		/**
		 * @constructor
		 */
		explicit tcp_client_impl_t(
			std::size_t init_buffer_size = tcp_frame_size,
			std::size_t max_buffer_size = (std::numeric_limits<std::size_t>::max)()
		)
			: super(1, init_buffer_size, max_buffer_size)
			, tcp_keepalive_cp<socket_t>(this->socket_)
			, tcp_send_op<derived_t, false>()
			, tcp_recv_op<derived_t, false>()
		{
		}

		/**
		 * @destructor
		 */
		~tcp_client_impl_t()
		{
			this->stop();
			this->iopool_.stop();
		}

		/**
		 * @function : start the client, blocking connect to server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename StrOrInt>
		bool start(std::string_view host, StrOrInt&& port)
		{
			return this->derived().template _do_connect<false>(host, to_string_port(std::forward<StrOrInt>(port)),
				condition_wrap<asio::detail::transfer_at_least_t>{asio::transfer_at_least(1)});
		}

		/**
		 * @function : start the client, blocking connect to server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 * @param condition The delimiter condition.Valid value types include the following:
		 * char,std::string,std::string_view,
		 * function:std::pair<iterator, bool> match_condition(iterator begin, iterator end),
		 * asio::transfer_at_least,asio::transfer_exactly
		 * more details see asio::read_until
		 */
		template<typename StrOrInt, typename MatchCondition>
		bool start(std::string_view host, StrOrInt&& port, MatchCondition condition)
		{
			return this->derived().template _do_connect<false>(host, to_string_port(std::forward<StrOrInt>(port)),
				condition_wrap<MatchCondition>(condition));
		}

		/**
		 * @function : start the client, asynchronous connect to server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt>
		void async_start(String&& host, StrOrInt&& port)
		{
			asio::post(this->io_.strand(), [this, h = to_string_host(std::forward<String>(host)),
				p = to_string_port(std::forward<StrOrInt>(port))]()
			{
				this->derived().template _do_connect<true>(h, p,
					condition_wrap<asio::detail::transfer_at_least_t>{asio::transfer_at_least(1)});
			});
		}

		/**
		 * @function : start the client, asynchronous connect to server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 * @param condition The delimiter condition.Valid value types include the following:
		 * char,std::string,std::string_view,
		 * function:std::pair<iterator, bool> match_condition(iterator begin, iterator end),
		 * asio::transfer_at_least,asio::transfer_exactly
		 * more details see asio::read_until
		 */
		template<typename String, typename StrOrInt, typename MatchCondition>
		void async_start(String&& host, StrOrInt&& port, MatchCondition condition)
		{
			asio::post(this->io_.strand(), [this, h = to_string_host(std::forward<String>(host)),
				p = to_string_port(std::forward<StrOrInt>(port)), condition]()
			{
				this->derived().template _do_connect<true>(h, p,
					condition_wrap<MatchCondition>(condition));
			});
		}

		/**
		 * @function : stop the client
		 * You can call this function on the communication thread and anywhere to stop the client.
		 */
		inline void stop()
		{
			// Asio end socket functions: cancel, shutdown, close, release :
			// https://stackoverflow.com/questions/51468848/asio-end-socket-functions-cancel-shutdown-close-release
			// The proper steps are:
			// 1.Call shutdown() to indicate that you will not write any more data to the socket.
			// 2.Continue to (async-) read from the socket until you get either an error or the connection is closed.
			// 3.Now close() the socket (in the async read handler).
			// If you don't do this, you may end up closing the connection while the other side is still sending data.
			// This will result in an ungraceful close.
			this->derived()._do_stop(asio::error::operation_aborted);

			this->iopool_.wait_iothreads();
		}

	public:
		/**
		 * @function : bind recv listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * Function signature : void(std::string_view s)
		 */
		template<class F, class ...C>
		inline derived_t & bind_recv(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::recv,
				observer_t<std::string_view>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

		/**
		 * @function : bind connect listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * This notification is called after the client connection completed, whether successful or unsuccessful
		 * Function signature : void(asio2::error_code ec)
		 */
		template<class F, class ...C>
		inline derived_t & bind_connect(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::connect, observer_t<error_code>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

		/**
		 * @function : bind disconnect listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * This notification is called before the client is ready to disconnect
		 * Function signature : void(asio2::error_code ec)
		 */
		template<class F, class ...C>
		inline derived_t & bind_disconnect(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::disconnect, observer_t<error_code>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

		/**
		 * @function : bind init listener,we should set socket options at here
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * Function signature : void()
		 */
		template<class F, class ...C>
		inline derived_t & bind_init(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::init, observer_t<>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	protected:
		template<bool isAsync, typename MatchCondition>
		bool _do_connect(std::string_view host, std::string_view port, condition_wrap<MatchCondition> condition)
		{
			state_t expected = state_t::stopped;
			if (!this->state_.compare_exchange_strong(expected, state_t::starting))
			{
				set_last_error(asio::error::already_started);
				return false;
			}

			try
			{
				clear_last_error();

				this->once_flag_ = std::make_unique<std::once_flag>();

				this->derived()._do_init(condition);

				super::start();

				return this->derived().template _start_connect<isAsync>(host, port, std::shared_ptr<derived_t>{}, condition);
			}
			catch (system_error & e)
			{
				set_last_error(e);
				this->derived().template _handle_connect<isAsync>(e.code(), std::shared_ptr<derived_t>{}, condition);
				this->derived()._do_stop(e.code());
			}
			return false;
		}

		template<typename MatchCondition>
		inline void _do_init(condition_wrap<MatchCondition>)
		{
			if constexpr (std::is_same_v<MatchCondition, use_dgram_t>)
				this->dgram_ = true;
			else
				this->dgram_ = false;
		}

		template<typename MatchCondition>
		inline void _do_start(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			this->reset_active_time();
			this->reset_connect_time();

			this->derived()._start_recv(std::move(this_ptr), std::move(condition));
		}

		inline void _do_stop(const error_code& ec)
		{
			state_t expected = state_t::starting;
			if (this->state_.compare_exchange_strong(expected, state_t::stopping))
				return this->derived()._post_stop(ec, std::shared_ptr<derived_t>{}, expected);

			expected = state_t::started;
			if (this->state_.compare_exchange_strong(expected, state_t::stopping))
				return this->derived()._post_stop(ec, std::shared_ptr<derived_t>{}, expected);
		}

		inline void _post_stop(const error_code& ec, std::shared_ptr<derived_t> self_ptr, state_t old_state)
		{
			// All pending sending events will be cancelled after enter the send strand below.
			asio::post(this->io_.strand(), [this, ec, this_ptr = std::move(self_ptr), old_state]()
			{
				set_last_error(ec);

				state_t expected = state_t::stopping;
				if (this->state_.compare_exchange_strong(expected, state_t::stopped))
				{
					// close the socket by post a event
					// asio don't allow operate the same socket in multi thread,if you close socket in one thread and another thread is 
					// calling socket's async_... function,it will crash.so we must care for operate the socket.when need close the
					// socket ,we use the strand to post a event,make sure the socket's close operation is in the same thread.
					this->derived()._fire_disconnect(this_ptr, ec);

					// call the base class stop function
					super::stop();

					// call CRTP polymorphic stop
					this->derived()._handle_stop(ec, std::move(this_ptr));
				}
				else
				{
					ASIO2_ASSERT(false);
				}
			});
		}

		inline void _handle_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			detail::ignore::unused(ec, this_ptr);

			// call socket's close function to notify the _handle_recv function response with 
			// error > 0 ,then the socket can get notify to exit
			// Call shutdown() to indicate that you will not write any more data to the socket.
			this->socket_.lowest_layer().shutdown(asio::socket_base::shutdown_both, ec_ignore);
			// Call close,otherwise the _handle_recv will never return
			this->socket_.lowest_layer().close(ec_ignore);
		}

		template<typename MatchCondition>
		inline void _start_recv(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			// Connect succeeded. post recv request.
			asio::post(this->io_.strand(), [this, self_ptr = std::move(this_ptr), condition]()
			{
				this->derived()._post_recv(std::move(self_ptr), std::move(condition));
			});
		}

		template<class Data, class Callback>
		inline bool _do_send(Data& data, Callback&& callback)
		{
			return this->derived()._tcp_send(data, std::forward<Callback>(callback));
		}

	protected:
		template<typename MatchCondition>
		inline void _post_recv(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			this->derived()._tcp_post_recv(std::move(this_ptr), std::move(condition));
		}

		template<typename MatchCondition>
		inline void _handle_recv(const error_code & ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			this->derived()._tcp_handle_recv(ec, bytes_recvd, std::move(this_ptr), std::move(condition));
		}

		inline void _fire_init()
		{
			this->listener_.notify(event::init);
		}

		inline void _fire_recv(detail::ignore, std::string_view s)
		{
			this->listener_.notify(event::recv, s);
		}

		inline void _fire_connect(detail::ignore, error_code ec)
		{
			this->listener_.notify(event::connect, ec);
		}

		inline void _fire_disconnect(detail::ignore, error_code ec)
		{
			this->listener_.notify(event::disconnect, ec);
		}

	protected:
		bool dgram_ = false;
	};
}

namespace asio2
{
	class tcp_client : public detail::tcp_client_impl_t<tcp_client, asio::ip::tcp::socket, asio::streambuf>
	{
	public:
		using tcp_client_impl_t<tcp_client, asio::ip::tcp::socket, asio::streambuf>::tcp_client_impl_t;
	};
}

#endif // !__ASIO2_TCP_CLIENT_HPP__
