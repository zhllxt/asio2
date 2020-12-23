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
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_CLIENT;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SESSION;

	template<bool isRequest, class Body, class Fields = http::fields>
	class http_request_impl_t
		: public http::message<isRequest, Body, Fields>
#ifndef ASIO2_DISABLE_HTTP_REQUEST_USER_DATA_CP
		, public user_data_cp<http_request_impl_t<isRequest, Body, Fields>>
#endif
	{
		template <class>                             friend class beast::websocket::listener;

		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION;

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
			std::memset((void*)(&url_parser_), 0, sizeof(http::http_parser_ns::http_parser_url));
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

		http_request_impl_t(const http::message<isRequest, Body, Fields>& req)
		{
			this->base() = req;
		}

		http_request_impl_t(http::message<isRequest, Body, Fields>&& req)
		{
			this->base() = std::move(req);
		}

		self& operator=(const http::message<isRequest, Body, Fields>& req)
		{
			this->base() = req;
			return *this;
		}

		self& operator=(http::message<isRequest, Body, Fields>&& req)
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

		inline void reset()
		{
			static_cast<super&>(*this) = {};
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
		inline std::string_view path()
		{
			if (!(url_parser_.field_set & (1 << (int)http::http_parser_ns::url_fields::UF_PATH)))
				return std::string_view{};

			std::string_view url = this->target();

			return std::string_view{ &url[url_parser_.field_data[(int)http::http_parser_ns::url_fields::UF_PATH].off],
				url_parser_.field_data[(int)http::http_parser_ns::url_fields::UF_PATH].len };
		}

		/**
		 * @function : Gets the content of the "query" section of the target
		 * <scheme>://<user>:<password>@<host>:<port>/<path>;<params>?<query>#<fragment>
		 */
		inline std::string_view query()
		{
			if (!(url_parser_.field_set & (1 << (int)http::http_parser_ns::url_fields::UF_QUERY)))
				return std::string_view{};

			std::string_view url = this->target();

			return std::string_view{ &url[url_parser_.field_data[(int)http::http_parser_ns::url_fields::UF_QUERY].off],
				url_parser_.field_data[(int)http::http_parser_ns::url_fields::UF_QUERY].len };
		}

		/**
		 * @function : Returns `true` if this HTTP request's Content-Type is "multipart/form-data";
		 */
		inline bool has_multipart()
		{
			return http::has_multipart(*this);
		}

		/**
		 * @function : Get the "multipart/form-data" body content.
		 */
		inline decltype(auto) multipart()
		{
			return http::multipart(*this);
		}

	protected:
		http::http_parser_ns::http_parser_url url_parser_;
		websocket::frame                      ws_frame_type_ = websocket::frame::unknown;
		std::string_view                      ws_frame_data_;
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
