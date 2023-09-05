/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_TCP_CLIENT_HPP__
#define __ASIO2_TCP_CLIENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <asio2/base/client.hpp>

#include <asio2/tcp/impl/tcp_keepalive_cp.hpp>
#include <asio2/tcp/impl/tcp_send_op.hpp>
#include <asio2/tcp/impl/tcp_recv_op.hpp>

namespace asio2::detail
{
	struct template_args_tcp_client : public tcp_tag
	{
		static constexpr bool is_session = false;
		static constexpr bool is_client  = true;
		static constexpr bool is_server  = false;

		using socket_t    = asio::ip::tcp::socket;
		using buffer_t    = asio::streambuf;
		using send_data_t = std::string_view;
		using recv_data_t = std::string_view;
	};

	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_CLIENT;

	template<class derived_t, class args_t = template_args_tcp_client>
	class tcp_client_impl_t
		: public client_impl_t         <derived_t, args_t>
		, public tcp_keepalive_cp      <derived_t, args_t>
		, public tcp_send_op           <derived_t, args_t>
		, public tcp_recv_op           <derived_t, args_t>
		, public tcp_tag
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT;

	public:
		using super = client_impl_t    <derived_t, args_t>;
		using self  = tcp_client_impl_t<derived_t, args_t>;

		using args_type   = args_t;
		using buffer_type = typename args_t::buffer_t;
		using send_data_t = typename args_t::send_data_t;
		using recv_data_t = typename args_t::recv_data_t;

	public:
		/**
		 * @brief constructor
		 */
		explicit tcp_client_impl_t(
			std::size_t init_buf_size = tcp_frame_size,
			std::size_t  max_buf_size = max_buffer_size,
			std::size_t   concurrency = 1
		)
			: super(init_buf_size, max_buf_size, concurrency)
			, tcp_keepalive_cp<derived_t, args_t>()
			, tcp_send_op     <derived_t, args_t>()
			, tcp_recv_op     <derived_t, args_t>()
		{
			this->set_connect_timeout(std::chrono::milliseconds(tcp_connect_timeout));
		}

		template<class Scheduler, std::enable_if_t<!std::is_integral_v<detail::remove_cvref_t<Scheduler>>, int> = 0>
		explicit tcp_client_impl_t(
			std::size_t init_buf_size,
			std::size_t  max_buf_size,
			Scheduler&& scheduler
		)
			: super(init_buf_size, max_buf_size, std::forward<Scheduler>(scheduler))
			, tcp_keepalive_cp<derived_t, args_t>()
			, tcp_send_op     <derived_t, args_t>()
			, tcp_recv_op     <derived_t, args_t>()
		{
			this->set_connect_timeout(std::chrono::milliseconds(tcp_connect_timeout));
		}

		template<class Scheduler, std::enable_if_t<!std::is_integral_v<detail::remove_cvref_t<Scheduler>>, int> = 0>
		explicit tcp_client_impl_t(Scheduler&& scheduler)
			: tcp_client_impl_t(tcp_frame_size, max_buffer_size, std::forward<Scheduler>(scheduler))
		{
		}

		// -- Support initializer_list causes the code of inherited classes to be not concised

		//template<class Scheduler, std::enable_if_t<!std::is_integral_v<detail::remove_cvref_t<Scheduler>>, int> = 0>
		//explicit tcp_client_impl_t(
		//	std::size_t init_buf_size,
		//	std::size_t  max_buf_size,
		//	std::initializer_list<Scheduler> scheduler
		//)
		//	: tcp_client_impl_t(init_buf_size, max_buf_size, std::vector<Scheduler>{std::move(scheduler)})
		//{
		//}

		//template<class Scheduler, std::enable_if_t<!std::is_integral_v<detail::remove_cvref_t<Scheduler>>, int> = 0>
		//explicit tcp_client_impl_t(std::initializer_list<Scheduler> scheduler)
		//	: tcp_client_impl_t(tcp_frame_size, max_buffer_size, std::move(scheduler))
		//{
		//}

		/**
		 * @brief destructor
		 */
		~tcp_client_impl_t()
		{
			this->stop();
		}

		/**
		 * @brief start the client, blocking connect to server
		 * @param host - A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port - A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 * @param args - The delimiter condition.Valid value types include the following:
		 * char,std::string,std::string_view,
		 * function:std::pair<iterator, bool> match_condition(iterator begin, iterator end),
		 * asio::transfer_at_least,asio::transfer_exactly
		 * more details see asio::read_until
		 */
		template<typename String, typename StrOrInt, typename... Args>
		inline bool start(String&& host, StrOrInt&& port, Args&&... args)
		{
			return this->derived().template _do_connect<false>(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				ecs_helper::make_ecs(asio::transfer_at_least(1), std::forward<Args>(args)...));
		}

		/**
		 * @brief start the client, asynchronous connect to server
		 * @param host - A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port - A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 * @param args - The delimiter condition.Valid value types include the following:
		 * char,std::string,std::string_view,
		 * function:std::pair<iterator, bool> match_condition(iterator begin, iterator end),
		 * asio::transfer_at_least,asio::transfer_exactly
		 * more details see asio::read_until
		 */
		template<typename String, typename StrOrInt, typename... Args>
		inline bool async_start(String&& host, StrOrInt&& port, Args&&... args)
		{
			return this->derived().template _do_connect<true>(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				ecs_helper::make_ecs(asio::transfer_at_least(1), std::forward<Args>(args)...));
		}

		/**
		 * @brief stop the client
		 * You can call this function in the communication thread and anywhere to stop the client.
		 * If this function is called in the communication thread, it will post a asynchronous
		 * event into the event queue, then return immediately.
		 * If this function is called not in the communication thread, it will blocking forever 
		 * util the client is stopped completed.
		 */
		inline void stop()
		{
			if (this->is_iopool_stopped())
				return;

			derived_t& derive = this->derived();

			derive.io_->unregobj(&derive);

			// use promise to get the result of stop
			std::promise<state_t> promise;
			std::future<state_t> future = promise.get_future();

			// use derfer to ensure the promise's value must be seted.
			detail::defer_event pg
			{
				[this, p = std::move(promise)]() mutable
				{
					p.set_value(this->state_.load());
				}
			};

			// if user call stop in the recv callback, use post event to executed a async event.
			derive.post_event([&derive, this_ptr = derive.selfptr(), pg = std::move(pg)]
			(event_queue_guard<derived_t> g) mutable
			{
				// first close the reconnect timer
				derive._stop_reconnect_timer();

				derive._do_disconnect(asio::error::operation_aborted, derive.selfptr(), defer_event
				{
					[&derive, this_ptr = std::move(this_ptr), pg = std::move(pg)]
					(event_queue_guard<derived_t> g) mutable
					{
						derive._do_stop(asio::error::operation_aborted, std::move(this_ptr), defer_event
						{
							[pg = std::move(pg)](event_queue_guard<derived_t> g) mutable
							{
								detail::ignore_unused(pg, g);

								// the "pg" should destroyed before the "g", otherwise if the "g"
								// is destroyed before "pg", the next event maybe called, then the
								// state maybe change to not stopped.
								{
									[[maybe_unused]] detail::defer_event t{ std::move(pg) };
								}
							}, std::move(g)
						});
					}, std::move(g)
				});
			});

			// use this to ensure the client is stopped completed when the stop is called not in
			// the io_context thread
			while (!derive.running_in_this_thread())
			{
				std::future_status status = future.wait_for(std::chrono::milliseconds(100));

				if (status == std::future_status::ready)
				{
					ASIO2_ASSERT(future.get() == state_t::stopped);
					break;
				}
				else
				{
					if (derive.get_thread_id() == std::thread::id{})
						break;

					if (derive.io_->context().stopped())
						break;
				}
			}

			this->stop_iopool();
		}

	public:
		/**
		 * @brief bind recv listener
		 * @param fun - a user defined callback function.
		 * @param obj - a pointer or reference to a class object, this parameter can be none.
		 * @li if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or reference.
		 * Function signature : void(std::string_view data)
		 */
		template<class F, class ...C>
		inline derived_t & bind_recv(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::recv,
				observer_t<std::string_view>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

		/**
		 * @brief bind connect listener
		 * @param fun - a user defined callback function.
		 * @param obj - a pointer or reference to a class object, this parameter can be none.
		 * @li if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or reference.
		 * This notification is called after the client connection completed, whether successful or unsuccessful
		 * Function signature : void()
		 */
		template<class F, class ...C>
		inline derived_t & bind_connect(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::connect,
				observer_t<>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

		/**
		 * @brief bind disconnect listener
		 * @param fun - a user defined callback function.
		 * @param obj - a pointer or reference to a class object, this parameter can be none.
		 * @li if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or reference.
		 * This notification is called before the client is ready to disconnect
		 * Function signature : void()
		 */
		template<class F, class ...C>
		inline derived_t & bind_disconnect(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::disconnect,
				observer_t<>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

		/**
		 * @brief bind init listener,we should set socket options at here
		 * @param fun - a user defined callback function.
		 * @param obj - a pointer or reference to a class object, this parameter can be none.
		 * @li if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or reference.
		 * Function signature : void()
		 */
		template<class F, class ...C>
		inline derived_t & bind_init(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::init,
				observer_t<>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	protected:
		template<bool IsAsync, typename String, typename StrOrInt, typename C>
		inline bool _do_connect(String&& host, StrOrInt&& port, std::shared_ptr<ecs_t<C>> ecs)
		{
			derived_t& derive = this->derived();

			// if log is enabled, init the log first, otherwise when "Too many open files" error occurs,
			// the log file will be created failed too.
		#if defined(ASIO2_ENABLE_LOG)
			asio2::detail::get_logger();
		#endif

			this->start_iopool();

			if (!this->is_iopool_started())
			{
				set_last_error(asio::error::operation_aborted);
				return false;
			}

			asio::dispatch(derive.io_->context(), [&derive, this_ptr = derive.selfptr()]() mutable
			{
				detail::ignore_unused(this_ptr);

				// init the running thread id 
				derive.io_->init_thread_id();
			});

			// use promise to get the result of async connect
			std::promise<error_code> promise;
			std::future<error_code> future = promise.get_future();

			// use derfer to ensure the promise's value must be seted.
			detail::defer_event pg
			{
				[promise = std::move(promise)]() mutable
				{
					promise.set_value(get_last_error());
				}
			};

			// if user call start in the recv callback, use post event to executed a async event.
			derive.post_event(
			[this, this_ptr = derive.selfptr(), ecs = std::move(ecs),
				host = std::forward<String>(host), port = std::forward<StrOrInt>(port), pg = std::move(pg)]
			(event_queue_guard<derived_t> g) mutable
			{
				derived_t& derive = this->derived();

				defer_event chain
				{
					[pg = std::move(pg)] (event_queue_guard<derived_t> g) mutable
					{
						detail::ignore_unused(pg, g);

						// the "pg" should destroyed before the "g", otherwise if the "g"
						// is destroyed before "pg", the next event maybe called, then the
						// state maybe change to not stopped.
						{
							[[maybe_unused]] detail::defer_event t{ std::move(pg) };
						}
					}, std::move(g)
				};

				state_t expected = state_t::stopped;
				if (!derive.state_.compare_exchange_strong(expected, state_t::starting))
				{
					// if the state is not stopped, set the last error to already_started
					set_last_error(asio::error::already_started);

					return;
				}

				// must read/write ecs in the io_context thread.
				derive.ecs_ = ecs;

				clear_last_error();

				derive.io_->regobj(&derive);

			#if defined(_DEBUG) || defined(DEBUG)
				this->is_stop_reconnect_timer_called_ = false;
				this->is_post_reconnect_timer_called_ = false;
				this->is_stop_connect_timeout_timer_called_ = false;
				this->is_disconnect_called_ = false;
			#endif

				// convert to string maybe throw some exception.
				this->host_ = detail::to_string(std::move(host));
				this->port_ = detail::to_string(std::move(port));

				super::start();

				derive._do_init(ecs);

				// ecs init
				derive._rdc_init(ecs);
				derive._socks5_init(ecs);

				derive.template _start_connect<IsAsync>(std::move(this_ptr), std::move(ecs), std::move(chain));
			});

			if constexpr (IsAsync)
			{
				set_last_error(asio::error::in_progress);

				return true;
			}
			else
			{
				if (!derive.io_->running_in_this_thread())
				{
					set_last_error(future.get());

					// beacuse here code is running in the user thread, not in the io_context thread,
					// so, even if the client is start successed, but if the server disconnect this
					// client after connect success, and when code run to here, the client's state
					// maybe stopping, so if we return derive.is_started();, the return value maybe 
					// false, but we did connect to the server is successfully.
					return static_cast<bool>(!get_last_error());
				}
				else
				{
					set_last_error(asio::error::in_progress);
				}

				// if the state is stopped , the return value is "is_started()".
				// if the state is stopping, the return value is false, the last error is already_started
				// if the state is starting, the return value is false, the last error is already_started
				// if the state is started , the return value is true , the last error is already_started
				return derive.is_started();
			}
		}

		template<typename C>
		inline void _do_init(std::shared_ptr<ecs_t<C>>&) noexcept
		{
		#if defined(ASIO2_ENABLE_LOG)
			// Used to test whether the behavior of different compilers is consistent
			static_assert(tcp_send_op<derived_t, args_t>::template has_member_dgram<self>::value,
				"The behavior of different compilers is not consistent");
		#endif

			if constexpr (std::is_same_v<typename ecs_t<C>::condition_lowest_type, use_dgram_t>)
				this->dgram_ = true;
			else
				this->dgram_ = false;
		}

		template<typename C, typename DeferEvent>
		inline void _do_start(
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			this->derived().update_alive_time();
			this->derived().reset_connect_time();

			this->derived()._start_recv(std::move(this_ptr), std::move(ecs), std::move(chain));
		}

		template<typename DeferEvent>
		inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			ASIO2_ASSERT(this->derived().io_->running_in_this_thread());

			ASIO2_ASSERT(this->state_ == state_t::stopped);

			ASIO2_LOG_DEBUG("tcp_client::_handle_disconnect: {} {}", ec.value(), ec.message());

			this->derived()._rdc_stop();

			// should we close the socket in handle disconnect function? otherwise when send
			// data failed, will cause the _do_disconnect function be called, then cause the
			// auto reconnect executed, and then the _post_recv will be return with some error,
			// and the _post_recv will cause the auto reconnect executed again.

			// can't use push event to close the socket, beacuse when used with websocket,
			// the websocket's async_close will be called, and the chain will passed into
			// the async_close, but the async_close will cause the chain interrupted, and
			// we don't know when the async_close will be completed, if another push event
			// was called during async_close executing, then here push event will after 
			// the another event in the queue.

			// call shutdown again, beacuse the do shutdown maybe not called, eg: when
			// protocol error is checked in the mqtt or http, then the do disconnect 
			// maybe called directly.
			// the socket maybe closed already in the connect timeout timer.
			if (this->socket().is_open())
			{
				error_code ec_linger{}, ec_ignore{};

				asio::socket_base::linger lnger{};

				this->socket().lowest_layer().get_option(lnger, ec_linger);

				// call socket's close function to notify the _handle_recv function response with 
				// error > 0 ,then the socket can get notify to exit
				// Call shutdown() to indicate that you will not write any more data to the socket.
				if (!ec_linger && !(lnger.enabled() == true && lnger.timeout() == 0))
				{
					this->socket().shutdown(asio::socket_base::shutdown_both, ec_ignore);
				}

				// if the socket is basic_stream with rate limit, we should call the cancel,
				// otherwise the rate timer maybe can't canceled, and cause the io_context
				// can't stopped forever, even if the socket is closed already.
				this->socket().cancel(ec_ignore);

				// Call close,otherwise the _handle_recv will never return
				this->socket().close(ec_ignore);
			}

			super::_handle_disconnect(ec, std::move(this_ptr), std::move(chain));
		}

		template<typename DeferEvent>
		inline void _do_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			// When use call client.stop in the io_context thread, then the iopool is not stopped,
			// but this client is stopped, When client.stop is called again in the not io_context
			// thread, then this client state is stopped.
			ASIO2_ASSERT(this->state_ == state_t::stopped);

			this->derived()._post_stop(ec, std::move(this_ptr), std::move(chain));
		}

		template<typename DeferEvent>
		inline void _post_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			// All pending sending events will be cancelled after enter the callback below.
			this->derived().disp_event([this, ec, this_ptr = std::move(this_ptr), e = chain.move_event()]
			(event_queue_guard<derived_t> g) mutable
			{
				set_last_error(ec);

				defer_event chain(std::move(e), std::move(g));

				// call the base class stop function
				super::stop();

				// call CRTP polymorphic stop
				this->derived()._handle_stop(ec, std::move(this_ptr), std::move(chain));
			}, chain.move_guard());
		}

		template<typename DeferEvent>
		inline void _handle_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			detail::ignore_unused(ec, this_ptr, chain);

			this->derived()._socks5_stop();

			ASIO2_ASSERT(this->state_ == state_t::stopped);
		}

		template<typename C, typename DeferEvent>
		inline void _start_recv(
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			// Connect succeeded. post recv request.
			asio::dispatch(this->derived().io_->context(), make_allocator(this->derived().wallocator(),
			[this, this_ptr = std::move(this_ptr), ecs = std::move(ecs), chain = std::move(chain)]
			() mutable
			{
				using condition_lowest_type = typename ecs_t<C>::condition_lowest_type;

				detail::ignore_unused(chain);

				if constexpr (!std::is_same_v<condition_lowest_type, asio2::detail::hook_buffer_t>)
				{
					this->derived().buffer().consume(this->derived().buffer().size());
				}
				else
				{
					std::ignore = true;
				}

				this->derived()._post_recv(std::move(this_ptr), std::move(ecs));
			}));
		}

		template<class Data, class Callback>
		inline bool _do_send(Data& data, Callback&& callback)
		{
			return this->derived()._tcp_send(data, std::forward<Callback>(callback));
		}

		template<class Data>
		inline send_data_t _rdc_convert_to_send_data(Data& data) noexcept
		{
			auto buffer = asio::buffer(data);
			return send_data_t{ reinterpret_cast<
				std::string_view::const_pointer>(buffer.data()),buffer.size() };
		}

		template<class Invoker>
		inline void _rdc_invoke_with_none(const error_code& ec, Invoker& invoker)
		{
			if (invoker)
				invoker(ec, send_data_t{}, recv_data_t{});
		}

		template<class Invoker>
		inline void _rdc_invoke_with_recv(const error_code& ec, Invoker& invoker, recv_data_t data)
		{
			if (invoker)
				invoker(ec, send_data_t{}, data);
		}

		template<class Invoker>
		inline void _rdc_invoke_with_send(const error_code& ec, Invoker& invoker, send_data_t data)
		{
			if (invoker)
				invoker(ec, data, recv_data_t{});
		}

	protected:
		template<typename C>
		inline void _post_recv(std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			this->derived()._tcp_post_recv(std::move(this_ptr), std::move(ecs));
		}

		template<typename C>
		inline void _handle_recv(const error_code & ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			this->derived()._tcp_handle_recv(ec, bytes_recvd, std::move(this_ptr), std::move(ecs));
		}

		inline void _fire_init()
		{
			// the _fire_init must be executed in the thread 0.
			ASIO2_ASSERT(this->derived().io_->running_in_this_thread());
			ASIO2_ASSERT(!get_last_error());

			this->listener_.notify(event_type::init);
		}

		template<typename C>
		inline void _fire_recv(
			std::shared_ptr<derived_t>& this_ptr, std::shared_ptr<ecs_t<C>>& ecs, std::string_view data)
		{
			data = detail::call_data_filter_before_recv(this->derived(), data);

			this->listener_.notify(event_type::recv, data);

			this->derived()._rdc_handle_recv(this_ptr, ecs, data);
		}

		template<typename C>
		inline void _fire_connect(std::shared_ptr<derived_t>& this_ptr, std::shared_ptr<ecs_t<C>>& ecs)
		{
			// the _fire_connect must be executed in the thread 0.
			ASIO2_ASSERT(this->derived().io_->running_in_this_thread());

		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(this->is_disconnect_called_ == false);
		#endif

			if (!get_last_error())
			{
				this->derived()._rdc_start(this_ptr, ecs);
			}

			this->listener_.notify(event_type::connect);
		}

		inline void _fire_disconnect(std::shared_ptr<derived_t>& this_ptr)
		{
			// the _fire_disconnect must be executed in the thread 0.
			ASIO2_ASSERT(this->derived().io_->running_in_this_thread());

		#if defined(_DEBUG) || defined(DEBUG)
			this->is_disconnect_called_ = true;
		#endif

			detail::ignore_unused(this_ptr);

			this->listener_.notify(event_type::disconnect);
		}

	protected:
		bool dgram_                = false;

	#if defined(_DEBUG) || defined(DEBUG)
		bool is_disconnect_called_ = false;
	#endif
	};
}

namespace asio2
{
	using tcp_client_args = detail::template_args_tcp_client;

	template<class derived_t, class args_t>
	using tcp_client_impl_t = detail::tcp_client_impl_t<derived_t, args_t>;

	/**
	 * @brief tcp client template class
	 * @throws constructor maybe throw exception "Too many open files" (exception code : 24)
	 * asio::error::no_descriptors - Too many open files
	 */
	template<class derived_t>
	class tcp_client_t : public detail::tcp_client_impl_t<derived_t, detail::template_args_tcp_client>
	{
	public:
		using detail::tcp_client_impl_t<derived_t, detail::template_args_tcp_client>::tcp_client_impl_t;
	};

	/**
	 * @brief tcp client 
	 * If this object is created as a shared_ptr like std::shared_ptr<asio2::tcp_client> client;
	 * you must call the client->stop() manual when exit, otherwise maybe cause memory leaks.
	 * @throws constructor maybe throw exception "Too many open files" (exception code : 24)
	 * asio::error::no_descriptors - Too many open files
	 */
	class tcp_client : public tcp_client_t<tcp_client>
	{
	public:
		using tcp_client_t<tcp_client>::tcp_client_t;
	};
}

#if defined(ASIO2_INCLUDE_RATE_LIMIT)
#include <asio2/tcp/tcp_stream.hpp>
namespace asio2
{
	struct tcp_rate_client_args : public tcp_client_args
	{
		using socket_t = asio2::tcp_stream<asio2::simple_rate_policy>;
	};

	template<class derived_t>
	class tcp_rate_client_t : public asio2::tcp_client_impl_t<derived_t, tcp_rate_client_args>
	{
	public:
		using asio2::tcp_client_impl_t<derived_t, tcp_rate_client_args>::tcp_client_impl_t;
	};

	class tcp_rate_client : public asio2::tcp_rate_client_t<tcp_rate_client>
	{
	public:
		using asio2::tcp_rate_client_t<tcp_rate_client>::tcp_rate_client_t;
	};
}
#endif

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_TCP_CLIENT_HPP__
