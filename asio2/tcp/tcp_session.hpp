/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
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
	struct template_args_tcp_session
	{
		static constexpr bool is_session = true;
		static constexpr bool is_client  = false;

		using socket_t    = asio::ip::tcp::socket;
		using buffer_t    = asio::streambuf;
		using send_data_t = std::string_view;
		using recv_data_t = std::string_view;
	};

	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SESSION;

	template<class derived_t, class args_t>
	class tcp_session_impl_t
		: public session_impl_t   <derived_t, args_t>
		, public tcp_keepalive_cp <derived_t, args_t>
		, public tcp_send_op      <derived_t, args_t>
		, public tcp_recv_op      <derived_t, args_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION;

	public:
		using super = session_impl_t    <derived_t, args_t>;
		using self  = tcp_session_impl_t<derived_t, args_t>;

		using key_type    = std::size_t;
		using buffer_type = typename args_t::buffer_t;
		using send_data_t = typename args_t::send_data_t;
		using recv_data_t = typename args_t::recv_data_t;

		using super::send;

		/**
		 * @constructor
		 */
		explicit tcp_session_impl_t(
			session_mgr_t<derived_t> & sessions,
			listener_t               & listener,
			io_t                     & rwio,
			std::size_t                init_buffer_size,
			std::size_t                max_buffer_size
		)
			: super(sessions, listener, rwio, init_buffer_size, max_buffer_size, rwio.context())
			, tcp_keepalive_cp<derived_t, args_t>(this->socket_)
			, tcp_send_op<derived_t, args_t>()
			, tcp_recv_op<derived_t, args_t>()
			, rallocator_()
			, wallocator_()
		{
			this->silence_timeout(std::chrono::milliseconds(tcp_silence_timeout));
			this->connect_timeout(std::chrono::milliseconds(tcp_connect_timeout));
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
			// Used to test whether the behavior of different compilers is consistent
			static_assert(tcp_send_op<derived_t, args_t>::template has_member_dgram<self>::value,
				"The behavior of different compilers is not consistent");

			try
			{
				state_t expected = state_t::stopped;
				if (!this->state_.compare_exchange_strong(expected, state_t::starting))
					asio::detail::throw_error(asio::error::already_started);

				std::shared_ptr<derived_t> this_ptr = this->derived().selfptr();

				this->derived()._do_init(this_ptr, condition);

				this->derived()._fire_accept(this_ptr);

				expected = state_t::starting;
				if (!this->state_.compare_exchange_strong(expected, state_t::starting))
					asio::detail::throw_error(asio::error::already_started);

				// First call the base class start function
				super::start();

				// if the match condition is remote data call mode,do some thing.
				this->derived()._rdc_init(condition);

				this->derived()._handle_connect(error_code{}, std::move(this_ptr), std::move(condition));
			}
			catch (system_error & e)
			{
				set_last_error(e);

				this->derived()._do_disconnect(e.code());
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
			this->derived()._do_disconnect(asio::error::operation_aborted);
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
			detail::ignore_unused(this_ptr, condition);

			// reset the variable to default status
			this->reset_connect_time();
			this->update_alive_time();

			if constexpr (std::is_same_v<MatchCondition, use_dgram_t>)
				this->dgram_ = true;
			else
				this->dgram_ = false;

			// set keeplive options
			this->keep_alive_options();
		}

		template<typename MatchCondition>
		inline void _do_start(std::shared_ptr<derived_t> this_ptr,
			condition_wrap<MatchCondition> condition)
		{
			this->derived()._join_session(std::move(this_ptr), std::move(condition));
		}

		inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			detail::ignore_unused(ec, this_ptr);

			this->derived()._rdc_stop();

			this->derived()._do_stop(ec);
		}

		inline void _do_stop(const error_code& ec)
		{
			detail::ignore_unused(ec);

			// call the base class stop function
			super::stop();

			// call socket's close function to notify the _handle_recv function response with error > 0 ,
			// then the socket can get notify to exit
			// Call shutdown() to indicate that you will not write any more data to the socket.
			this->socket_.lowest_layer().shutdown(asio::socket_base::shutdown_both, ec_ignore);
			// Call close,otherwise the _handle_recv will never return
			this->socket_.lowest_layer().close(ec_ignore);
		}

		template<typename MatchCondition>
		inline void _join_session(std::shared_ptr<derived_t> this_ptr,
			condition_wrap<MatchCondition> condition)
		{
			this->sessions_.emplace(this_ptr, [this, this_ptr, condition = std::move(condition)]
			(bool inserted) mutable
			{
				if (inserted)
					this->derived()._start_recv(std::move(this_ptr), std::move(condition));
				else
					this->derived()._do_disconnect(asio::error::address_in_use);
			});
		}

		template<typename MatchCondition>
		inline void _start_recv(std::shared_ptr<derived_t> this_ptr,
			condition_wrap<MatchCondition> condition)
		{
			// to avlid the user call stop in another thread,then it may be socket_.async_read_some
			// and socket_.close be called at the same time
			asio::post(this->io_.strand(), make_allocator(this->rallocator_,
				[this, self_ptr = std::move(this_ptr), condition = std::move(condition)]() mutable
			{
				if constexpr (!std::is_same_v<MatchCondition, asio2::detail::hook_buffer_t>)
				{
					this->derived().buffer().consume(this->derived().buffer().size());
				}
				else
				{
					std::ignore = true;
				}

				// start the timer of check silence timeout
				this->derived()._post_silence_timer(this->silence_timeout_, self_ptr);

				this->derived()._post_recv(std::move(self_ptr), std::move(condition));
			}));
		}

		template<class Data, class Callback>
		inline bool _do_send(Data& data, Callback&& callback)
		{
			return this->derived()._tcp_send(data, std::forward<Callback>(callback));
		}

		template<class Data>
		inline send_data_t _rdc_convert_to_send_data(Data& data)
		{
			auto buffer = asio::buffer(data);
			return send_data_t{ reinterpret_cast<
				std::string_view::const_pointer>(buffer.data()),buffer.size() };
		}

		template<class Invoker>
		inline void _rdc_invoke_with_none(const error_code& ec, Invoker& invoker)
		{
			invoker(ec, send_data_t{}, recv_data_t{});
		}

		template<class Invoker>
		inline void _rdc_invoke_with_recv(const error_code& ec, Invoker& invoker, recv_data_t data)
		{
			invoker(ec, send_data_t{}, data);
		}

		template<class Invoker, class FnData>
		inline void _rdc_invoke_with_send(const error_code& ec, Invoker& invoker, FnData& fn_data)
		{
			invoker(ec, fn_data(), recv_data_t{});
		}

	protected:
		template<typename MatchCondition>
		inline void _post_recv(std::shared_ptr<derived_t> this_ptr,
			condition_wrap<MatchCondition> condition)
		{
			this->derived()._tcp_post_recv(std::move(this_ptr), std::move(condition));
		}

		template<typename MatchCondition>
		inline void _handle_recv(const error_code& ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			this->derived()._tcp_handle_recv(ec, bytes_recvd, std::move(this_ptr), std::move(condition));
		}

		template<typename MatchCondition>
		inline void _fire_recv(std::shared_ptr<derived_t>& this_ptr, std::string_view s,
			condition_wrap<MatchCondition>& condition)
		{
			this->listener_.notify(event_type::recv, this_ptr, s);

			this->derived()._rdc_handle_recv(this_ptr, s, condition);
		}

		inline void _fire_accept(std::shared_ptr<derived_t>& this_ptr)
		{
			this->listener_.notify(event_type::accept, this_ptr);
		}

		template<typename MatchCondition>
		inline void _fire_connect(std::shared_ptr<derived_t>& this_ptr,
			condition_wrap<MatchCondition>& condition)
		{
			if constexpr (is_template_instance_of_v<use_rdc_t, MatchCondition>)
			{
				this->derived()._rdc_start();
				this->derived()._rdc_post_wait(this_ptr, condition);
			}
			else
			{
				std::ignore = true;
			}

			this->listener_.notify(event_type::connect, this_ptr);
		}

		inline void _fire_disconnect(std::shared_ptr<derived_t>& this_ptr)
		{
			this->listener_.notify(event_type::disconnect, this_ptr);
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
	class tcp_session : public detail::tcp_session_impl_t<tcp_session, detail::template_args_tcp_session>
	{
	public:
		using tcp_session_impl_t<tcp_session, detail::template_args_tcp_session>::tcp_session_impl_t;
	};
}

#endif // !__ASIO2_TCP_SESSION_HPP__
