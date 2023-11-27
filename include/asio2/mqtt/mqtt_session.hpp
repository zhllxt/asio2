/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
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

#include <asio2/mqtt/detail/mqtt_handler.hpp>
#include <asio2/mqtt/detail/mqtt_invoker.hpp>
#include <asio2/mqtt/detail/mqtt_topic_alias.hpp>
#include <asio2/mqtt/detail/mqtt_session_state.hpp>
#include <asio2/mqtt/detail/mqtt_broker_state.hpp>

#include <asio2/mqtt/idmgr.hpp>
#include <asio2/mqtt/options.hpp>

namespace asio2::detail
{
	struct template_args_mqtt_session : public template_args_tcp_session
	{
		static constexpr bool rdc_call_cp_enabled = false;

		template<class caller_t>
		struct subnode
		{
			explicit subnode(
				std::weak_ptr<caller_t>  c,
				mqtt::subscription       s,
				mqtt::v5::properties_set p = mqtt::v5::properties_set{}
			)
				: caller(std::move(c))
				, sub   (std::move(s))
				, props (std::move(p))
			{
			}

			inline std::string_view share_name  () { return sub.share_name  (); }
			inline std::string_view topic_filter() { return sub.topic_filter(); }

			// 
			std::weak_ptr<caller_t>   caller;

			// subscription info
			mqtt::subscription        sub;

			// subscription properties
			mqtt::v5::properties_set  props;
		};
	};

	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SESSION;

	template<class derived_t, class args_t = template_args_mqtt_session>
	class mqtt_session_impl_t
		: public tcp_session_impl_t<derived_t, args_t>
		, public mqtt_options
		, public mqtt_handler_t    <derived_t, args_t>
		, public mqtt_topic_alias_t<derived_t, args_t>
		, public mqtt_send_op      <derived_t, args_t>
		, public mqtt::session_state
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION;

		template <class> friend class mqtt::shared_target;

	public:
		using super = tcp_session_impl_t <derived_t, args_t>;
		using self  = mqtt_session_impl_t<derived_t, args_t>;

		using args_type    = args_t;
		using key_type     = std::size_t;
		using subnode_type = typename args_type::template subnode<derived_t>;

		using super::send;
		using super::async_send;

	public:
		/**
		 * @brief constructor
		 */
		explicit mqtt_session_impl_t(
			mqtt::broker_state<derived_t, args_t>& broker_state,
			session_mgr_t <derived_t>& sessions,
			listener_t               & listener,
			std::shared_ptr<io_t>      rwio,
			std::size_t                init_buf_size,
			std::size_t                max_buf_size
		)
			: super(sessions, listener, std::move(rwio), init_buf_size, max_buf_size)
			, mqtt_options                         ()
			, mqtt_handler_t    <derived_t, args_t>()
			, mqtt_topic_alias_t<derived_t, args_t>()
			, mqtt_send_op      <derived_t, args_t>()
			, broker_state_(broker_state)
		{
			this->set_silence_timeout(std::chrono::milliseconds(mqtt_silence_timeout));
		}

		/**
		 * @brief destructor
		 */
		~mqtt_session_impl_t()
		{
		}

	public:
		/**
		 * @brief get this object hash key,used for session map
		 */
		inline key_type hash_key() const
		{
			return reinterpret_cast<key_type>(this);
		}

		/**
		 * @brief get the mqtt version number
		 */
		inline mqtt::version version() const
		{
			return this->get_version();
		}

		/**
		 * @brief get the mqtt version number
		 */
		inline mqtt::version get_version() const
		{
			return this->version_;
		}

		/**
		 * @brief get the mqtt client id 
		 */
		inline std::string_view client_id() const
		{
			return this->get_client_id();
		}

		/**
		 * @brief get the mqtt client id 
		 */
		inline std::string_view get_client_id() const
		{
			std::string_view id{};
			if (!this->connect_message_.empty())
			{
				if /**/ (std::holds_alternative<mqtt::v3::connect>(connect_message_.base()))
				{
					id = this->connect_message_.template get_if<mqtt::v3::connect>()->client_id();
				}
				else if (std::holds_alternative<mqtt::v4::connect>(connect_message_.base()))
				{
					id = this->connect_message_.template get_if<mqtt::v4::connect>()->client_id();
				}
				else if (std::holds_alternative<mqtt::v5::connect>(connect_message_.base()))
				{
					id = this->connect_message_.template get_if<mqtt::v5::connect>()->client_id();
				}
			}
			return id;
		}

		inline void remove_subscribed_topic(std::string_view topic_filter)
		{
			this->subs_map().erase(topic_filter, this->client_id());
		}

		inline void remove_all_subscribed_topic()
		{
			this->subs_map().erase(this->client_id());
		}

	protected:
		template<typename E = defer_event<void, derived_t>>
		inline void _do_disconnect(
			const error_code& ec, std::shared_ptr<derived_t> this_ptr, E chain = defer_event<void, derived_t>{})
		{
			state_t expected = state_t::started;
			if (this->derived().state_.compare_exchange_strong(expected, state_t::started))
			{
				mqtt::version ver = this->derived().version();
				if /**/ (ver == mqtt::version::v3)
				{
					mqtt::v3::disconnect disconnect;
					this->derived().internal_async_send(std::move(this_ptr), std::move(disconnect),
					[this, ec, e = chain.move_event()]
					(std::shared_ptr<derived_t> this_ptr, const error_code&,
						std::size_t, event_queue_guard<derived_t> g) mutable
					{
						defer_event chain(std::move(e), std::move(g));
						super::_do_disconnect(ec, std::move(this_ptr), std::move(chain));
					}, chain.move_guard());
					return;
				}
				else if (ver == mqtt::version::v4)
				{
					mqtt::v4::disconnect disconnect;
					this->derived().internal_async_send(std::move(this_ptr), std::move(disconnect),
					[this, ec, e = chain.move_event()]
					(std::shared_ptr<derived_t> this_ptr, const error_code&,
						std::size_t, event_queue_guard<derived_t> g) mutable
					{
						defer_event chain(std::move(e), std::move(g));
						super::_do_disconnect(ec, std::move(this_ptr), std::move(chain));
					}, chain.move_guard());
					return;
				}
				else if (ver == mqtt::version::v5)
				{
					// https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901208
					mqtt::v5::disconnect disconnect;
					if (ec.value() != 4)
						disconnect.reason_code(static_cast<std::uint8_t>(ec.value()));
					this->derived().internal_async_send(std::move(this_ptr), std::move(disconnect),
					[this, ec, e = chain.move_event()]
					(std::shared_ptr<derived_t> this_ptr, const error_code&,
						std::size_t, event_queue_guard<derived_t> g) mutable
					{
						defer_event chain(std::move(e), std::move(g));
						super::_do_disconnect(ec, std::move(this_ptr), std::move(chain));
					}, chain.move_guard());
					return;
				}
				else
				{
					ASIO2_ASSERT(false);
				}
			}

			super::_do_disconnect(ec, std::move(this_ptr), std::move(chain));
		}

		template<typename C, typename DeferEvent>
		inline void _handle_connect(
			const error_code& ec,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			detail::ignore_unused(ec);

			ASIO2_ASSERT(!ec);
			ASIO2_ASSERT(this->derived().sessions_.io_->running_in_this_thread());

			asio::dispatch(this->derived().io_->context(), make_allocator(this->derived().wallocator(),
			[this, this_ptr = std::move(this_ptr), ecs = std::move(ecs), chain = std::move(chain)]
			() mutable
			{
				derived_t& derive = this->derived();

				ASIO2_ASSERT(derive.io_->running_in_this_thread());

				// wait for the connect message which send by the client.
				mqtt_recv_connect_op
				{
					derive.io_->context(),
					derive.stream(),
					[this, this_ptr = std::move(this_ptr), ecs = std::move(ecs), chain = std::move(chain)]
					(error_code ec, std::unique_ptr<asio::streambuf> stream) mutable
					{
						this->derived()._handle_mqtt_connect_message(ec,
								std::move(this_ptr),std::move(ecs), std::move(stream), std::move(chain));
					}
				};
			}));
		}

		template<typename C, typename DeferEvent>
		inline void _handle_mqtt_connect_message(
			error_code ec,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs,
			std::unique_ptr<asio::streambuf> stream, DeferEvent chain)
		{
			do
			{
				if (ec)
					break;

				std::string_view data{ reinterpret_cast<std::string_view::const_pointer>(
					static_cast<const char*>(stream->data().data())), stream->size() };

				mqtt::control_packet_type type = mqtt::message_type_from_data(data);

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
				this->invoker()._call_mqtt_handler(type, ec, this_ptr, static_cast<derived_t*>(this), data);

			} while (false);

			this->derived().sessions_.dispatch(
			[this, ec, this_ptr = std::move(this_ptr), ecs = std::move(ecs), chain = std::move(chain)]
			() mutable
			{
				super::_handle_connect(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
			});
		}

		template<typename DeferEvent>
		inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			std::string_view clientid = this->client_id();

			this->subs_map().erase(clientid);

			this->mqtt_sessions().erase_mqtt_session(clientid, static_cast<derived_t*>(this));

			super::_handle_disconnect(ec, std::move(this_ptr), std::move(chain));
		}

	protected:
		template<class Data, class Callback>
		inline bool _do_send(Data& data, Callback&& callback)
		{
			return this->derived()._mqtt_send(data, std::forward<Callback>(callback));
		}

	protected:
		template<typename C>
		inline void _fire_recv(
			std::shared_ptr<derived_t>& this_ptr, std::shared_ptr<ecs_t<C>>& ecs, std::string_view data)
		{
			data = detail::call_data_filter_before_recv(this->derived(), data);

			this->listener_.notify(event_type::recv, this_ptr, data);

			this->derived()._rdc_handle_recv(this_ptr, ecs, data);

			mqtt::control_packet_type type = mqtt::message_type_from_data(data);

			if (type > mqtt::control_packet_type::auth)
			{
				ASIO2_ASSERT(false);
				this->derived()._do_disconnect(mqtt::make_error_code(mqtt::error::malformed_packet), this_ptr);
				return;
			}

			error_code ec;

			this->invoker()._call_mqtt_handler(type, ec, this_ptr, static_cast<derived_t*>(this), data);

			if (ec)
			{
				this->derived()._do_disconnect(ec, this_ptr);
			}
		}

		inline auto& invoker          () noexcept { return this->broker_state_.invoker_          ; }
		inline auto& mqtt_sessions    () noexcept { return this->broker_state_.mqtt_sessions_    ; }
		inline auto& subs_map         () noexcept { return this->broker_state_.subs_map_         ; }
		inline auto& shared_targets   () noexcept { return this->broker_state_.shared_targets_   ; }
		inline auto& retained_messages() noexcept { return this->broker_state_.retained_messages_; }
		inline auto& security         () noexcept { return this->broker_state_.security_         ; }

		inline auto const& invoker          () const noexcept { return this->broker_state_.invoker_          ; }
		inline auto const& mqtt_sessions    () const noexcept { return this->broker_state_.mqtt_sessions_    ; }
		inline auto const& subs_map         () const noexcept { return this->broker_state_.subs_map_         ; }
		inline auto const& shared_targets   () const noexcept { return this->broker_state_.shared_targets_   ; }
		inline auto const& retained_messages() const noexcept { return this->broker_state_.retained_messages_; }
		inline auto const& security         () const noexcept { return this->broker_state_.security_         ; }

		inline void set_preauthed_username(std::optional<std::string> username)
		{
			preauthed_username_ = std::move(username);
		}
		inline std::optional<std::string> get_preauthed_username() const
		{
			return preauthed_username_;
		}

	protected:
		/// 
		mqtt::broker_state<derived_t, args_t>                               & broker_state_;

		/// packet id manager
		mqtt::idmgr<std::atomic<mqtt::two_byte_integer::value_type>>          idmgr_;

		/// user to find session for shared targets
		std::chrono::nanoseconds::rep                                         shared_target_key_;

		mqtt::message                                                         connect_message_{};

		std::optional<std::string> preauthed_username_;

		mqtt::version              version_ = static_cast<mqtt::version>(0);
	};
}

namespace asio2
{
	using mqtt_session_args = detail::template_args_mqtt_session;

	template<class derived_t, class args_t>
	using mqtt_session_impl_t = detail::mqtt_session_impl_t<derived_t, args_t>;

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
