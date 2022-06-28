/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_HTTP_SERVER_HPP__
#define __ASIO2_HTTP_SERVER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <asio2/tcp/tcp_server.hpp>
#include <asio2/http/http_session.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;

	template<class derived_t, class session_t>
	class http_server_impl_t
		: public tcp_server_impl_t<derived_t, session_t>
		, public http_router_t    <session_t, typename session_t::args_type>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;

	public:
		using super = tcp_server_impl_t <derived_t, session_t>;
		using self  = http_server_impl_t<derived_t, session_t>;

		using session_type = session_t;

	public:
		/**
		 * @constructor
		 */
		template<class... Args>
		explicit http_server_impl_t(Args&&... args)
			: super(std::forward<Args>(args)...)
		{
		}

		/**
		 * @destructor
		 */
		~http_server_impl_t()
		{
			this->stop();
		}

		/**
		 * @function : start the server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param service A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt, typename... Args>
		inline bool start(String&& host, StrOrInt&& service, Args&&... args)
		{
			return this->derived()._do_start(std::forward<String>(host), std::forward<StrOrInt>(service),
				condition_helper::make_condition('0', std::forward<Args>(args)...));
		}

	public:
		/**
		 * @function : bind recv listener
		 * @param    : fun - a user defined callback function
		 * Function signature : void(std::shared_ptr<asio2::http_session>& session_ptr,
		 *                           http::web_request& req, http::web_response& rep)
		 * or                 : void(http::web_request& req, http::web_response& rep)
		 */
		template<class F, class ...C>
		inline derived_t & bind_recv(F&& fun, C&&... obj)
		{
			if constexpr (is_template_callable_v<F,
				std::shared_ptr<session_t>&, http::web_request&, http::web_response&>)
			{
				this->is_arg0_session_ = true;
				this->listener_.bind(event_type::recv, observer_t<std::shared_ptr<session_t>&,
					http::web_request&, http::web_response&>(std::forward<F>(fun), std::forward<C>(obj)...));
			}
			else
			{
				this->is_arg0_session_ = false;
				this->listener_.bind(event_type::recv, observer_t<
					http::web_request&, http::web_response&>(std::forward<F>(fun), std::forward<C>(obj)...));
			}
			return (this->derived());
		}

		/**
		 * @function : bind websocket upgrade listener
		 * @param    : fun - a user defined callback function
		 * Function signature : void(std::shared_ptr<asio2::http_session>& session_ptr)
		 */
		template<class F, class ...C>
		inline derived_t & bind_upgrade(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::upgrade, observer_t<std::shared_ptr<session_t>&>
				(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	protected:
		template<typename... Args>
		inline std::shared_ptr<session_t> _make_session(Args&&... args)
		{
			return super::_make_session(std::forward<Args>(args)..., *this,
				this->root_directory_, this->is_arg0_session_, this->support_websocket_);
		}

	protected:
		bool                      is_arg0_session_    = false;
	};
}

namespace asio2
{
	template<class session_t>
	class http_server_t : public detail::http_server_impl_t<http_server_t<session_t>, session_t>
	{
	public:
		using detail::http_server_impl_t<http_server_t<session_t>, session_t>::http_server_impl_t;
	};

	using http_server = http_server_t<http_session>;
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_HTTP_SERVER_HPP__
