/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#if defined(ASIO2_USE_SSL)

#ifndef __ASIO2_SSL_STREAM_COMPONENT_HPP__
#define __ASIO2_SSL_STREAM_COMPONENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <future>
#include <utility>
#include <string_view>

#include <asio2/base/iopool.hpp>

#include <asio2/base/detail/allocator.hpp>
#include <asio2/base/detail/condition_wrap.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t>
	class ssl_stream_cp
	{
	public:
		using socket_type    = typename args_t::socket_t;
		using stream_type    = asio::ssl::stream<socket_type&>;
		using handshake_type = typename asio::ssl::stream_base::handshake_type;

		ssl_stream_cp(io_t& ssl_io, asio::ssl::context& ctx, handshake_type type) noexcept
			: ssl_ctx_(ctx)
			, ssl_type_(type)
		{
			detail::ignore_unused(ssl_io);
		}

		~ssl_stream_cp() = default;

		/**
		 * @function : get the ssl socket object refrence
		 */
		inline stream_type & ssl_stream() noexcept
		{
			ASIO2_ASSERT(bool(this->ssl_stream_));
			return (*(this->ssl_stream_));
		}

	protected:
		template<typename MatchCondition>
		inline void _ssl_init(
			const condition_wrap<MatchCondition>& condition,
			socket_type& socket, asio::ssl::context& ctx)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			detail::ignore_unused(derive, condition, socket, ctx);

			if constexpr (args_t::is_client)
			{
				ASIO2_ASSERT(derive.io().running_in_this_thread());
			}
			else
			{
				ASIO2_ASSERT(derive.sessions().io().running_in_this_thread());
			}

			// Why put the initialization code of ssl stream here ?
			// Why not put it in the constructor ?
			// -----------------------------------------------------------------------
			// Beacuse SSL_CTX_use_certificate_chain_file,SSL_CTX_use_PrivateKey and
			// other SSL_CTX_... functions must be called before SSL_new, otherwise,
			// those SSL_CTX_... function calls have no effect.
			// When construct a tcps_client object, beacuse the tcps_client is derived
			// from ssl_stream_cp, so the ssl_stream_cp's constructor will be called,
			// but at this time, the SSL_CTX_... function has not been called.
			this->ssl_stream_ = std::make_unique<stream_type>(socket, ctx);
		}

		template<typename MatchCondition>
		inline void _ssl_start(
			const std::shared_ptr<derived_t>& this_ptr,
			const condition_wrap<MatchCondition>& condition,
			socket_type& socket, asio::ssl::context& ctx) noexcept
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			detail::ignore_unused(derive, this_ptr, condition, socket, ctx);

			ASIO2_ASSERT(derive.io().running_in_this_thread());
		}

		template<typename DeferEvent>
		inline void _ssl_stop(std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(derive.io().running_in_this_thread());

			if (!this->ssl_stream_)
				return;

			derive.disp_event([this, &derive, this_ptr = std::move(this_ptr), e = chain.move_event()]
			(event_queue_guard<derived_t> g) mutable
			{
				// must construct a new chain
				defer_event chain(std::move(e), std::move(g));

				struct SSL_clear_op
				{
					stream_type* s{};

					// SSL_clear : 
					// Reset ssl to allow another connection. All settings (method, ciphers, BIOs) are kept.

					// When the client auto reconnect, SSL_clear must be called,
					// otherwise the SSL handshake will failed.

					SSL_clear_op(stream_type* p) : s(p) {}
					~SSL_clear_op() { if (s) SSL_clear(s->native_handle()); }
				};

				// use "std::shared_ptr<SSL_clear_op>" to enusre that the SSL_clear(...) function is 
				// called after "socket.shutdown, socket.close, ssl_stream.async_shutdown".
				std::shared_ptr<SSL_clear_op> SSL_clear_ptr =
					std::make_shared<SSL_clear_op>(this->ssl_stream_.get());

				// if the client socket is not closed forever,this async_shutdown
				// callback also can't be called forever, so we use a timer to 
				// force close the socket,then the async_shutdown callback will
				// be called.
				std::shared_ptr<asio::steady_timer> timer = 
					std::make_shared<asio::steady_timer>(derive.io().context());
				timer->expires_after(std::chrono::milliseconds(ssl_shutdown_timeout));
				timer->async_wait(
				[this_ptr, chain = std::move(chain), SSL_clear_ptr]
				(const error_code& ec) mutable
				{
					// note : lambda [chain = std::move(chain), SSL_clear_ptr]
					// SSL_clear_ptr will destroyed first
					// chain will destroyed second after SSL_clear_ptr.

					detail::ignore_unused(this_ptr, chain, SSL_clear_ptr);

					set_last_error(ec);
				});

				// when server call ssl stream sync shutdown first,if the client socket is
				// not closed forever,then here shutdowm will blocking forever.
				this->ssl_stream_->async_shutdown(
				[this_ptr = std::move(this_ptr), timer = std::move(timer), SSL_clear_ptr = std::move(SSL_clear_ptr)]
				(const error_code& ec) mutable
				{
					detail::ignore_unused(this_ptr, SSL_clear_ptr);

					set_last_error(ec);

					try
					{
						// clost the timer
						timer->cancel();
					}
					catch (system_error const&)
					{
					}
				});
			}, chain.move_guard());
		}

		template<typename MatchCondition, typename DeferEvent>
		inline void _post_handshake(
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(bool(this->ssl_stream_));
			ASIO2_ASSERT(derive.io().running_in_this_thread());

			// Used to chech whether the ssl handshake is timeout
			std::shared_ptr<std::atomic_flag> flag_ptr = std::make_shared<std::atomic_flag>();

			flag_ptr->clear();

			std::shared_ptr<asio::steady_timer> timer =
				std::make_shared<asio::steady_timer>(derive.io().context());
			timer->expires_after(std::chrono::milliseconds(ssl_handshake_timeout));
			timer->async_wait(
			[&derive, this_ptr, flag_ptr](const error_code& ec) mutable
			{
				detail::ignore_unused(this_ptr);

				// no errors indicating timed out
				if (!ec)
				{
					flag_ptr->test_and_set();

					error_code ec_ignore{};

					// we close the socket, so the async_handshake will returned 
					// with operation_aborted.
					derive.socket().lowest_layer().shutdown(asio::socket_base::shutdown_both, ec_ignore);
					derive.socket().lowest_layer().close(ec_ignore);
				}
			});

			this->ssl_stream_->async_handshake(this->ssl_type_, make_allocator(derive.wallocator(),
			[&derive, self_ptr = std::move(this_ptr), condition = std::move(condition),
				flag_ptr = std::move(flag_ptr), timer = std::move(timer), chain = std::move(chain)]
			(const error_code& ec) mutable
			{
				try
				{
					// clost the timer
					timer->cancel();
				}
				catch (system_error const&)
				{
				}

				if (flag_ptr->test_and_set())
					derive._handle_handshake(asio::error::timed_out,
						std::move(self_ptr), std::move(condition), std::move(chain));
				else
					derive._handle_handshake(ec,
						std::move(self_ptr), std::move(condition), std::move(chain));
			}));
		}

		template<typename MatchCondition, typename DeferEvent>
		inline void _handle_handshake(const error_code & ec, std::shared_ptr<derived_t> self_ptr,
			condition_wrap<MatchCondition> condition, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			if constexpr (args_t::is_session)
			{
				// Use "sessions().dispatch" to ensure that the _fire_accept function and the _fire_handshake
				// function are fired in the same thread
				derive.sessions().dispatch(
				[&derive, ec, this_ptr = std::move(self_ptr), condition = std::move(condition),
					chain = std::move(chain)]
				() mutable
				{
					try
					{
						set_last_error(ec);

						derive._fire_handshake(this_ptr);

						asio::detail::throw_error(ec);

						derive._done_connect(ec, std::move(this_ptr), std::move(condition), std::move(chain));
					}
					catch (system_error & e)
					{
						set_last_error(e);

						derive._do_disconnect(e.code(), std::move(this_ptr), std::move(chain));
					}
				});
			}
			else
			{
				set_last_error(ec);

				derive._fire_handshake(self_ptr);

				derive._done_connect(ec, std::move(self_ptr), std::move(condition), std::move(chain));
			}
		}

	protected:
		asio::ssl::context           & ssl_ctx_;

		handshake_type                 ssl_type_;

		std::unique_ptr<stream_type>   ssl_stream_;
	};
}

#endif // !__ASIO2_SSL_STREAM_COMPONENT_HPP__

#endif
