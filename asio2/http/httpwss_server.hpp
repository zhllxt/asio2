/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef ASIO_STANDALONE

#ifndef __ASIO2_HTTPWSS_SERVER_HPP__
#define __ASIO2_HTTPWSS_SERVER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/http/https_server.hpp>
#include <asio2/http/httpwss_session.hpp>

namespace asio2::detail
{
	template<class derived_t, class session_t>
	class httpwss_server_impl_t : public https_server_impl_t<derived_t, session_t>
	{
		template <class, bool>  friend class user_timer_cp;
		template <class, class> friend class server_impl_t;
		template <class, class> friend class tcp_server_impl_t;
		template <class, class> friend class tcps_server_impl_t;
		template <class, class> friend class https_server_impl_t;

	public:
		using self = httpwss_server_impl_t<derived_t, session_t>;
		using super = https_server_impl_t<derived_t, session_t>;
		using session_type = session_t;

		/**
		 * @constructor
		 */
		explicit httpwss_server_impl_t(
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
		~httpwss_server_impl_t()
		{
			this->stop();
		}

	public:
		/**
		 * @function : bind recv listener
		 * @param    : listener - a user defined callback function
		 * Function signature : void(std::shared_ptr<asio2::httpwss_session>& session_ptr,
		 * http::request<http::string_body>& req, std::string_view s)
		 */
		template<class T>
		inline derived_t & bind_recv(T && listener)
		{
			this->listener_.bind(event::recv, observer_t<std::shared_ptr<session_t>&,
				http::request<typename session_t::body_type>&, std::string_view>(std::forward<T>(listener)));
			return (this->derived());
		}

		/**
		 * @function : bind websocket upgrade listener
		 * @param    : listener - a user defined callback function
		 * Function signature : void(std::shared_ptr<asio2::httpwss_session>& session_ptr, asio::error_code ec)
		 */
		template<class T>
		inline derived_t & bind_upgrade(T && listener)
		{
			this->listener_.bind(event::upgrade, observer_t<std::shared_ptr<session_t>&, error_code>(std::forward<T>(listener)));
			return (this->derived());
		}

	protected:
	};
}

namespace asio2
{
	template<class session_t>
	class httpwss_server_t : public detail::httpwss_server_impl_t<httpwss_server_t<session_t>, session_t>
	{
	public:
		using detail::httpwss_server_impl_t<httpwss_server_t<session_t>, session_t>::httpwss_server_impl_t;
	};

	using httpwss_server = httpwss_server_t<httpwss_session>;
}

#endif // !__ASIO2_HTTPWSS_SERVER_HPP__

#endif
