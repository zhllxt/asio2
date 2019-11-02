/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_TCP_SESSION_HPP__
#define __ASIO2_TCP_SESSION_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/session.hpp>

#include <asio2/base/detail/condition_wrap.hpp>
#include <asio2/tcp/component/tcp_keepalive_cp.hpp>
#include <asio2/tcp/impl/tcp_send_op.hpp>
#include <asio2/tcp/impl/tcp_recv_op.hpp>

namespace asio2::detail
{
	template<class derived_t, class socket_t, class buffer_t>
	class tcp_session_impl_t
		: public session_impl_t<derived_t, socket_t, buffer_t>
		, public tcp_keepalive_cp<socket_t>
		, public tcp_send_op<derived_t, true>
		, public tcp_recv_op<derived_t, true>
	{
		template <class, bool>                friend class user_timer_cp;
		template <class, bool>                friend class send_queue_cp;
		template <class, bool>                friend class send_cp;
		template <class, bool>                friend class silence_timer_cp;
		template <class, bool>                friend class connect_timeout_cp;
		template <class, bool>                friend class tcp_send_op;
		template <class, bool>                friend class tcp_recv_op;
		template <class>                      friend class session_mgr_t;
		template <class, class, class>        friend class session_impl_t;
		template <class, class>               friend class tcp_server_impl_t;

	public:
		using self = tcp_session_impl_t<derived_t, socket_t, buffer_t>;
		using super = session_impl_t<derived_t, socket_t, buffer_t>;
		using key_type = std::size_t;
		using buffer_type = buffer_t;
		using super::send;

		/**
		 * @constructor
		 */
		explicit tcp_session_impl_t(
			session_mgr_t<derived_t> & sessions,
			listener_t & listener,
			io_t & rwio,
			std::size_t init_buffer_size,
			std::size_t max_buffer_size
		)
			: super(sessions, listener, rwio, init_buffer_size, max_buffer_size, rwio.context())
			, tcp_keepalive_cp<socket_t>(this->socket_)
			, tcp_send_op<derived_t, true>()
			, tcp_recv_op<derived_t, true>()
			, rallocator_()
			, wallocator_()
		{
			this->silence_timeout(std::chrono::milliseconds(tcp_silence_timeout));
		}

		/**
		 * @destructor
		 */
		~tcp_session_impl_t()
		{
		}

	protected:
		/**
		 * @function : start the session for prepare to recv/send msg
		 */
		template<typename MatchCondition>
		inline void start(condition_wrap<MatchCondition> condition)
		{
			try
			{
				state_t expected = state_t::stopped;
				if (!this->state_.compare_exchange_strong(expected, state_t::starting))
					asio::detail::throw_error(asio::error::already_started);

				std::shared_ptr<derived_t> this_ptr = this->shared_from_this();

				this->derived()._fire_accept(this_ptr);

				expected = state_t::starting;
				if (!this->state_.compare_exchange_strong(expected, state_t::starting))
					asio::detail::throw_error(asio::error::already_started);

				this->derived()._do_init(std::shared_ptr<derived_t>{}, condition);

				// First call the base class start function
				super::start();

				this->derived()._handle_connect(error_code{}, std::move(this_ptr), std::move(condition));
			}
			catch (system_error & e)
			{
				set_last_error(e);
				this->derived()._do_stop(e.code());
			}
		}

	public:
		/**
		 * @function : stop session
		 * note : this function must be noblocking,if it's blocking,maybe cause circle lock.
		 * You can call this function on the communication thread and anywhere to stop the session.
		 */
		inline void stop()
		{
			this->derived()._do_stop(asio::error::operation_aborted);
		}

		/**
		 * @function : get this object hash key,used for session map
		 */
		inline const key_type hash_key() const
		{
			return reinterpret_cast<key_type>(this);
		}

	protected:
		template<typename MatchCondition>
		inline void _do_init(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			detail::ignore::unused(this_ptr, condition);

			// reset the variable to default status
			this->reset_connect_time();
			this->reset_active_time();

			if constexpr (std::is_same_v<MatchCondition, use_dgram_t>)
				this->dgram_ = true;
			else
				this->dgram_ = false;

			// set keeplive options
			this->keep_alive_options();
		}

		template<typename MatchCondition>
		inline void _handle_connect(const error_code& ec, std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			this->derived()._done_connect(ec, std::move(this_ptr), std::move(condition));
		}

		template<typename MatchCondition>
		inline void _done_connect(const error_code& ec, std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			set_last_error(ec);
			try
			{
				// Whatever of connection success or failure or timeout, cancel the timeout timer.
				this->_stop_timeout_timer();

				// Set the state to started before fire_connect because the user may send data in
				// fire_connect and fail if the state is not set to started.
				state_t expected = state_t::starting;
				if (!ec)
					if (!this->state_.compare_exchange_strong(expected, state_t::started))
						asio::detail::throw_error(asio::error::operation_aborted);

				this->derived()._fire_connect(this_ptr);

				// may be user has called stop in the listener function,so we can't start continue.
				expected = state_t::started;
				if (!ec)
					if (!this->state_.compare_exchange_strong(expected, state_t::started))
						asio::detail::throw_error(asio::error::operation_aborted);

				asio::detail::throw_error(ec);

				this->derived()._do_start(std::move(this_ptr), std::move(condition));
			}
			catch (system_error & e)
			{
				set_last_error(e);
				this->derived()._do_stop(e.code());
			}
		}

		template<typename MatchCondition>
		inline void _do_start(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			this->derived()._join_session(std::move(this_ptr), std::move(condition));
		}

		inline void _do_stop(const error_code& ec)
		{
			state_t expected = state_t::starting;
			if (this->state_.compare_exchange_strong(expected, state_t::stopping))
				return this->derived()._post_stop(ec, this->shared_from_this(), expected);

			expected = state_t::started;
			if (this->state_.compare_exchange_strong(expected, state_t::stopping))
				return this->derived()._post_stop(ec, this->shared_from_this(), expected);
		}

		inline void _post_stop(const error_code& ec, std::shared_ptr<derived_t> self_ptr, state_t old_state)
		{
			// close the socket by post a event
			// asio don't allow operate the same socket in multi thread,if you close socket in one thread and another thread is 
			// calling socket's async_... function,it will crash.so we must care for operate the socket.when need close the
			// socket ,we use the strand to post a event,make sure the socket's close operation is in the same thread.

			// First ensure that all send and recv events are not executed again
			asio::post(this->io_.strand(), make_allocator(this->wallocator_,
				[this, ec, this_ptr = std::move(self_ptr), old_state]()
			{
				// All pending sending events will be cancelled when code run to here.

				// Second ensure that this session has removed from the session map.
				this->sessions_.erase(this_ptr, [this, ec, this_ptr, old_state](bool)
				{
					set_last_error(ec);

					state_t expected = state_t::stopping;
					if (this->state_.compare_exchange_strong(expected, state_t::stopped))
					{
						if (old_state == state_t::started)
							this->derived()._fire_disconnect(const_cast<std::shared_ptr<derived_t>&>(this_ptr));
					}
					else
					{
						ASIO2_ASSERT(false);
					}

					// Third we can stop this session and close this socket now.
					asio::post(this->io_.strand(), make_allocator(this->wallocator_,
						[this, ec, self_ptr = std::move(this_ptr)]()
					{
						// call the base class stop function
						super::stop();

						// call CRTP polymorphic stop
						this->derived()._handle_stop(ec, std::move(self_ptr));
					}));
				});
			}));
		}

		inline void _handle_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			detail::ignore::unused(ec, this_ptr);

			// call socket's close function to notify the _handle_recv function response with error > 0 ,then the socket 
			// can get notify to exit
			// Call shutdown() to indicate that you will not write any more data to the socket.
			this->socket_.lowest_layer().shutdown(asio::socket_base::shutdown_both, ec_ignore);
			// Call close,otherwise the _handle_recv will never return
			this->socket_.lowest_layer().close(ec_ignore);
		}

		template<typename MatchCondition>
		inline void _join_session(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			this->sessions_.emplace(this_ptr, [this, this_ptr, condition](bool inserted)
			{
				if (inserted)
					this->derived()._start_recv(std::move(this_ptr), std::move(condition));
				else
					this->derived()._do_stop(asio::error::address_in_use);
			});
		}

		template<typename MatchCondition>
		inline void _start_recv(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			// to avlid the user call stop in another thread,then it may be socket_.async_read_some
			// and socket_.close be called at the same time
			asio::post(this->io_.strand(), make_allocator(this->rallocator_,
				[this, self_ptr = std::move(this_ptr), condition]()
			{
				this->derived()._post_recv(std::move(self_ptr), std::move(condition));
			}));
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

		inline void _fire_recv(std::shared_ptr<derived_t> & this_ptr, std::string_view s)
		{
			this->listener_.notify(event::recv, this_ptr, std::move(s));
		}

		inline void _fire_accept(std::shared_ptr<derived_t> & this_ptr)
		{
			this->listener_.notify(event::accept, this_ptr);
		}

		inline void _fire_connect(std::shared_ptr<derived_t> & this_ptr)
		{
			this->listener_.notify(event::connect, this_ptr);
		}

		inline void _fire_disconnect(std::shared_ptr<derived_t> & this_ptr)
		{
			this->listener_.notify(event::disconnect, this_ptr);
		}

	protected:
		/**
		 * @function : get the recv/read allocator object refrence
		 */
		inline auto & rallocator() { return this->rallocator_; }
		/**
		 * @function : get the send/write allocator object refrence
		 */
		inline auto & wallocator() { return this->wallocator_; }

	protected:
		/// The memory to use for handler-based custom memory allocation. used fo recv/read.
		handler_memory<>                          rallocator_;

		/// The memory to use for handler-based custom memory allocation. used fo send/write.
		handler_memory<size_op<>, std::true_type> wallocator_;

		/// Does it have the same datagram mechanism as udp?
		bool                                      dgram_ = false;
	};
}

namespace asio2
{
	class tcp_session : public detail::tcp_session_impl_t<tcp_session, asio::ip::tcp::socket, asio::streambuf>
	{
	public:
		using tcp_session_impl_t<tcp_session, asio::ip::tcp::socket, asio::streambuf>::tcp_session_impl_t;
	};
}

#endif // !__ASIO2_TCP_SESSION_HPP__
