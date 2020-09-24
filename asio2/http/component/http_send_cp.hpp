/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_HTTP_SEND_COMPONENT_HPP__
#define __ASIO2_HTTP_SEND_COMPONENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstdint>
#include <memory>
#include <functional>
#include <string>
#include <future>
#include <queue>
#include <tuple>
#include <utility>
#include <string_view>

#include <asio2/base/selector.hpp>
#include <asio2/base/error.hpp>

#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/function_traits.hpp>
#include <asio2/base/detail/buffer_wrap.hpp>

#include <asio2/http/detail/http_util.hpp>
#include <asio2/http/request.hpp>
#include <asio2/http/response.hpp>

namespace asio2::detail
{
	template<class derived_t, class body_t, class buffer_t, bool isSession>
	class http_send_cp
	{
	public:
		using body_type = body_t;
		using buffer_type = buffer_t;

		/**
		 * @constructor
		 */
		http_send_cp(io_t & wio) : derive(static_cast<derived_t&>(*this)), wio_(wio)
		{
		}

		/**
		 * @destructor
		 */
		~http_send_cp() = default;

	public:
		/**
		 * @function : Asynchronous send data,supporting multi data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 */
		template<bool isRequest, class Body, class Fields = http::fields>
		inline bool send(http::message<isRequest, Body, Fields>& msg)
		{
			return derive.send(const_cast<const http::message<isRequest, Body, Fields>&>(msg));
		}

		template<bool isRequest, class Body, class Fields = http::fields>
		inline bool send(const http::message<isRequest, Body, Fields>& msg)
		{
			try
			{
				if (!derive.is_started())
					asio::detail::throw_error(asio::error::not_connected);

				derive.push_event([this, data = derive._data_persistence(msg)]
				(event_guard<derived_t>&& g) mutable
				{
					return derive._do_send(data,
						[g = std::move(g)](const error_code&, std::size_t) mutable {});
				});
				return true;
			}
			catch (system_error & e) { set_last_error(e); }
			catch (std::exception &) { set_last_error(asio::error::eof); }
			return false;
		}

		/**
		 * @function : Asynchronous send data,supporting multi data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 */
		template<bool isRequest, class Body, class Fields = http::fields>
		inline bool send(http::message<isRequest, Body, Fields>&& msg)
		{
			try
			{
				if (!derive.is_started())
					asio::detail::throw_error(asio::error::not_connected);

				derive.push_event([this, data = derive._data_persistence(std::move(msg))]
				(event_guard<derived_t>&& g) mutable
				{
					return derive._do_send(data,
						[g = std::move(g)](const error_code&, std::size_t) mutable {});
				});
				return true;
			}
			catch (system_error & e) { set_last_error(e); }
			catch (std::exception &) { set_last_error(asio::error::eof); }
			return false;
		}

		/**
		 * @function : Asynchronous send data,supporting multi data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * Callback signature : void() or void(std::size_t bytes_sent)
		 */
		template<class Callback, bool isRequest, class Body, class Fields = http::fields>
		inline bool send(http::message<isRequest, Body, Fields>& msg, Callback&& fn)
		{
			return derive.send(const_cast<const http::message<isRequest, Body, Fields>&>(msg),
				std::forward<Callback>(fn));
		}

		template<class Callback, bool isRequest, class Body, class Fields = http::fields>
		inline bool send(const http::message<isRequest, Body, Fields>& msg, Callback&& fn)
		{
			try
			{
				if (!derive.is_started())
					asio::detail::throw_error(asio::error::not_connected);

				derive.push_event([this, data = derive._data_persistence(msg),
					fn = std::forward<Callback>(fn)](event_guard<derived_t>&& g) mutable
				{
					return derive._do_send(data, [&fn, g = std::move(g)]
					(const error_code&, std::size_t bytes_sent) mutable
					{
						callback_helper::call(fn, bytes_sent);
					});
				});
				return true;
			}
			catch (system_error & e) { set_last_error(e); }
			catch (std::exception &) { set_last_error(asio::error::eof); }
			return false;
		}

		/**
		 * @function : Asynchronous send data,supporting multi data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * Callback signature : void() or void(std::size_t bytes_sent)
		 */
		template<class Callback, bool isRequest, class Body, class Fields = http::fields>
		inline bool send(http::message<isRequest, Body, Fields>&& msg, Callback&& fn)
		{
			try
			{
				if (!derive.is_started())
					asio::detail::throw_error(asio::error::not_connected);

				derive.push_event([this, data = derive._data_persistence(std::move(msg)),
					fn = std::forward<Callback>(fn)](event_guard<derived_t>&& g) mutable
				{
					return derive._do_send(data, [&fn, g = std::move(g)]
					(const error_code&, std::size_t bytes_sent) mutable
					{
						callback_helper::call(fn, bytes_sent);
					});
				});
				return true;
			}
			catch (system_error & e) { set_last_error(e); }
			catch (std::exception &) { set_last_error(asio::error::eof); }
			return false;
		}

		/**
		 * @function : Asynchronous send data,supporting multi data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 */
		template<bool isRequest, class Body, class Fields = http::fields>
		inline std::future<std::pair<error_code, std::size_t>> send(
			http::message<isRequest, Body, Fields>& msg, asio::use_future_t<> flag)
		{
			return derive.send(const_cast<const http::message<isRequest, Body, Fields>&>(msg), std::move(flag));
		}

		template<bool isRequest, class Body, class Fields = http::fields>
		inline std::future<std::pair<error_code, std::size_t>> send(
			const http::message<isRequest, Body, Fields>& msg, asio::use_future_t<> flag)
		{
			std::ignore = flag;
			copyable_wrapper<std::promise<std::pair<error_code, std::size_t>>> promise;
			std::future<std::pair<error_code, std::size_t>> future = promise().get_future();
			try
			{
				if (!derive.is_started())
					asio::detail::throw_error(asio::error::not_connected);

				derive.push_event([this, data = derive._data_persistence(msg),
					promise = std::move(promise)](event_guard<derived_t>&& g) mutable
				{
					return derive._do_send(data, [&promise, g = std::move(g)]
					(const error_code& ec, std::size_t bytes_sent) mutable
					{
						promise().set_value(std::pair<error_code, std::size_t>(ec, bytes_sent));
					});
				});
			}
			catch (system_error & e)
			{
				set_last_error(e);
				promise().set_value(std::pair<error_code, std::size_t>(e.code(), 0));
			}
			catch (std::exception &)
			{
				set_last_error(asio::error::eof);
				promise().set_value(std::pair<error_code, std::size_t>(asio::error::eof, 0));
			}
			return future;
		}

		/**
		 * @function : Asynchronous send data,supporting multi data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 */
		template<bool isRequest, class Body, class Fields = http::fields>
		inline std::future<std::pair<error_code, std::size_t>> send(
			http::message<isRequest, Body, Fields>&& msg, asio::use_future_t<> flag)
		{
			std::ignore = flag;
			copyable_wrapper<std::promise<std::pair<error_code, std::size_t>>> promise;
			std::future <std::pair<error_code, std::size_t>> future = promise().get_future();
			try
			{
				if (!derive.is_started())
					asio::detail::throw_error(asio::error::not_connected);

				derive.push_event([this, data = derive._data_persistence(std::move(msg)),
					promise = std::move(promise)](event_guard<derived_t>&& g) mutable
				{
					return derive._do_send(data, [&promise, g = std::move(g)]
					(const error_code& ec, std::size_t bytes_sent) mutable
					{
						promise().set_value(std::pair<error_code, std::size_t>(ec, bytes_sent));
					});
				});
			}
			catch (system_error & e)
			{
				set_last_error(e);
				promise().set_value(std::pair<error_code, std::size_t>(e.code(), 0));
			}
			catch (std::exception &)
			{
				set_last_error(asio::error::eof);
				promise().set_value(std::pair<error_code, std::size_t>(asio::error::eof, 0));
			}
			return future;
		}

		/**
		 * @function : Asynchronous send data,supporting multi data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 */
		template<bool isRequest, class Body, class Fields = http::fields>
		inline bool send(http_request_impl_t<isRequest, Body, Fields>& msg)
		{
			return derive.send(msg.base());
		}

		template<bool isRequest, class Body, class Fields = http::fields>
		inline bool send(const http_request_impl_t<isRequest, Body, Fields>& msg)
		{
			return derive.send(msg.base());
		}

		/**
		 * @function : Asynchronous send data,supporting multi data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 */
		template<bool isRequest, class Body, class Fields = http::fields>
		inline bool send(http_request_impl_t<isRequest, Body, Fields>&& msg)
		{
			return derive.send(std::move(msg.base()));
		}

		/**
		 * @function : Asynchronous send data,supporting multi data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * Callback signature : void() or void(std::size_t bytes_sent)
		 */
		template<class Callback, bool isRequest, class Body, class Fields = http::fields>
		inline bool send(http_request_impl_t<isRequest, Body, Fields>& msg, Callback&& fn)
		{
			return derive.send(msg.base(), std::forward<Callback>(fn));
		}

		template<class Callback, bool isRequest, class Body, class Fields = http::fields>
		inline bool send(const http_request_impl_t<isRequest, Body, Fields>& msg, Callback&& fn)
		{
			return derive.send(msg.base(), std::forward<Callback>(fn));
		}

		/**
		 * @function : Asynchronous send data,supporting multi data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * Callback signature : void() or void(std::size_t bytes_sent)
		 */
		template<class Callback, bool isRequest, class Body, class Fields = http::fields>
		inline bool send(http_request_impl_t<isRequest, Body, Fields>&& msg, Callback&& fn)
		{
			return derive.send(std::move(msg.base()), std::forward<Callback>(fn));
		}

		/**
		 * @function : Asynchronous send data,supporting multi data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 */
		template<bool isRequest, class Body, class Fields = http::fields>
		inline std::future<std::pair<error_code, std::size_t>> send(
			http_request_impl_t<isRequest, Body, Fields>& msg, asio::use_future_t<> flag)
		{
			return derive.send(msg.base(), std::move(flag));
		}

		template<bool isRequest, class Body, class Fields = http::fields>
		inline std::future<std::pair<error_code, std::size_t>> send(
			const http_request_impl_t<isRequest, Body, Fields>& msg, asio::use_future_t<> flag)
		{
			return derive.send(msg.base(), std::move(flag));
		}

		/**
		 * @function : Asynchronous send data,supporting multi data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 */
		template<bool isRequest, class Body, class Fields = http::fields>
		inline std::future<std::pair<error_code, std::size_t>> send(
			http_request_impl_t<isRequest, Body, Fields>&& msg, asio::use_future_t<> flag)
		{
			return derive.send(std::move(msg.base()), std::move(flag));
		}

		/**
		 * @function : Asynchronous send data,supporting multi data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 */
		template<bool isRequest, class Body, class Fields = http::fields>
		inline bool send(http_response_impl_t<isRequest, Body, Fields>& msg)
		{
			return derive.send(msg.base());
		}

		template<bool isRequest, class Body, class Fields = http::fields>
		inline bool send(const http_response_impl_t<isRequest, Body, Fields>& msg)
		{
			return derive.send(msg.base());
		}

		/**
		 * @function : Asynchronous send data,supporting multi data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 */
		template<bool isRequest, class Body, class Fields = http::fields>
		inline bool send(http_response_impl_t<isRequest, Body, Fields>&& msg)
		{
			return derive.send(std::move(msg.base()));
		}

		/**
		 * @function : Asynchronous send data,supporting multi data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * Callback signature : void() or void(std::size_t bytes_sent)
		 */
		template<class Callback, bool isRequest, class Body, class Fields = http::fields>
		inline bool send(http_response_impl_t<isRequest, Body, Fields>& msg, Callback&& fn)
		{
			return derive.send(msg.base(), std::forward<Callback>(fn));
		}

		template<class Callback, bool isRequest, class Body, class Fields = http::fields>
		inline bool send(const http_response_impl_t<isRequest, Body, Fields>& msg, Callback&& fn)
		{
			return derive.send(msg.base(), std::forward<Callback>(fn));
		}

		/**
		 * @function : Asynchronous send data,supporting multi data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * Callback signature : void() or void(std::size_t bytes_sent)
		 */
		template<class Callback, bool isRequest, class Body, class Fields = http::fields>
		inline bool send(http_response_impl_t<isRequest, Body, Fields>&& msg, Callback&& fn)
		{
			return derive.send(std::move(msg.base()), std::forward<Callback>(fn));
		}

		/**
		 * @function : Asynchronous send data,supporting multi data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 */
		template<bool isRequest, class Body, class Fields = http::fields>
		inline std::future<std::pair<error_code, std::size_t>> send(
			http_response_impl_t<isRequest, Body, Fields>& msg, asio::use_future_t<> flag)
		{
			return derive.send(msg.base(), std::move(flag));
		}

		template<bool isRequest, class Body, class Fields = http::fields>
		inline std::future<std::pair<error_code, std::size_t>> send(
			const http_response_impl_t<isRequest, Body, Fields>& msg, asio::use_future_t<> flag)
		{
			return derive.send(msg.base(), std::move(flag));
		}

		/**
		 * @function : Asynchronous send data,supporting multi data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 */
		template<bool isRequest, class Body, class Fields = http::fields>
		inline std::future<std::pair<error_code, std::size_t>> send(
			http_response_impl_t<isRequest, Body, Fields>&& msg, asio::use_future_t<> flag)
		{
			return derive.send(std::move(msg.base()), std::move(flag));
		}

	protected:
		derived_t & derive;

		io_t      & wio_;
	};
}

#endif // !__ASIO2_HTTP_SEND_COMPONENT_HPP__
