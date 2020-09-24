/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_HTTP_REQUEST_IMPL_HPP__
#define __ASIO2_HTTP_REQUEST_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/component/user_data_cp.hpp>

#include <asio2/http/detail/http_util.hpp>

#ifdef BEAST_HEADER_ONLY
namespace beast::websocket
#else
namespace boost::beast::websocket
#endif
{
	template <class> class listener;
}

namespace asio2::detail
{
	template <class, class, class, class, class> class http_session_impl_t;

	template<bool isRequest, class Body, class Fields = http::fields>
	class http_request_impl_t
		: public http::message<isRequest, Body, Fields>
		, public user_data_cp<http_request_impl_t<isRequest, Body, Fields>>
	{
		template <class, class, class, bool>		 friend class http_send_cp;
		template <class, class, class, bool>		 friend class http_send_op;
		template <class, class, class, bool>		 friend class http_recv_op;
		template <class, class, class>				 friend class client_impl_t;
		template <class, class, class>				 friend class tcp_client_impl_t;
		template <class, class, class, class, class> friend class http_session_impl_t;
		template <class>                             friend class beast::websocket::listener;

	public:
		using self = http_request_impl_t<isRequest, Body, Fields>;
		using super = http::message<isRequest, Body, Fields>;
		using header_type = typename super::header_type;
		using body_type = typename super::body_type;

	public:
		/**
		 * @constructor
		 */
		template<typename... Args>
		explicit http_request_impl_t(Args&&... args)
			: super(std::forward<Args>(args)...)
		{
		}

		http_request_impl_t(const http_request_impl_t& o)
		{
			this->base() = o.base();
			this->ws_frame_type_ = o.ws_frame_type_;
			this->ws_frame_data_ = o.ws_frame_data_;
		}

		http_request_impl_t(http_request_impl_t&& o)
		{
			this->base() = std::move(o.base());
			this->ws_frame_type_ = o.ws_frame_type_;
			this->ws_frame_data_ = o.ws_frame_data_;
		}

		self& operator=(const http_request_impl_t& o)
		{
			this->base() = o.base();
			this->ws_frame_type_ = o.ws_frame_type_;
			this->ws_frame_data_ = o.ws_frame_data_;
			return *this;
		}

		self& operator=(http_request_impl_t&& o)
		{
			this->base() = std::move(o.base());
			this->ws_frame_type_ = o.ws_frame_type_;
			this->ws_frame_data_ = o.ws_frame_data_;
			return *this;
		}

		http_request_impl_t(const http::request_t<http::string_body>& req)
		{
			this->base() = req;
		}

		http_request_impl_t(http::request_t<http::string_body>&& req)
		{
			this->base() = std::move(req);
		}

		self& operator=(const http::request_t<http::string_body>& req)
		{
			this->base() = req;
			return *this;
		}

		self& operator=(http::request_t<http::string_body>&& req)
		{
			this->base() = std::move(req);
			return *this;
		}

		/**
		 * @destructor
		 */
		~http_request_impl_t()
		{
		}

		/// Returns the base portion of the message
		inline super const& base() const
		{
			return *this;
		}

		/// Returns the base portion of the message
		inline super& base()
		{
			return *this;
		}

	public:
		/**
		 * @function : Returns `true` if this HTTP request is a WebSocket Upgrade.
		 */
		inline bool is_upgrade() { return websocket::is_upgrade(*this); }

		/**
		 * @function : Gets the content of the "path" section of the target
		 * <scheme>://<user>:<password>@<host>:<port>/<path>;<params>?<query>#<fragment>
		 */
		inline std::string_view path() { return http::url_to_path(this->target()); }

		/**
		 * @function : Gets the content of the "query" section of the target
		 * <scheme>://<user>:<password>@<host>:<port>/<path>;<params>?<query>#<fragment>
		 */
		inline std::string_view query() { return http::url_to_query(this->target()); }

		inline void reset()
		{
			static_cast<super&>(*this) = {};
		}

	protected:
		websocket::frame ws_frame_type_;
		std::string_view ws_frame_data_;
	};
}

#ifdef BEAST_HEADER_ONLY
namespace beast::http
#else
namespace boost::beast::http
#endif
{
	using request = asio2::detail::http_request_impl_t<true, http::string_body>;
}

#endif // !__ASIO2_HTTP_REQUEST_IMPL_HPP__
