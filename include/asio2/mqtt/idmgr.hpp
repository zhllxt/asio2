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

#ifndef __ASIO2_MQTT_ID_MGR_HPP__
#define __ASIO2_MQTT_ID_MGR_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstdint>
#include <set>
#include <limits>

namespace asio2::mqtt
{
	template <typename Integer = std::uint16_t>
	class idmgr
	{
	public:
		using id_type = std::remove_cv_t<std::remove_reference_t<Integer>>;

		static_assert(std::is_integral_v<id_type>);

		 idmgr() = default;
		~idmgr() = default;

		/**
		 * @function Get a new unique id, return 0 if failed.
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
		 * @function Checks whether contains the given id. 
		 */
		bool contains(id_type id)
		{
			return (used_.find(id) != used_.end());
		}

		/**
		 * @function Release the id.
		 */
		void release(id_type id)
		{
			used_.erase(id);
		}

		/**
		 * @function Clear all ids.
		 */
		void clear()
		{
			used_.clear();
		}

	private:
		std::set<id_type> used_;
	};
}

#endif // !__ASIO2_MQTT_ID_MGR_HPP__
