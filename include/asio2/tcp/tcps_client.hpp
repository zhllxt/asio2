/*
 * COPYRIGHT (C) 2017-2021, zhllxt
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

#include <asio2/base/detail/push_options.hpp>

#include <asio2/tcp/tcp_client.hpp>
#include <asio2/tcp/component/ssl_stream_cp.hpp>
#include <asio2/tcp/component/ssl_context_cp.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_CLIENT;

	template<class derived_t, class args_t = template_args_tcp_client>
	class tcps_client_impl_t
		: public ssl_context_cp    <derived_t, args_t>
		, public tcp_client_impl_t <derived_t, args_t>
		, public ssl_stream_cp     <derived_t, args_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT;

	public:
		using super = tcp_client_impl_t <derived_t, args_t>;
		using self  = tcps_client_impl_t<derived_t, args_t>;

		using args_type   = args_t;
		using buffer_type = typename args_t::buffer_t;

		using ssl_context_comp = ssl_context_cp<derived_t, args_t>;
		using ssl_stream_comp  = ssl_stream_cp <derived_t, args_t>;

	public:
		/**
		 * @constructor
		 */
		template<class... Args>
		explicit tcps_client_impl_t(
			asio::ssl::context::method method = asio::ssl::context::sslv23,
			Args&&... args
		)
			: ssl_context_comp(method)
			, super(std::forward<Args>(args)...)
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
		inline typename ssl_stream_comp::stream_type & stream() noexcept
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
		template<typename MatchCondition>
		inline void _do_init(condition_wrap<MatchCondition> condition)
		{
			super::_do_init(condition);

			this->derived()._ssl_init(condition, this->socket_, *this);
		}

		template<typename DeferEvent>
		inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
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

		template<typename MatchCondition, typename DeferEvent>
		inline void _handle_connect(const error_code & ec, std::shared_ptr<derived_t> this_ptr,
			condition_wrap<MatchCondition> condition, DeferEvent chain)
		{
			set_last_error(ec);

			if (ec)
				return this->derived()._done_connect(ec, std::move(this_ptr), std::move(condition), std::move(chain));

			this->derived()._ssl_start(this_ptr, condition, this->socket_, *this);

			this->derived()._post_handshake(std::move(this_ptr), std::move(condition), std::move(chain));
		}

		inline void _fire_handshake(std::shared_ptr<derived_t>& this_ptr)
		{
			// the _fire_handshake must be executed in the thread 0.
			ASIO2_ASSERT(this->derived().io().running_in_this_thread());

			detail::ignore_unused(this_ptr);

			this->listener_.notify(event_type::handshake);
		}
	};
}

namespace asio2
{
	/**
	 * ssl tcp client
	 * constructor maybe throw exception "Too many open files" (exception code : 24)
	 * asio::error::no_descriptors - Too many open files
	 */
	template<class derived_t>
	class tcps_client_t : public detail::tcps_client_impl_t<derived_t, detail::template_args_tcp_client>
	{
	public:
		using detail::tcps_client_impl_t<derived_t, detail::template_args_tcp_client>::tcps_client_impl_t;
	};

	/**
	 * ssl tcp client
	 * constructor maybe throw exception "Too many open files" (exception code : 24)
	 * asio::error::no_descriptors - Too many open files
	 */
	class tcps_client : public tcps_client_t<tcps_client>
	{
	public:
		using tcps_client_t<tcps_client>::tcps_client_t;
	};
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_TCPS_CLIENT_HPP__

#endif
