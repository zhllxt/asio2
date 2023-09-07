/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_UDP_CAST_HPP__
#define __ASIO2_UDP_CAST_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <cstdint>
#include <memory>
#include <chrono>
#include <functional>
#include <atomic>
#include <string>
#include <string_view>
#include <queue>
#include <any>
#include <future>
#include <tuple>
#include <unordered_map>

#include <asio2/base/iopool.hpp>
#include <asio2/base/listener.hpp>

#include <asio2/base/detail/object.hpp>
#include <asio2/base/detail/allocator.hpp>
#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/buffer_wrap.hpp>
#include <asio2/base/detail/ecs.hpp>

#include <asio2/base/impl/io_context_cp.hpp>
#include <asio2/base/impl/thread_id_cp.hpp>
#include <asio2/base/impl/alive_time_cp.hpp>
#include <asio2/base/impl/user_data_cp.hpp>
#include <asio2/base/impl/socket_cp.hpp>
#include <asio2/base/impl/user_timer_cp.hpp>
#include <asio2/base/impl/post_cp.hpp>
#include <asio2/base/impl/event_queue_cp.hpp>
#include <asio2/base/impl/condition_event_cp.hpp>
#include <asio2/base/impl/connect_cp.hpp>

#include <asio2/base/detail/linear_buffer.hpp>
#include <asio2/udp/impl/udp_send_cp.hpp>
#include <asio2/udp/impl/udp_send_op.hpp>

//#include <asio2/component/socks/socks5_client_cp.hpp>

//#include <asio2/proxy/socks5_client.hpp>

namespace asio2::detail
{
	struct template_args_udp_cast : public udp_tag
	{
		using socket_t = asio::ip::udp::socket;
		using buffer_t = asio2::linear_buffer;

		//template<class derived_t>
		//using socks5_client_t = asio2::socks5_tcp_client_t<derived_t>;

		static constexpr std::size_t function_storage_size = 88;
		static constexpr std::size_t allocator_storage_size = 256;
	};

	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_UDP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_UDP_CLIENT;

	template<class derived_t, class args_t = template_args_udp_cast>
	class udp_cast_impl_t
		: public object_t          <derived_t        >
		, public iopool_cp         <derived_t, args_t>
		, public io_context_cp     <derived_t, args_t>
		, public thread_id_cp      <derived_t, args_t>
		, public event_queue_cp    <derived_t, args_t>
		, public user_data_cp      <derived_t, args_t>
		, public alive_time_cp     <derived_t, args_t>
		, public socket_cp         <derived_t, args_t>
		, public user_timer_cp     <derived_t, args_t>
		, public post_cp           <derived_t, args_t>
		, public condition_event_cp<derived_t, args_t>
		, public udp_send_cp       <derived_t, args_t>
		, public udp_send_op       <derived_t, args_t>
		, public connect_cp_member_variables<derived_t, args_t, false>
		//, public socks5_client_cp  <derived_t, args_t>
		, public udp_tag
		, public cast_tag
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_UDP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_UDP_CLIENT;

	public:
		using super = object_t       <derived_t        >;
		using self  = udp_cast_impl_t<derived_t, args_t>;

		using iopoolcp = iopool_cp   <derived_t, args_t>;

		using args_type   = args_t;
		using buffer_type = typename args_t::buffer_t;

		/**
		 * @brief constructor
		 * @throws maybe throw exception "Too many open files" (exception code : 24)
		 * asio::error::no_descriptors - Too many open files
		 */
		explicit udp_cast_impl_t(
			std::size_t init_buf_size = udp_frame_size,
			std::size_t max_buf_size  = max_buffer_size,
			std::size_t concurrency   = 1
		)
			: super()
			, iopool_cp         <derived_t, args_t>(concurrency)
			, io_context_cp     <derived_t, args_t>(iopoolcp::_get_io(0))
			, event_queue_cp    <derived_t, args_t>()
			, user_data_cp      <derived_t, args_t>()
			, alive_time_cp     <derived_t, args_t>()
			, socket_cp         <derived_t, args_t>(iopoolcp::_get_io(0)->context())
			, user_timer_cp     <derived_t, args_t>()
			, post_cp           <derived_t, args_t>()
			, condition_event_cp<derived_t, args_t>()
			, udp_send_cp       <derived_t, args_t>()
			, udp_send_op       <derived_t, args_t>()
			, rallocator_()
			, wallocator_()
			, listener_  ()
			, buffer_    (init_buf_size, max_buf_size)
		{
		}

		template<class Scheduler, std::enable_if_t<!std::is_integral_v<detail::remove_cvref_t<Scheduler>>, int> = 0>
		explicit udp_cast_impl_t(
			std::size_t init_buf_size,
			std::size_t max_buf_size,
			Scheduler && scheduler
		)
			: super()
			, iopool_cp         <derived_t, args_t>(std::forward<Scheduler>(scheduler))
			, io_context_cp     <derived_t, args_t>(iopoolcp::_get_io(0))
			, event_queue_cp    <derived_t, args_t>()
			, user_data_cp      <derived_t, args_t>()
			, alive_time_cp     <derived_t, args_t>()
			, socket_cp         <derived_t, args_t>(iopoolcp::_get_io(0)->context())
			, user_timer_cp     <derived_t, args_t>()
			, post_cp           <derived_t, args_t>()
			, condition_event_cp<derived_t, args_t>()
			, udp_send_cp       <derived_t, args_t>()
			, udp_send_op       <derived_t, args_t>()
			, rallocator_()
			, wallocator_()
			, listener_  ()
			, buffer_    (init_buf_size, max_buf_size)
		{
		}

		template<class Scheduler, std::enable_if_t<!std::is_integral_v<detail::remove_cvref_t<Scheduler>>, int> = 0>
		explicit udp_cast_impl_t(Scheduler&& scheduler)
			: udp_cast_impl_t(udp_frame_size, max_buffer_size, std::forward<Scheduler>(scheduler))
		{
		}

		/**
		 * @brief destructor
		 */
		~udp_cast_impl_t()
		{
			this->stop();
		}

		/**
		 * @brief start the udp cast
		 * @param host - A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param service - A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt, typename... Args>
		inline bool start(String&& host, StrOrInt&& service, Args&&... args)
		{
		#if defined(ASIO2_ENABLE_LOG)
		#if defined(ASIO2_ALLOCATOR_STORAGE_SIZE)
			static_assert(decltype(rallocator_)::storage_size == ASIO2_ALLOCATOR_STORAGE_SIZE);
			static_assert(decltype(wallocator_)::storage_size == ASIO2_ALLOCATOR_STORAGE_SIZE);
		#else
			static_assert(decltype(rallocator_)::storage_size == args_t::allocator_storage_size);
			static_assert(decltype(wallocator_)::storage_size == args_t::allocator_storage_size);
		#endif
		#endif

			return this->derived().template _do_start<false>(
				std::forward<String>(host), std::forward<StrOrInt>(service),
				ecs_helper::make_ecs('0', std::forward<Args>(args)...));
		}

		/**
		 * @brief start the udp cast
		 * @param host - A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param service - A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt, typename... Args>
		inline bool async_start(String&& host, StrOrInt&& service, Args&&... args)
		{
			return this->derived().template _do_start<true>(
				std::forward<String>(host), std::forward<StrOrInt>(service),
				ecs_helper::make_ecs('0', std::forward<Args>(args)...));
		}

		/**
		 * @brief stop the udp cast
		 * You can call this function in the communication thread and anywhere to stop the udp cast.
		 * If this function is called in the communication thread, it will post a asynchronous
		 * event into the event queue, then return immediately.
		 * If this function is called not in the communication thread, it will blocking forever
		 * util the udp cast is stopped completed.
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

			derive.post_event([&derive, this_ptr = derive.selfptr(), pg = std::move(pg)]
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
			});

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

		/**
		 * @brief destroy the content of all member variables, this is used for solve the memory leaks.
		 * After this function is called, this class object cannot be used again.
		 */
		inline void destroy()
		{
			derived_t& derive = this->derived();

			derive.socket_.reset();
			derive.io_.reset();
			derive.listener_.clear();

			derive.destroy_iopool();
		}

		/**
		 * @brief check whether the udp cast is started
		 */
		inline bool is_started() const
		{
			return (this->state_ == state_t::started && this->socket().is_open());
		}

		/**
		 * @brief check whether the udp cast is stopped
		 */
		inline bool is_stopped() const
		{
			return (this->state_ == state_t::stopped && !this->socket().is_open());
		}

	public:
		/**
		 * @brief bind recv listener
		 * @param fun - a user defined callback function.
		 * @param obj - a pointer or reference to a class object, this parameter can be none.
		 * @li if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or reference.
		 * Function signature : void(asio::ip::udp::endpoint& endpoint, std::string_view data)
		 */
		template<class F, class ...C>
		inline derived_t & bind_recv(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::recv,
				observer_t<asio::ip::udp::endpoint&, std::string_view>(
					std::forward<F>(fun), std::forward<C>(obj)...));
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

		/**
		 * @brief bind start listener
		 * @param fun - a user defined callback function.
		 * @param obj - a pointer or reference to a class object, this parameter can be none.
		 * @li if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or reference.
		 * This notification is called after the server starts up, whether successful or unsuccessful
		 * Function signature : void()
		 */
		template<class F, class ...C>
		inline derived_t & bind_start(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::start, observer_t<>(
				std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

		/**
		 * @brief bind stop listener
		 * @param fun - a user defined callback function.
		 * @param obj - a pointer or reference to a class object, this parameter can be none.
		 * @li if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or reference.
		 * This notification is called before the server is ready to stop
		 * Function signature : void()
		 */
		template<class F, class ...C>
		inline derived_t & bind_stop(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::stop, observer_t<>(
				std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	protected:
		template<bool IsAsync, typename String, typename StrOrInt, typename C>
		bool _do_start(String&& host, StrOrInt&& port, std::shared_ptr<ecs_t<C>> ecs)
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

			derive.post_event(
			[this, this_ptr = derive.selfptr(), ecs = std::move(ecs), pg = std::move(pg),
				host = std::forward<String>(host), port = std::forward<StrOrInt>(port)]
			(event_queue_guard<derived_t> g) mutable
			{
				derived_t& derive = this->derived();

				defer_event chain
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
				};

				state_t expected = state_t::stopped;
				if (!this->state_.compare_exchange_strong(expected, state_t::starting))
				{
					// if the state is not stopped, set the last error to already_started
					set_last_error(asio::error::already_started);

					return;
				}

				// must read/write ecs in the io_context thread.
				derive.ecs_ = ecs;

				derive.io_->regobj(&derive);

			#if defined(_DEBUG) || defined(DEBUG)
				this->is_stop_called_ = false;
			#endif

				// convert to string maybe throw some exception.
				this->host_ = detail::to_string(std::move(host));
				this->port_ = detail::to_string(std::move(port));

				expected = state_t::starting;
				if (!this->state_.compare_exchange_strong(expected, state_t::starting))
				{
					ASIO2_ASSERT(false);
					derive._handle_connect(asio::error::operation_aborted,
						std::move(this_ptr), std::move(ecs), std::move(chain));
					return;
				}

				error_code ec, ec_ignore;

				this->socket().cancel(ec_ignore);
				this->socket().close(ec_ignore);

				// parse address and port
				asio::ip::udp::resolver resolver(this->io_->context());

				auto results = resolver.resolve(this->host_, this->port_,
					asio::ip::resolver_base::flags::passive |
					asio::ip::resolver_base::flags::address_configured, ec);
				if (ec)
				{
					derive._handle_connect(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
					return;
				}
				if (results.empty())
				{
					derive._handle_connect(asio::error::host_not_found,
						std::move(this_ptr), std::move(ecs), std::move(chain));
					return;
				}

				asio::ip::udp::endpoint endpoint = *results.begin();

				this->socket().open(endpoint.protocol(), ec);
				if (ec)
				{
					derive._handle_connect(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
					return;
				}

				// when you close socket in linux system,and start socket
				// immediate,you will get like this "the address is in use",
				// and bind is failed,but i'm suer i close the socket correct
				// already before,why does this happen? the reasion is the
				// socket option "TIME_WAIT",although you close the socket,
				// but the system not release the socket,util 2~4 seconds later,
				// so we can use the SO_REUSEADDR option to avoid this problem,
				// like below

				// set port reuse
				this->socket().set_option(asio::ip::udp::socket::reuse_address(true), ec_ignore);

				//derive._socks5_init(ecs);

				clear_last_error();

				derive._fire_init();

				this->socket().bind(endpoint, ec);
				if (ec)
				{
					derive._handle_connect(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
					return;
				}

				derive._post_proxy(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
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

		template<typename C, typename DeferEvent>
		inline void _post_proxy(
			const error_code& ec,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			set_last_error(ec);

			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(derive.io_->running_in_this_thread());

			//if constexpr (std::is_base_of_v<component_tag, detail::remove_cvref_t<C>>)
			//{
			//	if (ec)
			//		return derive._handle_proxy(ec, std::move(this_ptr), std::move(ecs), std::move(chain));

			//	if constexpr (C::has_socks5())
			//		derive._socks5_start(std::move(this_ptr), std::move(ecs), std::move(chain));
			//	else
			//		derive._handle_proxy(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
			//}
			//else
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

			this->derived()._handle_start(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
		}

		template<typename C, typename DeferEvent>
		inline void _handle_start(
			error_code ec,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			ASIO2_ASSERT(this->derived().io_->running_in_this_thread());

			// Whether the startup succeeds or fails, always call fire_start notification
			state_t expected = state_t::starting;
			if (!ec)
				if (!this->state_.compare_exchange_strong(expected, state_t::started))
					ec = asio::error::operation_aborted;

			set_last_error(ec);

			this->derived()._fire_start();

			expected = state_t::started;
			if (!ec)
				if (!this->state_.compare_exchange_strong(expected, state_t::started))
					ec = asio::error::operation_aborted;

			if (ec)
			{
				this->derived()._do_stop(ec, std::move(this_ptr), std::move(chain));

				return;
			}

			this->buffer_.consume(this->buffer_.size());

			this->derived()._post_recv(std::move(this_ptr), std::move(ecs));
		}

		template<typename E = defer_event<void, derived_t>>
		inline void _do_disconnect(
			const error_code& ec, std::shared_ptr<derived_t> this_ptr, E chain = defer_event<void, derived_t>{})
		{
			derived_t& derive = this->derived();

			derive.dispatch([&derive, ec, this_ptr = std::move(this_ptr), chain = std::move(chain)]() mutable
			{
				derive._do_stop(ec, std::move(this_ptr), std::move(chain));
			});
		}

		template<typename E = defer_event<void, derived_t>>
		inline void _do_stop(
			const error_code& ec, std::shared_ptr<derived_t> this_ptr, E chain = defer_event<void, derived_t>{})
		{
			ASIO2_ASSERT(this->derived().io_->running_in_this_thread());

			state_t expected = state_t::starting;
			if (this->state_.compare_exchange_strong(expected, state_t::stopping))
				return this->derived()._post_stop(ec, std::move(this_ptr), expected, std::move(chain));

			expected = state_t::started;
			if (this->state_.compare_exchange_strong(expected, state_t::stopping))
				return this->derived()._post_stop(ec, std::move(this_ptr), expected, std::move(chain));
		}

		template<typename DeferEvent>
		inline void _post_stop(
			const error_code& ec, std::shared_ptr<derived_t> this_ptr, state_t old_state, DeferEvent chain)
		{
			// psot a recv signal to ensure that all recv events has finished already.
			this->derived().disp_event(
			[this, ec, this_ptr = std::move(this_ptr), old_state, e = chain.move_event()]
			(event_queue_guard<derived_t> g) mutable
			{
				detail::ignore_unused(old_state);

				// When the code runs here,no new session can be emplace or erase to session_mgr.
				// stop all the sessions, the session::stop must be no blocking,
				// otherwise it may be cause loop lock.
				set_last_error(ec);

				defer_event chain(std::move(e), std::move(g));

				state_t expected = state_t::stopping;
				if (this->state_.compare_exchange_strong(expected, state_t::stopped))
				{
					this->derived()._fire_stop();

					// call CRTP polymorphic stop
					this->derived()._handle_stop(ec, std::move(this_ptr), std::move(chain));
				}
				else
				{
					ASIO2_ASSERT(false);
				}
			}, chain.move_guard());
		}

		template<typename DeferEvent>
		inline void _handle_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			derived_t& derive = this->derived();

			detail::ignore_unused(ec, this_ptr, chain);

			ASIO2_ASSERT(derive.io_->running_in_this_thread());

			// close user custom timers
			derive._dispatch_stop_all_timers();

			// close all posted timed tasks
			derive._dispatch_stop_all_timed_events();

			// close all async_events
			derive.notify_all_condition_events();

			//derive._socks5_stop();

			error_code ec_ignore{};

			// call socket's close function to notify the _handle_recv function
			// response with error > 0 ,then the socket can get notify to exit
			// Call shutdown() to indicate that you will not write any more data to the socket.
			derive.socket().shutdown(asio::socket_base::shutdown_both, ec_ignore);

			derive.socket().cancel(ec_ignore);

			// Call close,otherwise the _handle_recv will never return
			derive.socket().close(ec_ignore);

			// clear recv buffer
			derive.buffer().consume(derive.buffer().size());

			// destroy user data, maybe the user data is self shared_ptr,
			// if don't destroy it, will cause loop reference.
			derive.user_data_.reset();

			// destroy the ecs
			derive.ecs_.reset();

			//
			derive.reset_life_id();
		}

	protected:
		template<class Endpoint, class Data, class Callback>
		inline bool _do_send(Endpoint& endpoint, Data& data, Callback&& callback)
		{
			return this->derived()._udp_send_to(endpoint, data, std::forward<Callback>(callback));
		}

	protected:
		template<typename C>
		void _post_recv(std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			if (!this->is_started())
			{
				if (this->derived().state_ == state_t::started)
				{
					this->derived()._do_stop(asio2::get_last_error(), std::move(this_ptr));
				}
				return;
			}

		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(this->derived().post_recv_counter_.load() == 0);
			this->derived().post_recv_counter_++;
		#endif

			this->socket().async_receive_from(
				this->buffer_.prepare(this->buffer_.pre_size()),
				this->remote_endpoint_,
				make_allocator(this->rallocator_,
			[this, this_ptr = std::move(this_ptr), ecs = std::move(ecs)]
			(const error_code& ec, std::size_t bytes_recvd) mutable
			{
			#if defined(_DEBUG) || defined(DEBUG)
				this->derived().post_recv_counter_--;
			#endif

				this->derived()._handle_recv(ec, bytes_recvd, std::move(this_ptr), std::move(ecs));
			}));
		}

		template<typename C>
		void _handle_recv(
			const error_code& ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			set_last_error(ec);

			if (!this->derived().is_started())
			{
				if (this->derived().state_ == state_t::started)
				{
					this->derived()._do_stop(ec, std::move(this_ptr));
				}
				return;
			}

			if (ec == asio::error::operation_aborted)
			{
				this->derived()._do_stop(ec, std::move(this_ptr));
				return;
			}

			this->buffer_.commit(bytes_recvd);

			if (!ec)
			{
				this->derived()._fire_recv(this_ptr, ecs,
					std::string_view(static_cast<std::string_view::const_pointer>(
						this->buffer_.data().data()), bytes_recvd));
			}

			this->buffer_.consume(this->buffer_.size());

			if (bytes_recvd == this->buffer_.pre_size())
			{
				this->buffer_.pre_size((std::min)(this->buffer_.pre_size() * 2, this->buffer_.max_size()));
			}

			this->derived()._post_recv(std::move(this_ptr), std::move(ecs));
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
			detail::ignore_unused(this_ptr, ecs);

			data = detail::call_data_filter_before_recv(this->derived(), data);

			this->listener_.notify(event_type::recv, this->remote_endpoint_, data);
		}

		inline void _fire_start()
		{
			// the _fire_start must be executed in the thread 0.
			ASIO2_ASSERT(this->derived().io_->running_in_this_thread());

		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(this->is_stop_called_ == false);
		#endif

			this->listener_.notify(event_type::start);
		}

		inline void _fire_stop()
		{
			// the _fire_stop must be executed in the thread 0.
			ASIO2_ASSERT(this->derived().io_->running_in_this_thread());

		#if defined(_DEBUG) || defined(DEBUG)
			this->is_stop_called_ = true;
		#endif

			this->listener_.notify(event_type::stop);
		}

	public:
		/**
		 * @brief get the buffer object reference
		 */
		inline buffer_wrap<buffer_type> & buffer() noexcept { return this->buffer_; }

	protected:
		/**
		 * @brief get the recv/read allocator object reference
		 */
		inline auto & rallocator() noexcept { return this->rallocator_; }
		/**
		 * @brief get the send/write allocator object reference
		 */
		inline auto & wallocator() noexcept { return this->wallocator_; }

		inline const char*                  life_id () noexcept { return this->life_id_.get(); }
		inline void                   reset_life_id () noexcept { this->life_id_ = std::make_unique<char>(); }

	protected:
		/// The memory to use for handler-based custom memory allocation. used fo recv/read.
		handler_memory<std::true_type , assizer<args_t>>   rallocator_;

		/// The memory to use for handler-based custom memory allocation. used fo send/write.
		handler_memory<std::false_type, assizer<args_t>>   wallocator_;

		/// listener 
		listener_t                                  listener_;

		/// buffer
		buffer_wrap<buffer_type>                    buffer_;

		/// state
		std::atomic<state_t>                        state_ = state_t::stopped;

		/// the pointer of ecs_t
		std::shared_ptr<ecs_base>                   ecs_;

		/// @see client life id
		std::unique_ptr<char>                       life_id_ = std::make_unique<char>();

	#if defined(_DEBUG) || defined(DEBUG)
		bool                                        is_stop_called_  = false;
		std::atomic<int>                            post_send_counter_ = 0;
		std::atomic<int>                            post_recv_counter_ = 0;
	#endif
	};
}

namespace asio2
{
	using udp_cast_args = detail::template_args_udp_cast;

	template<class derived_t, class args_t>
	using udp_cast_impl_t = detail::udp_cast_impl_t<derived_t, args_t>;

	/**
	 * udp unicast/multicast/broadcast
	 * @throws constructor maybe throw exception "Too many open files" (exception code : 24)
	 * asio::error::no_descriptors - Too many open files
	 */
	template<class derived_t>
	class udp_cast_t : public detail::udp_cast_impl_t<derived_t, detail::template_args_udp_cast>
	{
	public:
		using detail::udp_cast_impl_t<derived_t, detail::template_args_udp_cast>::udp_cast_impl_t;
	};

	/*
	 * udp unicast/multicast/broadcast
	 * If this object is created as a shared_ptr like std::shared_ptr<asio2::udp_cast> cast;
	 * you must call the cast->stop() manual when exit, otherwise maybe cause memory leaks.
	 * @throws constructor maybe throw exception "Too many open files" (exception code : 24)
	 * asio::error::no_descriptors - Too many open files
	 */
	class udp_cast : public udp_cast_t<udp_cast>
	{
	public:
		using udp_cast_t<udp_cast>::udp_cast_t;
	};
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_UDP_CAST_HPP__
