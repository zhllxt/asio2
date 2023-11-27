/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * https://www.zhihu.com/question/25016042
 * Windows API : send 
 * The successful completion of a send function does not indicate that the data 
 * was successfully delivered and received to the recipient. This function only 
 * indicates the data was successfully sent.
 * 
 */

#ifndef __ASIO2_SEND_COMPONENT_HPP__
#define __ASIO2_SEND_COMPONENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstdint>
#include <memory>
#include <functional>
#include <string>
#include <future>
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
	class send_cp : public data_persistence_cp<derived_t, args_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;

	public:
		/**
		 * @brief constructor
		 */
		send_cp() noexcept {}

		/**
		 * @brief destructor
		 */
		~send_cp() = default;

	public:
		/**
		 * @brief Asynchronous send data, support multiple data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * use like this : std::string m; async_send(std::move(m)); can reducing memory allocation.
		 * PodType * : async_send("abc");
		 * PodType (&data)[N] : double m[10]; async_send(m);
		 * std::array<PodType, N> : std::array<int,10> m; async_send(m);
		 * std::vector<PodType, Allocator> : std::vector<float> m; async_send(m);
		 * std::basic_string<Elem, Traits, Allocator> : std::string m; async_send(m);
		 */
		template<class DataT>
		inline void async_send(DataT&& data) noexcept
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// use this guard to fix the issue of below "# issue x:"
			detail::integer_add_sub_guard asg(derive.io_->pending());

			// We must ensure that there is only one operation to send data
			// at the same time,otherwise may be cause crash.

			// # issue x:
			// 1. user call "async_send" function in some worker thread (not io_context thread)
			// 2. code run to here, "here" means between "if (!derive.is_started())" and
			//    "derive.push_event("
			// 3. the operating system switched from this thread to other thread
			// 4. user call "client.stop", and the client is stopped successed
			// 5. the operating system switched from other thread to this thread
			// 6. code run to below, it means code run to "derive.push_event("
			// 7. beacuse the iopool is stopped alreadly, so the "derive.push_event(" and
			//    "derive._do_send(..." will can't be executed.

			derive.push_event(
			[&derive, p = derive.selfptr(), id = derive.life_id(),
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

				derive._do_send(data, [g = std::move(g)](const error_code&, std::size_t) mutable {});
			});
		}

		/**
		 * @brief Asynchronous send data
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * PodType * : async_send("abc");
		 */
		template<class CharT, class Traits = std::char_traits<CharT>>
		inline typename std::enable_if_t<detail::is_char_v<CharT>, void> async_send(CharT * s) noexcept
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			derive.async_send(s, s ? Traits::length(s) : 0);
		}

		/**
		 * @brief Asynchronous send data
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * PodType (&data)[N] : double m[10]; async_send(m,5);
		 */
		template<class CharT, class SizeT>
		inline typename std::enable_if_t<std::is_integral_v<detail::remove_cvref_t<SizeT>>, void>
			async_send(CharT* s, SizeT count) noexcept
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
			[&derive, p = derive.selfptr(), id = derive.life_id(), data = derive._data_persistence(s, count)]
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

				derive._do_send(data, [g = std::move(g)](const error_code&, std::size_t) mutable {});
			});
		}

		/**
		 * @brief Asynchronous send data, support multiple data formats,
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
		template<class DataT>
		inline std::future<std::pair<error_code, std::size_t>> async_send(DataT&& data, asio::use_future_t<>)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// why use copyable_wrapper? beacuse std::promise is moveable-only, but
			// std::function need copy-constructible.

			// 20220211 : 
			// Due to the event_queue_cp uses a custom detail::function that supports moveable-only instead
			// of std::function, so copyable_wrapper is no longer required.

			// https://en.cppreference.com/w/cpp/utility/functional/function/function
			// The program is ill-formed if the target type is not copy-constructible or
			// initialization of the target is ill-formed.

			std::promise<std::pair<error_code, std::size_t>> promise;
			std::future<std::pair<error_code, std::size_t>> future = promise.get_future();

			detail::integer_add_sub_guard asg(derive.io_->pending());

			derive.push_event(
			[&derive, p = derive.selfptr(), id = derive.life_id(), promise = std::move(promise),
				data = derive._data_persistence(std::forward<DataT>(data))]
			(event_queue_guard<derived_t> g) mutable
			{
				if (!derive.is_started())
				{
					set_last_error(asio::error::not_connected);
					promise.set_value(std::pair<error_code, std::size_t>(asio::error::not_connected, 0));
					return;
				}

				if (id != derive.life_id())
				{
					set_last_error(asio::error::operation_aborted);
					promise.set_value(std::pair<error_code, std::size_t>(asio::error::operation_aborted, 0));
					return;
				}

				clear_last_error();

				derive._do_send(data, [&promise, g = std::move(g)]
				(const error_code& ec, std::size_t bytes_sent) mutable
				{
					promise.set_value(std::pair<error_code, std::size_t>(ec, bytes_sent));
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
		template<class CharT, class Traits = std::char_traits<CharT>>
		inline typename std::enable_if_t<detail::is_char_v<CharT>, std::future<std::pair<error_code, std::size_t>>>
			async_send(CharT * s, asio::use_future_t<> flag)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return derive.async_send(s, s ? Traits::length(s) : 0, std::move(flag));
		}

		/**
		 * @brief Asynchronous send data
		 * the pair.first save the send result error_code,the pair.second save the sent_bytes.
		 * note : Do not call this function in any listener callback function like this:
		 * auto future = async_send(msg,asio::use_future); future.get(); it will cause deadlock,
		 * the future.get() will never return.
		 * PodType (&data)[N] : double m[10]; async_send(m,5);
		 */
		template<class CharT, class SizeT>
		inline typename std::enable_if_t<std::is_integral_v<detail::remove_cvref_t<SizeT>>,
			std::future<std::pair<error_code, std::size_t>>>
			async_send(CharT * s, SizeT count, asio::use_future_t<>)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			std::promise<std::pair<error_code, std::size_t>> promise;
			std::future<std::pair<error_code, std::size_t>> future = promise.get_future();

			if (!s)
			{
				set_last_error(asio::error::invalid_argument);
				promise.set_value(std::pair<error_code, std::size_t>(asio::error::invalid_argument, 0));
				return future;
			}

			detail::integer_add_sub_guard asg(derive.io_->pending());

			derive.push_event(
			[&derive, p = derive.selfptr(), id = derive.life_id(), promise = std::move(promise),
				data = derive._data_persistence(s, count)]
			(event_queue_guard<derived_t> g) mutable
			{
				if (!derive.is_started())
				{
					set_last_error(asio::error::not_connected);
					promise.set_value(std::pair<error_code, std::size_t>(asio::error::not_connected, 0));
					return;
				}

				if (id != derive.life_id())
				{
					set_last_error(asio::error::operation_aborted);
					promise.set_value(std::pair<error_code, std::size_t>(asio::error::operation_aborted, 0));
					return;
				}

				clear_last_error();

				derive._do_send(data, [&promise, g = std::move(g)]
				(const error_code& ec, std::size_t bytes_sent) mutable
				{
					promise.set_value(std::pair<error_code, std::size_t>(ec, bytes_sent));
				});
			});

			return future;
		}

		/**
		 * @brief Asynchronous send data, support multiple data formats,
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
		template<class DataT, class Callback>
		inline typename std::enable_if_t<is_callable_v<Callback>, void> async_send(DataT&& data, Callback&& fn)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			detail::integer_add_sub_guard asg(derive.io_->pending());

			// We must ensure that there is only one operation to send data
			// at the same time,otherwise may be cause crash.

			derive.push_event(
			[&derive, p = derive.selfptr(), id = derive.life_id(), fn = std::forward<Callback>(fn),
				data = derive._data_persistence(std::forward<DataT>(data))]
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

				derive._do_send(data, [&fn, g = std::move(g)]
				(const error_code&, std::size_t bytes_sent) mutable
				{
					ASIO2_ASSERT(!g.is_empty());
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
		template<class Callback, class CharT, class Traits = std::char_traits<CharT>>
		inline typename std::enable_if_t<is_callable_v<Callback> && detail::is_char_v<CharT>, void>
			async_send(CharT * s, Callback&& fn)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			derive.async_send(s, s ? Traits::length(s) : 0, std::forward<Callback>(fn));
		}

		/**
		 * @brief Asynchronous send data
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * PodType (&data)[N] : double m[10]; async_send(m,5);
		 * Callback signature : void() or void(std::size_t bytes_sent)
		 */
		template<class Callback, class CharT, class SizeT>
		inline typename std::enable_if_t<is_callable_v<Callback> &&
			std::is_integral_v<detail::remove_cvref_t<SizeT>>, void>
			async_send(CharT * s, SizeT count, Callback&& fn)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			detail::integer_add_sub_guard asg(derive.io_->pending());

			// We must ensure that there is only one operation to send data
			// at the same time,otherwise may be cause crash.

			derive.push_event(
			[&derive, p = derive.selfptr(), id = derive.life_id(), fn = std::forward<Callback>(fn),
				s, data = derive._data_persistence(s, count)]
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

				derive._do_send(data, [&fn, g = std::move(g)]
				(const error_code&, std::size_t bytes_sent) mutable
				{
					callback_helper::call(fn, bytes_sent);
				});
			});
		}

	public:
		/**
		 * @brief Synchronous send data, support multiple data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * @return Number of bytes written from the data. If an error occurred,
		 *           this will be less than the sum of the data size.
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
		template<class DataT>
		inline std::size_t send(DataT&& data)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			std::future<std::pair<error_code, std::size_t>> future = derive.async_send(
				std::forward<DataT>(data), asio::use_future);

			// Whether we run on the io_context thread
			if (derive.io_->running_in_this_thread())
			{
				std::future_status status = future.wait_for(std::chrono::nanoseconds(0));

				// async_send failed :
				// beacuse current thread is the io_context thread, so the data must be sent in the future,
				// so if the "status" is ready, it means that the data must have failed to be sent.
				if (status == std::future_status::ready)
				{
					set_last_error(future.get().first);
					return std::size_t(0);
				}
				// async_send in_progress.
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
		 * @return Number of bytes written from the data. If an error occurred,
		 *           this will be less than the sum of the data size.
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * Note : If this function is called in communication thread, it will degenerates into async_send
		 *        and the return value is 0, you can use asio2::get_last_error() to check whether the
		 *        send is success, if asio2::get_last_error() is equal to asio::error::in_progress, it
		 *        means success, otherwise failed.
		 * PodType * : send("abc");
		 */
		template<class CharT, class Traits = std::char_traits<CharT>>
		inline typename std::enable_if_t<detail::is_char_v<CharT>, std::size_t> send(CharT * s)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return derive.send(s, s ? Traits::length(s) : 0);
		}

		/**
		 * @brief Synchronous send data
		 * @return Number of bytes written from the data. If an error occurred,
		 *           this will be less than the sum of the data size.
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * Note : If this function is called in communication thread, it will degenerates into async_send
		 *        and the return value is 0, you can use asio2::get_last_error() to check whether the
		 *        send is success, if asio2::get_last_error() is equal to asio::error::in_progress, it
		 *        means success, otherwise failed.
		 * PodType (&data)[N] : double m[10]; send(m,5);
		 */
		template<class CharT, class SizeT>
		inline typename std::enable_if_t<std::is_integral_v<detail::remove_cvref_t<SizeT>>, std::size_t>
			send(CharT* s, SizeT count)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return derive.send(derive._data_persistence(s, count));
		}

	protected:
		/**
		 * @brief Asynchronous send data, support multiple data formats,
		 *             see asio::buffer(...) in /asio/buffer.hpp
		 * You can call this function on the communication thread and anywhere,it's multi thread safed.
		 * use like this : std::string m; async_send(std::move(m)); can reducing memory allocation.
		 * PodType * : async_send("abc");
		 * PodType (&data)[N] : double m[10]; async_send(m);
		 * std::array<PodType, N> : std::array<int,10> m; async_send(m);
		 * std::vector<PodType, Allocator> : std::vector<float> m; async_send(m);
		 * std::basic_string<Elem, Traits, Allocator> : std::string m; async_send(m);
		 */
		template<class DataT>
		inline void internal_async_send(std::shared_ptr<derived_t> this_ptr, DataT&& data) noexcept
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(derive.io_->running_in_this_thread());

			derive.push_event(
			[&derive, p = std::move(this_ptr), id = derive.life_id(),
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

				derive._do_send(data, [g = std::move(g), p = std::move(p)](const error_code&, std::size_t) mutable
				{
					{
						[[maybe_unused]] auto t{ std::move(g) };
					}
				});
			});
		}

		/**
		 * @brief Asynchronous send data, support multiple data formats,
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
		template<class DataT, class Callback>
		inline typename std::enable_if_t<is_callable_v<Callback>, void> internal_async_send(
			std::shared_ptr<derived_t> this_ptr, DataT&& data, Callback&& fn)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(derive.io_->running_in_this_thread());

			derive.push_event(
			[&derive, p = std::move(this_ptr), id = derive.life_id(), fn = std::forward<Callback>(fn),
				data = derive._data_persistence(std::forward<DataT>(data))]
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

				derive._do_send(data, [&fn, g = std::move(g), p = std::move(p)]
				(const error_code&, std::size_t bytes_sent) mutable
				{
					ASIO2_ASSERT(!g.is_empty());
					callback_helper::call(fn, bytes_sent);
					{
						[[maybe_unused]] auto t{ std::move(g) };
					}
				});
			});
		}

		/**
		 * @brief Asynchronous send data, support multiple data formats,
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
		template<class DataT, class Callback>
		inline void internal_async_send(
			std::shared_ptr<derived_t> this_ptr, DataT&& data, Callback&& fn, event_queue_guard<derived_t> g)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(derive.io_->running_in_this_thread());

			derive.disp_event(
			[&derive, p = std::move(this_ptr), id = derive.life_id(), fn = std::forward<Callback>(fn),
				data = derive._data_persistence(std::forward<DataT>(data))]
			(event_queue_guard<derived_t> g) mutable
			{
				if (!derive.is_started())
				{
					set_last_error(asio::error::not_connected);
					fn(std::move(p), asio::error::not_connected, 0, std::move(g));
					return;
				}

				if (id != derive.life_id())
				{
					set_last_error(asio::error::operation_aborted);
					fn(std::move(p), asio::error::operation_aborted, 0, std::move(g));
					return;
				}

				clear_last_error();

				derive._do_send(data, [fn = std::move(fn), p = std::move(p), g = std::move(g)]
				(const error_code& ec, std::size_t bytes_sent) mutable
				{
					fn(std::move(p), ec, bytes_sent, std::move(g));
				});
			}, std::move(g));
		}
	};
}

#endif // !__ASIO2_SEND_COMPONENT_HPP__
