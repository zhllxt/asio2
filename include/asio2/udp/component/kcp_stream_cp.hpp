/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
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
		ASIO2_CLASS_FRIEND_DECLARE_UDP_CLIENT;
		ASIO2_CLASS_FRIEND_DECLARE_UDP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_UDP_SESSION;

	public:
		/**
		 * @constructor
		 */
		kcp_stream_cp(derived_t& d, io_t& io)
			: derive(d), kcp_timer_(io.context())
		{
		}

		/**
		 * @destructor
		 */
		~kcp_stream_cp() noexcept
		{
			if (this->kcp_)
			{
				kcp::ikcp_release(this->kcp_);
				this->kcp_ = nullptr;
			}
		}

	protected:
		inline void _kcp_start(std::shared_ptr<derived_t> this_ptr, std::uint32_t conv)
		{
			if (this->kcp_)
			{
				kcp::ikcp_release(this->kcp_);
				this->kcp_ = nullptr;
			}

			this->kcp_ = kcp::ikcp_create(conv, (void*)this);
			this->kcp_->output = &kcp_stream_cp<derived_t, args_t>::_kcp_output;

			kcp::ikcp_nodelay(this->kcp_, 1, 10, 2, 1);
			kcp::ikcp_wndsize(this->kcp_, 128, 512);

			this->_post_kcp_timer(std::move(this_ptr));
		}

		inline void _kcp_stop()
		{
			error_code ec_ignore{};

			// if is kcp mode, send FIN handshake before close
			if (this->send_fin_)
				this->_kcp_send_hdr(kcp::make_kcphdr_fin(0), ec_ignore);

			try
			{
				this->kcp_timer_.cancel();
			}
			catch (system_error const&)
			{
			}
		}

	protected:
		inline std::size_t _kcp_send_hdr(kcp::kcphdr hdr, error_code& ec)
		{
			std::string msg = kcp::to_string(hdr);
			std::size_t sent_bytes = 0;
			if constexpr (args_t::is_session)
				sent_bytes = derive.stream().send_to(asio::buffer(msg), derive.remote_endpoint_, 0, ec);
			else
				sent_bytes = derive.stream().send(asio::buffer(msg), 0, ec);
			return sent_bytes;
		}

		template<class Data, class Callback>
		inline bool _kcp_send(Data& data, Callback&& callback)
		{
			auto buffer = asio::buffer(data);

			int ret = kcp::ikcp_send(this->kcp_, (const char *)buffer.data(), (int)buffer.size());
			switch (ret)
			{
			case  0: set_last_error(error_code{}                        ); break;
			case -1: set_last_error(asio::error::invalid_argument       ); break;
			case -2: set_last_error(asio::error::no_memory              ); break;
			default: set_last_error(asio::error::operation_not_supported); break;
			}
			if (ret == 0)
				kcp::ikcp_flush(this->kcp_);
			callback(get_last_error(), ret < 0 ? 0 : buffer.size());

			return (ret == 0);
		}

		inline void _post_kcp_timer(std::shared_ptr<derived_t> this_ptr)
		{
			std::uint32_t clock1 = static_cast<std::uint32_t>(std::chrono::duration_cast<
				std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
			std::uint32_t clock2 = kcp::ikcp_check(this->kcp_, clock1);

			this->kcp_timer_.expires_after(std::chrono::milliseconds(clock2 - clock1));
			this->kcp_timer_.async_wait(make_allocator(this->tallocator_,
			[this, self_ptr = std::move(this_ptr)](const error_code & ec) mutable
			{
				this->_handle_kcp_timer(ec, std::move(self_ptr));
			}));
		}

		inline void _handle_kcp_timer(const error_code & ec, std::shared_ptr<derived_t> this_ptr)
		{
			if (ec == asio::error::operation_aborted) return;

			std::uint32_t clock = static_cast<std::uint32_t>(std::chrono::duration_cast<
				std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
			kcp::ikcp_update(this->kcp_, clock);
			if (derive.is_started())
				this->_post_kcp_timer(std::move(this_ptr));
		}

		template<class buffer_t, typename MatchCondition>
		inline void _kcp_recv(std::shared_ptr<derived_t>& this_ptr, std::string_view data, buffer_t& buffer,
			condition_wrap<MatchCondition>& condition)
		{
			int len = kcp::ikcp_input(this->kcp_, (const char *)data.data(), (long)data.size());
			buffer.consume(buffer.size());
			if (len != 0)
			{
				set_last_error(asio::error::message_size);
				if (derive.state() == state_t::started)
				{
					derive._do_disconnect(asio::error::message_size, this_ptr);
				}
				return;
			}
			for (;;)
			{
				len = kcp::ikcp_recv(this->kcp_, (char *)buffer.prepare(
					buffer.pre_size()).data(), (int)buffer.pre_size());
				if /**/ (len >= 0)
				{
					buffer.commit(len);
					derive._fire_recv(this_ptr, std::string_view(static_cast
						<std::string_view::const_pointer>(buffer.data().data()), len), condition);
					buffer.consume(len);
				}
				else if (len == -3)
				{
					buffer.pre_size(buffer.pre_size() * 2);
				}
				else break;
			}
			kcp::ikcp_flush(this->kcp_);
		}

		template<typename MatchCondition, typename DeferEvent>
		inline void _post_handshake(
			std::shared_ptr<derived_t> self_ptr, condition_wrap<MatchCondition> condition, DeferEvent chain)
		{
			try
			{
				error_code ec;
				if constexpr (args_t::is_session)
				{
					// step 3 : server recvd syn from client (the first_ is the syn)

					// step 4 : server send synack to client
					kcp::kcphdr hdr = kcp::to_kcphdr(derive.first_);
					const auto & key = derive.hash_key();
					std::uint32_t conv = fnv1a_hash<std::uint32_t>(
						(const unsigned char * const)&key, std::uint32_t(sizeof(key)));
					this->seq_ = conv;
					kcp::kcphdr synack = kcp::make_kcphdr_synack(this->seq_, hdr.th_seq);
					this->_kcp_send_hdr(synack, ec);
					asio::detail::throw_error(ec);

					this->_kcp_start(self_ptr, this->seq_);
					this->_handle_handshake(ec, std::move(self_ptr), std::move(condition), std::move(chain));
				}
				else
				{
					// step 1 : client send syn to server
					this->seq_ = static_cast<std::uint32_t>(
						std::chrono::duration_cast<std::chrono::milliseconds>(
						std::chrono::system_clock::now().time_since_epoch()).count());
					kcp::kcphdr syn = kcp::make_kcphdr_syn(this->seq_);
					this->_kcp_send_hdr(syn, ec);
					asio::detail::throw_error(ec);

					// use a loop timer to execute "client send syn to server" until the server
					// has recvd the syn packet and this client recvd reply.
					std::shared_ptr<asio::steady_timer> timer =
						mktimer(derive.io().context(), std::chrono::milliseconds(500),
						[this, self_ptr, syn](error_code ec) mutable
					{
						if (ec == asio::error::operation_aborted)
							return false;
						this->_kcp_send_hdr(syn, ec);
						if (ec)
						{
							set_last_error(ec);
							if (derive.state() == state_t::started)
							{
								derive._do_disconnect(ec, std::move(self_ptr));
							}
							return false;
						}
						// return true  : let the timer continue execute.
						// return false : kill the timer.
						return true;
					});

					// step 2 : client wait for recv synack util connect timeout or recvd some data
					derive.socket().async_receive(derive.buffer().prepare(derive.buffer().pre_size()),
						make_allocator(derive.rallocator(),
					[this, this_ptr = std::move(self_ptr), condition = std::move(condition),
						timer = std::move(timer), chain = std::move(chain)]
					(const error_code & ec, std::size_t bytes_recvd) mutable
					{
						ASIO2_ASSERT(derive.io().running_in_this_thread());

						try
						{
							timer->cancel();
						}
						catch (system_error const&)
						{
						}

						if (ec)
						{
							// if connect_timeout_timer_ is empty, it means that the connect timeout timer is
							// timeout and the callback has called already, so reset the error to timed_out.
							// note : when the async_resolve is failed, the socket is invalid to.
							this->_handle_handshake(
								derive.connect_timeout_timer_ ? ec : asio::error::timed_out,
								std::move(this_ptr), std::move(condition), std::move(chain));
							return;
						}

						derive.buffer().commit(bytes_recvd);

						std::string_view data = std::string_view(static_cast<std::string_view::const_pointer>
							(derive.buffer().data().data()), bytes_recvd);

						// Check whether the data is the correct handshake information
						if (kcp::is_kcphdr_synack(data, this->seq_))
						{
							kcp::kcphdr hdr = kcp::to_kcphdr(data);
							std::uint32_t conv = hdr.th_seq;
							this->_kcp_start(this_ptr, conv);
							this->_handle_handshake(ec, std::move(this_ptr), std::move(condition), std::move(chain));
						}
						else
						{
							this->_handle_handshake(asio::error::address_family_not_supported,
								std::move(this_ptr), std::move(condition), std::move(chain));
						}

						derive.buffer().consume(bytes_recvd);
					}));
				}
			}
			catch (system_error & e)
			{
				set_last_error(e);

				if constexpr (args_t::is_session)
				{
					derive._do_disconnect(e.code(), derive.selfptr(), std::move(chain));
				}
				else
				{
					derive._do_disconnect(e.code(), derive.selfptr(), defer_event(chain.move_guard()));
				}
			}
		}

		template<typename MatchCondition, typename DeferEvent>
		inline void _handle_handshake(const error_code & ec, std::shared_ptr<derived_t> this_ptr,
			condition_wrap<MatchCondition> condition, DeferEvent chain)
		{
			set_last_error(ec);

			try
			{
				if constexpr (args_t::is_session)
				{
					derive._fire_handshake(this_ptr);

					asio::detail::throw_error(ec);

					derive._done_connect(ec, std::move(this_ptr), std::move(condition), std::move(chain));
				}
				else
				{
					derive._fire_handshake(this_ptr);

					derive._done_connect(ec, std::move(this_ptr), std::move(condition), std::move(chain));
				}
			}
			catch (system_error & e)
			{
				set_last_error(e);

				derive._do_disconnect(e.code(), derive.selfptr(), defer_event(chain.move_guard()));
			}
		}

		static int _kcp_output(const char *buf, int len, kcp::ikcpcb *kcp, void *user)
		{
			std::ignore = kcp;

			kcp_stream_cp * zhis = ((kcp_stream_cp*)user);

			derived_t & derive = zhis->derive;

			error_code ec;
			if constexpr (args_t::is_session)
				derive.stream().send_to(asio::buffer(buf, len),
					derive.remote_endpoint_, 0, ec);
			else
				derive.stream().send(asio::buffer(buf, len), 0, ec);

			return 0;
		}

	protected:
		derived_t                   & derive;

		kcp::ikcpcb                 * kcp_ = nullptr;

		std::uint32_t                 seq_ = 0;

		bool                          send_fin_ = true;

		handler_memory<>              tallocator_;

		asio::steady_timer            kcp_timer_;
	};
}

#endif // !__ASIO2_KCP_STREAM_CP_HPP__
