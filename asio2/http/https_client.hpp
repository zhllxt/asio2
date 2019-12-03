/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#if !defined(ASIO_STANDALONE) && defined(ASIO2_USE_SSL)

#ifndef __ASIO2_HTTPS_CLIENT_HPP__
#define __ASIO2_HTTPS_CLIENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/http/http_client.hpp>
#include <asio2/tcp/component/ssl_stream_cp.hpp>

namespace asio2::detail
{
	template<class derived_t, class socket_t, class body_t, class buffer_t>
	class https_client_impl_t
		: public asio::ssl::context
		, public http_client_impl_t<derived_t, socket_t, body_t, buffer_t>
		, public ssl_stream_cp<derived_t, socket_t, false>
	{
		template <class, bool>                       friend class user_timer_cp;
		template <class, bool>                       friend class connect_timeout_cp;
		template <class, class>                      friend class connect_cp;
		template <class, bool>                       friend class send_queue_cp;
		template <class, bool>                       friend class send_cp;
		template <class, bool>                       friend class tcp_send_op;
		template <class, bool>                       friend class tcp_recv_op;
		template <class, class, class, bool>         friend class http_send_cp;
		template <class, class, class, bool>         friend class http_send_op;
		template <class, class, class, bool>         friend class http_recv_op;
		template <class, class, bool>                friend class ssl_stream_cp;
		template <class, class, class>               friend class client_impl_t;
		template <class, class, class>               friend class tcp_client_impl_t;
		template <class, class, class, class>        friend class http_client_impl_t;

	public:
		using self = https_client_impl_t<derived_t, socket_t, body_t, buffer_t>;
		using super = http_client_impl_t<derived_t, socket_t, body_t, buffer_t>;
		using body_type = body_t;
		using buffer_type = buffer_t;
		using ssl_stream_comp = ssl_stream_cp<derived_t, socket_t, false>;
		using super::send;

	public:
		/**
		 * @constructor
		 */
		explicit https_client_impl_t(
			asio::ssl::context::method method = asio::ssl::context::sslv23,
			std::size_t init_buffer_size = tcp_frame_size,
			std::size_t max_buffer_size = (std::numeric_limits<std::size_t>::max)()
		)
			: asio::ssl::context(method)
			, super(init_buffer_size, max_buffer_size)
			, ssl_stream_comp(this->io_, this->socket_, *this, asio::ssl::stream_base::client)
		{
		}

		/**
		 * @destructor
		 */
		~https_client_impl_t()
		{
			this->stop();
			this->iopool_.stop();
		}

		inline derived_t & set_cert(std::string_view cert)
		{
			this->add_certificate_authority(asio::buffer(cert));
			return (this->derived());
		}

		inline derived_t & set_cert_file(const std::string& file)
		{
			this->load_verify_file(file);
			return (this->derived());
		}

		/**
		 * @function : get the stream object refrence
		 */
		inline typename ssl_stream_comp::stream_type & stream()
		{
			return this->ssl_stream_;
		}

	public:
		/**
		 * @function : bind ssl handshake listener
		 * @param    : fun - a user defined callback function
		 * Function signature : void(asio2::error_code ec)
		 */
		template<class F, class ...C>
		inline derived_t & bind_handshake(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::handshake, observer_t<error_code>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	protected:
		inline void _handle_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			this->derived()._ssl_stop(this_ptr, [this, ec, this_ptr]()
			{
				super::_handle_stop(ec, std::move(this_ptr));
			});
		}

		template<bool isAsync, typename MatchCondition>
		inline void _handle_connect(const error_code & ec, std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			set_last_error(ec);

			if (ec)
				return this->derived()._done_connect(ec, std::move(this_ptr), std::move(condition));

			this->derived()._post_handshake(std::move(this_ptr), std::move(condition));
		}

		inline void _fire_handshake(detail::ignore, error_code ec)
		{
			this->listener_.notify(event::handshake, ec);
		}

	protected:
	};
}

namespace asio2
{
	class https_client : public detail::https_client_impl_t<https_client, asio::ip::tcp::socket, http::string_body, beast::flat_buffer>
	{
	public:
		using https_client_impl_t<https_client, asio::ip::tcp::socket, http::string_body, beast::flat_buffer>::https_client_impl_t;
	};
}

#endif // !__ASIO2_HTTPS_CLIENT_HPP__

#endif
