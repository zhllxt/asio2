/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_MQTT_OPTIONS_HPP__
#define __ASIO2_MQTT_OPTIONS_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/util.hpp>

#include <asio2/mqtt/message.hpp>

namespace asio2::detail
{
	class mqtt_options
	{
	public:
		using self = mqtt_options;

		/**
		 * @brief constructor
		 */
		mqtt_options()
		{
			_receive_maximum     = static_cast<decltype(_receive_maximum    )>(65535); // broker.hivemq.com is 10

			_topic_alias_maximum = static_cast<decltype(_topic_alias_maximum)>(65535); // broker.hivemq.com is 5

			_maximum_qos         = static_cast<decltype(_maximum_qos        )>(mqtt::qos_type::exactly_once);

			_retain_available    = static_cast<decltype(_retain_available   )>(1);
		}

		/**
		 * @brief destructor
		 */
		~mqtt_options() = default;

		/**
		 * set the mqtt options
		 */
		inline self& set_mqtt_options(const mqtt_options& options)
		{
			this->_mqtt_options_copy_from(options);
			return (*this);
		}

		// The Client uses this value to limit the number of QoS 1 and QoS 2 publications that it is willing to 
		// process concurrently. There is no mechanism to limit the QoS 0 publications that the Server might try to send.
		// The value of Receive Maximum applies only to the current Network Connection. 
		// If the Receive Maximum value is absent then its value defaults to 65,535.
		template<class Integer>
		inline self& receive_maximum(Integer v)
		{
			static_assert(std::is_integral_v<detail::remove_cvref_t<Integer>>);
			_receive_maximum = static_cast<decltype(_receive_maximum)>(v);
			return (*this);
		}

		inline auto receive_maximum() const { return _receive_maximum; }


		// This value indicates the highest value that the Client will accept as a Topic Alias sent by the Server.
		// The Client uses this value to limit the number of Topic Aliases that it is willing to hold on this Connection.
		// The Server MUST NOT send a Topic Alias in a PUBLISH packet to the Client greater than Topic Alias Maximum [MQTT-3.1.2-26].
		// A value of 0 indicates that the Client does not accept any Topic Aliases on this connection.
		// If Topic Alias Maximum is absent or zero, the Server MUST NOT send any Topic Aliases to the Client [MQTT-3.1.2-27].
		template<class Integer>
		inline self& topic_alias_maximum(Integer v)
		{
			static_assert(std::is_integral_v<detail::remove_cvref_t<Integer>>);
			_topic_alias_maximum = static_cast<decltype(_topic_alias_maximum)>(v);
			return (*this);
		}

		inline auto topic_alias_maximum() const { return _topic_alias_maximum; }


		// Followed by a Byte with a value of either 0 or 1. 
		// It is a Protocol Error to include Maximum QoS more than once, or to have a value other than 0 or 1.
		// If the Maximum QoS is absent, the Client uses a Maximum QoS of 2.
		template<class Integer>
		inline self& maximum_qos(Integer v)
		{
			static_assert(std::is_integral_v<detail::remove_cvref_t<Integer>>);
			ASIO2_ASSERT(v >= 0 && v <= 2);
			_maximum_qos = static_cast<decltype(_maximum_qos)>(v);
			return (*this);
		}

		inline auto maximum_qos() const { return _maximum_qos; }


		// If present, this byte declares whether the Server supports retained messages. 
		// A value of 0 means that retained messages are not supported. 
		// A value of 1 means retained messages are supported. 
		// If not present, then retained messages are supported. 
		// It is a Protocol Error to include Retain Available more than once or to use a value other than 0 or 1.
		inline self& retain_available(bool v)
		{
			_retain_available = static_cast<decltype(_retain_available)>(v);
			return (*this);
		}

		inline bool retain_available() const { return (_retain_available == static_cast<decltype(_retain_available)>(1)); }

	protected:
		template<class MqttOptions>
		inline void _mqtt_options_copy_from(const MqttOptions& o)
		{
			//this->payload_format_indicator          ( o._payload_format_indicator          );
			//this->message_expiry_interval           ( o._message_expiry_interval           );
			//this->content_type                      ( o._content_type                      );
			//this->response_topic                    ( o._response_topic                    );
			//this->correlation_data                  ( o._correlation_data                  );
			//this->subscription_identifier           ( o._subscription_identifier           );
			//this->session_expiry_interval           ( o._session_expiry_interval           );
			//this->assigned_client_identifier        ( o._assigned_client_identifier        );
			//this->server_keep_alive                 ( o._server_keep_alive                 );
			//this->authentication_method             ( o._authentication_method             );
			//this->authentication_data               ( o._authentication_data               );
			//this->request_problem_information       ( o._request_problem_information       );
			//this->will_delay_interval               ( o._will_delay_interval               );
			//this->request_response_information      ( o._request_response_information      );
			//this->response_information              ( o._response_information              );
			//this->server_reference                  ( o._server_reference                  );
			//this->reason_string                     ( o._reason_string                     );
			this->receive_maximum                   ( o._receive_maximum                   );//
			this->topic_alias_maximum               ( o._topic_alias_maximum               );//
			//this->topic_alias                       ( o._topic_alias                       );
			this->maximum_qos                       ( o._maximum_qos                       );//
			this->retain_available                  ( o._retain_available                  );//
			//this->user_property                     ( o._user_property                     );
			//this->maximum_packet_size               ( o._maximum_packet_size               );
			//this->wildcard_subscription_available   ( o._wildcard_subscription_available   );
			//this->subscription_identifier_available ( o._subscription_identifier_available );
			//this->shared_subscription_available     ( o._shared_subscription_available     );
		}

	protected:
		decltype(std::declval<mqtt::v5::payload_format_indicator          >().value()) _payload_format_indicator          {};
		decltype(std::declval<mqtt::v5::message_expiry_interval           >().value()) _message_expiry_interval           {};
		decltype(std::declval<mqtt::v5::content_type                      >().value()) _content_type                      {};
		decltype(std::declval<mqtt::v5::response_topic                    >().value()) _response_topic                    {};
		decltype(std::declval<mqtt::v5::correlation_data                  >().value()) _correlation_data                  {};
		decltype(std::declval<mqtt::v5::subscription_identifier           >().value()) _subscription_identifier           {};
		decltype(std::declval<mqtt::v5::session_expiry_interval           >().value()) _session_expiry_interval           {};
		decltype(std::declval<mqtt::v5::assigned_client_identifier        >().value()) _assigned_client_identifier        {};
		decltype(std::declval<mqtt::v5::server_keep_alive                 >().value()) _server_keep_alive                 {};
		decltype(std::declval<mqtt::v5::authentication_method             >().value()) _authentication_method             {};
		decltype(std::declval<mqtt::v5::authentication_data               >().value()) _authentication_data               {};
		decltype(std::declval<mqtt::v5::request_problem_information       >().value()) _request_problem_information       {};
		decltype(std::declval<mqtt::v5::will_delay_interval               >().value()) _will_delay_interval               {};
		decltype(std::declval<mqtt::v5::request_response_information      >().value()) _request_response_information      {};
		decltype(std::declval<mqtt::v5::response_information              >().value()) _response_information              {};
		decltype(std::declval<mqtt::v5::server_reference                  >().value()) _server_reference                  {};
		decltype(std::declval<mqtt::v5::reason_string                     >().value()) _reason_string                     {};
		decltype(std::declval<mqtt::v5::receive_maximum                   >().value()) _receive_maximum                   {};
		decltype(std::declval<mqtt::v5::topic_alias_maximum               >().value()) _topic_alias_maximum               {};
		decltype(std::declval<mqtt::v5::topic_alias                       >().value()) _topic_alias                       {};
		decltype(std::declval<mqtt::v5::maximum_qos                       >().value()) _maximum_qos                       {};
		decltype(std::declval<mqtt::v5::retain_available                  >().value()) _retain_available                  {};
		decltype(std::declval<mqtt::v5::user_property                     >().value()) _user_property                     {};
		decltype(std::declval<mqtt::v5::maximum_packet_size               >().value()) _maximum_packet_size               {};
		decltype(std::declval<mqtt::v5::wildcard_subscription_available   >().value()) _wildcard_subscription_available   {};
		decltype(std::declval<mqtt::v5::subscription_identifier_available >().value()) _subscription_identifier_available {};
		decltype(std::declval<mqtt::v5::shared_subscription_available     >().value()) _shared_subscription_available     {};
	};
}

namespace asio2::mqtt
{
	using options = asio2::detail::mqtt_options;
}

namespace asio2
{
	using mqtt_options = asio2::detail::mqtt_options;
}

#endif // !__ASIO2_MQTT_OPTIONS_HPP__
