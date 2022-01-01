/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 *
 * refrenced from : mqtt_cpp/include/mqtt/packet_id_manager.hpp
 */

#ifndef __ASIO2_MQTT_PACKET_ID_MGR_HPP__
#define __ASIO2_MQTT_PACKET_ID_MGR_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstdint>
#include <set>
#include <limits>

namespace asio2::mqtt
{
	template <typename Integer = std::uint16_t>
	class packet_id_mgr
	{
	public:
		using packet_id_t = std::remove_cv_t<std::remove_reference_t<Integer>>;

		 packet_id_mgr() = default;
		~packet_id_mgr() = default;

		/**
		 * @brief Acquire the new unique packet id.
		 *        If all packet ids are already in use, then returns 0
		 *        After acquiring the packet id, you can call acquired_* functions.
		 *        The ownership of packet id is moved to the library.
		 *        Or you can call release_id to release it.
		 * @return packet id, 0 if failed.
		 */
		packet_id_t acquire_id()
		{
			if (used_.size() == (std::numeric_limits<packet_id_t>::max)())
				return static_cast<packet_id_t>(0);

			packet_id_t id;

			if (used_.empty())
			{
				id = static_cast<packet_id_t>(1);
			}
			else
			{
				id = *(used_.rbegin());

				for (;;)
				{
					id++;

					if (id == static_cast<packet_id_t>(0))
						id++;

					if (used_.find(id) == used_.end())
						break;
				}
			}

			used_.emplace(id);

			return id;
		}

		/**
		 * @brief Register packet_id to the library.
		 *        After registering the packet_id, you can call acquired_* functions.
		 *        The ownership of packet id is moved to the library.
		 *        Or you can call release_id to release it.
		 * @return If packet_id is successfully registerd then return true, otherwise return false.
		 */
		bool acquired(packet_id_t packet_id)
		{
			return (used_.find(packet_id) != used_.end());
		}

		/**
		 * @brief Release packet_id.
		 * @param packet_id packet id to release.
		 *                   only the packet_id gotten by acquire_unique_packet_id, or
		 *                   register_packet_id is permitted.
		 */
		void release_id(packet_id_t packet_id)
		{
			used_.erase(packet_id);
		}

		/**
		 * @brief Clear all packet ids.
		 */
		void clear()
		{
			used_.clear();
		}

	private:
		std::set<packet_id_t> used_;
	};
}

#endif // !__ASIO2_MQTT_PACKET_ID_MGR_HPP__
