/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_MQTT_SERVER_HPP__
#define __ASIO2_MQTT_SERVER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <asio2/tcp/tcp_server.hpp>
#include <asio2/mqtt/mqtt_session.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;

	template<class derived_t, class session_t>
	class mqtt_server_impl_t
		: public tcp_server_impl_t<derived_t, session_t>
		, public mqtt_options
		, public mqtt_invoker_t   <session_t, typename session_t::args_type>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;

	public:
		using super = tcp_server_impl_t <derived_t, session_t>;
		using self  = mqtt_server_impl_t<derived_t, session_t>;

		using session_type = session_t;

		using super::async_send;

	public:
		/**
		 * @brief constructor
		 */
		explicit mqtt_server_impl_t(
			std::size_t init_buf_size = tcp_frame_size,
			std::size_t max_buf_size  = mqtt::max_payload,
			std::size_t concurrency   = default_concurrency() + 1 // The 1 is used for tcp acceptor
		)
			: super(init_buf_size, max_buf_size, concurrency)
			, mqtt_options()
			, mqtt_invoker_t<session_t, typename session_t::args_type>()
			, broker_state_(*this, *this)
		{
		}

		template<class Scheduler, std::enable_if_t<!std::is_integral_v<detail::remove_cvref_t<Scheduler>>, int> = 0>
		explicit mqtt_server_impl_t(
			std::size_t init_buf_size,
			std::size_t max_buf_size,
			Scheduler&& scheduler
		)
			: super(init_buf_size, max_buf_size, std::forward<Scheduler>(scheduler))
			, mqtt_options()
			, mqtt_invoker_t<session_t, typename session_t::args_type>()
			, broker_state_(*this, *this)
		{
		}

		template<class Scheduler, std::enable_if_t<!std::is_integral_v<detail::remove_cvref_t<Scheduler>>, int> = 0>
		explicit mqtt_server_impl_t(Scheduler&& scheduler)
			: mqtt_server_impl_t(tcp_frame_size, mqtt::max_payload, std::forward<Scheduler>(scheduler))
		{
		}

		/**
		 * @brief destructor
		 */
		~mqtt_server_impl_t()
		{
			this->stop();
		}

		/**
		 * @brief start the server
		 * @param host - A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param service - A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt, typename... Args>
		bool start(String&& host, StrOrInt&& service, Args&&... args)
		{
			return this->derived()._do_start(
				std::forward<String>(host), std::forward<StrOrInt>(service),
				ecs_helper::make_ecs(asio::transfer_at_least(1),
					mqtt::mqtt_match_role, std::forward<Args>(args)...));
		}

	protected:
		template<typename C>
		inline void _handle_start(
			error_code ec, std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			this->derived()._bind_default_mqtt_handler(ecs);

			return super::_handle_start(std::move(ec), std::move(this_ptr), std::move(ecs));
		}

		inline void _post_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr, state_t old_state)
		{
			asio::dispatch(this->derived().io_->context(), make_allocator(this->derived().wallocator(),
			[this, this_ptr]() mutable
			{
				this->mqtt_sessions().clear_mqtt_sessions();
			}));

			super::_post_stop(ec, std::move(this_ptr), old_state);
		}

		template<typename C>
		inline void _bind_default_mqtt_handler(std::shared_ptr<ecs_t<C>>& ecs)
		{
			detail::ignore_unused(ecs);

			// must set default callback for every mqtt message.
			if (!(this->_find_mqtt_handler(mqtt::control_packet_type::connect    ))) this->on_connect    ([](std::shared_ptr<session_t>&, mqtt::message&, mqtt::message&) mutable {});
			if (!(this->_find_mqtt_handler(mqtt::control_packet_type::connack    ))) this->on_connack    ([](std::shared_ptr<session_t>&, mqtt::message&                ) mutable {});
			if (!(this->_find_mqtt_handler(mqtt::control_packet_type::publish    ))) this->on_publish    ([](std::shared_ptr<session_t>&, mqtt::message&, mqtt::message&) mutable {});
			if (!(this->_find_mqtt_handler(mqtt::control_packet_type::puback     ))) this->on_puback     ([](std::shared_ptr<session_t>&, mqtt::message&                ) mutable {});
			if (!(this->_find_mqtt_handler(mqtt::control_packet_type::pubrec     ))) this->on_pubrec     ([](std::shared_ptr<session_t>&, mqtt::message&, mqtt::message&) mutable {});
			if (!(this->_find_mqtt_handler(mqtt::control_packet_type::pubrel     ))) this->on_pubrel     ([](std::shared_ptr<session_t>&, mqtt::message&, mqtt::message&) mutable {});
			if (!(this->_find_mqtt_handler(mqtt::control_packet_type::pubcomp    ))) this->on_pubcomp    ([](std::shared_ptr<session_t>&, mqtt::message&                ) mutable {});
			if (!(this->_find_mqtt_handler(mqtt::control_packet_type::subscribe  ))) this->on_subscribe  ([](std::shared_ptr<session_t>&, mqtt::message&, mqtt::message&) mutable {});
			if (!(this->_find_mqtt_handler(mqtt::control_packet_type::suback     ))) this->on_suback     ([](std::shared_ptr<session_t>&, mqtt::message&                ) mutable {});
			if (!(this->_find_mqtt_handler(mqtt::control_packet_type::unsubscribe))) this->on_unsubscribe([](std::shared_ptr<session_t>&, mqtt::message&, mqtt::message&) mutable {});
			if (!(this->_find_mqtt_handler(mqtt::control_packet_type::unsuback   ))) this->on_unsuback   ([](std::shared_ptr<session_t>&, mqtt::message&                ) mutable {});
			if (!(this->_find_mqtt_handler(mqtt::control_packet_type::pingreq    ))) this->on_pingreq    ([](std::shared_ptr<session_t>&, mqtt::message&, mqtt::message&) mutable {});
			if (!(this->_find_mqtt_handler(mqtt::control_packet_type::pingresp   ))) this->on_pingresp   ([](std::shared_ptr<session_t>&, mqtt::message&                ) mutable {});
			if (!(this->_find_mqtt_handler(mqtt::control_packet_type::disconnect ))) this->on_disconnect ([](std::shared_ptr<session_t>&, mqtt::message&                ) mutable {});
			if (!(this->_find_mqtt_handler(mqtt::control_packet_type::auth       ))) this->on_auth       ([](std::shared_ptr<session_t>&, mqtt::message&, mqtt::message&) mutable {});
		}

		template<typename... Args>
		inline std::shared_ptr<session_t> _make_session(Args&&... args)
		{
			std::shared_ptr<session_t> p = super::_make_session(std::forward<Args>(args)..., this->broker_state_);
			// Copy the parameter configuration of user calls for the "server" to each "session"
			p->_mqtt_options_copy_from(*this);
			return p;
		}

		inline auto& invoker          () noexcept { return this->broker_state_.invoker_          ; }
		inline auto& mqtt_sessions    () noexcept { return this->broker_state_.mqtt_sessions_    ; }
		inline auto& subs_map         () noexcept { return this->broker_state_.subs_map_         ; }
		inline auto& shared_targets   () noexcept { return this->broker_state_.shared_targets_   ; }
		inline auto& retained_messages() noexcept { return this->broker_state_.retained_messages_; }

		inline auto const& invoker          () const noexcept { return this->broker_state_.invoker_          ; }
		inline auto const& mqtt_sessions    () const noexcept { return this->broker_state_.mqtt_sessions_    ; }
		inline auto const& subs_map         () const noexcept { return this->broker_state_.subs_map_         ; }
		inline auto const& shared_targets   () const noexcept { return this->broker_state_.shared_targets_   ; }
		inline auto const& retained_messages() const noexcept { return this->broker_state_.retained_messages_; }

	public:
		inline auto& security         () noexcept { return this->broker_state_.security_; }
		inline auto& get_security     () noexcept { return this->broker_state_.security_; }

		inline auto const& security         () const noexcept { return this->broker_state_.security_; }
		inline auto const& get_security     () const noexcept { return this->broker_state_.security_; }

	protected:
		/// 
		mqtt::broker_state<session_t, typename session_t::args_type>  broker_state_;
	};
}

namespace asio2
{
	template<class derived_t, class session_t>
	using mqtt_server_impl_t = detail::mqtt_server_impl_t<derived_t, session_t>;

	template<class session_t>
	class mqtt_server_t : public detail::mqtt_server_impl_t<mqtt_server_t<session_t>, session_t>
	{
	public:
		using detail::mqtt_server_impl_t<mqtt_server_t<session_t>, session_t>::mqtt_server_impl_t;
	};

	using mqtt_server = mqtt_server_t<mqtt_session>;
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_MQTT_SERVER_HPP__
