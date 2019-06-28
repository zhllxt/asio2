/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 */

#ifndef ASIO_STANDALONE

#ifndef __ASIO2_HTTP_SEND_COMPONENT_HPP__
#define __ASIO2_HTTP_SEND_COMPONENT_HPP__

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
		 * @function : Asynchronous send data,supporting multi data formats,see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 */
		template<bool isRequest, class Body, class Fields = http::fields>
		inline bool send(http::message<isRequest, Body, Fields>& msg)
		{
			try
			{
				if (!derive.is_started())
					asio::detail::throw_error(asio::error::not_connected);

				// Make sure we run on the strand
				if (!this->wio_.strand().running_in_this_thread())
				{
					asio::post(this->wio_.strand(), make_allocator(derive.wallocator(),
						[this, p = this->_mkptr(), msg]() mutable
					{
						derive._do_send(msg);
					}));
					return true;
				}

				return derive._do_send(msg);
			}
			catch (system_error & e) { set_last_error(e); }
			catch (std::exception &) { set_last_error(asio::error::eof); }
			return false;
		}

		/**
		 * @function : Asynchronous send data,supporting multi data formats,see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 */
		template<bool isRequest, class Body, class Fields = http::fields>
		inline bool send(http::message<isRequest, Body, Fields>&& msg)
		{
			try
			{
				if (!derive.is_started())
					asio::detail::throw_error(asio::error::not_connected);

				// Make sure we run on the strand
				if (!this->wio_.strand().running_in_this_thread())
				{
					asio::post(this->wio_.strand(), make_allocator(derive.wallocator(),
						[this, p = this->_mkptr(), d = std::move(msg)]() mutable
					{
						derive._do_send(d);
					}));
					return true;
				}

				return derive._do_send(msg);
			}
			catch (system_error & e) { set_last_error(e); }
			catch (std::exception &) { set_last_error(asio::error::eof); }
			return false;
		}

		/**
		 * @function : Asynchronous send data,supporting multi data formats,see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * Callback signature : void() or void(std::size_t bytes_sent)
		 */
		template<class Callback, bool isRequest, class Body, class Fields = http::fields>
		inline bool send(http::message<isRequest, Body, Fields>& msg, Callback&& fn)
		{
			try
			{
				if (!derive.is_started())
					asio::detail::throw_error(asio::error::not_connected);

				// Make sure we run on the strand
				if (!this->wio_.strand().running_in_this_thread())
				{
					asio::post(this->wio_.strand(), make_allocator(derive.wallocator(),
						[this, p = this->_mkptr(), msg, f = std::forward<Callback>(fn)]() mutable
					{
						derive._do_send(msg, f);
					}));
					return true;
				}

				return derive._do_send(msg, fn);
			}
			catch (system_error & e) { set_last_error(e); }
			catch (std::exception &) { set_last_error(asio::error::eof); }
			return false;
		}

		/**
		 * @function : Asynchronous send data,supporting multi data formats,see asio::buffer(...) in /asio/buffer.hpp
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

				// Make sure we run on the strand
				if (!this->wio_.strand().running_in_this_thread())
				{
					asio::post(this->wio_.strand(), make_allocator(derive.wallocator(),
						[this, p = this->_mkptr(), d = std::move(msg), f = std::forward<Callback>(fn)]() mutable
					{
						derive._do_send(d, f);
					}));
					return true;
				}

				return derive._do_send(msg, fn);
			}
			catch (system_error & e) { set_last_error(e); }
			catch (std::exception &) { set_last_error(asio::error::eof); }
			return false;
		}

		/**
		 * @function : Asynchronous send data,supporting multi data formats,see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 */
		template<bool isRequest, class Body, class Fields = http::fields>
		inline std::future<std::pair<error_code, std::size_t>> send(
			http::message<isRequest, Body, Fields>& msg, asio::use_future_t<> flag)
		{
			std::promise<std::pair<error_code, std::size_t>> promise;
			std::future <std::pair<error_code, std::size_t>> future = promise.get_future();
			try
			{
				if (!derive.is_started())
					asio::detail::throw_error(asio::error::not_connected);

				// Make sure we run on the strand
				if (!this->wio_.strand().running_in_this_thread())
				{
					asio::post(this->wio_.strand(), make_allocator(derive.wallocator(),
						[this, p = this->_mkptr(), msg, pm = std::move(promise)]() mutable
					{
						derive._do_send(msg, pm);
					}));
					return future;
				}

				derive._do_send(msg, promise);
			}
			catch (system_error & e)
			{
				set_last_error(e);
				promise.set_value(std::pair<error_code, std::size_t>(e.code(), 0));
			}
			catch (std::exception &)
			{
				set_last_error(asio::error::eof);
				promise.set_value(std::pair<error_code, std::size_t>(asio::error::eof, 0));
			}
			return future;
		}

		/**
		 * @function : Asynchronous send data,supporting multi data formats,see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 */
		template<bool isRequest, class Body, class Fields = http::fields>
		inline std::future<std::pair<error_code, std::size_t>> send(
			http::message<isRequest, Body, Fields>&& msg, asio::use_future_t<> flag)
		{
			std::promise<std::pair<error_code, std::size_t>> promise;
			std::future <std::pair<error_code, std::size_t>> future = promise.get_future();
			try
			{
				if (!derive.is_started())
					asio::detail::throw_error(asio::error::not_connected);

				// Make sure we run on the strand
				if (!this->wio_.strand().running_in_this_thread())
				{
					asio::post(this->wio_.strand(), make_allocator(derive.wallocator(),
						[this, p = this->_mkptr(), d = std::move(msg), pm = std::move(promise)]() mutable
					{
						derive._do_send(d, pm);
					}));
					return future;
				}

				derive._do_send(msg, promise);
			}
			catch (system_error & e)
			{
				set_last_error(e);
				promise.set_value(std::pair<error_code, std::size_t>(e.code(), 0));
			}
			catch (std::exception &)
			{
				set_last_error(asio::error::eof);
				promise.set_value(std::pair<error_code, std::size_t>(asio::error::eof, 0));
			}
			return future;
		}

	protected:
		inline std::shared_ptr<derived_t> _mkptr()
		{
			if constexpr (isSession)
				return derive.shared_from_this();
			else
				return std::shared_ptr<derived_t>{};
		}

	protected:
		derived_t & derive;

		io_t      & wio_;
	};
}

#endif // !__ASIO2_HTTP_SEND_COMPONENT_HPP__

#endif
