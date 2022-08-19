/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
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

#include <asio2/base/component/thread_id_cp.hpp>
#include <asio2/base/component/alive_time_cp.hpp>
#include <asio2/base/component/user_data_cp.hpp>
#include <asio2/base/component/socket_cp.hpp>
#include <asio2/base/component/user_timer_cp.hpp>
#include <asio2/base/component/post_cp.hpp>
#include <asio2/base/component/event_queue_cp.hpp>
#include <asio2/base/component/condition_event_cp.hpp>

#include <asio2/base/detail/linear_buffer.hpp>
#include <asio2/udp/component/udp_send_cp.hpp>
#include <asio2/udp/impl/udp_send_op.hpp>

namespace asio2::detail
{
	struct template_args_udp_cast
	{
		using socket_t = asio::ip::udp::socket;
		using buffer_t = asio2::linear_buffer;
	};

	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_UDP_BASE;

	template<class derived_t, class args_t = template_args_udp_cast>
	class udp_cast_impl_t
		: public object_t          <derived_t        >
		, public iopool_cp         <derived_t, args_t>
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
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_UDP_BASE;

	public:
		using super = object_t       <derived_t        >;
		using self  = udp_cast_impl_t<derived_t, args_t>;

		using iopoolcp = iopool_cp   <derived_t, args_t>;

		using args_type   = args_t;
		using buffer_type = typename args_t::buffer_t;

		/**
		 * @constructor
		 * maybe throw exception "Too many open files" (exception code : 24)
		 * asio::error::no_descriptors - Too many open files
		 */
		explicit udp_cast_impl_t(
			std::size_t init_buf_size = udp_frame_size,
			std::size_t max_buf_size  = max_buffer_size,
			std::size_t concurrency   = 1
		)
			: super()
			, iopool_cp         <derived_t, args_t>(concurrency)
			, event_queue_cp    <derived_t, args_t>()
			, user_data_cp      <derived_t, args_t>()
			, alive_time_cp     <derived_t, args_t>()
			, socket_cp         <derived_t, args_t>(iopoolcp::_get_io(0).context())
			, user_timer_cp     <derived_t, args_t>()
			, post_cp           <derived_t, args_t>()
			, condition_event_cp<derived_t, args_t>()
			, udp_send_cp       <derived_t, args_t>(iopoolcp::_get_io(0))
			, udp_send_op       <derived_t, args_t>()
			, rallocator_()
			, wallocator_()
			, listener_  ()
			, io_        (iopoolcp::_get_io(0))
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
			, event_queue_cp    <derived_t, args_t>()
			, user_data_cp      <derived_t, args_t>()
			, alive_time_cp     <derived_t, args_t>()
			, socket_cp         <derived_t, args_t>(iopoolcp::_get_io(0).context())
			, user_timer_cp     <derived_t, args_t>()
			, post_cp           <derived_t, args_t>()
			, condition_event_cp<derived_t, args_t>()
			, udp_send_cp       <derived_t, args_t>(iopoolcp::_get_io(0))
			, udp_send_op       <derived_t, args_t>()
			, rallocator_()
			, wallocator_()
			, listener_  ()
			, io_        (iopoolcp::_get_io(0))
			, buffer_    (init_buf_size, max_buf_size)
		{
		}

		template<class Scheduler, std::enable_if_t<!std::is_integral_v<detail::remove_cvref_t<Scheduler>>, int> = 0>
		explicit udp_cast_impl_t(Scheduler&& scheduler)
			: udp_cast_impl_t(udp_frame_size, max_buffer_size, std::forward<Scheduler>(scheduler))
		{
		}

		/**
		 * @destructor
		 */
		~udp_cast_impl_t()
		{
			this->stop();
		}

		/**
		 * @function : start the udp cast
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param service A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt, typename... Args>
		inline bool start(String&& host, StrOrInt&& service, Args&&... args)
		{
			return this->derived()._do_start(
				std::forward<String>(host), std::forward<StrOrInt>(service),
				condition_helper::make_condition('0', std::forward<Args>(args)...));
		}

		/**
		 * @function : stop the udp cast
		 * You can call this function on the communication thread and anywhere to stop the udp cast.
		 */
		inline void stop()
		{
			if (this->is_iopool_stopped())
				return;

			derived_t& derive = this->derived();

			derive.io().unregobj(&derive);

			// use promise to get the result of stop
			std::promise<state_t> promise;
			std::future<state_t> future = promise.get_future();

			// use derfer to ensure the promise's value must be seted.
			detail::defer_event pg
			{
				[this, p = std::move(promise)]() mutable { p.set_value(this->state().load()); }
			};

			derive.push_event([&derive, this_ptr = derive.selfptr(), pg = std::move(pg)]
			(event_queue_guard<derived_t> g) mutable
			{
				derive._do_stop(asio::error::operation_aborted, std::move(this_ptr),
					defer_event
					{
						[pg = std::move(pg)](event_queue_guard<derived_t> g) mutable
						{
							detail::ignore_unused(pg, g);

							// the "pg" should destroyed before the "g", otherwise if the "g"
							// is destroyed before "pg", the next event maybe called, then the
							// state maybe change to not stopped.
							{
								detail::defer_event{ std::move(pg) };
							}
						}, std::move(g)
					}
				);
			});

			if (!derive.running_in_this_thread())
			{
				[[maybe_unused]] state_t state = future.get();
				ASIO2_ASSERT(state == state_t::stopped);
			}

			this->stop_iopool();
		}

		/**
		 * @function : check whether the udp cast is started
		 */
		inline bool is_started() const
		{
			return (this->state_ == state_t::started && this->socket_.lowest_layer().is_open());
		}

		/**
		 * @function : check whether the udp cast is stopped
		 */
		inline bool is_stopped() const
		{
			return (this->state_ == state_t::stopped && !this->socket_.lowest_layer().is_open() && this->is_iopool_stopped());
		}

	public:
		/**
		 * @function : bind recv listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
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
			this->listener_.bind(event_type::init,
				observer_t<>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

		/**
		 * @function : bind start listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
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
		 * @function : bind stop listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
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
		template<typename String, typename StrOrInt, typename MatchCondition>
		bool _do_start(String&& host, StrOrInt&& port, condition_wrap<MatchCondition> condition)
		{
			derived_t& derive = this->derived();

			this->start_iopool();

			if (this->is_iopool_stopped())
			{
				set_last_error(asio::error::operation_aborted);
				return false;
			}

			asio::dispatch(derive.io().context(), [&derive, this_ptr = derive.selfptr()]() mutable
			{
				detail::ignore_unused(this_ptr);

				// init the running thread id 
				if (derive.io().get_thread_id() == std::thread::id{})
					derive.io().init_thread_id();
			});

			// use promise to get the result of async connect
			std::promise<error_code> promise;
			std::future<error_code> future = promise.get_future();

			// use derfer to ensure the promise's value must be seted.
			detail::defer_event pg
			{
				[promise = std::move(promise)]() mutable { promise.set_value(get_last_error()); }
			};

			derive.push_event(
			[this, this_ptr = derive.selfptr(),
				host = std::forward<String>(host), port = std::forward<StrOrInt>(port),
				condition = std::move(condition), pg = std::move(pg)]
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
							detail::defer_event{ std::move(pg) };
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

				try
				{
					clear_last_error();

					derive.io().regobj(&derive);

				#if defined(_DEBUG) || defined(DEBUG)
					this->is_stop_called_ = false;
				#endif

					// convert to string maybe throw some exception.
					std::string h = detail::to_string(std::move(host));
					std::string p = detail::to_string(std::move(port));

					expected = state_t::starting;
					if (!this->state_.compare_exchange_strong(expected, state_t::starting))
					{
						ASIO2_ASSERT(false);
						asio::detail::throw_error(asio::error::operation_aborted);
					}

					error_code ec_ignore{};

					this->socket_.close(ec_ignore);

					// parse address and port
					asio::ip::udp::resolver resolver(this->io_.context());
					asio::ip::udp::endpoint endpoint = *resolver.resolve(h, p,
						asio::ip::resolver_base::flags::passive |
						asio::ip::resolver_base::flags::address_configured).begin();

					this->socket_.open(endpoint.protocol());

					// when you close socket in linux system,and start socket
					// immediate,you will get like this "the address is in use",
					// and bind is failed,but i'm suer i close the socket correct
					// already before,why does this happen? the reasion is the
					// socket option "TIME_WAIT",although you close the socket,
					// but the system not release the socket,util 2~4 seconds later,
					// so we can use the SO_REUSEADDR option to avoid this problem,
					// like below

					// set port reuse
					this->socket_.set_option(asio::ip::udp::socket::reuse_address(true));

					derive._fire_init();

					this->socket_.bind(endpoint);

					derive._handle_start(error_code{}, std::move(this_ptr), std::move(condition), std::move(chain));

					return;
				}
				catch (system_error const& e)
				{
					set_last_error(e.code());
				}
				catch (std::exception const&)
				{
					set_last_error(asio::error::invalid_argument);
				}

				derive._handle_start(get_last_error(), std::move(this_ptr), std::move(condition), std::move(chain));
			});

			if (!derive.io().running_in_this_thread())
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

		template<typename MatchCondition, typename DeferEvent>
		void _handle_start(error_code ec, std::shared_ptr<derived_t> this_ptr,
			condition_wrap<MatchCondition> condition, DeferEvent chain)
		{
			ASIO2_ASSERT(this->derived().io().running_in_this_thread());

			try
			{
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

				asio::detail::throw_error(ec);

				this->buffer_.consume(this->buffer_.size());

				this->derived()._post_recv(std::move(this_ptr), std::move(condition));
			}
			catch (system_error & e)
			{
				set_last_error(e);

				this->derived()._do_stop(e.code(), this->derived().selfptr(), std::move(chain));
			}
		}

		template<typename DeferEvent = defer_event<void, derived_t>>
		inline void _do_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr,
			DeferEvent chain = defer_event<void, derived_t>{})
		{
			ASIO2_ASSERT(this->derived().io().running_in_this_thread());

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

				state_t expected = state_t::stopping;
				if (this->state_.compare_exchange_strong(expected, state_t::stopped))
				{
					this->derived()._fire_stop();

					// call CRTP polymorphic stop
					this->derived()._handle_stop(ec, std::move(this_ptr), defer_event(std::move(e), std::move(g)));
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
			detail::ignore_unused(ec, this_ptr, chain);

			error_code ec_ignore{};

			// close user custom timers
			this->stop_all_timers();

			// close all posted timed tasks
			this->stop_all_timed_tasks();

			// close all async_events
			this->notify_all_condition_events();

			// destroy user data, maybe the user data is self shared_ptr,
			// if don't destroy it, will cause loop refrence.
			this->user_data_.reset();

			// call socket's close function to notify the _handle_recv function
			// response with error > 0 ,then the socket can get notify to exit
			// Call shutdown() to indicate that you will not write any more data to the socket.
			this->socket_.shutdown(asio::socket_base::shutdown_both, ec_ignore);
			// Call close,otherwise the _handle_recv will never return
			this->socket_.close(ec_ignore);
		}

	protected:
		template<class Endpoint, class Data, class Callback>
		inline bool _do_send(Endpoint& endpoint, Data& data, Callback&& callback)
		{
			return this->derived()._udp_send_to(endpoint, data, std::forward<Callback>(callback));
		}

	protected:
		template<typename MatchCondition>
		void _post_recv(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			if (!this->is_started())
			{
				if (this->derived().state() == state_t::started)
				{
					this->derived()._do_stop(asio2::get_last_error(), std::move(this_ptr));
				}
				return;
			}

			try
			{
				this->socket_.async_receive_from(
					this->buffer_.prepare(this->buffer_.pre_size()), this->remote_endpoint_,
					make_allocator(this->rallocator_,
					[this, this_ptr = std::move(this_ptr), condition = std::move(condition)]
				(const error_code& ec, std::size_t bytes_recvd) mutable
				{
					this->derived()._handle_recv(ec, bytes_recvd, std::move(this_ptr), std::move(condition));
				}));
			}
			catch (system_error & e)
			{
				set_last_error(e);

				this->derived()._do_stop(e.code(), this->derived().selfptr());
			}
		}

		template<typename MatchCondition>
		void _handle_recv(const error_code& ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			set_last_error(ec);

			if (!this->derived().is_started())
			{
				if (this->derived().state() == state_t::started)
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
				this->derived()._fire_recv(this_ptr,
					std::string_view(static_cast<std::string_view::const_pointer>(
						this->buffer_.data().data()), bytes_recvd), condition);
			}

			this->buffer_.consume(this->buffer_.size());

			if (bytes_recvd == this->buffer_.pre_size())
			{
				this->buffer_.pre_size((std::min)(this->buffer_.pre_size() * 2, this->buffer_.max_size()));
			}

			this->derived()._post_recv(std::move(this_ptr), std::move(condition));
		}

		inline void _fire_init()
		{
			// the _fire_init must be executed in the thread 0.
			ASIO2_ASSERT(this->derived().io().running_in_this_thread());
			ASIO2_ASSERT(!get_last_error());

			this->listener_.notify(event_type::init);
		}

		template<typename MatchCondition>
		inline void _fire_recv(std::shared_ptr<derived_t>& this_ptr, std::string_view data,
			condition_wrap<MatchCondition>& condition)
		{
			detail::ignore_unused(this_ptr, condition);

			this->listener_.notify(event_type::recv, this->remote_endpoint_, data);
		}

		inline void _fire_start()
		{
			// the _fire_start must be executed in the thread 0.
			ASIO2_ASSERT(this->derived().io().running_in_this_thread());

		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(this->is_stop_called_ == false);
		#endif

			this->listener_.notify(event_type::start);
		}

		inline void _fire_stop()
		{
			// the _fire_stop must be executed in the thread 0.
			ASIO2_ASSERT(this->derived().io().running_in_this_thread());

		#if defined(_DEBUG) || defined(DEBUG)
			this->is_stop_called_ = true;
		#endif

			this->listener_.notify(event_type::stop);
		}

	public:
		/**
		 * @function : get the buffer object refrence
		 */
		inline buffer_wrap<buffer_type> & buffer() noexcept { return this->buffer_; }

		/**
		 * @function : get the io object refrence
		 */
		inline io_t & io() noexcept { return this->io_; }

	protected:
		/**
		 * @function : get the recv/read allocator object refrence
		 */
		inline auto & rallocator() noexcept { return this->rallocator_; }
		/**
		 * @function : get the send/write allocator object refrence
		 */
		inline auto & wallocator() noexcept { return this->wallocator_; }

		inline listener_t                 & listener() noexcept { return this->listener_; }
		inline std::atomic<state_t>       & state   () noexcept { return this->state_; }

	protected:
		/// The memory to use for handler-based custom memory allocation. used fo recv/read.
		handler_memory<>                            rallocator_;

		/// The memory to use for handler-based custom memory allocation. used fo send/write.
		handler_memory<size_op<>, std::true_type>   wallocator_;

		/// listener 
		listener_t                                  listener_;

		/// The io_context wrapper used to handle the connect/recv/send event.
		io_t                                      & io_;

		/// buffer
		buffer_wrap<buffer_type>                    buffer_;

		/// state
		std::atomic<state_t>                        state_ = state_t::stopped;

		/// endpoint for udp 
		asio::ip::udp::endpoint                     remote_endpoint_;

	#if defined(_DEBUG) || defined(DEBUG)
		bool                                        is_stop_called_  = false;
	#endif
	};
}

namespace asio2
{
	/**
	 * udp unicast/multicast/broadcast
	 * constructor maybe throw exception "Too many open files" (exception code : 24)
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
	 * constructor maybe throw exception "Too many open files" (exception code : 24)
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
