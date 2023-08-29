/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#if defined(ASIO2_ENABLE_SSL) || defined(ASIO2_USE_SSL)

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

#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/allocator.hpp>
#include <asio2/base/detail/ecs.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t>
	class ssl_stream_cp : public detail::ssl_stream_tag
	{
	public:
		using ssl_socket_type    = typename args_t::socket_t;
		using ssl_stream_type    = asio::ssl::stream<ssl_socket_type&>;
		using ssl_handshake_type = typename asio::ssl::stream_base::handshake_type;

		ssl_stream_cp(asio::ssl::context& ctx, ssl_handshake_type type) noexcept
			: ssl_ctx_(ctx)
			, ssl_type_(type)
		{
		}

		~ssl_stream_cp() = default;

		/**
		 * @brief get the ssl socket object reference
		 */
		inline ssl_stream_type & ssl_stream() noexcept
		{
			ASIO2_ASSERT(bool(this->ssl_stream_));
			return (*(this->ssl_stream_));
		}

		/**
		 * @brief get the ssl socket object reference
		 */
		inline ssl_stream_type const& ssl_stream() const noexcept
		{
			ASIO2_ASSERT(bool(this->ssl_stream_));
			return (*(this->ssl_stream_));
		}

	protected:
		template<typename C>
		inline void _ssl_init(std::shared_ptr<ecs_t<C>>& ecs, ssl_socket_type& socket, asio::ssl::context& ctx)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			detail::ignore_unused(derive, ecs, socket, ctx);

			if constexpr (args_t::is_client)
			{
				ASIO2_ASSERT(derive.io_->running_in_this_thread());
			}
			else
			{
				ASIO2_ASSERT(derive.sessions_.io_->running_in_this_thread());
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
			this->ssl_stream_ = std::make_unique<ssl_stream_type>(socket, ctx);
		}

		template<typename C>
		inline void _ssl_start(
			std::shared_ptr<derived_t>& this_ptr, std::shared_ptr<ecs_t<C>>& ecs, ssl_socket_type& socket,
			asio::ssl::context& ctx) noexcept
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			detail::ignore_unused(derive, this_ptr, ecs, socket, ctx);

			ASIO2_ASSERT(derive.io_->running_in_this_thread());
		}

		template<typename DeferEvent>
		inline void _ssl_stop(std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(derive.io_->running_in_this_thread());

			if (!this->ssl_stream_)
				return;

			derive.disp_event([this, &derive, this_ptr = std::move(this_ptr), e = chain.move_event()]
			(event_queue_guard<derived_t> g) mutable
			{
				// must construct a new chain
				defer_event chain(std::move(e), std::move(g));

				struct SSL_clear_op
				{
					ssl_stream_type* s{};

					// SSL_clear : 
					// Reset ssl to allow another connection. All settings (method, ciphers, BIOs) are kept.

					// When the client auto reconnect, SSL_clear must be called,
					// otherwise the SSL handshake will failed.

					SSL_clear_op(ssl_stream_type* p) : s(p)
					{
					}

					~SSL_clear_op()
					{
						if (s)
							SSL_clear(s->native_handle());
					}
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
					std::make_shared<asio::steady_timer>(derive.io_->context());
				timer->expires_after(derive.get_disconnect_timeout());
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

			#if defined(_DEBUG) || defined(DEBUG)
				ASIO2_ASSERT(derive.post_send_counter_.load() == 0);
				derive.post_send_counter_++;
			#endif

				// https://stackoverflow.com/questions/52990455/boost-asio-ssl-stream-shutdownec-always-had-error-which-is-boostasiossl
				error_code ec_ignore{};
				derive.socket().cancel(ec_ignore);

				ASIO2_LOG_DEBUG("ssl_stream_cp enter async_shutdown");

				// when server call ssl stream sync shutdown first,if the client socket is
				// not closed forever,then here shutdowm will blocking forever.
				this->ssl_stream_->async_shutdown(
				[&derive, p = std::move(this_ptr), timer = std::move(timer), clear_ptr = std::move(SSL_clear_ptr)]
				(const error_code& ec) mutable
				{
				#if defined(_DEBUG) || defined(DEBUG)
					derive.post_send_counter_--;
				#endif

					detail::ignore_unused(derive, p, clear_ptr);

					ASIO2_LOG_DEBUG("ssl_stream_cp leave async_shutdown: {} {}", ec.value(), ec.message());

					set_last_error(ec);

					detail::cancel_timer(*timer);
				});
			}, chain.move_guard());
		}

		template<typename C, typename DeferEvent>
		inline void _post_handshake(
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(bool(this->ssl_stream_));
			ASIO2_ASSERT(derive.io_->running_in_this_thread());

			// Used to chech whether the ssl handshake is timeout
			std::shared_ptr<std::atomic_flag> flag_ptr = std::make_shared<std::atomic_flag>();

			flag_ptr->clear();

			std::shared_ptr<asio::steady_timer> timer =
				std::make_shared<asio::steady_timer>(derive.io_->context());
			timer->expires_after(derive.get_connect_timeout());
			timer->async_wait(
			[&derive, this_ptr, flag_ptr](const error_code& ec) mutable
			{
				detail::ignore_unused(this_ptr);

				// no errors indicating timed out
				if (!ec)
				{
					flag_ptr->test_and_set();

					error_code ec_ignore{};

					if (derive.socket().is_open())
					{
						error_code oldec = get_last_error();

						asio::socket_base::linger linger = derive.get_linger();

						// the get_linger maybe change the last error value.
						set_last_error(oldec);

						// we close the socket, so the async_handshake will returned 
						// with operation_aborted.
						if (!(linger.enabled() == true && linger.timeout() == 0))
						{
							derive.socket().shutdown(asio::socket_base::shutdown_both, ec_ignore);
						}
					}

					derive.socket().cancel(ec_ignore);

					derive.socket().close(ec_ignore);
				}
			});

		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(derive.post_send_counter_.load() == 0);
			derive.post_send_counter_++;
		#endif

			this->ssl_stream_->async_handshake(this->ssl_type_, make_allocator(derive.wallocator(),
			[&derive, this_ptr = std::move(this_ptr), ecs = std::move(ecs),
				flag_ptr = std::move(flag_ptr), timer = std::move(timer), chain = std::move(chain)]
			(const error_code& ec) mutable
			{
			#if defined(_DEBUG) || defined(DEBUG)
				derive.post_send_counter_--;
			#endif

				detail::cancel_timer(*timer);

				if (flag_ptr->test_and_set())
					derive._handle_handshake(asio::error::timed_out,
						std::move(this_ptr), std::move(ecs), std::move(chain));
				else
					derive._handle_handshake(ec,
						std::move(this_ptr), std::move(ecs), std::move(chain));
			}));
		}

		template<typename C, typename DeferEvent>
		inline void _session_handle_handshake(
			const error_code& ec,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// Use "sessions_.dispatch" to ensure that the _fire_accept function and the _fire_handshake
			// function are fired in the same thread
			derive.sessions_.dispatch(
			[&derive, ec, this_ptr = std::move(this_ptr), ecs = std::move(ecs), chain = std::move(chain)]
			() mutable
			{
				set_last_error(ec);

				derive._fire_handshake(this_ptr);

				if (ec)
				{
					ASIO2_LOG_DEBUG("ssl_stream_cp::handle_handshake {} {}", ec.value(), ec.message());
					derive._do_disconnect(ec, std::move(this_ptr), std::move(chain));
					return;
				}

				derive._done_connect(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
			});
		}

		template<typename C, typename DeferEvent>
		inline void _client_handle_handshake(
			const error_code& ec,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			set_last_error(ec);

			derive._fire_handshake(this_ptr);

			derive._done_connect(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
		}

		template<typename C, typename DeferEvent>
		inline void _handle_handshake(
			const error_code& ec,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			if constexpr (args_t::is_session)
			{
				derive._session_handle_handshake(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
			}
			else
			{
				derive._client_handle_handshake(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
			}
		}

	protected:
		asio::ssl::context               & ssl_ctx_;

		ssl_handshake_type                 ssl_type_;

		std::unique_ptr<ssl_stream_type>   ssl_stream_;
	};
}

#endif // !__ASIO2_SSL_STREAM_COMPONENT_HPP__

#endif
