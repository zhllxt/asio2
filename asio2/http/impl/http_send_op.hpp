/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_HTTP_SEND_OP_HPP__
#define __ASIO2_HTTP_SEND_OP_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <future>
#include <utility>
#include <string_view>

#include <asio2/base/selector.hpp>
#include <asio2/base/error.hpp>

#include <asio2/http/detail/http_util.hpp>

namespace asio2::detail
{
	template<class derived_t, class body_t, class buffer_t, bool isSession>
	class http_send_op
	{
	public:
		using body_type = body_t;
		using buffer_type = buffer_t;

		/**
		 * @constructor
		 */
		http_send_op() : derive(static_cast<derived_t&>(*this)) {}

		/**
		 * @destructor
		 */
		~http_send_op() = default;

	protected:
		template<class Data, class Callback>
		inline bool _http_send(Data& data, Callback&& callback)
		{
			return derive._tcp_send(data, std::forward<Callback>(callback));
		}

		template<bool isRequest, class Body, class Fields, class Callback>
		inline bool _http_send(copyable_wrapper<http::message<isRequest, Body, Fields>>& data,
			Callback&& callback)
		{
			using msg_type = std::remove_cv_t<std::remove_reference_t<decltype(data())>>;

			http::async_write(derive.stream(), const_cast<msg_type&>(data()),
				asio::bind_executor(derive.io().strand(), make_allocator(derive.wallocator(),
					[this, p = derive.selfptr(), callback = std::forward<Callback>(callback)]
			(const error_code& ec, std::size_t bytes_sent) mutable
			{
				set_last_error(ec);

				callback(ec, bytes_sent);

				if (ec)
				{
					// must stop, otherwise re-sending will cause body confusion
					derive._do_disconnect(ec);
				}
			})));
			return true;
		}

	protected:
		derived_t & derive;
	};
}

#endif // !__ASIO2_HTTP_SEND_OP_HPP__
