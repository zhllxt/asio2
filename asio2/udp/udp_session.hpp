/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_UDP_SESSION_HPP__
#define __ASIO2_UDP_SESSION_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/session.hpp>
#include <asio2/udp/impl/udp_send_op.hpp>
#include <asio2/udp/detail/kcp_util.hpp>

namespace asio2::detail
{
	template <class>                      class session_mgr_t;
	template <class, class>               class udp_server_impl_t;

	template<class derived_t, class socket_t, class buffer_t>
	class udp_session_impl_t
		: public session_impl_t<derived_t, socket_t, buffer_t>
		, public udp_send_op<derived_t, true>
	{
		template <class, bool>                friend class user_timer_cp;
		template <class>                      friend class post_cp;
		template <class, bool>                friend class silence_timer_cp;
		template <class, bool>                friend class connect_timeout_cp;
		template <class, class, bool>         friend class connect_cp;
		template <class, class, bool>         friend class disconnect_cp;
		template <class>                      friend class data_persistence_cp;
		template <class>                      friend class event_queue_cp;
		template <class, bool>                friend class send_cp;
		template <class, bool>                friend class udp_send_op;
		template <class, bool>                friend class kcp_stream_cp;
		template <class>                      friend class session_mgr_t;
		template <class, class, class>        friend class session_impl_t;
		template <class, class>               friend class udp_server_impl_t;

	public:
		using self = udp_session_impl_t<derived_t, socket_t, buffer_t>;
		using super = session_impl_t<derived_t, socket_t, buffer_t>;
		using key_type = asio::ip::udp::endpoint;
		using buffer_type = buffer_t;
		using super::send;

		/**
		 * @constructor
		 */
		explicit udp_session_impl_t(
			session_mgr_t<derived_t> & sessions,
			listener_t & listener,
			io_t & rwio,
			std::size_t init_buffer_size,
			std::size_t max_buffer_size,
			asio2::buffer_wrap<asio2::linear_buffer> & buffer,
			socket_t socket,
			asio::ip::udp::endpoint & endpoint
		)
			: super(sessions, listener, rwio, init_buffer_size, max_buffer_size, socket)
			, udp_send_op<derived_t, true>()
			, buffer_ref_(buffer)
			, remote_endpoint_(endpoint)
			, wallocator_()
		{
			this->silence_timeout(std::chrono::milliseconds(udp_silence_timeout));
			this->connect_timeout(std::chrono::milliseconds(udp_connect_timeout));
		}

		/**
		 * @destructor
		 */
		~udp_session_impl_t()
		{
		}

	protected:
		/**
		 * @function : start this session for prepare to recv msg
		 */
		template<typename MatchCondition>
		inline void start(condition_wrap<MatchCondition> condition)
		{
			try
			{
				state_t expected = state_t::stopped;
				if (!this->state_.compare_exchange_strong(expected, state_t::starting))
					asio::detail::throw_error(asio::error::already_started);

				std::shared_ptr<derived_t> this_ptr = this->derived().selfptr();

				this->derived()._do_init(this_ptr, condition);

				// First call the base class start function
				super::start();

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

	public:
		/**
		 * @function : get the remote address
		 */
		inline std::string remote_address()
		{
			try
			{
				return this->remote_endpoint_.address().to_string();
			}
			catch (system_error & e) { set_last_error(e); }
			return std::string();
		}

		/**
		 * @function : get the remote port
		 */
		inline unsigned short remote_port()
		{
			try
			{
				return this->remote_endpoint_.port();
			}
			catch (system_error & e) { set_last_error(e); }
			return static_cast<unsigned short>(0);
		}

		/**
		 * @function : get this object hash key,used for session map
		 */
		inline const key_type & hash_key() const
		{
			return this->remote_endpoint_;
		}

		/**
		 * @function : get the kcp pointer, just used for kcp mode
		 * default mode : ikcp_nodelay(kcp, 0, 10, 0, 0);
		 * generic mode : ikcp_nodelay(kcp, 0, 10, 0, 1);
		 * fast    mode : ikcp_nodelay(kcp, 1, 10, 2, 1);
		 */
		inline kcp::ikcpcb* kcp()
		{
			return (this->kcp_ ? this->kcp_->kcp_ : nullptr);
		}

	protected:
		template<typename MatchCondition>
		inline void _do_init(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			detail::ignore::unused(this_ptr, condition);

			// reset the variable to default status
			this->reset_connect_time();
			this->update_alive_time();
		}

		template<typename MatchCondition>
		inline void _handle_connect(const error_code& ec, std::shared_ptr<derived_t> this_ptr,
			condition_wrap<MatchCondition> condition)
		{
			if constexpr (std::is_same_v<MatchCondition, use_kcp_t>)
			{
				// step 3 : server recvd syn from client (the first_ is syn)
				// Check whether the first_ packet is SYN handshake
				if (!kcp::is_kcphdr_syn(this->first_))
				{
					set_last_error(asio::error::no_protocol_option);
					this->derived()._fire_handshake(this_ptr, asio::error::no_protocol_option);
					this->derived()._do_disconnect(asio::error::no_protocol_option);
					return;
				}
				this->kcp_ = std::make_unique<kcp_stream_cp<derived_t, true>>(this->derived(), this->io_);
				this->kcp_->_post_handshake(std::move(this_ptr), std::move(condition));
			}
			else
			{
				this->kcp_.reset();
				this->derived()._done_connect(ec, std::move(this_ptr), std::move(condition));
			}
		}

		template<typename MatchCondition>
		inline void _do_start(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			this->derived()._join_session(std::move(this_ptr), condition);
		}

		inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			detail::ignore::unused(ec, this_ptr);

			this->derived()._do_stop(ec);
		}

		inline void _do_stop(const error_code& ec)
		{
			detail::ignore::unused(ec);

			// call the base class stop function
			super::stop();

			if (this->kcp_)
				this->kcp_->_kcp_stop();
		}

		template<typename MatchCondition>
		inline void _join_session(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			this->sessions_.emplace(this_ptr, [this, this_ptr, condition](bool inserted) mutable
			{
				if (inserted)
					this->derived()._start_recv(std::move(this_ptr), condition);
				else
					this->derived()._do_disconnect(asio::error::address_in_use);
			});
		}

		template<typename MatchCondition>
		inline void _start_recv(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			// start the timer of check silence timeout
			this->derived()._post_silence_timer(this->silence_timeout_, this_ptr);

			if constexpr (std::is_same_v<MatchCondition, use_kcp_t>)
				std::ignore = true;
			else
				this->derived()._handle_recv(error_code{}, this->first_, this_ptr, condition);
		}

	protected:
		template<class Data, class Callback>
		inline bool _do_send(Data& data, Callback&& callback)
		{
			if (!this->kcp_)
				return this->derived()._udp_send_to(
					this->remote_endpoint_, data, std::forward<Callback>(callback));
			return this->kcp_->_kcp_send(data, std::forward<Callback>(callback));
		}

	protected:
		template<typename MatchCondition>
		inline void _handle_recv(const error_code& ec, std::string_view s,
			std::shared_ptr<derived_t>& this_ptr, condition_wrap<MatchCondition> condition)
		{
			if (!this->is_started())
				return;

			this->update_alive_time();

			if constexpr (!std::is_same_v<MatchCondition, use_kcp_t>)
			{
				this->derived()._fire_recv(this_ptr, std::move(s));
			}
			else
			{
				if (s.size() == sizeof(kcp::kcphdr))
				{
					if /**/ (kcp::is_kcphdr_fin(s))
					{
						this->kcp_->send_fin_ = false;
						this->derived()._do_disconnect(asio::error::eof);
					}
					// Check whether the packet is SYN handshake
					// It is possible that the client did not receive the synack package, then the client
					// will resend the syn package, so we just need to reply to the syncack package directly.
					else if (kcp::is_kcphdr_syn(s))
					{
						ASIO2_ASSERT(this->kcp_ && this->kcp_->kcp_);
						// step 4 : server send synack to client
						kcp::kcphdr * hdr = (kcp::kcphdr*)(s.data());
						kcp::kcphdr synack = kcp::make_kcphdr_synack(this->kcp_->seq_, hdr->th_seq);
						error_code ed;
						this->kcp_->_kcp_send_hdr(synack, ed);
						if (ed)
							this->derived()._do_disconnect(ed);
					}
				}
				else
					this->kcp_->_kcp_recv(this_ptr, s, this->buffer_ref_);
			}
		}

		inline void _fire_recv(std::shared_ptr<derived_t>& this_ptr, std::string_view s)
		{
			this->listener_.notify(event::recv, this_ptr, std::move(s));
		}

		inline void _fire_handshake(std::shared_ptr<derived_t>& this_ptr, error_code ec)
		{
			this->listener_.notify(event::handshake, this_ptr, ec);
		}

		inline void _fire_connect(std::shared_ptr<derived_t>& this_ptr)
		{
			this->listener_.notify(event::connect, this_ptr);
		}

		inline void _fire_disconnect(std::shared_ptr<derived_t>& this_ptr)
		{
			this->listener_.notify(event::disconnect, this_ptr);
		}

	protected:
		/**
		 * @function : get the recv/read allocator object refrence
		 */
		inline auto & rallocator() { return this->wallocator_; }
		/**
		 * @function : get the send/write allocator object refrence
		 */
		inline auto & wallocator() { return this->wallocator_; }

	protected:
		/// buffer
		asio2::buffer_wrap<asio2::linear_buffer>      & buffer_ref_;

		/// used for session_mgr's session unordered_map key
		asio::ip::udp::endpoint                         remote_endpoint_;

		/// The memory to use for handler-based custom memory allocation. used fo send/write.
		handler_memory<size_op<>, std::true_type>       wallocator_;

		std::unique_ptr<kcp_stream_cp<derived_t, true>> kcp_;

		/// first recvd data packet
		std::string_view                                first_;
	};
}

namespace asio2
{
	class udp_session : public detail::udp_session_impl_t<udp_session, asio::ip::udp::socket&, detail::empty_buffer>
	{
	public:
		using udp_session_impl_t<udp_session, asio::ip::udp::socket&, detail::empty_buffer>::udp_session_impl_t;
	};
}

#endif // !__ASIO2_UDP_SESSION_HPP__
