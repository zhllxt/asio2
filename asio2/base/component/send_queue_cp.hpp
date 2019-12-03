/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_SEND_QUEUE_COMPONENT_HPP__
#define __ASIO2_SEND_QUEUE_COMPONENT_HPP__

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

namespace asio2::detail
{
	template<class derived_t, bool isSession>
	class send_queue_cp
	{
	public:
		/**
		 * @constructor
		 */
		send_queue_cp(io_t & wio) : derive(static_cast<derived_t&>(*this)), wio_(wio) {}

		/**
		 * @destructor
		 */
		~send_queue_cp() = default;

	protected:
		template<class Callback>
		inline bool _send_enqueue(Callback&& f)
		{
#if defined(ASIO2_SEND_CORE_ASYNC)
			// Make sure we run on the strand
			if (this->wio_.strand().running_in_this_thread())
			{
				bool empty = this->send_queue_.empty();
				this->send_queue_.emplace(std::forward<Callback>(f));
				if (empty)
				{
					return (this->send_queue_.front())();
				}
				return true;
			}

			asio::post(this->wio_.strand(), make_allocator(derive.wallocator(),
				[this, p = derive.selfptr(), f = std::forward<Callback>(f)]() mutable
			{
				bool empty = this->send_queue_.empty();
				this->send_queue_.emplace(std::move(f));
				if (empty)
				{
					(this->send_queue_.front())();
				}
			}));

			return true;
#else
			// Make sure we run on the strand
			if (this->wio_.strand().running_in_this_thread())
			{
				return f();
			}

			asio::post(this->wio_.strand(), make_allocator(derive.wallocator(),
				[this, p = derive.selfptr(), f = std::forward<Callback>(f)]() mutable
			{
				f();
			}));

			return true;
#endif
		}

		template<typename = void>
		inline void _send_dequeue()
		{
#if defined(ASIO2_SEND_CORE_ASYNC)
			ASIO2_ASSERT(this->wio_.strand().running_in_this_thread());

			if (!this->send_queue_.empty())
			{
				this->send_queue_.pop();

				if (!this->send_queue_.empty())
				{
					(this->send_queue_.front())();
				}
			}
#else
#endif
		}

		template<class T>
		inline auto _data_persistence(T&& data)
		{
			using data_type = std::remove_cv_t<std::remove_reference_t<T>>;
			if constexpr (
				std::is_move_assignable_v<data_type> ||
				std::is_trivially_move_assignable_v<data_type> ||
				std::is_nothrow_move_assignable_v<data_type>)
			{
				if constexpr (is_string_view_v<data_type>)
				{
					using value_type = typename data_type::value_type;
					return std::basic_string<value_type>(data.data(), data.size());
				}
				else
				{
					return std::forward<T>(data);
				}
			}
			else
			{
				auto buffer = asio::buffer(data);
				return std::string(reinterpret_cast<const std::string::value_type*>(
					const_cast<const void*>(buffer.data())), buffer.size());
			}
		}

		template<class CharT, class SizeT>
		inline auto _data_persistence(CharT * s, SizeT count)
		{
			using value_type = typename std::remove_cv_t<std::remove_reference_t<CharT>>;
			return std::basic_string<value_type>(s, count);
		}

		template<typename = void>
		inline auto _data_persistence(asio::const_buffer& data)
		{
			return derive._data_persistence(asio::const_buffer(data));
		}

		template<typename = void>
		inline auto _data_persistence(const asio::const_buffer& data)
		{
			return derive._data_persistence(asio::const_buffer(data));
		}

		template<typename = void>
		inline auto _data_persistence(asio::const_buffer&& data)
		{
			return std::move(data);
		}

		template<typename = void>
		inline auto _data_persistence(asio::mutable_buffer& data)
		{
			return derive._data_persistence(asio::const_buffer(data));
		}

		template<typename = void>
		inline auto _data_persistence(const asio::mutable_buffer& data)
		{
			return derive._data_persistence(asio::const_buffer(data));
		}

		template<typename = void>
		inline auto _data_persistence(asio::mutable_buffer&& data)
		{
			return derive._data_persistence(asio::const_buffer(std::move(data)));
		}

#if !defined(ASIO_NO_DEPRECATED) && !defined(BOOST_ASIO_NO_DEPRECATED)
		template<typename = void>
		inline auto _data_persistence(asio::const_buffers_1& data)
		{
			return derive._data_persistence(asio::const_buffer(data));
		}

		template<typename = void>
		inline auto _data_persistence(const asio::const_buffers_1& data)
		{
			return derive._data_persistence(asio::const_buffer(data));
		}

		template<typename = void>
		inline auto _data_persistence(asio::const_buffers_1&& data)
		{
			return derive._data_persistence(asio::const_buffer(std::move(data)));
		}

		template<typename = void>
		inline auto _data_persistence(asio::mutable_buffers_1& data)
		{
			return derive._data_persistence(asio::const_buffer(data));
		}

		template<typename = void>
		inline auto _data_persistence(const asio::mutable_buffers_1& data)
		{
			return derive._data_persistence(asio::const_buffer(data));
		}

		template<typename = void>
		inline auto _data_persistence(asio::mutable_buffers_1&& data)
		{
			return derive._data_persistence(asio::const_buffer(std::move(data)));
		}
#endif

	protected:
		derived_t                        & derive;

		io_t                             & wio_;

#if defined(ASIO2_SEND_CORE_ASYNC)
		/// data queue to be sent, used for async_write 
		std::queue<std::function<bool()>>  send_queue_;
#endif
	};
}

#endif // !__ASIO2_SEND_QUEUE_COMPONENT_HPP__
