/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_UDP_SEND_COMPONENT_HPP__
#define __ASIO2_UDP_SEND_COMPONENT_HPP__

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
#include <asio2/base/iopool.hpp>
#include <asio2/base/error.hpp>

#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/function_traits.hpp>
#include <asio2/base/detail/buffer_wrap.hpp>

#include <asio2/base/component/data_persistence_cp.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;

	template<class derived_t, class args_t = void>
	class udp_send_cp : public data_persistence_cp<derived_t, args_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;

	public:
		/**
		 * @constructor
		 */
		udp_send_cp(io_t&) {}

		/**
		 * @destructor
		 */
		~udp_send_cp() = default;

	public:
		/**
		 * @function : Asynchronous send data,supporting multi data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * use like this : std::string m; send(std::move(m)); can reducing memory allocation.
		 * PodType * : send("abc");
		 * PodType (&data)[N] : double m[10]; send(m);
		 * std::array<PodType, N> : std::array<int,10> m; send(m);
		 * std::vector<PodType, Allocator> : std::vector<float> m; send(m);
		 * std::basic_string<Elem, Traits, Allocator> : std::string m; send(m);
		 * We do not provide synchronous send function,because the synchronous send code is very simple,
		 * if you want use synchronous send data,you can do it like this (example):
		 * asio::write(session_ptr->stream(), asio::buffer(std::string("abc")));
		 */
		template<typename String, typename StrOrInt, class DataT>
		inline typename std::enable_if_t<!std::is_same_v<std::remove_cv_t<
			std::remove_reference_t<String>>, asio::ip::udp::endpoint>, bool>
			send(String&& host, StrOrInt&& port, DataT&& data)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// We must ensure that there is only one operation to send data
			// at the same time,otherwise may be cause crash.
			try
			{
				if (!derive.is_started())
					asio::detail::throw_error(asio::error::not_connected);

				return derive._do_resolve(
					std::forward<String>(host), std::forward<StrOrInt>(port),
					derive._data_persistence(std::forward<DataT>(data)),
					[](const error_code&, std::size_t) {});
			}
			catch (system_error & e) { set_last_error(e); }
			catch (std::exception &) { set_last_error(asio::error::eof); }
			return false;
		}

		/**
		 * @function : Asynchronous send data
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * PodType * : send("abc");
		 * We do not provide synchronous send function,because the synchronous send code is very simple,
		 * if you want use synchronous send data,you can do it like this (example):
		 * asio::write(session_ptr->stream(), asio::buffer(std::string("abc")));
		 */
		template<typename String, typename StrOrInt, class CharT, class Traits = std::char_traits<CharT>>
		inline typename std::enable_if_t<!std::is_same_v<std::remove_cv_t<std::remove_reference_t<String>>,
			asio::ip::udp::endpoint> && (
				std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, char> ||
				std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, wchar_t> ||
				std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, char16_t> ||
				std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, char32_t>), bool>
			send(String&& host, StrOrInt&& port, CharT* s)
		{
			return this->send(std::forward<String>(host),
				std::forward<StrOrInt>(port), s, s ? Traits::length(s) : 0);
		}

		/**
		 * @function : Asynchronous send data
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * PodType (&data)[N] : double m[10]; send(m,5);
		 * We do not provide synchronous send function,because the synchronous send code is very simple,
		 * if you want use synchronous send data,you can do it like this (example):
		 * asio::write(session_ptr->stream(), asio::buffer(std::string("abc")));
		 */
		template<typename String, typename StrOrInt, class CharT, class SizeT>
		inline typename std::enable_if_t<std::is_integral_v<std::remove_cv_t<std::remove_reference_t<SizeT>>> &&
			!std::is_same_v<std::remove_cv_t<std::remove_reference_t<String>>, asio::ip::udp::endpoint>, bool>
			send(String&& host, StrOrInt&& port, CharT * s, SizeT count)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// We must ensure that there is only one operation to send data
			// at the same time,otherwise may be cause crash.
			try
			{
				if (!derive.is_started())
					asio::detail::throw_error(asio::error::not_connected);

				if (!s)
					asio::detail::throw_error(asio::error::invalid_argument);

				return derive._do_resolve(std::forward<String>(host), std::forward<StrOrInt>(port),
					derive._data_persistence(s, count), [](const error_code&, std::size_t) {});
			}
			catch (system_error & e) { set_last_error(e); }
			catch (std::exception &) { set_last_error(asio::error::eof); }
			return false;
		}

		/**
		 * @function : Asynchronous send data,supporting multi data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * use like this : std::string m; send(std::move(m)); can reducing memory allocation.
		 * the pair.first save the send result error_code,the pair.second save the sent_bytes.
		 * note : Do not call this function in any listener callback function like this:
		 * auto future = send(msg,asio::use_future); future.get(); it will cause deadlock,
		 * the future.get() will never return.
		 * PodType * : send("abc");
		 * PodType (&data)[N] : double m[10]; send(m);
		 * std::array<PodType, N> : std::array<int,10> m; send(m);
		 * std::vector<PodType, Allocator> : std::vector<float> m; send(m);
		 * std::basic_string<Elem, Traits, Allocator> : std::string m; send(m);
		 */
		template<typename String, typename StrOrInt, class DataT>
		inline typename std::enable_if_t<
			!std::is_same_v<std::remove_cv_t<std::remove_reference_t<String>>, asio::ip::udp::endpoint>,
			std::future<std::pair<error_code, std::size_t>>>
			send(String&& host, StrOrInt&& port, DataT&& data, asio::use_future_t<> flag)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			std::ignore = flag;
			std::shared_ptr<std::promise<std::pair<error_code, std::size_t>>> promise =
				std::make_shared<std::promise<std::pair<error_code, std::size_t>>>();
			std::future<std::pair<error_code, std::size_t>> future = promise->get_future();
			try
			{
				if (!derive.is_started())
					asio::detail::throw_error(asio::error::not_connected);

				derive._do_resolve(std::forward<String>(host), std::forward<StrOrInt>(port),
					derive._data_persistence(std::forward<DataT>(data)),
					[promise = std::move(promise)](const error_code& ec, std::size_t bytes_sent) mutable
				{
					promise->set_value(std::pair<error_code, std::size_t>(ec, bytes_sent));
				});
			}
			catch (system_error & e)
			{
				set_last_error(e);
				promise->set_value(std::pair<error_code, std::size_t>(e.code(), 0));
			}
			catch (std::exception &)
			{
				set_last_error(asio::error::eof);
				promise->set_value(std::pair<error_code, std::size_t>(asio::error::eof, 0));
			}
			return future;
		}

		/**
		 * @function : Asynchronous send data
		 * the pair.first save the send result error_code,the pair.second save the sent_bytes.
		 * note : Do not call this function in any listener callback function like this:
		 * auto future = send(msg,asio::use_future); future.get(); it will cause deadlock,
		 * the future.get() will never return.
		 * PodType * : send("abc");
		 */
		template<typename String, typename StrOrInt, class CharT, class Traits = std::char_traits<CharT>>
		inline typename std::enable_if_t<
			!std::is_same_v<std::remove_cv_t<std::remove_reference_t<String>>, asio::ip::udp::endpoint> && (
				std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, char> ||
				std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, wchar_t> ||
				std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, char16_t> ||
				std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, char32_t>),
			std::future<std::pair<error_code, std::size_t>>>
			send(String&& host, StrOrInt&& port, CharT * s, asio::use_future_t<> flag)
		{
			return this->send(std::forward<String>(host), std::forward<StrOrInt>(port), s,
				s ? Traits::length(s) : 0, std::move(flag));
		}

		/**
		 * @function : Asynchronous send data
		 * the pair.first save the send result error_code,the pair.second save the sent_bytes.
		 * note : Do not call this function in any listener callback function like this:
		 * auto future = send(msg,asio::use_future); future.get(); it will cause deadlock,
		 * the future.get() will never return.
		 * PodType (&data)[N] : double m[10]; send(m,5);
		 */
		template<typename String, typename StrOrInt, class CharT, class SizeT>
		inline typename std::enable_if_t<std::is_integral_v<std::remove_cv_t<std::remove_reference_t<SizeT>>> &&
			!std::is_same_v<std::remove_cv_t<std::remove_reference_t<String>>, asio::ip::udp::endpoint>,
			std::future<std::pair<error_code, std::size_t>>>
			send(String&& host, StrOrInt&& port, CharT * s, SizeT count, asio::use_future_t<> flag)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			std::ignore = flag;
			std::shared_ptr<std::promise<std::pair<error_code, std::size_t>>> promise =
				std::make_shared<std::promise<std::pair<error_code, std::size_t>>>();
			std::future<std::pair<error_code, std::size_t>> future = promise->get_future();
			try
			{
				if (!derive.is_started())
					asio::detail::throw_error(asio::error::not_connected);

				if (!s)
					asio::detail::throw_error(asio::error::invalid_argument);

				derive._do_resolve(std::forward<String>(host), std::forward<StrOrInt>(port),
					derive._data_persistence(s, count),
					[promise = std::move(promise)](const error_code& ec, std::size_t bytes_sent) mutable
				{
					promise->set_value(std::pair<error_code, std::size_t>(ec, bytes_sent));
				});
			}
			catch (system_error & e)
			{
				set_last_error(e);
				promise->set_value(std::pair<error_code, std::size_t>(e.code(), 0));
			}
			catch (std::exception &)
			{
				set_last_error(asio::error::eof);
				promise->set_value(std::pair<error_code, std::size_t>(asio::error::eof, 0));
			}
			return future;
		}

		/**
		 * @function : Asynchronous send data,supporting multi data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * use like this : std::string m; send(std::move(m)); can reducing memory allocation.
		 * PodType * : send("abc");
		 * PodType (&data)[N] : double m[10]; send(m);
		 * std::array<PodType, N> : std::array<int,10> m; send(m);
		 * std::vector<PodType, Allocator> : std::vector<float> m; send(m);
		 * std::basic_string<Elem, Traits, Allocator> : std::string m; send(m);
		 * We do not provide synchronous send function,because the synchronous send code is very simple,
		 * if you want use synchronous send data,you can do it like this (example):
		 * asio::write(session_ptr->stream(), asio::buffer(std::string("abc")));
		 * Callback signature : void() or void(std::size_t bytes_sent)
		 */
		template<typename String, typename StrOrInt, class DataT, class Callback>
		inline typename std::enable_if_t<is_callable_v<Callback> &&
			!std::is_same_v<std::remove_cv_t<std::remove_reference_t<String>>, asio::ip::udp::endpoint>, bool>
			send(String&& host, StrOrInt&& port, DataT&& data, Callback&& fn)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// We must ensure that there is only one operation to send data
			// at the same time,otherwise may be cause crash.
			try
			{
				if (!derive.is_started())
					asio::detail::throw_error(asio::error::not_connected);

				return derive._do_resolve(std::forward<String>(host), std::forward<StrOrInt>(port),
					derive._data_persistence(std::forward<DataT>(data)),
					[fn = std::forward<Callback>(fn)](const error_code&, std::size_t bytes_sent) mutable
				{
					callback_helper::call(fn, bytes_sent);
				});
			}
			catch (system_error & e) { set_last_error(e); }
			catch (std::exception &) { set_last_error(asio::error::eof); }
			return false;
		}

		/**
		 * @function : Asynchronous send data
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * PodType * : send("abc");
		 * We do not provide synchronous send function,because the synchronous send code is very simple,
		 * if you want use synchronous send data,you can do it like this (example):
		 * asio::write(session_ptr->stream(), asio::buffer(std::string("abc")));
		 * Callback signature : void() or void(std::size_t bytes_sent)
		 */
		template<typename String, typename StrOrInt, class Callback, class CharT,
			class Traits = std::char_traits<CharT>>
		inline typename std::enable_if_t<is_callable_v<Callback> &&
			!std::is_same_v<std::remove_cv_t<std::remove_reference_t<String>>, asio::ip::udp::endpoint> && (
				std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, char> ||
				std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, wchar_t> ||
				std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, char16_t> ||
				std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, char32_t>), bool>
			send(String&& host, StrOrInt&& port, CharT * s, Callback&& fn)
		{
			return this->send(std::forward<String>(host),
				std::forward<StrOrInt>(port), s, s ? Traits::length(s) : 0,
				std::forward<Callback>(fn));
		}

		/**
		 * @function : Asynchronous send data
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * PodType (&data)[N] : double m[10]; send(m,5);
		 * We do not provide synchronous send function,because the synchronous send code is very simple,
		 * if you want use synchronous send data,you can do it like this (example):
		 * asio::write(session_ptr->stream(), asio::buffer(std::string("abc")));
		 * Callback signature : void() or void(std::size_t bytes_sent)
		 */
		template<typename String, typename StrOrInt, class Callback, class CharT, class SizeT>
		inline typename std::enable_if_t<is_callable_v<Callback> &&
			!std::is_same_v<std::remove_cv_t<std::remove_reference_t<String>>, asio::ip::udp::endpoint> &&
			std::is_integral_v<std::remove_cv_t<std::remove_reference_t<SizeT>>>, bool>
			send(String&& host, StrOrInt&& port, CharT * s, SizeT count, Callback&& fn)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// We must ensure that there is only one operation to send data
			// at the same time,otherwise may be cause crash.
			try
			{
				if (!derive.is_started())
					asio::detail::throw_error(asio::error::not_connected);

				if (!s)
					asio::detail::throw_error(asio::error::invalid_argument);

				return derive._do_resolve(std::forward<String>(host), std::forward<StrOrInt>(port),
					derive._data_persistence(s, count),
					[fn = std::forward<Callback>(fn)](const error_code&, std::size_t bytes_sent) mutable
				{
					callback_helper::call(fn, bytes_sent);
				});
			}
			catch (system_error & e) { set_last_error(e); }
			catch (std::exception &) { set_last_error(asio::error::eof); }
			return false;
		}

	public:
		/**
		 * @function : Asynchronous send data,supporting multi data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * use like this : std::string m; send(std::move(m)); can reducing memory allocation.
		 * PodType * : send("abc");
		 * PodType (&data)[N] : double m[10]; send(m);
		 * std::array<PodType, N> : std::array<int,10> m; send(m);
		 * std::vector<PodType, Allocator> : std::vector<float> m; send(m);
		 * std::basic_string<Elem, Traits, Allocator> : std::string m; send(m);
		 * We do not provide synchronous send function,because the synchronous send code is very simple,
		 * if you want use synchronous send data,you can do it like this (example):
		 * asio::write(session_ptr->stream(), asio::buffer(std::string("abc")));
		 */
		template<class Endpoint, class DataT>
		inline typename std::enable_if_t<
			std::is_same_v<std::remove_cv_t<std::remove_reference_t<Endpoint>>, asio::ip::udp::endpoint>, bool>
			send(Endpoint&& endpoint, DataT&& data)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// We must ensure that there is only one operation to send data
			// at the same time,otherwise may be cause crash.
			try
			{
				if (!derive.is_started())
					asio::detail::throw_error(asio::error::not_connected);

				derive.push_event([&derive, endpoint = std::forward<Endpoint>(endpoint),
					data = derive._data_persistence(std::forward<DataT>(data))]
					(event_queue_guard<derived_t>&& g) mutable
				{
					return derive._do_send(endpoint, data,
						[g = std::move(g)](const error_code&, std::size_t) mutable {});
				});
				return true;
			}
			catch (system_error & e) { set_last_error(e); }
			catch (std::exception &) { set_last_error(asio::error::eof); }
			return false;
		}

		/**
		 * @function : Asynchronous send data
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * PodType * : send("abc");
		 * We do not provide synchronous send function,because the synchronous send code is very simple,
		 * if you want use synchronous send data,you can do it like this (example):
		 * asio::write(session_ptr->stream(), asio::buffer(std::string("abc")));
		 */
		template<class Endpoint, class CharT, class Traits = std::char_traits<CharT>>
		inline typename std::enable_if_t<
			std::is_same_v<std::remove_cv_t<std::remove_reference_t<Endpoint>>, asio::ip::udp::endpoint> && (
				std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, char> ||
				std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, wchar_t> ||
				std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, char16_t> ||
				std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, char32_t>), bool>
			send(Endpoint&& endpoint, CharT * s)
		{
			return this->send(std::forward<Endpoint>(endpoint), s, s ? Traits::length(s) : 0);
		}

		/**
		 * @function : Asynchronous send data
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * PodType (&data)[N] : double m[10]; send(m,5);
		 * We do not provide synchronous send function,because the synchronous send code is very simple,
		 * if you want use synchronous send data,you can do it like this (example):
		 * asio::write(session_ptr->stream(), asio::buffer(std::string("abc")));
		 */
		template<class Endpoint, class CharT, class SizeT>
		inline typename std::enable_if_t<std::is_integral_v<std::remove_cv_t<std::remove_reference_t<SizeT>>> &&
			std::is_same_v<std::remove_cv_t<std::remove_reference_t<Endpoint>>, asio::ip::udp::endpoint>, bool>
			send(Endpoint&& endpoint, CharT * s, SizeT count)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// We must ensure that there is only one operation to send data
			// at the same time,otherwise may be cause crash.
			try
			{
				if (!derive.is_started())
					asio::detail::throw_error(asio::error::not_connected);

				if (!s)
					asio::detail::throw_error(asio::error::invalid_argument);

				derive.push_event([&derive, endpoint = std::forward<Endpoint>(endpoint),
					data = derive._data_persistence(s, count)](event_queue_guard<derived_t>&& g) mutable
				{
					return derive._do_send(endpoint, data,
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
		 * use like this : std::string m; send(std::move(m)); can reducing memory allocation.
		 * the pair.first save the send result error_code,the pair.second save the sent_bytes.
		 * note : Do not call this function in any listener callback function like this:
		 * auto future = send(msg,asio::use_future); future.get(); it will cause deadlock,
		 * the future.get() will never return.
		 * PodType * : send("abc");
		 * PodType (&data)[N] : double m[10]; send(m);
		 * std::array<PodType, N> : std::array<int,10> m; send(m);
		 * std::vector<PodType, Allocator> : std::vector<float> m; send(m);
		 * std::basic_string<Elem, Traits, Allocator> : std::string m; send(m);
		 */
		template<class Endpoint, class DataT>
		inline typename std::enable_if_t<
			std::is_same_v<std::remove_cv_t<std::remove_reference_t<Endpoint>>, asio::ip::udp::endpoint>,
			std::future<std::pair<error_code, std::size_t>>>
			send(Endpoint&& endpoint, DataT&& data, asio::use_future_t<> flag)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			std::ignore = flag;
			std::shared_ptr<std::promise<std::pair<error_code, std::size_t>>> promise =
				std::make_shared<std::promise<std::pair<error_code, std::size_t>>>();
			std::future<std::pair<error_code, std::size_t>> future = promise->get_future();
			try
			{
				if (!derive.is_started())
					asio::detail::throw_error(asio::error::not_connected);

				derive.push_event([&derive, endpoint = std::forward<Endpoint>(endpoint),
					data = derive._data_persistence(std::forward<DataT>(data)),
					promise = std::move(promise)](event_queue_guard<derived_t>&& g) mutable
				{
					return derive._do_send(endpoint, data, [&promise, g = std::move(g)]
					(const error_code& ec, std::size_t bytes_sent) mutable
					{
						promise->set_value(std::pair<error_code, std::size_t>(ec, bytes_sent));
					});
				});
			}
			catch (system_error & e)
			{
				set_last_error(e);
				promise->set_value(std::pair<error_code, std::size_t>(e.code(), 0));
			}
			catch (std::exception &)
			{
				set_last_error(asio::error::eof);
				promise->set_value(std::pair<error_code, std::size_t>(asio::error::eof, 0));
			}
			return future;
		}

		/**
		 * @function : Asynchronous send data
		 * the pair.first save the send result error_code,the pair.second save the sent_bytes.
		 * note : Do not call this function in any listener callback function like this:
		 * auto future = send(msg,asio::use_future); future.get(); it will cause deadlock,
		 * the future.get() will never return.
		 * PodType * : send("abc");
		 */
		template<class Endpoint, class CharT, class Traits = std::char_traits<CharT>>
		inline typename std::enable_if_t<
			std::is_same_v<std::remove_cv_t<std::remove_reference_t<Endpoint>>, asio::ip::udp::endpoint> && (
				std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, char> ||
				std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, wchar_t> ||
				std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, char16_t> ||
				std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, char32_t>),
			std::future<std::pair<error_code, std::size_t>>>
			send(Endpoint&& endpoint, CharT * s, asio::use_future_t<> flag)
		{
			return this->send(std::forward<Endpoint>(endpoint), s,
				s ? Traits::length(s) : 0, std::move(flag));
		}

		/**
		 * @function : Asynchronous send data
		 * the pair.first save the send result error_code,the pair.second save the sent_bytes.
		 * note : Do not call this function in any listener callback function like this:
		 * auto future = send(msg,asio::use_future); future.get(); it will cause deadlock,
		 * the future.get() will never return.
		 * PodType (&data)[N] : double m[10]; send(m,5);
		 */
		template<class Endpoint, class CharT, class SizeT>
		inline typename std::enable_if_t<std::is_integral_v<std::remove_cv_t<std::remove_reference_t<SizeT>>> &&
			std::is_same_v<std::remove_cv_t<std::remove_reference_t<Endpoint>>, asio::ip::udp::endpoint>,
			std::future<std::pair<error_code, std::size_t>>>
			send(Endpoint&& endpoint, CharT * s, SizeT count, asio::use_future_t<> flag)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			std::ignore = flag;
			std::shared_ptr<std::promise<std::pair<error_code, std::size_t>>> promise =
				std::make_shared<std::promise<std::pair<error_code, std::size_t>>>();
			std::future<std::pair<error_code, std::size_t>> future = promise->get_future();
			try
			{
				if (!derive.is_started())
					asio::detail::throw_error(asio::error::not_connected);

				if (!s)
					asio::detail::throw_error(asio::error::invalid_argument);

				derive.push_event([&derive, endpoint = std::forward<Endpoint>(endpoint),
					data = derive._data_persistence(s, count),
					promise = std::move(promise)](event_queue_guard<derived_t>&& g) mutable
				{
					return derive._do_send(endpoint, data, [&promise, g = std::move(g)]
					(const error_code& ec, std::size_t bytes_sent) mutable
					{
						promise->set_value(std::pair<error_code, std::size_t>(ec, bytes_sent));
					});
				});
			}
			catch (system_error & e)
			{
				set_last_error(e);
				promise->set_value(std::pair<error_code, std::size_t>(e.code(), 0));
			}
			catch (std::exception &)
			{
				set_last_error(asio::error::eof);
				promise->set_value(std::pair<error_code, std::size_t>(asio::error::eof, 0));
			}
			return future;
		}

		/**
		 * @function : Asynchronous send data,supporting multi data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * use like this : std::string m; send(std::move(m)); can reducing memory allocation.
		 * PodType * : send("abc");
		 * PodType (&data)[N] : double m[10]; send(m);
		 * std::array<PodType, N> : std::array<int,10> m; send(m);
		 * std::vector<PodType, Allocator> : std::vector<float> m; send(m);
		 * std::basic_string<Elem, Traits, Allocator> : std::string m; send(m);
		 * We do not provide synchronous send function,because the synchronous send code is very simple,
		 * if you want use synchronous send data,you can do it like this (example):
		 * asio::write(session_ptr->stream(), asio::buffer(std::string("abc")));
		 * Callback signature : void() or void(std::size_t bytes_sent)
		 */
		template<class Endpoint, class DataT, class Callback>
		inline typename std::enable_if_t<is_callable_v<Callback> &&
			std::is_same_v<std::remove_cv_t<std::remove_reference_t<Endpoint>>, asio::ip::udp::endpoint>, bool>
			send(Endpoint&& endpoint, DataT&& data, Callback&& fn)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// We must ensure that there is only one operation to send data
			// at the same time,otherwise may be cause crash.
			try
			{
				if (!derive.is_started())
					asio::detail::throw_error(asio::error::not_connected);

				derive.push_event([&derive, endpoint = std::forward<Endpoint>(endpoint),
					data = derive._data_persistence(std::forward<DataT>(data)),
					fn = std::forward<Callback>(fn)](event_queue_guard<derived_t>&& g) mutable
				{
					return derive._do_send(endpoint, data, [&fn, g = std::move(g)]
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
		 * @function : Asynchronous send data
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * PodType * : send("abc");
		 * We do not provide synchronous send function,because the synchronous send code is very simple,
		 * if you want use synchronous send data,you can do it like this (example):
		 * asio::write(session_ptr->stream(), asio::buffer(std::string("abc")));
		 * Callback signature : void() or void(std::size_t bytes_sent)
		 */
		template<class Endpoint, class Callback, class CharT, class Traits = std::char_traits<CharT>>
		inline typename std::enable_if_t<is_callable_v<Callback> &&
			std::is_same_v<std::remove_cv_t<std::remove_reference_t<Endpoint>>, asio::ip::udp::endpoint> && (
			std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, char> ||
			std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, wchar_t> ||
			std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, char16_t> ||
			std::is_same_v<std::remove_cv_t<std::remove_reference_t<CharT>>, char32_t>), bool>
			send(Endpoint&& endpoint, CharT * s, Callback&& fn)
		{
			return this->send(std::forward<Endpoint>(endpoint), s,
				s ? Traits::length(s) : 0, std::forward<Callback>(fn));
		}

		/**
		 * @function : Asynchronous send data
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * PodType (&data)[N] : double m[10]; send(m,5);
		 * We do not provide synchronous send function,because the synchronous send code is very simple,
		 * if you want use synchronous send data,you can do it like this (example):
		 * asio::write(session_ptr->stream(), asio::buffer(std::string("abc")));
		 * Callback signature : void() or void(std::size_t bytes_sent)
		 */
		template<class Endpoint, class Callback, class CharT, class SizeT>
		inline typename std::enable_if_t<is_callable_v<Callback> &&
			std::is_same_v<std::remove_cv_t<std::remove_reference_t<Endpoint>>, asio::ip::udp::endpoint> &&
			std::is_integral_v<std::remove_cv_t<std::remove_reference_t<SizeT>>>, bool>
			send(Endpoint&& endpoint, CharT * s, SizeT count, Callback&& fn)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// We must ensure that there is only one operation to send data
			// at the same time,otherwise may be cause crash.
			try
			{
				if (!derive.is_started())
					asio::detail::throw_error(asio::error::not_connected);

				if (!s)
					asio::detail::throw_error(asio::error::invalid_argument);

				derive.push_event([&derive, endpoint = std::forward<Endpoint>(endpoint),
					data = derive._data_persistence(s, count),
					fn = std::forward<Callback>(fn)](event_queue_guard<derived_t>&& g) mutable
				{
					return derive._do_send(endpoint, data, [&fn, g = std::move(g)]
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

	protected:
		template<typename String, typename StrOrInt, typename Data, typename Callback>
		inline bool _do_resolve(String&& host, StrOrInt&& port, Data&& data, Callback&& callback)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			using resolver_type = asio::ip::udp::resolver;
			using endpoints_type = typename resolver_type::results_type;
			//using endpoints_iterator = typename endpoints_type::iterator;

			std::unique_ptr<resolver_type> resolver_ptr = std::make_unique<resolver_type>(
				derive.io().context());

			// Before async_resolve execution is complete, we must hold the resolver object.
			// so we captured the resolver_ptr into the lambda callback function.
			resolver_type * resolver_pointer = resolver_ptr.get();
			resolver_pointer->async_resolve(std::forward<String>(host), to_string(std::forward<StrOrInt>(port)),
				asio::bind_executor(derive.io().strand(),
					[&derive, p = derive.selfptr(), resolver_ptr = std::move(resolver_ptr),
					data = std::forward<Data>(data), callback = std::forward<Callback>(callback)]
			(const error_code& ec, const endpoints_type& endpoints) mutable
			{
				set_last_error(ec);

				if (ec)
				{
					callback(ec, 0);
				}
				else
				{
					decltype(endpoints.size()) i = 1;
					for (auto iter = endpoints.begin(); iter != endpoints.end(); ++iter, ++i)
					{
						derive.push_event([&derive, endpoint = iter->endpoint(),
							data = (endpoints.size() == i ? std::move(data) : data),
							callback = (endpoints.size() == i ? std::move(callback) : callback)
						](event_queue_guard<derived_t>&& g) mutable
						{
							return derive._do_send(endpoint, data,
								[g = std::move(g), f = std::move(callback)]
							(const error_code& ec, std::size_t bytes_sent) mutable
							{
								f(ec, bytes_sent);
							});
						});
					}
				}
			}));
			return true;
		}
	};
}

#endif // !__ASIO2_UDP_SEND_COMPONENT_HPP__
