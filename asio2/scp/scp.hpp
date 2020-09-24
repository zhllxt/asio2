/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_SCP_HPP__
#define __ASIO2_SCP_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

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

#include <asio2/base/selector.hpp>
#include <asio2/base/iopool.hpp>
#include <asio2/base/error.hpp>
#include <asio2/base/listener.hpp>

#include <asio2/base/detail/object.hpp>
#include <asio2/base/detail/allocator.hpp>
#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/buffer_wrap.hpp>
#include <asio2/base/detail/condition_wrap.hpp>

#include <asio2/base/component/alive_time_cp.hpp>
#include <asio2/base/component/user_data_cp.hpp>
#include <asio2/base/component/socket_cp.hpp>
#include <asio2/base/component/user_timer_cp.hpp>
#include <asio2/base/component/post_cp.hpp>
#include <asio2/base/component/send_cp.hpp>
#include <asio2/base/component/event_queue_cp.hpp>

#include <asio2/icmp/detail/icmp_header.hpp>
#include <asio2/icmp/detail/ipv4_header.hpp>

#include <asio2/tcp/impl/tcp_send_op.hpp>
#include <asio2/tcp/impl/tcp_recv_op.hpp>

namespace asio2::detail
{
	/**
	 * The serial_port class provides a wrapper over serial port functionality.
	 */
	template<class derived_t, class socket_t, class buffer_t>
	class scp_impl_t
		: public object_t<derived_t>
		, public iopool_cp
		, public event_queue_cp<derived_t>
		, public user_data_cp<derived_t>
		, public alive_time_cp<derived_t>
		, public user_timer_cp<derived_t, false>
		, public send_cp<derived_t, false>
		, public post_cp<derived_t>
		, public tcp_send_op<derived_t, false>
		, public tcp_recv_op<derived_t, false>
	{
		template <class, bool>         friend class user_timer_cp;
		template <class>               friend class post_cp;
		template <class>               friend class data_persistence_cp;
		template <class>               friend class event_queue_cp;
		template <class, bool>         friend class send_cp;
		template <class, bool>         friend class tcp_send_op;
		template <class, bool>         friend class tcp_recv_op;
		template <class>               friend class event_guard;

	public:
		using self = scp_impl_t<derived_t, socket_t, buffer_t>;
		using super = object_t<derived_t>;
		using buffer_type = buffer_t;

		/**
		 * @constructor
		 */
		scp_impl_t(
			std::size_t init_buffer_size = 1024,
			std::size_t max_buffer_size = (std::numeric_limits<std::size_t>::max)()
		)
			: super()
			, iopool_cp(1)
			, event_queue_cp<derived_t>()
			, user_data_cp<derived_t>()
			, alive_time_cp<derived_t>()
			, user_timer_cp<derived_t, false>(iopool_.get(0))
			, send_cp<derived_t, false>(iopool_.get(0))
			, post_cp<derived_t>()
			, tcp_send_op<derived_t, false>()
			, tcp_recv_op<derived_t, false>()
			, socket_(iopool_.get(0).context())
			, rallocator_()
			, wallocator_()
			, listener_()
			, io_(iopool_.get(0))
			, buffer_(init_buffer_size, max_buffer_size)
		{
		}

		/**
		 * @destructor
		 */
		~scp_impl_t()
		{
			this->stop();
		}

		/**
		 * @function : start
		 * @param device The platform-specific device name for this serial, example "/dev/ttyS0" or "COM1"
		 * @param baud_rate Communication speed, example 9600 or 115200
		 */
		template<typename String, typename StrOrInt>
		inline bool start(String&& device, StrOrInt&& baud_rate)
		{
			return this->derived()._do_start(
				std::forward<String>(device), std::forward<StrOrInt>(baud_rate),
				condition_wrap<asio::detail::transfer_at_least_t>{asio::transfer_at_least(1)});
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
		template<typename String, typename StrOrInt, typename MatchCondition>
		inline bool start(String&& device, StrOrInt&& baud_rate, MatchCondition condition)
		{
			return this->derived()._do_start(
				std::forward<String>(device), std::forward<StrOrInt>(baud_rate),
				condition_wrap<MatchCondition>(condition));
		}

		/**
		 * @function : stop
		 */
		inline void stop()
		{
			this->derived()._do_stop(asio::error::operation_aborted);

			this->iopool_.stop();
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
			return (this->state_ == state_t::stopped && !this->socket_.lowest_layer().is_open());
		}

	public:
		/**
		 * @function : bind recv listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * void on_recv(std::string_view s){...}
		 * or a lumbda function like this :
		 * [&](std::string_view s){...}
		 */
		template<class F, class ...C>
		inline derived_t & bind_recv(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::recv,
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
			this->listener_.bind(event::init,
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
		 * Function signature : void(asio2::error_code ec)
		 */
		template<class F, class ...C>
		inline derived_t & bind_start(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::start,
				observer_t<error_code>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

		/**
		 * @function : bind stop listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * This notification is called before the server is ready to stop
		 * Function signature : void(asio2::error_code ec)
		 */
		template<class F, class ...C>
		inline derived_t & bind_stop(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::stop,
				observer_t<error_code>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	public:
		/**
		 * @function : get the socket object refrence
		 */
		inline socket_t & socket() { return this->socket_; }

		/**
		 * @function : get the stream object refrence
		 */
		inline socket_t & stream() { return this->socket_; }

	protected:
		template<typename String, typename StrOrInt, typename MatchCondition>
		bool _do_start(String&& device, StrOrInt&& baud_rate, condition_wrap<MatchCondition> condition)
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

				this->iopool_.start();

				if (this->iopool_.is_stopped())
				{
					set_last_error(asio::error::shut_down);
					return false;
				}

				this->socket_.close(ec_ignore);
				this->socket_.open(to_string(device));
				this->socket_.set_option(asio::serial_port::baud_rate(to_integer<unsigned int>(baud_rate)));

				this->derived()._fire_init();
				// You can set other serial port parameters in on_init(bind_init) callback function like this:
				// sp.socket().set_option(asio::serial_port::flow_control(serial_port::flow_control::type(flow_control)));
				// sp.socket().set_option(asio::serial_port::parity(serial_port::parity::type(parity)));
				// sp.socket().set_option(asio::serial_port::stop_bits(serial_port::stop_bits::type(stop_bits)));
				// sp.socket().set_option(asio::serial_port::character_size(character_size));

				this->derived()._handle_start(error_code{}, std::move(condition));

				return (this->is_started());
			}
			catch (system_error & e)
			{
				this->derived()._handle_start(e.code(), std::move(condition));
			}
			return false;
		}

		template<typename MatchCondition>
		void _handle_start(error_code ec, condition_wrap<MatchCondition> condition)
		{
			try
			{
				// Whether the startup succeeds or fails, always call fire_start notification
				state_t expected = state_t::starting;
				if (!ec)
					if (!this->state_.compare_exchange_strong(expected, state_t::started))
						ec = asio::error::operation_aborted;

				set_last_error(ec);

				this->derived()._fire_start(ec);

				expected = state_t::started;
				if (!ec)
					if (!this->state_.compare_exchange_strong(expected, state_t::started))
						asio::detail::throw_error(asio::error::operation_aborted);

				asio::detail::throw_error(ec);

				asio::post(this->io_.strand(), [this, condition]() mutable
				{
					this->derived()._start_recv(std::move(condition));
				});
			}
			catch (system_error & e)
			{
				set_last_error(e);
				this->derived()._do_stop(e.code());
			}
		}

		inline void _do_disconnect(const error_code& ec, std::shared_ptr<defer> defer_task = {})
		{
			this->derived()._do_stop(ec);
		}

		inline void _do_stop(const error_code& ec)
		{
			state_t expected = state_t::starting;
			if (this->state_.compare_exchange_strong(expected, state_t::stopping))
				return this->derived()._post_stop(ec, this->derived().selfptr(), expected);

			expected = state_t::started;
			if (this->state_.compare_exchange_strong(expected, state_t::stopping))
				return this->derived()._post_stop(ec, this->derived().selfptr(), expected);
		}

		inline void _post_stop(const error_code& ec, std::shared_ptr<derived_t> self_ptr, state_t old_state)
		{
			// All pending sending events will be cancelled after enter the send strand below.
			asio::post(this->io_.strand(), [this, ec, this_ptr = std::move(self_ptr), old_state]() mutable
			{
				detail::ignore::unused(old_state);

				set_last_error(ec);

				state_t expected = state_t::stopping;
				if (this->state_.compare_exchange_strong(expected, state_t::stopped))
				{
					this->derived()._fire_stop(ec);

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

			// close user custom timers
			this->stop_all_timers();

			// destroy user data, maybe the user data is self shared_ptr,
			// if don't destroy it, will cause loop refrence.
			this->user_data_.reset();

			// Call close,otherwise the _handle_recv will never return
			this->socket_.close(ec_ignore);

			// set the state to stopped
			this->state_.store(state_t::stopped);
		}

		template<typename MatchCondition>
		inline void _start_recv(condition_wrap<MatchCondition> condition)
		{
			// Connect succeeded. post recv request.
			asio::post(this->io_.strand(), [this, condition]() mutable
			{
				this->derived().buffer().consume(this->derived().buffer().size());

				this->derived()._post_recv(this->derived().selfptr(), condition);
			});
		}

		template<class Data, class Callback>
		inline bool _do_send(Data& data, Callback&& callback)
		{
			return this->derived()._tcp_send(data, std::forward<Callback>(callback));
		}

	protected:
		template<typename MatchCondition>
		inline void _post_recv(std::shared_ptr<derived_t> this_ptr,
			condition_wrap<MatchCondition> condition)
		{
			this->derived()._tcp_post_recv(std::move(this_ptr), condition);
		}

		template<typename MatchCondition>
		inline void _handle_recv(const error_code & ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			this->derived()._tcp_handle_recv(ec, bytes_recvd, std::move(this_ptr), condition);
		}

		inline void _fire_init()
		{
			this->listener_.notify(event::init);
		}

		inline void _fire_start(error_code ec)
		{
			this->listener_.notify(event::start, ec);
		}

		inline void _fire_stop(error_code ec)
		{
			this->listener_.notify(event::stop, ec);
		}

		inline void _fire_recv(ignore, std::string_view s)
		{
			this->listener_.notify(event::recv, s);
		}

	protected:
		/**
		 * @function : get the buffer object refrence
		 */
		inline buffer_wrap<buffer_t> & buffer() { return this->buffer_; }
		/**
		 * @function : get the io object refrence
		 */
		inline io_t & io() { return this->io_; }
		/**
		 * @function : get the recv/read allocator object refrence
		 */
		inline auto & rallocator() { return this->rallocator_; }
		/**
		 * @function : get the send/write allocator object refrence
		 */
		inline auto & wallocator() { return this->wallocator_; }

		inline listener_t                 & listener() { return this->listener_; }
		inline std::atomic<state_t>       & state()    { return this->state_;    }
		inline std::shared_ptr<derived_t>   selfptr()  { return std::shared_ptr<derived_t>{}; }

	protected:
		/// socket 
		socket_t                                  socket_;

		/// The memory to use for handler-based custom memory allocation. used fo recv/read.
		handler_memory<>                          rallocator_;

		/// The memory to use for handler-based custom memory allocation. used fo send/write.
		handler_memory<size_op<>, std::true_type> wallocator_;

		/// listener
		listener_t                                listener_;

		/// The io (include io_context and strand) used to handle the accept event.
		io_t                                    & io_;

		/// buffer
		buffer_wrap<buffer_t>                     buffer_;

		/// state
		std::atomic<state_t>                      state_ = state_t::stopped;
	};
}

namespace asio2
{
	/**
	 * The serial_port class provides a wrapper over serial port functionality.
	 * You can use the following commands to query the serial device under Linux:
	 * cat /proc/tty/driver/serial
	 */
	class scp : public detail::scp_impl_t<scp, asio::serial_port, asio::streambuf>
	{
	public:
		using scp_impl_t<scp, asio::serial_port, asio::streambuf>::scp_impl_t;
	};
}

#endif // !__ASIO2_SCP_HPP__
