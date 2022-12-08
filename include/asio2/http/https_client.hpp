/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#if defined(ASIO2_ENABLE_SSL) || defined(ASIO2_USE_SSL)

#ifndef __ASIO2_HTTPS_CLIENT_HPP__
#define __ASIO2_HTTPS_CLIENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <fstream>

#include <asio2/base/detail/push_options.hpp>

#include <asio2/http/http_client.hpp>

#include <asio2/http/https_execute.hpp>
#include <asio2/http/https_download.hpp>

#include <asio2/tcp/impl/ssl_stream_cp.hpp>
#include <asio2/tcp/impl/ssl_context_cp.hpp>

namespace asio2::detail
{
	struct template_args_https_client : public template_args_http_client
	{
		// Used to remove inherited http_client_impl_t::execute and http_client_impl_t::download
		static constexpr bool http_execute_download_enabled = false;
	};

	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_CLIENT;

	template<class derived_t, class args_t = template_args_https_client>
	class https_client_impl_t
		: public ssl_context_cp     <derived_t, args_t>
		, public http_client_impl_t <derived_t, args_t>
		, public ssl_stream_cp      <derived_t, args_t>
		, public https_execute_impl <derived_t, args_t>
		, public https_download_impl<derived_t, args_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT;

	public:
		using super = http_client_impl_t <derived_t, args_t>;
		using self  = https_client_impl_t<derived_t, args_t>;

		using args_type   = args_t;
		using body_type   = typename args_t::body_t;
		using buffer_type = typename args_t::buffer_t;

		using ssl_context_comp = ssl_context_cp<derived_t, args_t>;
		using ssl_stream_comp  = ssl_stream_cp <derived_t, args_t>;

		using super::send;
		using super::async_send;

	public:
		/**
		 * @brief constructor
		 */
		template<class... Args>
		explicit https_client_impl_t(
			asio::ssl::context::method method = asio::ssl::context::sslv23,
			Args&&... args
		)
			: ssl_context_comp(method)
			, super(std::forward<Args>(args)...)
			, ssl_stream_comp(this->io_, *this, asio::ssl::stream_base::client)
		{
		}

		/**
		 * @brief destructor
		 */
		~https_client_impl_t()
		{
			this->stop();
		}

		/**
		 * @brief get the stream object refrence
		 */
		inline typename ssl_stream_comp::stream_type & stream() noexcept
		{
			ASIO2_ASSERT(bool(this->ssl_stream_));
			return (*(this->ssl_stream_));
		}

	public:
		/**
		 * @brief bind ssl handshake listener
		 * @param fun - a user defined callback function
		 * Function signature : void()
		 */
		template<class F, class ...C>
		inline derived_t & bind_handshake(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::handshake,
				observer_t<>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	protected:
		template<typename C>
		inline void _do_init(ecs_t<C>& ecs)
		{
			super::_do_init(ecs);

			this->derived()._ssl_init(ecs, this->socket_, *this);
		}

		template<typename DeferEvent>
		inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			this->derived()._rdc_stop();

			this->derived()._ssl_stop(this_ptr,
				defer_event
				{
					[this, ec, this_ptr, e = chain.move_event()] (event_queue_guard<derived_t> g) mutable
					{
						super::_handle_disconnect(ec, std::move(this_ptr), defer_event(std::move(e), std::move(g)));
					}, chain.move_guard()
				}
			);
		}

		template<typename C, typename DeferEvent>
		inline void _handle_connect(
			const error_code& ec, std::shared_ptr<derived_t> this_ptr, ecs_t<C>& ecs, DeferEvent chain)
		{
			set_last_error(ec);

			if (ec)
			{
				return this->derived()._done_connect(ec, std::move(this_ptr), ecs, std::move(chain));
			}

			this->derived()._ssl_start(this_ptr, ecs, this->socket_, *this);

			this->derived()._post_handshake(std::move(this_ptr), ecs, std::move(chain));
		}

		inline void _fire_handshake(std::shared_ptr<derived_t>& this_ptr)
		{
			// the _fire_handshake must be executed in the thread 0.
			ASIO2_ASSERT(this->derived().io().running_in_this_thread());

			detail::ignore_unused(this_ptr);

			this->listener_.notify(event_type::handshake);
		}

	protected:
	};
}

namespace asio2
{
	template<class derived_t>
	class https_client_t : public detail::https_client_impl_t<derived_t, detail::template_args_https_client>
	{
	public:
		using detail::https_client_impl_t<derived_t, detail::template_args_https_client>::https_client_impl_t;
	};

	class https_client : public https_client_t<https_client>
	{
	public:
		using https_client_t<https_client>::https_client_t;
	};
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_HTTPS_CLIENT_HPP__

#endif
