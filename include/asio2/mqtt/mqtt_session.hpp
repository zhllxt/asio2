/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_MQTT_SESSION_HPP__
#define __ASIO2_MQTT_SESSION_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <asio2/tcp/tcp_session.hpp>

#include <asio2/mqtt/impl/mqtt_recv_connect_op.hpp>

#include <asio2/mqtt/impl/mqtt_send_op.hpp>

#include <asio2/mqtt/detail/mqtt_options.hpp>
#include <asio2/mqtt/detail/mqtt_handler.hpp>
#include <asio2/mqtt/detail/mqtt_invoker.hpp>

#include <asio2/mqtt/detail/mqtt_topic_alias.hpp>
#include <asio2/mqtt/detail/mqtt_session_state.hpp>
#include <asio2/mqtt/detail/mqtt_packet_id_mgr.hpp>

namespace asio2::detail
{
	struct template_args_mqtt_session : public template_args_tcp_session
	{
	};

	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SESSION;

	template<class derived_t, class args_t = template_args_mqtt_session>
	class mqtt_session_impl_t
		: public tcp_session_impl_t<derived_t, args_t>
		, public mqtt_options
		, public mqtt_handler_t    <derived_t        >
		, public mqtt_topic_alias_t<derived_t        >
		, public mqtt_send_op      <derived_t, args_t>
		, public mqtt::session_state
		, public mqtt::packet_id_mgr<mqtt::two_byte_integer::value_type>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION;

		template <class> friend class mqtt::shared_target;

	public:
		using super = tcp_session_impl_t <derived_t, args_t>;
		using self  = mqtt_session_impl_t<derived_t, args_t>;

		using key_type    = std::size_t;

	//protected:
		using super::send;
		using super::async_send;
		using super::call;
		using super::async_call;

	public:
		/**
		 * @constructor
		 */
		explicit mqtt_session_impl_t(
			mqtt_invoker_t<derived_t>                                       & invoker,
			asio2_shared_mutex                                              & mqttid_sessions_mtx,
			std::unordered_map<std::string_view, std::shared_ptr<derived_t>>& mqttid_sessions,
			mqtt::multiple_subscription_map<std::string_view, mqtt::subscription_entry<derived_t>>& subs_map,
			mqtt::shared_target<mqtt::shared_entry<derived_t>>& shared_targets,
			mqtt::retained_messages<mqtt::retained_entry>& retained_messages,
			session_mgr_t <derived_t>& sessions,
			listener_t               & listener,
			io_t                     & rwio,
			std::size_t                init_buf_size,
			std::size_t                max_buf_size
		)
			: super(sessions, listener, rwio, init_buf_size, max_buf_size)
			, mqtt_options                         ()
			, mqtt_handler_t    <derived_t        >()
			, mqtt_topic_alias_t<derived_t        >()
			, mqtt_send_op      <derived_t, args_t>()
			, invoker_            (invoker)
			, mqttid_sessions_mtx_(mqttid_sessions_mtx)
			, mqttid_sessions_    (mqttid_sessions    )
			, subs_map_           (subs_map           )
			, shared_targets_     (shared_targets     )
			, retained_messages_  (retained_messages  )
		{
			this->set_silence_timeout(std::chrono::milliseconds(mqtt_silence_timeout));
		}

		/**
		 * @destructor
		 */
		~mqtt_session_impl_t()
		{
		}

	public:
		/**
		 * @function : get this object hash key,used for session map
		 */
		inline key_type hash_key() const
		{
			return reinterpret_cast<key_type>(this);
		}

		/**
		 * @function : get the mqtt version number
		 */
		inline mqtt::version version()
		{
			return this->get_version();
		}

		/**
		 * @function : get the mqtt version number
		 */
		inline mqtt::version get_version()
		{
			return this->version_;
		}

		/**
		 * @function : get the mqtt client id 
		 */
		inline std::string_view client_id()
		{
			return this->get_client_id();
		}

		/**
		 * @function : get the mqtt client id 
		 */
		inline std::string_view get_client_id()
		{
			std::string_view id{};
			if (this->connect_message_.index() != std::variant_npos)
			{
				std::visit([&id](auto& connect) mutable
				{
					id = connect.client_id();
				}, this->connect_message_);
			}
			return id;
		}

	protected:
		template<typename DeferEvent = defer_event<void, derived_t>>
		inline void _do_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr,
			DeferEvent chain = defer_event<void, derived_t>{})
		{
			state_t expected = state_t::started;
			if (this->derived().state_.compare_exchange_strong(expected, state_t::started))
			{
				mqtt::version ver = this->derived().version();
				if /**/ (ver == mqtt::version::v3)
				{
					mqtt::v3::disconnect disconnect;
					this->derived().async_send(std::move(disconnect));
				}
				else if (ver == mqtt::version::v4)
				{
					mqtt::v4::disconnect disconnect;
					this->derived().async_send(std::move(disconnect));
				}
				else if (ver == mqtt::version::v5)
				{
					// https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901208
					mqtt::v5::disconnect disconnect;
					if (ec.value() != 4)
						disconnect.reason_code(static_cast<std::uint8_t>(ec.value()));
					this->derived().async_send(std::move(disconnect));
				}
				else
				{
					ASIO2_ASSERT(false);
				}
			}

			super::_do_disconnect(ec, std::move(this_ptr), std::move(chain));
		}

		template<typename MatchCondition, typename DeferEvent>
		inline void _handle_connect(const error_code& ec, std::shared_ptr<derived_t> this_ptr,
			condition_wrap<MatchCondition> condition, DeferEvent chain)
		{
			detail::ignore_unused(ec);

			ASIO2_ASSERT(!ec);
			ASIO2_ASSERT(this->derived().sessions().io().strand().running_in_this_thread());

			asio::dispatch(this->derived().io().strand(), make_allocator(this->derived().wallocator(),
			[this, this_ptr = std::move(this_ptr), condition = std::move(condition), chain = std::move(chain)]
			() mutable
			{
				derived_t& derive = this->derived();

				ASIO2_ASSERT(derive.io().strand().running_in_this_thread());

				// wait for the connect message which send by the client.
				mqtt_recv_connect_op
				{
					derive.io().context(), derive.io().strand(),
					derive.stream(),
					[this, this_ptr = std::move(this_ptr), condition = std::move(condition), chain = std::move(chain)]
					(error_code ec, std::unique_ptr<asio::streambuf> stream) mutable
					{
						this->derived()._handle_mqtt_connect_message(ec, std::move(this_ptr),
								std::move(condition), std::move(stream), std::move(chain));
					}
				};
			}));
		}

		template<typename MatchCondition, typename DeferEvent>
		inline void _handle_mqtt_connect_message(error_code ec, std::shared_ptr<derived_t> this_ptr,
			condition_wrap<MatchCondition> condition, std::unique_ptr<asio::streambuf> stream, DeferEvent chain)
		{
			do
			{
				if (ec)
					break;

				std::string_view data{ reinterpret_cast<std::string_view::const_pointer>(
					asio::buffer_cast<const char*>(stream->data())), stream->size() };

				mqtt::control_packet_type type = mqtt::message_type_from_byte(data.front());

				// If the server does not receive a CONNECT message within a reasonable amount of time 
				// after the TCP/IP connection is established, the server should close the connection.
				if (type != mqtt::control_packet_type::connect)
				{
					ec = mqtt::make_error_code(mqtt::error::malformed_packet);
					break;
				}

				// parse the connect message to get the mqtt version
				mqtt::version ver = mqtt::version_from_connect_data(data);

				if (ver != mqtt::version::v3 && ver != mqtt::version::v4 && ver != mqtt::version::v5)
				{
					ec = mqtt::make_error_code(mqtt::error::unsupported_protocol_version);
					break;
				}

				this->version_ = ver;

				// If the client sends an invalid CONNECT message, the server should close the connection.
				// This includes CONNECT messages that provide invalid Protocol Name or Protocol Version Numbers.
				// If the server can parse enough of the CONNECT message to determine that an invalid protocol
				// has been requested, it may try to send a CONNACK containing the "Connection Refused:
				// unacceptable protocol version" code before dropping the connection.
				this->invoker_._call_mqtt_handler(type, ec, this_ptr, static_cast<derived_t*>(this), data);

			} while (false);

			this->derived().sessions().dispatch(
			[this, ec, this_ptr = std::move(this_ptr), condition = std::move(condition), chain = std::move(chain)]
			() mutable
			{
				super::_handle_connect(ec, std::move(this_ptr), std::move(condition), std::move(chain));
			});
		}

		template<typename DeferEvent>
		inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			{
				asio2_unique_lock lock{ this->mqttid_sessions_mtx_ };

				std::string_view id = this->client_id();

				auto iter = this->mqttid_sessions_.find(id);
				if (iter != this->mqttid_sessions_.end())
				{
					if (iter->second.get() == this)
					{
						this->mqttid_sessions_.erase(id);
					}
				}
			}

			super::_handle_disconnect(ec, std::move(this_ptr), std::move(chain));
		}

	protected:
		template<class Data, class Callback>
		inline bool _do_send(Data& data, Callback&& callback)
		{
			return this->derived()._mqtt_send(data, std::forward<Callback>(callback));
		}

	protected:
		template<typename MatchCondition>
		inline void _fire_recv(std::shared_ptr<derived_t>& this_ptr, std::string_view data,
			condition_wrap<MatchCondition>& condition)
		{
			this->listener_.notify(event_type::recv, this_ptr, data);

			this->derived()._rdc_handle_recv(this_ptr, data, condition);

			mqtt::control_packet_type type = mqtt::message_type_from_byte(data.front());

			if (type > mqtt::control_packet_type::auth)
			{
				ASIO2_ASSERT(false);
				this->derived()._do_disconnect(mqtt::make_error_code(mqtt::error::malformed_packet), this_ptr);
			}

			error_code ec;

			this->invoker_._call_mqtt_handler(type, ec, this_ptr, static_cast<derived_t*>(this), data);

			if (ec)
			{
				this->derived()._do_disconnect(ec, this_ptr);
			}
		}

	protected:
		mqtt_invoker_t<derived_t>                                           & invoker_;

		asio2_shared_mutex                                                  & mqttid_sessions_mtx_;

		std::unordered_map<std::string_view, std::shared_ptr<derived_t>>    & mqttid_sessions_;

		/// subscription information map
		mqtt::multiple_subscription_map<std::string_view, mqtt::subscription_entry<derived_t>>& subs_map_;

		/// shared subscription targets
		mqtt::shared_target<mqtt::shared_entry<derived_t>>& shared_targets_;

		/// A list of messages retained so they can be sent to newly subscribed clients.
		mqtt::retained_messages<mqtt::retained_entry>& retained_messages_;

		/// user to find session for shared targets
		std::chrono::nanoseconds::rep shared_target_key_;

		std::variant<mqtt::v3::connect, mqtt::v4::connect, mqtt::v5::connect> connect_message_{};

		mqtt::version              version_ = static_cast<mqtt::version>(0);
	};
}

namespace asio2
{
	template<class derived_t>
	class mqtt_session_t : public detail::mqtt_session_impl_t<derived_t, detail::template_args_mqtt_session>
	{
	public:
		using detail::mqtt_session_impl_t<derived_t, detail::template_args_mqtt_session>::mqtt_session_impl_t;
	};

	class mqtt_session : public mqtt_session_t<mqtt_session>
	{
	public:
		using mqtt_session_t<mqtt_session>::mqtt_session_t;
	};
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_MQTT_SESSION_HPP__
