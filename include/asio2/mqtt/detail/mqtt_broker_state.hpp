/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_MQTT_BROKER_STATE_HPP__
#define __ASIO2_MQTT_BROKER_STATE_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/shared_mutex.hpp>

#include <asio2/mqtt/options.hpp>

#include <asio2/mqtt/detail/mqtt_invoker.hpp>
#include <asio2/mqtt/detail/mqtt_subscription_map.hpp>
#include <asio2/mqtt/detail/mqtt_shared_target.hpp>
#include <asio2/mqtt/detail/mqtt_retained_message.hpp>
#include <asio2/mqtt/detail/mqtt_security.hpp>
#include <asio2/mqtt/detail/mqtt_session_persistence.hpp>

namespace asio2::mqtt
{
	template<class session_t, class args_t>
	struct broker_state
	{
		using session_type = session_t;
		using subnode_type = typename session_type::subnode_type;

		broker_state(
			asio2::detail::mqtt_options& options,
			asio2::detail::mqtt_invoker_t<session_t, args_t>& invoker
		)
			: options_(options)
			, invoker_(invoker)
		{
			security_.default_config();
		}

		asio2::detail::mqtt_options                                      & options_;

		asio2::detail::mqtt_invoker_t<session_t, args_t>                 & invoker_;

		/// client id map
		asio2::detail::mqtt_session_persistence<session_t, args_t>         mqtt_sessions_;

		/// subscription information map
		mqtt::subscription_map<std::string_view, subnode_type>             subs_map_;

		/// shared subscription targets
		mqtt::shared_target<mqtt::stnode<session_t>>                       shared_targets_;

		/// A list of messages retained so they can be sent to newly subscribed clients.
		mqtt::retained_messages<mqtt::rmnode>                              retained_messages_;

		// Authorization and authentication settings
		mqtt::security                                                     security_;
	};
}

#endif // !__ASIO2_MQTT_BROKER_STATE_HPP__
