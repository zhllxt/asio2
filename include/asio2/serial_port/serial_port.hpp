/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_SERIAL_PORT_HPP__
#define __ASIO2_SERIAL_PORT_HPP__

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

#include <asio2/base/iopool.hpp>
#include <asio2/base/listener.hpp>
#include <asio2/base/define.hpp>

#include <asio2/base/detail/object.hpp>
#include <asio2/base/detail/allocator.hpp>
#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/buffer_wrap.hpp>
#include <asio2/base/detail/condition_wrap.hpp>

#include <asio2/base/component/thread_id_cp.hpp>
#include <asio2/base/component/alive_time_cp.hpp>
#include <asio2/base/component/user_data_cp.hpp>
#include <asio2/base/component/socket_cp.hpp>
#include <asio2/base/component/user_timer_cp.hpp>
#include <asio2/base/component/post_cp.hpp>
#include <asio2/base/component/event_queue_cp.hpp>
#include <asio2/base/component/condition_event_cp.hpp>
#include <asio2/base/component/send_cp.hpp>

#include <asio2/tcp/impl/tcp_send_op.hpp>
#include <asio2/tcp/impl/tcp_recv_op.hpp>

#include <asio2/ecs/rdc/rdc_call_cp.hpp>

namespace asio2::detail
{
	struct template_args_serial_port
	{
		using socket_t    = asio::serial_port;
		using buffer_t    = asio::streambuf;
		using send_data_t = std::string_view;
		using recv_data_t = std::string_view;
	};

	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;

	/**
	 * The serial_port class provides a wrapper over serial port functionality.
	 */
	template<class derived_t, class args_t = template_args_serial_port>
	class serial_port_impl_t
		: public object_t          <derived_t        >
		, public iopool_cp         <derived_t, args_t>
		, public thread_id_cp      <derived_t, args_t>
		, public event_queue_cp    <derived_t, args_t>
		, public user_data_cp      <derived_t, args_t>
		, public alive_time_cp     <derived_t, args_t>
		, public user_timer_cp     <derived_t, args_t>
		, public send_cp           <derived_t, args_t>
		, public tcp_send_op       <derived_t, args_t>
		, public tcp_recv_op       <derived_t, args_t>
		, public post_cp           <derived_t, args_t>
		, public condition_event_cp<derived_t, args_t>
		, public rdc_call_cp       <derived_t, args_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;

	public:
		using super = object_t          <derived_t        >;
		using self  = serial_port_impl_t<derived_t, args_t>;

		using iopoolcp = iopool_cp      <derived_t, args_t>;

		using args_type   = args_t;
		using socket_type = typename args_t::socket_t;
		using buffer_type = typename args_t::buffer_t;
		using send_data_t = typename args_t::send_data_t;
		using recv_data_t = typename args_t::recv_data_t;

		/**
		 * @constructor
		 */
		explicit serial_port_impl_t(
			std::size_t init_buf_size = 1024,
			std::size_t max_buf_size  = max_buffer_size,
			std::size_t concurrency   = 1
		)
			: super()
			, iopool_cp         <derived_t, args_t>(concurrency)
			, event_queue_cp    <derived_t, args_t>()
			, user_data_cp      <derived_t, args_t>()
			, alive_time_cp     <derived_t, args_t>()
			, user_timer_cp     <derived_t, args_t>()
			, send_cp           <derived_t, args_t>()
			, tcp_send_op       <derived_t, args_t>()
			, tcp_recv_op       <derived_t, args_t>()
			, post_cp           <derived_t, args_t>()
			, condition_event_cp<derived_t, args_t>()
			, rdc_call_cp       <derived_t, args_t>()
			, socket_    (iopoolcp::_get_io(0).context())
			, rallocator_()
			, wallocator_()
			, listener_  ()
			, io_        (iopoolcp::_get_io(0))
			, buffer_    (init_buf_size, max_buf_size)
		{
		}

		template<class Scheduler, std::enable_if_t<!std::is_integral_v<detail::remove_cvref_t<Scheduler>>, int> = 0>
		explicit serial_port_impl_t(
			std::size_t init_buf_size,
			std::size_t max_buf_size,
			Scheduler&& scheduler
		)
			: super()
			, iopool_cp         <derived_t, args_t>(std::forward<Scheduler>(scheduler))
			, event_queue_cp    <derived_t, args_t>()
			, user_data_cp      <derived_t, args_t>()
			, alive_time_cp     <derived_t, args_t>()
			, user_timer_cp     <derived_t, args_t>()
			, send_cp           <derived_t, args_t>()
			, tcp_send_op       <derived_t, args_t>()
			, tcp_recv_op       <derived_t, args_t>()
			, post_cp           <derived_t, args_t>()
			, condition_event_cp<derived_t, args_t>()
			, rdc_call_cp       <derived_t, args_t>()
			, socket_    (iopoolcp::_get_io(0).context())
			, rallocator_()
			, wallocator_()
			, listener_  ()
			, io_        (iopoolcp::_get_io(0))
			, buffer_    (init_buf_size, max_buf_size)
		{
		}

		template<class Scheduler, std::enable_if_t<!std::is_integral_v<detail::remove_cvref_t<Scheduler>>, int> = 0>
		explicit serial_port_impl_t(Scheduler&& scheduler)
			: serial_port_impl_t(1024, max_buffer_size, std::forward<Scheduler>(scheduler))
		{
		}

		/**
		 * @destructor
		 */
		~serial_port_impl_t()
		{
			this->stop();
		}

		/**
		 * @function : start
		 * @param device The platform-specific device name for this serial, example "/dev/ttyS0" or "COM1"
		 * @param baud_rate Communication speed, example 9600 or 115200
		 * @param condition The delimiter condition.Valid value types include the following:
		 * char,std::string,std::string_view,
		 * function:std::pair<iterator, bool> match_condition(iterator begin, iterator end),
		 * asio::transfer_at_least,asio::transfer_exactly
		 * more details see asio::read_until
		 */
		template<typename String, typename StrOrInt, typename... Args>
		inline bool start(String&& device, StrOrInt&& baud_rate, Args&&... args)
		{
			return this->derived()._do_start(
				std::forward<String>(device), std::forward<StrOrInt>(baud_rate),
				condition_helper::make_condition(asio::transfer_at_least(1), std::forward<Args>(args)...));
		}

		/**
		 * @function : stop
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
				derive._do_disconnect(asio::error::operation_aborted, std::move(this_ptr),
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
		 * @function : check whether the client is started
		 */
		inline bool is_started() const
		{
			return (this->state_ == state_t::started && this->socket_.lowest_layer().is_open());
		}

		/**
		 * @function : check whether the client is stopped
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
		 * void on_recv(std::string_view data){...}
		 * or a lumbda function like this :
		 * [&](std::string_view data){...}
		 */
		template<class F, class ...C>
		inline derived_t & bind_recv(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::recv,
				observer_t<std::string_view>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

		/**
		 * @function : bind init listener,we should set socket options at here
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * void on_init(){...}
		 * or a lumbda function like this :
		 * [&](){...}
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
			this->listener_.bind(event_type::start,
				observer_t<>(std::forward<F>(fun), std::forward<C>(obj)...));
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
			this->listener_.bind(event_type::stop,
				observer_t<>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	public:
		/**
		 * @function : get the socket object refrence
		 */
		inline socket_type & socket() noexcept { return this->socket_; }

		/**
		 * @function : get the stream object refrence
		 */
		inline socket_type & stream() noexcept { return this->socket_; }

		/**
		 * This function is used to set an option on the serial port.
		 *
		 * @param option The option value to be set on the serial port.
		 *
		 * asio::serial_port::baud_rate
		 * asio::serial_port::flow_control
		 * asio::serial_port::parity
		 * asio::serial_port::stop_bits
		 * asio::serial_port::character_size
		 */
		template <typename SettableSerialPortOption>
		derived_t& set_option(const SettableSerialPortOption& option) noexcept
		{
			try
			{
				clear_last_error();

				this->socket_.set_option(option);
			}
			catch (system_error const& e)
			{
				set_last_error(e);
			}
			return (this->derived());
		}

		/**
		 * This function is used to get the current value of an option on the serial
		 * port.
		 *
		 * @param option The option value to be obtained from the serial port.
		 *
		 * asio::serial_port::baud_rate
		 * asio::serial_port::flow_control
		 * asio::serial_port::parity
		 * asio::serial_port::stop_bits
		 * asio::serial_port::character_size
		 */
		template <typename GettableSerialPortOption>
		GettableSerialPortOption get_option() const
		{
			GettableSerialPortOption option{};
			try
			{
				clear_last_error();

				this->socket_.get_option(option);
			}
			catch (system_error const& e)
			{
				set_last_error(e);
			}
			return option;
		}

		/**
		 * This function is used to get the current value of an option on the serial
		 * port.
		 *
		 * @param option The option value to be obtained from the serial port.
		 *
		 * asio::serial_port_base::baud_rate
		 * asio::serial_port_base::flow_control
		 * asio::serial_port_base::parity
		 * asio::serial_port_base::stop_bits
		 * asio::serial_port_base::character_size
		 */
		template <typename GettableSerialPortOption>
		derived_t& get_option(GettableSerialPortOption& option)
		{
			try
			{
				clear_last_error();

				this->socket_.get_option(option);
			}
			catch (system_error const& e)
			{
				set_last_error(e);
			}
			return (this->derived());
		}

	protected:
		template<typename String, typename StrOrInt, typename MatchCondition>
		bool _do_start(String&& device, StrOrInt&& baud_rate, condition_wrap<MatchCondition> condition)
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
				device = std::forward<String>(device), baud_rate = std::forward<StrOrInt>(baud_rate),
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
					std::string  d = detail::to_string(std::move(device));
					unsigned int b = detail::to_integer<unsigned int>(std::move(baud_rate));

					expected = state_t::starting;
					if (!this->state_.compare_exchange_strong(expected, state_t::starting))
					{
						ASIO2_ASSERT(false);
						asio::detail::throw_error(asio::error::operation_aborted);
					}

					error_code ec_ignore{};

					this->socket_.close(ec_ignore);
					this->socket_.open(d);
					this->socket_.set_option(asio::serial_port::baud_rate(b));

					// if the match condition is remote data call mode,do some thing.
					derive._rdc_init(condition);

					derive._fire_init();
					// You can set other serial port parameters in on_init(bind_init) callback function like this:
					// sp.set_option(asio::serial_port::flow_control(serial_port::flow_control::type(flow_control)));
					// sp.set_option(asio::serial_port::parity(serial_port::parity::type(parity)));
					// sp.set_option(asio::serial_port::stop_bits(serial_port::stop_bits::type(stop_bits)));
					// sp.set_option(asio::serial_port::character_size(character_size));

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

				this->derived()._fire_start(condition);

				expected = state_t::started;
				if (!ec)
					if (!this->state_.compare_exchange_strong(expected, state_t::started))
						ec = asio::error::operation_aborted;

				asio::detail::throw_error(ec);

				this->derived()._start_recv(std::move(this_ptr), std::move(condition));
			}
			catch (system_error & e)
			{
				set_last_error(e);

				this->derived()._do_disconnect(e.code(), this->derived().selfptr(), std::move(chain));
			}
		}

		template<typename DeferEvent = defer_event<void, derived_t>>
		inline void _do_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr,
			DeferEvent chain = defer_event<void, derived_t>{})
		{
			ASIO2_ASSERT(this->derived().io().running_in_this_thread());

			state_t expected = state_t::started;
			if (this->state_.compare_exchange_strong(expected, state_t::stopping))
			{
				return this->derived()._post_disconnect(ec, std::move(this_ptr), expected, std::move(chain));
			}

			expected = state_t::starting;
			if (this->state_.compare_exchange_strong(expected, state_t::stopping))
			{
				return this->derived()._post_disconnect(ec, std::move(this_ptr), expected, std::move(chain));
			}
		}

		template<typename DeferEvent>
		inline void _post_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr,
			state_t old_state, DeferEvent chain)
		{
			// All pending sending events will be cancelled after enter the callback below.
			this->derived().disp_event(
			[this, ec, old_state, this_ptr = std::move(this_ptr), e = chain.move_event()]
			(event_queue_guard<derived_t> g) mutable
			{
				detail::ignore_unused(g);

				set_last_error(ec);

				this->derived()._handle_disconnect(ec, std::move(this_ptr), old_state,
					defer_event(std::move(e), std::move(g)));
			}, chain.move_guard());
		}

		template<typename DeferEvent>
		inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr,
			state_t old_state, DeferEvent chain)
		{
			this->derived()._rdc_stop();

			this->derived()._do_stop(ec, std::move(this_ptr), old_state, std::move(chain));
		}

		template<typename DeferEvent>
		inline void _do_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr,
			state_t old_state, DeferEvent chain)
		{
			this->derived()._post_stop(ec, std::move(this_ptr), old_state, std::move(chain));
		}

		template<typename DeferEvent>
		inline void _post_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr,
			state_t old_state, DeferEvent chain)
		{
			// All pending sending events will be cancelled after enter the callback below.
			this->derived().disp_event(
			[this, ec, old_state, this_ptr = std::move(this_ptr), e = chain.move_event()]
			(event_queue_guard<derived_t> g) mutable
			{
				detail::ignore_unused(g, old_state);

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

			ASIO2_ASSERT(this->state_ == state_t::stopped);

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

			// Call close,otherwise the _handle_recv will never return
			this->socket_.close(ec_ignore);
		}

		template<typename MatchCondition>
		inline void _start_recv(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			// Connect succeeded. post recv request.
			asio::dispatch(this->derived().io().context(), make_allocator(this->derived().wallocator(),
			[this, this_ptr = std::move(this_ptr), condition = std::move(condition)]() mutable
			{
				using condition_type = typename condition_wrap<MatchCondition>::condition_type;

				if constexpr (!std::is_same_v<condition_type, asio2::detail::hook_buffer_t>)
				{
					this->derived().buffer().consume(this->derived().buffer().size());
				}
				else
				{
					std::ignore = true;
				}

				this->derived()._post_recv(std::move(this_ptr), std::move(condition));
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

		template<class Invoker, class FnData>
		inline void _rdc_invoke_with_send(const error_code& ec, Invoker& invoker, FnData& fn_data)
		{
			if (invoker)
				invoker(ec, fn_data(), recv_data_t{});
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
			// the _fire_init must be executed in the thread 0.
			ASIO2_ASSERT(this->derived().io().running_in_this_thread());
			ASIO2_ASSERT(!get_last_error());

			this->listener_.notify(event_type::init);
		}

		template<typename MatchCondition>
		inline void _fire_start(condition_wrap<MatchCondition>& condition)
		{
			// the _fire_start must be executed in the thread 0.
			ASIO2_ASSERT(this->derived().io().running_in_this_thread());

		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(this->is_stop_called_ == false);
		#endif

			if (!get_last_error())
			{
				this->derived()._rdc_start(this->derived().selfptr(), condition);
			}

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

		template<typename MatchCondition>
		inline void _fire_recv(std::shared_ptr<derived_t>& this_ptr, std::string_view data,
			condition_wrap<MatchCondition>& condition)
		{
			this->listener_.notify(event_type::recv, data);

			this->derived()._rdc_handle_recv(this_ptr, data, condition);
		}

	public:
		/**
		 * @function : set the default remote call timeout for rpc/rdc
		 */
		template<class Rep, class Period>
		inline derived_t & set_default_timeout(std::chrono::duration<Rep, Period> duration) noexcept
		{
			this->rc_timeout_ = duration;
			return (this->derived());
		}

		/**
		 * @function : get the default remote call timeout for rpc/rdc
		 */
		inline std::chrono::steady_clock::duration get_default_timeout() noexcept
		{
			return this->rc_timeout_;
		}

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
		inline std::atomic<state_t>       & state   () noexcept { return this->state_;    }

	protected:
		/// socket 
		socket_type                               socket_;

		/// The memory to use for handler-based custom memory allocation. used fo recv/read.
		handler_memory<>                          rallocator_;

		/// The memory to use for handler-based custom memory allocation. used fo send/write.
		handler_memory<size_op<>, std::true_type> wallocator_;

		/// listener
		listener_t                                listener_;

		/// The io_context wrapper used to handle the accept event.
		io_t                                    & io_;

		/// buffer
		buffer_wrap<buffer_type>                  buffer_;

		/// state
		std::atomic<state_t>                      state_ = state_t::stopped;

		/// Remote call (rpc/rdc) response timeout.
		std::chrono::steady_clock::duration       rc_timeout_ = std::chrono::milliseconds(http_execute_timeout);

	#if defined(_DEBUG) || defined(DEBUG)
		bool                                      is_stop_called_  = false;
	#endif
	};
}

namespace asio2
{
	template<class derived_t>
	class serial_port_t : public detail::serial_port_impl_t<derived_t, detail::template_args_serial_port>
	{
	public:
		using detail::serial_port_impl_t<derived_t, detail::template_args_serial_port>::serial_port_impl_t;
	};

	/**
	 * The serial_port class provides a wrapper over serial port functionality.
	 * You can use the following commands to query the serial device under Linux:
	 * cat /proc/tty/driver/serial
	 */
	class serial_port : public serial_port_t<serial_port>
	{
	public:
		using serial_port_t<serial_port>::serial_port_t;
	};
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_SERIAL_PORT_HPP__
