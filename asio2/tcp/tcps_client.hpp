/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#if defined(ASIO2_USE_SSL)

#ifndef __ASIO2_TCPS_CLIENT_HPP__
#define __ASIO2_TCPS_CLIENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/tcp/tcp_client.hpp>
#include <asio2/tcp/component/ssl_stream_cp.hpp>
#include <asio2/tcp/component/ssl_context_cp.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_CLIENT;

	template<class derived_t, class args_t>
	class tcps_client_impl_t
		: public ssl_context_cp    <derived_t, false >
		, public tcp_client_impl_t <derived_t, args_t>
		, public ssl_stream_cp     <derived_t, args_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT;

	public:
		using super = tcp_client_impl_t <derived_t, args_t>;
		using self  = tcps_client_impl_t<derived_t, args_t>;

		using buffer_type = typename args_t::buffer_t;

		using ssl_context_comp = ssl_context_cp<derived_t, false >;
		using ssl_stream_comp  = ssl_stream_cp <derived_t, args_t>;

		using super::send;

		/**
		 * @constructor
		 */
		explicit tcps_client_impl_t(
			asio::ssl::context::method method = asio::ssl::context::sslv23,
			std::size_t init_buffer_size      = tcp_frame_size,
			std::size_t max_buffer_size       = (std::numeric_limits<std::size_t>::max)()
		)
			: ssl_context_comp(method)
			, super(init_buffer_size, max_buffer_size)
			, ssl_stream_comp(this->io_, *this, asio::ssl::stream_base::client)
		{
		}

		/**
		 * @destructor
		 */
		~tcps_client_impl_t()
		{
			this->stop();
		}

		/**
		 * @function : get the stream object refrence
		 * 
		 */
		inline typename ssl_stream_comp::stream_type & stream()
		{
			ASIO2_ASSERT(bool(this->ssl_stream_));
			return (*(this->ssl_stream_));
		}

	public:
		/**
		 * @function : bind ssl handshake listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * Function signature : void(asio2::error_code ec)
		 */
		template<class F, class ...C>
		inline derived_t & bind_handshake(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::handshake,
				observer_t<error_code>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	protected:
		template<typename MatchCondition>
		inline void _do_init(condition_wrap<MatchCondition> condition)
		{
			super::_do_init(condition);

			this->derived()._ssl_init(condition, this->socket_, *this);
		}

		inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			this->derived()._ssl_stop(this_ptr, [this, ec, this_ptr]() mutable
			{
				super::_handle_disconnect(ec, std::move(this_ptr));
			});
		}

		template<typename MatchCondition>
		inline void _handle_connect(const error_code & ec, std::shared_ptr<derived_t> this_ptr,
			condition_wrap<MatchCondition> condition)
		{
			set_last_error(ec);

			if (ec)
				return this->derived()._done_connect(ec, std::move(this_ptr), std::move(condition));

			this->derived()._ssl_start(this_ptr, condition, this->socket_, *this);

			this->derived()._post_handshake(std::move(this_ptr), std::move(condition));
		}

		inline void _fire_handshake(std::shared_ptr<derived_t>& this_ptr, error_code ec)
		{
			detail::ignore_unused(this_ptr);

			this->listener_.notify(event_type::handshake, ec);
		}

	protected:
	};
}

namespace asio2
{
	class tcps_client : public detail::tcps_client_impl_t<tcps_client, detail::template_args_tcp_client>
	{
	public:
		using tcps_client_impl_t<tcps_client, detail::template_args_tcp_client>::tcps_client_impl_t;
	};
}

#endif // !__ASIO2_TCPS_CLIENT_HPP__

#endif
