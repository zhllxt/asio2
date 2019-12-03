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

#ifndef __ASIO2_HTTPWS_SERVER_HPP__
#define __ASIO2_HTTPWS_SERVER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/http/http_server.hpp>
#include <asio2/http/httpws_session.hpp>

namespace asio2::detail
{
	template<class derived_t, class session_t>
	class httpws_server_impl_t : public http_server_impl_t<derived_t, session_t>
	{
		template <class, bool>  friend class user_timer_cp;
		template <class, class> friend class server_impl_t;
		template <class, class> friend class tcp_server_impl_t;
		template <class, class> friend class http_server_impl_t;

	public:
		using self = httpws_server_impl_t<derived_t, session_t>;
		using super = http_server_impl_t<derived_t, session_t>;
		using session_type = session_t;

		/**
		 * @constructor
		 */
		explicit httpws_server_impl_t(
			std::size_t init_buffer_size = tcp_frame_size,
			std::size_t max_buffer_size = (std::numeric_limits<std::size_t>::max)(),
			std::size_t concurrency = std::thread::hardware_concurrency() * 2
		)
			: super(init_buffer_size, max_buffer_size, concurrency)
		{
		}

		/**
		 * @destructor
		 */
		~httpws_server_impl_t()
		{
			this->stop();
			this->iopool_.stop();
		}

	public:
		/**
		 * @function : bind recv listener
		 * @param    : fun - a user defined callback function
		 * Function signature : void(std::shared_ptr<asio2::httpws_session>& session_ptr,
		 * http::request<http::string_body>& req, std::string_view s)
		 */
		template<class F, class ...C>
		inline derived_t & bind_recv(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::recv, observer_t<std::shared_ptr<session_t>&,
				http::request<typename session_t::body_type>&, std::string_view>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

		/**
		 * @function : bind websocket upgrade listener
		 * @param    : fun - a user defined callback function
		 * Function signature : void(std::shared_ptr<asio2::httpws_session>& session_ptr, asio::error_code ec)
		 */
		template<class F, class ...C>
		inline derived_t & bind_upgrade(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::upgrade, observer_t<std::shared_ptr<session_t>&, error_code>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	protected:
	};
}

namespace asio2
{
	template<class session_t>
	class httpws_server_t : public detail::httpws_server_impl_t<httpws_server_t<session_t>, session_t>
	{
	public:
		using detail::httpws_server_impl_t<httpws_server_t<session_t>, session_t>::httpws_server_impl_t;
	};

	using httpws_server = httpws_server_t<httpws_session>;
}

#endif // !__ASIO2_HTTPWS_SERVER_HPP__

#endif
