/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
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

#include <asio2/base/iopool.hpp>
#include <asio2/base/define.hpp>

#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/function_traits.hpp>
#include <asio2/base/detail/buffer_wrap.hpp>

#include <asio2/base/impl/data_persistence_cp.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;

	template<class derived_t, class args_t>
	class udp_send_cp : public data_persistence_cp<derived_t, args_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;

	public:
		/**
		 * @brief constructor
		 */
		udp_send_cp() noexcept {}

		/**
		 * @brief destructor
		 */
		~udp_send_cp() = default;

	public:
		/**
		 * @brief Asynchronous send data,supporting multi data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * use like this : std::string m; async_send(std::move(m)); can reducing memory allocation.
		 * PodType * : async_send("abc");
		 * PodType (&data)[N] : double m[10]; async_send(m);
		 * std::array<PodType, N> : std::array<int,10> m; async_send(m);
		 * std::vector<PodType, Allocator> : std::vector<float> m; async_send(m);
		 * std::basic_string<Elem, Traits, Allocator> : std::string m; async_send(m);
		 */
		template<typename String, typename StrOrInt, class DataT>
		inline typename std::enable_if_t<!std::is_same_v<detail::remove_cvref_t<String>,
			asio::ip::udp::endpoint>, void>
			async_send(String&& host, StrOrInt&& port, DataT&& data) noexcept
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			detail::integer_add_sub_guard asg(derive.io_->pending());

			// We must ensure that there is only one operation to send data
			// at the same time,otherwise may be cause crash.

			derive._do_resolve(std::forward<String>(host), std::forward<StrOrInt>(port),
				derive._data_persistence(std::forward<DataT>(data)),
				[](const error_code&, std::size_t) mutable {});
		}

		/**
		 * @brief Asynchronous send data
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * PodType * : async_send("abc");
		 */
		template<typename String, typename StrOrInt, class CharT, class Traits = std::char_traits<CharT>>
		inline typename std::enable_if_t<!std::is_same_v<detail::remove_cvref_t<String>,
			asio::ip::udp::endpoint> && detail::is_char_v<CharT>, void>
			async_send(String&& host, StrOrInt&& port, CharT* s) noexcept
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			derive.async_send(std::forward<String>(host), std::forward<StrOrInt>(port),
				s, s ? Traits::length(s) : 0);
		}

		/**
		 * @brief Asynchronous send data
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * PodType (&data)[N] : double m[10]; async_send(m,5);
		 */
		template<typename String, typename StrOrInt, class CharT, class SizeT>
		inline typename std::enable_if_t<std::is_integral_v<detail::remove_cvref_t<SizeT>> &&
			!std::is_same_v<detail::remove_cvref_t<String>, asio::ip::udp::endpoint>, void>
			async_send(String&& host, StrOrInt&& port, CharT * s, SizeT count) noexcept
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			if (!s)
			{
				set_last_error(asio::error::invalid_argument);
				return;
			}

			detail::integer_add_sub_guard asg(derive.io_->pending());

			// We must ensure that there is only one operation to send data
			// at the same time,otherwise may be cause crash.

			derive._do_resolve(std::forward<String>(host), std::forward<StrOrInt>(port),
				derive._data_persistence(s, count), [](const error_code&, std::size_t) mutable {});
		}

		/**
		 * @brief Asynchronous send data,supporting multi data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * use like this : std::string m; async_send(std::move(m)); can reducing memory allocation.
		 * the pair.first save the send result error_code,the pair.second save the sent_bytes.
		 * note : Do not call this function in any listener callback function like this:
		 * auto future = async_send(msg,asio::use_future); future.get(); it will cause deadlock and
		 * the future.get() will never return.
		 * PodType * : async_send("abc");
		 * PodType (&data)[N] : double m[10]; async_send(m);
		 * std::array<PodType, N> : std::array<int,10> m; async_send(m);
		 * std::vector<PodType, Allocator> : std::vector<float> m; async_send(m);
		 * std::basic_string<Elem, Traits, Allocator> : std::string m; async_send(m);
		 */
		template<typename String, typename StrOrInt, class DataT>
		inline typename std::enable_if_t<
			!std::is_same_v<detail::remove_cvref_t<String>, asio::ip::udp::endpoint>,
			std::future<std::pair<error_code, std::size_t>>>
			async_send(String&& host, StrOrInt&& port, DataT&& data, asio::use_future_t<>)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			detail::integer_add_sub_guard asg(derive.io_->pending());

			std::shared_ptr<std::promise<std::pair<error_code, std::size_t>>> promise =
				std::make_shared<std::promise<std::pair<error_code, std::size_t>>>();
			std::future<std::pair<error_code, std::size_t>> future = promise->get_future();

			derive._do_resolve(std::forward<String>(host), std::forward<StrOrInt>(port),
				derive._data_persistence(std::forward<DataT>(data)),
				[promise = std::move(promise)](const error_code& ec, std::size_t bytes_sent) mutable
			{
				// if multiple addresses is resolved for the host and port, then the promise
				// will set_value many times, then will cause exception
				try
				{
					promise->set_value(std::pair<error_code, std::size_t>(ec, bytes_sent));
				}
				catch (std::future_errc const& e)
				{
					set_last_error(e);
				}
			});

			return future;
		}

		/**
		 * @brief Asynchronous send data
		 * the pair.first save the send result error_code,the pair.second save the sent_bytes.
		 * note : Do not call this function in any listener callback function like this:
		 * auto future = async_send(msg,asio::use_future); future.get(); it will cause deadlock and
		 * the future.get() will never return.
		 * PodType * : async_send("abc");
		 */
		template<typename String, typename StrOrInt, class CharT, class Traits = std::char_traits<CharT>>
		inline typename std::enable_if_t<
			!std::is_same_v<detail::remove_cvref_t<String>, asio::ip::udp::endpoint> && detail::is_char_v<CharT>,
			std::future<std::pair<error_code, std::size_t>>>
			async_send(String&& host, StrOrInt&& port, CharT * s, asio::use_future_t<> flag)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return derive.async_send(std::forward<String>(host), std::forward<StrOrInt>(port), s,
				s ? Traits::length(s) : 0, std::move(flag));
		}

		/**
		 * @brief Asynchronous send data
		 * the pair.first save the send result error_code,the pair.second save the sent_bytes.
		 * note : Do not call this function in any listener callback function like this:
		 * auto future = async_send(msg,asio::use_future); future.get(); it will cause deadlock and
		 * the future.get() will never return.
		 * PodType (&data)[N] : double m[10]; async_send(m,5);
		 */
		template<typename String, typename StrOrInt, class CharT, class SizeT>
		inline typename std::enable_if_t<std::is_integral_v<detail::remove_cvref_t<SizeT>> &&
			!std::is_same_v<detail::remove_cvref_t<String>, asio::ip::udp::endpoint>,
			std::future<std::pair<error_code, std::size_t>>>
			async_send(String&& host, StrOrInt&& port, CharT * s, SizeT count, asio::use_future_t<>)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			detail::integer_add_sub_guard asg(derive.io_->pending());

			std::shared_ptr<std::promise<std::pair<error_code, std::size_t>>> promise =
				std::make_shared<std::promise<std::pair<error_code, std::size_t>>>();
			std::future<std::pair<error_code, std::size_t>> future = promise->get_future();

			if (!s)
			{
				set_last_error(asio::error::invalid_argument);
				promise->set_value(std::pair<error_code, std::size_t>(asio::error::invalid_argument, 0));
				return future;
			}

			derive._do_resolve(std::forward<String>(host), std::forward<StrOrInt>(port),
				derive._data_persistence(s, count),
				[promise = std::move(promise)](const error_code& ec, std::size_t bytes_sent) mutable
			{
				// if multiple addresses is resolved for the host and port, then the promise
				// will set_value many times, then will cause exception
				try
				{
					promise->set_value(std::pair<error_code, std::size_t>(ec, bytes_sent));
				}
				catch (std::future_errc const& e)
				{
					set_last_error(e);
				}
			});

			return future;
		}

		/**
		 * @brief Asynchronous send data,supporting multi data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * use like this : std::string m; async_send(std::move(m)); can reducing memory allocation.
		 * PodType * : async_send("abc");
		 * PodType (&data)[N] : double m[10]; async_send(m);
		 * std::array<PodType, N> : std::array<int,10> m; async_send(m);
		 * std::vector<PodType, Allocator> : std::vector<float> m; async_send(m);
		 * std::basic_string<Elem, Traits, Allocator> : std::string m; async_send(m);
		 * Callback signature : void() or void(std::size_t bytes_sent)
		 */
		template<typename String, typename StrOrInt, class DataT, class Callback>
		inline typename std::enable_if_t<is_callable_v<Callback> &&
			!std::is_same_v<detail::remove_cvref_t<String>, asio::ip::udp::endpoint>, void>
			async_send(String&& host, StrOrInt&& port, DataT&& data, Callback&& fn)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			detail::integer_add_sub_guard asg(derive.io_->pending());

			// We must ensure that there is only one operation to send data
			// at the same time,otherwise may be cause crash.

			derive._do_resolve(std::forward<String>(host), std::forward<StrOrInt>(port),
				derive._data_persistence(std::forward<DataT>(data)),
				[fn = std::forward<Callback>(fn)](const error_code&, std::size_t bytes_sent) mutable
			{
				callback_helper::call(fn, bytes_sent);
			});
		}

		/**
		 * @brief Asynchronous send data
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * PodType * : async_send("abc");
		 * Callback signature : void() or void(std::size_t bytes_sent)
		 */
		template<typename String, typename StrOrInt, class Callback, class CharT,
			class Traits = std::char_traits<CharT>>
		inline typename std::enable_if_t<is_callable_v<Callback> &&
			!std::is_same_v<detail::remove_cvref_t<String>, asio::ip::udp::endpoint> &&
			detail::is_char_v<CharT>, void>
			async_send(String&& host, StrOrInt&& port, CharT * s, Callback&& fn)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			derive.async_send(std::forward<String>(host), std::forward<StrOrInt>(port),
				s, s ? Traits::length(s) : 0, std::forward<Callback>(fn));
		}

		/**
		 * @brief Asynchronous send data
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * PodType (&data)[N] : double m[10]; async_send(m,5);
		 * Callback signature : void() or void(std::size_t bytes_sent)
		 */
		template<typename String, typename StrOrInt, class Callback, class CharT, class SizeT>
		inline typename std::enable_if_t<is_callable_v<Callback> &&
			!std::is_same_v<detail::remove_cvref_t<String>, asio::ip::udp::endpoint> &&
			std::is_integral_v<detail::remove_cvref_t<SizeT>>, void>
			async_send(String&& host, StrOrInt&& port, CharT * s, SizeT count, Callback&& fn)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			detail::integer_add_sub_guard asg(derive.io_->pending());

			// We must ensure that there is only one operation to send data
			// at the same time,otherwise may be cause crash.

			//// don't need do this
			//if (!s)
			//{
			//	// ... 
			//}

			derive._do_resolve(std::forward<String>(host), std::forward<StrOrInt>(port),
				derive._data_persistence(s, count),
				[fn = std::forward<Callback>(fn)](const error_code&, std::size_t bytes_sent) mutable
			{
				callback_helper::call(fn, bytes_sent);
			});
		}

	public:
		/**
		 * @brief Asynchronous send data,supporting multi data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * use like this : std::string m; async_send(std::move(m)); can reducing memory allocation.
		 * PodType * : async_send("abc");
		 * PodType (&data)[N] : double m[10]; async_send(m);
		 * std::array<PodType, N> : std::array<int,10> m; async_send(m);
		 * std::vector<PodType, Allocator> : std::vector<float> m; async_send(m);
		 * std::basic_string<Elem, Traits, Allocator> : std::string m; async_send(m);
		 */
		template<class Endpoint, class DataT>
		inline typename std::enable_if_t<
			std::is_same_v<detail::remove_cvref_t<Endpoint>, asio::ip::udp::endpoint>, void>
			async_send(Endpoint&& endpoint, DataT&& data) noexcept
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			detail::integer_add_sub_guard asg(derive.io_->pending());

			// We must ensure that there is only one operation to send data
			// at the same time,otherwise may be cause crash.

			derive.push_event(
			[&derive, p = derive.selfptr(), id = derive.life_id(), endpoint = std::forward<Endpoint>(endpoint),
				data = derive._data_persistence(std::forward<DataT>(data))]
			(event_queue_guard<derived_t> g) mutable
			{
				if (!derive.is_started())
				{
					set_last_error(asio::error::not_connected);
					return;
				}

				if (id != derive.life_id())
				{
					set_last_error(asio::error::operation_aborted);
					return;
				}

				clear_last_error();

				derive._do_send(endpoint, data, [g = std::move(g)](const error_code&, std::size_t) mutable {});
			});
		}

		/**
		 * @brief Asynchronous send data
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * PodType * : async_send("abc");
		 */
		template<class Endpoint, class CharT, class Traits = std::char_traits<CharT>>
		inline typename std::enable_if_t<
			std::is_same_v<detail::remove_cvref_t<Endpoint>, asio::ip::udp::endpoint> &&
			detail::is_char_v<CharT>, void>
			async_send(Endpoint&& endpoint, CharT * s) noexcept
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			derive.async_send(std::forward<Endpoint>(endpoint), s, s ? Traits::length(s) : 0);
		}

		/**
		 * @brief Asynchronous send data
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * PodType (&data)[N] : double m[10]; async_send(m,5);
		 */
		template<class Endpoint, class CharT, class SizeT>
		inline typename std::enable_if_t<std::is_integral_v<detail::remove_cvref_t<SizeT>> &&
			std::is_same_v<detail::remove_cvref_t<Endpoint>, asio::ip::udp::endpoint>, void>
			async_send(Endpoint&& endpoint, CharT * s, SizeT count) noexcept
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			if (!s)
			{
				set_last_error(asio::error::invalid_argument);
				return;
			}

			detail::integer_add_sub_guard asg(derive.io_->pending());

			// We must ensure that there is only one operation to send data
			// at the same time,otherwise may be cause crash.

			derive.push_event(
			[&derive, p = derive.selfptr(), id = derive.life_id(), endpoint = std::forward<Endpoint>(endpoint),
				data = derive._data_persistence(s, count)]
			(event_queue_guard<derived_t> g) mutable
			{
				if (!derive.is_started())
				{
					set_last_error(asio::error::not_connected);
					return;
				}

				if (id != derive.life_id())
				{
					set_last_error(asio::error::operation_aborted);
					return;
				}

				clear_last_error();

				derive._do_send(endpoint, data, [g = std::move(g)](const error_code&, std::size_t) mutable {});
			});
		}

		/**
		 * @brief Asynchronous send data,supporting multi data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * use like this : std::string m; async_send(std::move(m)); can reducing memory allocation.
		 * the pair.first save the send result error_code,the pair.second save the sent_bytes.
		 * note : Do not call this function in any listener callback function like this:
		 * auto future = async_send(msg,asio::use_future); future.get(); it will cause deadlock and
		 * the future.get() will never return.
		 * PodType * : async_send("abc");
		 * PodType (&data)[N] : double m[10]; async_send(m);
		 * std::array<PodType, N> : std::array<int,10> m; async_send(m);
		 * std::vector<PodType, Allocator> : std::vector<float> m; async_send(m);
		 * std::basic_string<Elem, Traits, Allocator> : std::string m; async_send(m);
		 */
		template<class Endpoint, class DataT>
		inline typename std::enable_if_t<
			std::is_same_v<detail::remove_cvref_t<Endpoint>, asio::ip::udp::endpoint>,
			std::future<std::pair<error_code, std::size_t>>>
			async_send(Endpoint&& endpoint, DataT&& data, asio::use_future_t<>)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			detail::integer_add_sub_guard asg(derive.io_->pending());

			std::shared_ptr<std::promise<std::pair<error_code, std::size_t>>> promise =
				std::make_shared<std::promise<std::pair<error_code, std::size_t>>>();
			std::future<std::pair<error_code, std::size_t>> future = promise->get_future();

			derive.push_event(
			[&derive, p = derive.selfptr(), id = derive.life_id(), endpoint = std::forward<Endpoint>(endpoint),
				data = derive._data_persistence(std::forward<DataT>(data)), promise = std::move(promise)]
			(event_queue_guard<derived_t> g) mutable
			{
				if (!derive.is_started())
				{
					set_last_error(asio::error::not_connected);
					promise->set_value(std::pair<error_code, std::size_t>(asio::error::not_connected, 0));
					return;
				}

				if (id != derive.life_id())
				{
					set_last_error(asio::error::operation_aborted);
					promise->set_value(std::pair<error_code, std::size_t>(asio::error::operation_aborted, 0));
					return;
				}

				clear_last_error();

				derive._do_send(endpoint, data, [&promise, g = std::move(g)]
				(const error_code& ec, std::size_t bytes_sent) mutable
				{
					promise->set_value(std::pair<error_code, std::size_t>(ec, bytes_sent));
				});
			});

			return future;
		}

		/**
		 * @brief Asynchronous send data
		 * the pair.first save the send result error_code,the pair.second save the sent_bytes.
		 * note : Do not call this function in any listener callback function like this:
		 * auto future = async_send(msg,asio::use_future); future.get(); it will cause deadlock and
		 * the future.get() will never return.
		 * PodType * : async_send("abc");
		 */
		template<class Endpoint, class CharT, class Traits = std::char_traits<CharT>>
		inline typename std::enable_if_t<
			std::is_same_v<detail::remove_cvref_t<Endpoint>, asio::ip::udp::endpoint> && detail::is_char_v<CharT>,
			std::future<std::pair<error_code, std::size_t>>>
			async_send(Endpoint&& endpoint, CharT * s, asio::use_future_t<> flag)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return derive.async_send(std::forward<Endpoint>(endpoint), s,
				s ? Traits::length(s) : 0, std::move(flag));
		}

		/**
		 * @brief Asynchronous send data
		 * the pair.first save the send result error_code,the pair.second save the sent_bytes.
		 * note : Do not call this function in any listener callback function like this:
		 * auto future = async_send(msg,asio::use_future); future.get(); it will cause deadlock and
		 * the future.get() will never return.
		 * PodType (&data)[N] : double m[10]; async_send(m,5);
		 */
		template<class Endpoint, class CharT, class SizeT>
		inline typename std::enable_if_t<std::is_integral_v<detail::remove_cvref_t<SizeT>> &&
			std::is_same_v<detail::remove_cvref_t<Endpoint>, asio::ip::udp::endpoint>,
			std::future<std::pair<error_code, std::size_t>>>
			async_send(Endpoint&& endpoint, CharT * s, SizeT count, asio::use_future_t<>)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			detail::integer_add_sub_guard asg(derive.io_->pending());

			std::shared_ptr<std::promise<std::pair<error_code, std::size_t>>> promise =
				std::make_shared<std::promise<std::pair<error_code, std::size_t>>>();
			std::future<std::pair<error_code, std::size_t>> future = promise->get_future();

			if (!s)
			{
				set_last_error(asio::error::invalid_argument);
				promise->set_value(std::pair<error_code, std::size_t>(asio::error::invalid_argument, 0));
				return future;
			}

			derive.push_event(
			[&derive, p = derive.selfptr(), id = derive.life_id(), endpoint = std::forward<Endpoint>(endpoint),
				data = derive._data_persistence(s, count), promise = std::move(promise)]
			(event_queue_guard<derived_t> g) mutable
			{
				if (!derive.is_started())
				{
					set_last_error(asio::error::not_connected);
					promise->set_value(std::pair<error_code, std::size_t>(asio::error::not_connected, 0));
					return;
				}

				if (id != derive.life_id())
				{
					set_last_error(asio::error::operation_aborted);
					promise->set_value(std::pair<error_code, std::size_t>(asio::error::operation_aborted, 0));
					return;
				}

				clear_last_error();

				derive._do_send(endpoint, data, [&promise, g = std::move(g)]
				(const error_code& ec, std::size_t bytes_sent) mutable
				{
					promise->set_value(std::pair<error_code, std::size_t>(ec, bytes_sent));
				});
			});

			return future;
		}

		/**
		 * @brief Asynchronous send data,supporting multi data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * use like this : std::string m; async_send(std::move(m)); can reducing memory allocation.
		 * PodType * : async_send("abc");
		 * PodType (&data)[N] : double m[10]; async_send(m);
		 * std::array<PodType, N> : std::array<int,10> m; async_send(m);
		 * std::vector<PodType, Allocator> : std::vector<float> m; async_send(m);
		 * std::basic_string<Elem, Traits, Allocator> : std::string m; async_send(m);
		 * Callback signature : void() or void(std::size_t bytes_sent)
		 */
		template<class Endpoint, class DataT, class Callback>
		inline typename std::enable_if_t<is_callable_v<Callback> &&
			std::is_same_v<detail::remove_cvref_t<Endpoint>, asio::ip::udp::endpoint>, void>
			async_send(Endpoint&& endpoint, DataT&& data, Callback&& fn)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			detail::integer_add_sub_guard asg(derive.io_->pending());

			// We must ensure that there is only one operation to send data
			// at the same time,otherwise may be cause crash.
			derive.push_event(
			[&derive, p = derive.selfptr(), id = derive.life_id(), endpoint = std::forward<Endpoint>(endpoint),
				data = derive._data_persistence(std::forward<DataT>(data)), fn = std::forward<Callback>(fn)]
			(event_queue_guard<derived_t> g) mutable
			{
				if (!derive.is_started())
				{
					set_last_error(asio::error::not_connected);
					callback_helper::call(fn, 0);
					return;
				}

				if (id != derive.life_id())
				{
					set_last_error(asio::error::operation_aborted);
					callback_helper::call(fn, 0);
					return;
				}

				clear_last_error();

				derive._do_send(endpoint, data, [&fn, g = std::move(g)]
				(const error_code&, std::size_t bytes_sent) mutable
				{
					callback_helper::call(fn, bytes_sent);
				});
			});
		}

		/**
		 * @brief Asynchronous send data
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * PodType * : async_send("abc");
		 * Callback signature : void() or void(std::size_t bytes_sent)
		 */
		template<class Endpoint, class Callback, class CharT, class Traits = std::char_traits<CharT>>
		inline typename std::enable_if_t<is_callable_v<Callback> &&
			std::is_same_v<detail::remove_cvref_t<Endpoint>, asio::ip::udp::endpoint> &&
			detail::is_char_v<CharT>, void>
			async_send(Endpoint&& endpoint, CharT * s, Callback&& fn)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			derive.async_send(std::forward<Endpoint>(endpoint),
				s, s ? Traits::length(s) : 0, std::forward<Callback>(fn));
		}

		/**
		 * @brief Asynchronous send data
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * PodType (&data)[N] : double m[10]; async_send(m,5);
		 * Callback signature : void() or void(std::size_t bytes_sent)
		 */
		template<class Endpoint, class Callback, class CharT, class SizeT>
		inline typename std::enable_if_t<is_callable_v<Callback> &&
			std::is_same_v<detail::remove_cvref_t<Endpoint>, asio::ip::udp::endpoint> &&
			std::is_integral_v<detail::remove_cvref_t<SizeT>>, void>
			async_send(Endpoint&& endpoint, CharT * s, SizeT count, Callback&& fn)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			detail::integer_add_sub_guard asg(derive.io_->pending());

			// We must ensure that there is only one operation to send data
			// at the same time,otherwise may be cause crash.

			derive.push_event(
			[&derive, p = derive.selfptr(), id = derive.life_id(), endpoint = std::forward<Endpoint>(endpoint),
				s, data = derive._data_persistence(s, count), fn = std::forward<Callback>(fn)]
			(event_queue_guard<derived_t> g) mutable
			{
				if (!s)
				{
					set_last_error(asio::error::invalid_argument);
					callback_helper::call(fn, 0);
					return;
				}

				if (!derive.is_started())
				{
					set_last_error(asio::error::not_connected);
					callback_helper::call(fn, 0);
					return;
				}

				if (id != derive.life_id())
				{
					set_last_error(asio::error::operation_aborted);
					callback_helper::call(fn, 0);
					return;
				}

				clear_last_error();

				derive._do_send(endpoint, data, [&fn, g = std::move(g)]
				(const error_code&, std::size_t bytes_sent) mutable
				{
					callback_helper::call(fn, bytes_sent);
				});
			});
		}

	public:
		/**
		 * @brief Synchronous send data,supporting multi data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * Note : If this function is called in communication thread, it will degenerates into async_send
		 *        and the return value is 0, you can use asio2::get_last_error() to check whether the
		 *        send is success, if asio2::get_last_error() is equal to asio::error::in_progress, it
		 *        means success, otherwise failed.
		 * use like this : std::string m; send(std::move(m)); can reducing memory allocation.
		 * PodType * : send("abc");
		 * PodType (&data)[N] : double m[10]; send(m);
		 * std::array<PodType, N> : std::array<int,10> m; send(m);
		 * std::vector<PodType, Allocator> : std::vector<float> m; send(m);
		 * std::basic_string<Elem, Traits, Allocator> : std::string m; send(m);
		 */
		template<typename String, typename StrOrInt, class DataT>
		inline typename std::enable_if_t<
			!std::is_same_v<detail::remove_cvref_t<String>, asio::ip::udp::endpoint>, std::size_t>
			send(String&& host, StrOrInt&& port, DataT&& data)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			std::future<std::pair<error_code, std::size_t>> future = derive.async_send(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				std::forward<DataT>(data), asio::use_future);

			// Whether we run on the io_context thread
			if (derive.io_->running_in_this_thread())
			{
				std::future_status status = future.wait_for(std::chrono::nanoseconds(0));

				// async_send failed.
				if (status == std::future_status::ready)
				{
					set_last_error(future.get().first);
					return std::size_t(0);
				}
				// async_send success.
				else
				{
					set_last_error(asio::error::in_progress);
					return std::size_t(0);
				}
			}

			std::pair<error_code, std::size_t> pair = future.get();

			set_last_error(pair.first);

			return pair.second;
		}

		/**
		 * @brief Synchronous send data
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * Note : If this function is called in communication thread, it will degenerates into async_send
		 *        and the return value is 0, you can use asio2::get_last_error() to check whether the
		 *        send is success, if asio2::get_last_error() is equal to asio::error::in_progress, it
		 *        means success, otherwise failed.
		 * PodType * : send("abc");
		 */
		template<typename String, typename StrOrInt, class CharT, class Traits = std::char_traits<CharT>>
		inline typename std::enable_if_t<!std::is_same_v<detail::remove_cvref_t<String>,
			asio::ip::udp::endpoint> && detail::is_char_v<CharT>, std::size_t>
			send(String&& host, StrOrInt&& port, CharT* s)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return derive.send(std::forward<String>(host), std::forward<StrOrInt>(port),
				s, s ? Traits::length(s) : 0);
		}

		/**
		 * @brief Synchronous send data
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * Note : If this function is called in communication thread, it will degenerates into async_send
		 *        and the return value is 0, you can use asio2::get_last_error() to check whether the
		 *        send is success, if asio2::get_last_error() is equal to asio::error::in_progress, it
		 *        means success, otherwise failed.
		 * PodType (&data)[N] : double m[10]; send(m,5);
		 */
		template<typename String, typename StrOrInt, class CharT, class SizeT>
		inline typename std::enable_if_t<std::is_integral_v<detail::remove_cvref_t<SizeT>> &&
			!std::is_same_v<detail::remove_cvref_t<String>, asio::ip::udp::endpoint>, std::size_t>
			send(String&& host, StrOrInt&& port, CharT * s, SizeT count)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return derive.send(std::forward<String>(host), std::forward<StrOrInt>(port),
				derive._data_persistence(s, count));
		}

	public:
		/**
		 * @brief Synchronous send data,supporting multi data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * Note : If this function is called in communication thread, it will degenerates into async_send
		 *        and the return value is 0, you can use asio2::get_last_error() to check whether the
		 *        send is success, if asio2::get_last_error() is equal to asio::error::in_progress, it
		 *        means success, otherwise failed.
		 * use like this : std::string m; send(std::move(m)); can reducing memory allocation.
		 * PodType * : send("abc");
		 * PodType (&data)[N] : double m[10]; send(m);
		 * std::array<PodType, N> : std::array<int,10> m; send(m);
		 * std::vector<PodType, Allocator> : std::vector<float> m; send(m);
		 * std::basic_string<Elem, Traits, Allocator> : std::string m; send(m);
		 */
		template<class Endpoint, class DataT>
		inline typename std::enable_if_t<
			std::is_same_v<detail::remove_cvref_t<Endpoint>, asio::ip::udp::endpoint>, std::size_t>
			send(Endpoint&& endpoint, DataT&& data)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			std::future<std::pair<error_code, std::size_t>> future = derive.async_send(
				std::forward<Endpoint>(endpoint), std::forward<DataT>(data), asio::use_future);

			// Whether we run on the io_context thread
			if (derive.io_->running_in_this_thread())
			{
				std::future_status status = future.wait_for(std::chrono::nanoseconds(0));

				// async_send failed.
				if (status == std::future_status::ready)
				{
					set_last_error(future.get().first);
					return std::size_t(0);
				}
				// async_send success.
				else
				{
					set_last_error(asio::error::in_progress);
					return std::size_t(0);
				}
			}

			std::pair<error_code, std::size_t> pair = future.get();

			set_last_error(pair.first);

			return pair.second;
		}

		/**
		 * @brief Synchronous send data
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * Note : If this function is called in communication thread, it will degenerates into async_send
		 *        and the return value is 0, you can use asio2::get_last_error() to check whether the
		 *        send is success, if asio2::get_last_error() is equal to asio::error::in_progress, it
		 *        means success, otherwise failed.
		 * PodType * : send("abc");
		 */
		template<class Endpoint, class CharT, class Traits = std::char_traits<CharT>>
		inline typename std::enable_if_t<
			std::is_same_v<detail::remove_cvref_t<Endpoint>, asio::ip::udp::endpoint> &&
			detail::is_char_v<CharT>, std::size_t>
			send(Endpoint&& endpoint, CharT * s)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return derive.send(std::forward<Endpoint>(endpoint), s, s ? Traits::length(s) : 0);
		}

		/**
		 * @brief Synchronous send data
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * Note : If this function is called in communication thread, it will degenerates into async_send
		 *        and the return value is 0, you can use asio2::get_last_error() to check whether the
		 *        send is success, if asio2::get_last_error() is equal to asio::error::in_progress, it
		 *        means success, otherwise failed.
		 * PodType (&data)[N] : double m[10]; send(m,5);
		 */
		template<class Endpoint, class CharT, class SizeT>
		inline typename std::enable_if_t<std::is_integral_v<detail::remove_cvref_t<SizeT>> &&
			std::is_same_v<detail::remove_cvref_t<Endpoint>, asio::ip::udp::endpoint>, std::size_t>
			send(Endpoint&& endpoint, CharT * s, SizeT count)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return derive.send(std::forward<Endpoint>(endpoint), derive._data_persistence(s, count));
		}

	protected:
		template<typename String, typename StrOrInt, typename Data, typename Callback>
		inline void _do_resolve(String&& host, StrOrInt&& port, Data&& data, Callback&& callback)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			using resolver_type = asio::ip::udp::resolver;
			using endpoints_type = typename resolver_type::results_type;
			//using endpoints_iterator = typename endpoints_type::iterator;

			std::unique_ptr<resolver_type> resolver_ptr = std::make_unique<resolver_type>(
				derive.io_->context());

			// Before async_resolve execution is complete, we must hold the resolver object.
			// so we captured the resolver_ptr into the lambda callback function.

			resolver_type * rp = resolver_ptr.get();

			rp->async_resolve(std::forward<String>(host), detail::to_string(std::forward<StrOrInt>(port)),
			[&derive, p = derive.selfptr(), resolver_ptr = std::move(resolver_ptr),
				data = std::forward<Data>(data), callback = std::forward<Callback>(callback)]
			(const error_code& ec, const endpoints_type& endpoints) mutable
			{
				set_last_error(ec);

				if (ec)
				{
					callback(ec, 0);
					return;
				}

				decltype(endpoints.size()) i = 1;

				for (auto iter = endpoints.begin(); iter != endpoints.end(); ++iter, ++i)
				{
					derive.push_event(
					[&derive, id = derive.life_id(), endpoint = iter->endpoint(),
						p = (endpoints.size() == i ? std::move(p) : p),
						data = (endpoints.size() == i ? std::move(data) : data),
						callback = (endpoints.size() == i ? std::move(callback) : callback)]
					(event_queue_guard<derived_t> g) mutable
					{
						if (!derive.is_started())
						{
							set_last_error(asio::error::not_connected);

							callback(asio::error::not_connected, 0);

							return;
						}

						if (id != derive.life_id())
						{
							set_last_error(asio::error::operation_aborted);

							callback(asio::error::operation_aborted, 0);

							return;
						}

						clear_last_error();

						derive._do_send(endpoint, data, [g = std::move(g), f = std::move(callback)]
						(const error_code& ec, std::size_t bytes_sent) mutable
						{
							f(ec, bytes_sent);
						});
					});
				}
			});
		}
	};
}

#endif // !__ASIO2_UDP_SEND_COMPONENT_HPP__
