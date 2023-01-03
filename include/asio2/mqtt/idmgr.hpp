/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * refrenced from : mqtt_cpp/include/mqtt/packet_id_manager.hpp
 */

#ifndef __ASIO2_MQTT_ID_MGR_HPP__
#define __ASIO2_MQTT_ID_MGR_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstdint>
#include <set>
#include <limits>
#include <atomic>

namespace asio2::mqtt
{
	template<typename T>
	class idmgr;

	template<typename Integer>
	class idmgr<std::atomic<Integer>>
	{
	public:
		using id_type = std::remove_cv_t<std::remove_reference_t<Integer>>;

		static_assert(std::is_integral_v<id_type>);

		 idmgr() = default;
		~idmgr() = default;

		/**
		 * @brief Get a new unique id
		 */
		inline id_type get() noexcept
		{
			id_type r = id_.fetch_add(static_cast<id_type>(1));
			while (r == 0)
			{
				r = id_.fetch_add(static_cast<id_type>(1));
			}
			return r;
		}

		/**
		 * @brief Checks whether contains the given id.
		 */
		inline bool contains(id_type id) const noexcept
		{
			return (id_.load() == id);
		}

		/**
		 * @brief Release the id.
		 */
		inline void release(id_type id) noexcept
		{
			std::ignore = id;
		}

		/**
		 * @brief Clear all ids.
		 */
		inline void clear() noexcept
		{
			id_.store(0);
		}

	private:
		std::atomic<id_type> id_{ static_cast<id_type>(1) };
	};

	template<typename Integer>
	class idmgr<std::set<Integer>>
	{
	public:
		using id_type = std::remove_cv_t<std::remove_reference_t<Integer>>;

		static_assert(std::is_integral_v<id_type>);

		 idmgr() = default;
		~idmgr() = default;

		/**
		 * @brief Get a new unique id, return 0 if failed.
		 */
		id_type get()
		{
			if (used_.size() == (std::numeric_limits<id_type>::max)())
				return static_cast<id_type>(0);

			id_type id;

			if (used_.empty())
			{
				id = static_cast<id_type>(1);
			}
			else
			{
				id = *(used_.rbegin());

				for (;;)
				{
					id++;

					if (id == static_cast<id_type>(0))
						id++;

					if (used_.find(id) == used_.end())
						break;
				}
			}

			used_.emplace(id);

			return id;
		}

		/**
		 * @brief Checks whether contains the given id. 
		 */
		bool contains(id_type id)
		{
			return (used_.find(id) != used_.end());
		}

		/**
		 * @brief Release the id.
		 */
		void release(id_type id)
		{
			used_.erase(id);
		}

		/**
		 * @brief Clear all ids.
		 */
		void clear()
		{
			used_.clear();
		}

	private:
		std::set<id_type> used_{};
	};
}

#endif // !__ASIO2_MQTT_ID_MGR_HPP__
