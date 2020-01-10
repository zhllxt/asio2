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

#ifndef __ASIO2_HTTPS_SERVER_HPP__
#define __ASIO2_HTTPS_SERVER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/tcp/tcps_server.hpp>
#include <asio2/http/https_session.hpp>

namespace asio2::detail
{
	template<class derived_t, class session_t>
	class https_server_impl_t : public tcps_server_impl_t<derived_t, session_t>
	{
		template <class, bool>  friend class user_timer_cp;
		template <class, class> friend class server_impl_t;
		template <class, class> friend class tcps_server_impl_t;

	public:
		using self = https_server_impl_t<derived_t, session_t>;
		using super = tcps_server_impl_t<derived_t, session_t>;
		using session_type = session_t;

		/**
		 * @constructor
		 */
		explicit https_server_impl_t(
			asio::ssl::context::method method = asio::ssl::context::sslv23,
			std::size_t init_buffer_size = tcp_frame_size,
			std::size_t max_buffer_size = (std::numeric_limits<std::size_t>::max)(),
			std::size_t concurrency = std::thread::hardware_concurrency() * 2
		)
			: super(method, init_buffer_size, max_buffer_size, concurrency)
		{
		}

		/**
		 * @destructor
		 */
		~https_server_impl_t()
		{
			this->stop();
		}

		/**
		 * @function : start the server
		 * @param service A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename StrOrInt>
		bool start(StrOrInt&& service)
		{
			return this->start(std::string_view{}, std::forward<StrOrInt>(service));
		}

		/**
		 * @function : start the server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param service A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt>
		bool start(String&& host, StrOrInt&& service)
		{
			return this->derived()._do_start(std::forward<String>(host), std::forward<StrOrInt>(service),
				condition_wrap<void>{});
		}

	public:
		/**
		 * @function : bind recv listener
		 * @param    : fun - a user defined callback function
		 * Function signature : void(std::shared_ptr<asio2::https_session>& session_ptr, http::request<http::string_body>& req)
		 */
		template<class F, class ...C>
		inline derived_t & bind_recv(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::recv, observer_t<std::shared_ptr<session_t>&,
				http::request<typename session_t::body_type>&>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	protected:

	};
}

namespace asio2
{
	template<class session_t>
	class https_server_t : public detail::https_server_impl_t<https_server_t<session_t>, session_t>
	{
	public:
		using detail::https_server_impl_t<https_server_t<session_t>, session_t>::https_server_impl_t;
	};

	using https_server = https_server_t<https_session>;
}

#endif // !__ASIO2_HTTPS_SERVER_HPP__

#endif
