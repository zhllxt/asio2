/*
 * COPYRIGHT (C) 2017-2019, zhllxt
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

#include <asio2/tcp/tcp_server.hpp>
#include <asio2/http/http_session.hpp>

namespace asio2::detail
{
	template<class derived_t, class session_t>
	class http_server_impl_t
		: public tcp_server_impl_t<derived_t, session_t>
		, public http_router_t<session_t>
	{
		template <class, bool>  friend class user_timer_cp;
		template <class>        friend class post_cp;
		template <class, class> friend class server_impl_t;
		template <class, class> friend class tcp_server_impl_t;

	public:
		using self = http_server_impl_t<derived_t, session_t>;
		using super = tcp_server_impl_t<derived_t, session_t>;
		using session_type = session_t;

		/**
		 * @constructor
		 */
		explicit http_server_impl_t(
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
		~http_server_impl_t()
		{
			this->stop();
		}

		/**
		 * @function : start the server
		 * @param service A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename StrOrInt>
		inline bool start(StrOrInt&& service)
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
		inline bool start(String&& host, StrOrInt&& service)
		{
			return this->derived()._do_start(std::forward<String>(host), std::forward<StrOrInt>(service),
				condition_wrap<void>{});
		}

	public:
		/**
		 * @function : bind recv listener
		 * @param    : fun - a user defined callback function
		 * Function signature : void(std::shared_ptr<asio2::http_session>& session_ptr,
		 *                           http::request& req, http::response& rep)
		 * or                 : void(http::request& req, http::response& rep)
		 */
		template<class F, class ...C>
		inline derived_t & bind_recv(F&& fun, C&&... obj)
		{
			if constexpr (is_template_callable_v<F,
				std::shared_ptr<session_t>&, http::request&, http::response&>)
			{
				this->is_arg0_session_ = true;
				this->listener_.bind(event::recv, observer_t<std::shared_ptr<session_t>&,
					http::request&, http::response&>(std::forward<F>(fun), std::forward<C>(obj)...));
			}
			else
			{
				this->is_arg0_session_ = false;
				this->listener_.bind(event::recv, observer_t<
					http::request&, http::response&>(std::forward<F>(fun), std::forward<C>(obj)...));
			}
			return (this->derived());
		}

		/**
		 * @function : bind websocket upgrade listener
		 * @param    : fun - a user defined callback function
		 * Function signature : void(std::shared_ptr<asio2::http_session>& session_ptr, asio::error_code ec)
		 */
		template<class F, class ...C>
		inline derived_t & bind_upgrade(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::upgrade, observer_t<std::shared_ptr<session_t>&, error_code>
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

#endif // !__ASIO2_HTTP_SERVER_HPP__
