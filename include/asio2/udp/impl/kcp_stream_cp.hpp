/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_KCP_STREAM_CP_HPP__
#define __ASIO2_KCP_STREAM_CP_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/iopool.hpp>
#include <asio2/base/define.hpp>
#include <asio2/base/listener.hpp>
#include <asio2/base/session_mgr.hpp>

#include <asio2/base/detail/object.hpp>
#include <asio2/base/detail/allocator.hpp>
#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/buffer_wrap.hpp>

#include <asio2/udp/detail/kcp_util.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_UDP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_UDP_CLIENT;
	ASIO2_CLASS_FORWARD_DECLARE_UDP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_UDP_SESSION;

	/*
	 * because udp is connectionless, in order to simplify the code logic, KCP shakes
	 * hands only twice (compared with TCP handshakes three times)
	 * 1 : client send syn to server
	 * 2 : server send synack to client
	 */
	template<class derived_t, class args_t>
	class kcp_stream_cp
	{
		friend derived_t; // C++11

		ASIO2_CLASS_FRIEND_DECLARE_UDP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_UDP_CLIENT;
		ASIO2_CLASS_FRIEND_DECLARE_UDP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_UDP_SESSION;

	public:
		/**
		 * @brief constructor
		 */
		kcp_stream_cp(derived_t& d, asio::io_context& ioc)
			: derive(d), kcp_timer_(ioc)
		{
		}

		/**
		 * @brief destructor
		 */
		~kcp_stream_cp() noexcept
		{
			if (this->kcp_)
			{
				kcp::ikcp_release(this->kcp_);
				this->kcp_ = nullptr;
			}
		}

		/**
		 * @brief The udp protocol may received some illegal data. Use this function 
		 * to set up an illegal data handler. The default illegal data handler is empty,
		 * when recvd illegal data, the default illegal data handler will do nothing.
		 */
		inline void set_illegal_response_handler(std::function<void(std::string_view)> fn) noexcept
		{
			this->illegal_response_handler_ = std::move(fn);
		}

	protected:
		void _kcp_start(std::shared_ptr<derived_t> this_ptr, std::uint32_t conv)
		{
			// used to restore configs
			kcp::ikcpcb* old = this->kcp_;

			struct old_kcp_destructor
			{
				 old_kcp_destructor(kcp::ikcpcb* p) : p_(p) {}
				~old_kcp_destructor()
				{
					if (p_)
						kcp::ikcp_release(p_);
				}

				kcp::ikcpcb* p_ = nullptr;
			} old_kcp_destructor_guard(old);

			ASIO2_ASSERT(conv != 0);

			this->kcp_ = kcp::ikcp_create(conv, (void*)this);
			this->kcp_->output = &kcp_stream_cp<derived_t, args_t>::_kcp_output;

			if (old)
			{
				// ikcp_setmtu
				kcp::ikcp_setmtu(this->kcp_, old->mtu);

				// ikcp_wndsize
				kcp::ikcp_wndsize(this->kcp_, old->snd_wnd, old->rcv_wnd);

				// ikcp_nodelay
				kcp::ikcp_nodelay(this->kcp_, old->nodelay, old->interval, old->fastresend, old->nocwnd);
			}
			else
			{
				kcp::ikcp_nodelay(this->kcp_, 1, 10, 2, 1);
				kcp::ikcp_wndsize(this->kcp_, 128, 512);
			}

			// if call kcp_timer_.cancel first, then call _post_kcp_timer second immediately,
			// use asio::post to avoid start timer failed.
			asio::post(derive.io_->context(), make_allocator(derive.wallocator(),
			[this, this_ptr = std::move(this_ptr)]() mutable
			{
				this->_post_kcp_timer(std::move(this_ptr));
			}));
		}

		void _kcp_stop()
		{
			error_code ec_ignore{};

			// if is kcp mode, send FIN handshake before close
			if (this->send_fin_)
				this->_kcp_send_hdr(kcp::make_kcphdr_fin(0), ec_ignore);

			detail::cancel_timer(this->kcp_timer_);
		}

		inline void _kcp_reset()
		{
			kcp::ikcp_reset(this->kcp_);
		}

	protected:
		inline std::size_t _kcp_send_hdr(kcp::kcphdr hdr, error_code& ec)
		{
			std::string msg = kcp::to_string(hdr);
			std::size_t sent_bytes = 0;

		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(derive.post_send_counter_.load() == 0);
			derive.post_send_counter_++;
		#endif

			if constexpr (args_t::is_session)
				sent_bytes = derive.stream().send_to(asio::buffer(msg), derive.remote_endpoint_, 0, ec);
			else
				sent_bytes = derive.stream().send(asio::buffer(msg), 0, ec);

		#if defined(_DEBUG) || defined(DEBUG)
			derive.post_send_counter_--;
		#endif

			return sent_bytes;
		}

		inline std::size_t _kcp_send_syn(std::uint32_t seq, error_code& ec)
		{
			kcp::kcphdr syn = kcp::make_kcphdr_syn(derive.kcp_conv_, seq);
			return this->_kcp_send_hdr(syn, ec);
		}

		inline std::size_t _kcp_send_synack(kcp::kcphdr syn, error_code& ec)
		{
			// the syn.th_ack is the kcp conv
			kcp::kcphdr synack = kcp::make_kcphdr_synack(syn.th_ack, syn.th_seq);
			return this->_kcp_send_hdr(synack, ec);
		}

		template<class Data, class Callback>
		inline bool _kcp_send(Data& data, Callback&& callback)
		{
		#if defined(ASIO2_ENABLE_LOG)
			static_assert(decltype(tallocator_)::storage_size == 168);
		#endif

			auto buffer = asio::buffer(data);

		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(derive.post_send_counter_.load() == 0);
			derive.post_send_counter_++;
		#endif

			int ret = kcp::ikcp_send(this->kcp_, (const char *)buffer.data(), (int)buffer.size());

		#if defined(_DEBUG) || defined(DEBUG)
			derive.post_send_counter_--;
		#endif

			switch (ret)
			{
			case  0: set_last_error(error_code{}                        ); break;
			case -1: set_last_error(asio::error::invalid_argument       ); break;
			case -2: set_last_error(asio::error::no_memory              ); break;
			default: set_last_error(asio::error::operation_not_supported); break;
			}
			if (ret == 0)
			{
				kcp::ikcp_flush(this->kcp_);
			}
			callback(get_last_error(), ret < 0 ? 0 : buffer.size());

			return (ret == 0);
		}

		void _post_kcp_timer(std::shared_ptr<derived_t> this_ptr)
		{
			std::uint32_t clock1 = static_cast<std::uint32_t>(std::chrono::duration_cast<
				std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
			std::uint32_t clock2 = kcp::ikcp_check(this->kcp_, clock1);

			this->kcp_timer_.expires_after(std::chrono::milliseconds(clock2 - clock1));
			this->kcp_timer_.async_wait(make_allocator(this->tallocator_,
			[this, this_ptr = std::move(this_ptr)](const error_code & ec) mutable
			{
				this->_handle_kcp_timer(ec, std::move(this_ptr));
			}));
		}

		void _handle_kcp_timer(const error_code & ec, std::shared_ptr<derived_t> this_ptr)
		{
			if (ec == asio::error::operation_aborted) return;

			std::uint32_t clock = static_cast<std::uint32_t>(std::chrono::duration_cast<
				std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
			kcp::ikcp_update(this->kcp_, clock);
			if (this->kcp_->state == (kcp::IUINT32)-1)
			{
				if (derive.state_ == state_t::started)
				{
					derive._do_disconnect(asio::error::network_reset, std::move(this_ptr));
				}
				return;
			}
			if (derive.is_started())
				this->_post_kcp_timer(std::move(this_ptr));
		}

		template<typename C>
		void _kcp_recv(
			std::shared_ptr<derived_t>& this_ptr, std::shared_ptr<ecs_t<C>>& ecs, std::string_view data)
		{
			auto& buffer = derive.buffer();

			int len = kcp::ikcp_input(this->kcp_, (const char *)data.data(), (long)data.size());

			buffer.consume(buffer.size());

			if (len != 0)
			{
				set_last_error(asio::error::message_size);

				this->_call_illegal_data_callback(data);

				return;
			}

			for (;;)
			{
				len = kcp::ikcp_recv(this->kcp_, (char *)buffer.prepare(
					buffer.pre_size()).data(), (int)buffer.pre_size());

				if /**/ (len >= 0)
				{
					buffer.commit(len);

					derive._fire_recv(this_ptr, ecs, std::string_view(static_cast
						<std::string_view::const_pointer>(buffer.data().data()), len));

					buffer.consume(len);
				}
				else if (len == -3)
				{
					buffer.pre_size((std::min)(buffer.pre_size() * 2, buffer.max_size()));
				}
				else
				{
					break;
				}
			}

			kcp::ikcp_flush(this->kcp_);
		}

		template<typename C>
		inline void _kcp_handle_recv(
			error_code ec, std::string_view data,
			std::shared_ptr<derived_t>& this_ptr, std::shared_ptr<ecs_t<C>>& ecs)
		{
			// the kcp message header length is 24
			// the kcphdr length is 12 
			if /**/ (data.size() > kcp::kcphdr::required_size())
			{
				this->_kcp_recv(this_ptr, ecs, data);
			}
			else if (data.size() == kcp::kcphdr::required_size())
			{
				// Check whether the packet is SYN handshake
				// It is possible that the client did not receive the synack package, then the client
				// will resend the syn package, so we just need to reply to the syncack package directly.
				// If the client is disconnect without send a "fin" or the server has't recvd the 
				// "fin", and then the client connect again a later, at this time, the client
				// is in the session map already, and we need check whether the first message is fin
				if /**/ (kcp::is_kcphdr_syn(data))
				{
					ASIO2_ASSERT(this->kcp_);

					if (this->kcp_)
					{
						kcp::kcphdr syn = kcp::to_kcphdr(data);
						std::uint32_t conv = syn.th_ack;
						if (conv == 0)
						{
							conv = this->kcp_->conv;
							syn.th_ack = conv;
						}

						// If the client is forced disconnect after sent some messages, and the server
						// has recvd the messages already, we must recreated the kcp object, otherwise
						// the client and server will can't handle the next messages correctly.
					#if 0
						// set send_fin_ = false to make the _kcp_stop don't sent the fin frame.
						this->send_fin_ = false;

						this->_kcp_stop();

						this->_kcp_start(this_ptr, conv);
					#else
						this->_kcp_reset();
					#endif

						this->send_fin_ = true;

						// every time we recv kcp syn, we sent synack to the client
						this->_kcp_send_synack(syn, ec);
						if (ec)
						{
							derive._do_disconnect(ec, this_ptr);
						}
					}
					else
					{
						derive._do_disconnect(asio::error::operation_aborted, this_ptr);
					}
				}
				else if (kcp::is_kcphdr_synack(data, 0, true))
				{
					ASIO2_ASSERT(this->kcp_);
				}
				else if (kcp::is_kcphdr_ack(data, 0, true))
				{
					ASIO2_ASSERT(this->kcp_);
				}
				else if (kcp::is_kcphdr_fin(data))
				{
					ASIO2_ASSERT(this->kcp_);

					this->send_fin_ = false;

					derive._do_disconnect(asio::error::connection_reset, this_ptr);
				}
				else
				{
					this->_call_illegal_data_callback(data);
				}
			}
			else
			{
				this->_call_illegal_data_callback(data);
			}
		}

		template<typename C, typename DeferEvent>
		void _session_post_handshake(
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			error_code ec;

			std::string& data = *(derive.first_data_);

			// step 3 : server recvd syn from client (the first data is the syn)
			kcp::kcphdr syn = kcp::to_kcphdr(data);
			std::uint32_t conv = syn.th_ack;
			if (conv == 0)
			{
				conv = derive.kcp_conv_;
				syn.th_ack = conv;
			}

			// step 4 : server send synack to client
			this->_kcp_send_synack(syn, ec);

			if (ec)
			{
				derive._do_disconnect(ec, std::move(this_ptr), std::move(chain));
				return;
			}

			this->_kcp_start(this_ptr, conv);
			this->_handle_handshake(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
		}

		template<typename C, typename DeferEvent>
		void _client_post_handshake(
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			error_code ec;

			// step 1 : client send syn to server
			std::uint32_t seq = static_cast<std::uint32_t>(
				std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch()).count());

			this->_kcp_send_syn(seq, ec);

			if (ec)
			{
				derive._do_disconnect(ec, std::move(this_ptr), defer_event(chain.move_guard()));
				return;
			}

			// use a loop timer to execute "client send syn to server" until the server
			// has recvd the syn packet and this client recvd reply.
			std::shared_ptr<detail::safe_timer> timer =
				mktimer(derive.io_->context(), std::chrono::milliseconds(500),
			[this, this_ptr, seq](error_code ec) mutable
			{
				if (ec == asio::error::operation_aborted)
					return false;
				this->_kcp_send_syn(seq, ec);
				if (ec)
				{
					set_last_error(ec);
					if (derive.state_ == state_t::started)
					{
						derive._do_disconnect(ec, std::move(this_ptr));
					}
					return false;
				}
				// return true  : let the timer continue execute.
				// return false : kill the timer.
				return true;
			});

		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(derive.post_recv_counter_.load() == 0);
			derive.post_recv_counter_++;
		#endif

			// step 2 : client wait for recv synack util connect timeout or recvd some data
			derive.socket().async_receive(derive.buffer().prepare(derive.buffer().pre_size()),
				make_allocator(derive.rallocator(),
			[this, this_ptr = std::move(this_ptr), ecs = std::move(ecs), chain = std::move(chain),
				seq, timer = std::move(timer)]
			(const error_code & ec, std::size_t bytes_recvd) mutable
			{
			#if defined(_DEBUG) || defined(DEBUG)
				derive.post_recv_counter_--;
			#endif

				ASIO2_ASSERT(derive.io_->running_in_this_thread());

				timer->cancel();

				if (ec)
				{
					// if connect_timeout_timer_ is empty, it means that the connect timeout timer is
					// timeout and the callback has called already, so reset the error to timed_out.
					// note : when the async_resolve is failed, the socket is invalid to.
					this->_handle_handshake(
						derive.connect_timeout_timer_ ? ec : asio::error::timed_out,
						std::move(this_ptr), std::move(ecs), std::move(chain));
					return;
				}

				derive.buffer().commit(bytes_recvd);

				std::string_view data = std::string_view(static_cast<std::string_view::const_pointer>
					(derive.buffer().data().data()), bytes_recvd);

				// Check whether the data is the correct handshake information
				if (kcp::is_kcphdr_synack(data, seq))
				{
					kcp::kcphdr hdr = kcp::to_kcphdr(data);
					std::uint32_t conv = hdr.th_seq;
					if (derive.kcp_conv_ != 0)
					{
						ASIO2_ASSERT(derive.kcp_conv_ == conv);
					}
					this->_kcp_start(this_ptr, conv);
					this->_handle_handshake(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
				}
				else
				{
					this->_handle_handshake(asio::error::address_family_not_supported,
						std::move(this_ptr), std::move(ecs), std::move(chain));
				}

				derive.buffer().consume(bytes_recvd);
			}));
		}

		template<typename C, typename DeferEvent>
		void _post_handshake(std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			if constexpr (args_t::is_session)
			{
				this->_session_post_handshake(std::move(this_ptr), std::move(ecs), std::move(chain));
			}
			else
			{
				this->_client_post_handshake(std::move(this_ptr), std::move(ecs), std::move(chain));
			}
		}

		template<typename C, typename DeferEvent>
		void _handle_handshake(
			const error_code& ec, std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs,
			DeferEvent chain)
		{
			set_last_error(ec);

			if constexpr (args_t::is_session)
			{
				derive._fire_handshake(this_ptr);

				if (ec)
				{
					derive._do_disconnect(ec, std::move(this_ptr), std::move(chain));

					return;
				}

				derive._done_connect(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
			}
			else
			{
				derive._fire_handshake(this_ptr);

				derive._done_connect(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
			}
		}

		static int _kcp_output(const char *buf, int len, kcp::ikcpcb *kcp, void *user)
		{
			std::ignore = kcp;

			kcp_stream_cp * zhis = ((kcp_stream_cp*)user);

			derived_t & derive = zhis->derive;

			error_code ec;
			if constexpr (args_t::is_session)
				derive.stream().send_to(asio::buffer(buf, len), derive.remote_endpoint_, 0, ec);
			else
				derive.stream().send(asio::buffer(buf, len), 0, ec);

			return 0;
		}

		inline void _call_illegal_data_callback(std::string_view data)
		{
			if (this->illegal_response_handler_)
			{
				this->illegal_response_handler_(data);
			}
		}

	protected:
		derived_t                                    & derive;
									                          
		kcp::ikcpcb                                  * kcp_ = nullptr;
									                          
		bool                                           send_fin_ = true;

		asio::steady_timer                             kcp_timer_;

		handler_memory<std::true_type, allocator_fixed_size_op<168>> tallocator_;

		std::function<void(std::string_view)>          illegal_response_handler_;
	};
}

#endif // !__ASIO2_KCP_STREAM_CP_HPP__
