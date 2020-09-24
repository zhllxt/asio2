/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_UDP_CLIENT_HPP__
#define __ASIO2_UDP_CLIENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/client.hpp>
#include <asio2/base/detail/linear_buffer.hpp>
#include <asio2/udp/impl/udp_send_op.hpp>
#include <asio2/udp/detail/kcp_util.hpp>
#include <asio2/udp/component/kcp_stream_cp.hpp>

namespace asio2::detail
{
	template<class derived_t, class socket_t, class buffer_t>
	class udp_client_impl_t
		: public client_impl_t<derived_t, socket_t, buffer_t>
		, public udp_send_op<derived_t, false>
	{
		template <class, bool>                friend class user_timer_cp;
		template <class>                      friend class post_cp;
		template <class, bool>                friend class reconnect_timer_cp;
		template <class, bool>                friend class connect_timeout_cp;
		template <class, class, bool>         friend class connect_cp;
		template <class, class, bool>         friend class disconnect_cp;
		template <class>                      friend class data_persistence_cp;
		template <class>                      friend class event_queue_cp;
		template <class, bool>                friend class send_cp;
		template <class, bool>                friend class udp_send_op;
		template <class, bool>                friend class kcp_stream_cp;
		template <class, class, class>        friend class client_impl_t;

	public:
		using self = udp_client_impl_t<derived_t, socket_t, buffer_t>;
		using super = client_impl_t<derived_t, socket_t, buffer_t>;
		using buffer_type = buffer_t;
		using super::send;

		/**
		 * @constructor
		 */
		explicit udp_client_impl_t(
			std::size_t init_buffer_size = udp_frame_size,
			std::size_t max_buffer_size = (std::numeric_limits<std::size_t>::max)()
		)
			: super(1, init_buffer_size, max_buffer_size)
			, udp_send_op<derived_t, false>()
		{
			this->connect_timeout(std::chrono::milliseconds(udp_connect_timeout));
		}

		/**
		 * @destructor
		 */
		~udp_client_impl_t()
		{
			this->stop();
		}

		/**
		 * @function : start the client, blocking connect to server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt>
		bool start(String&& host, StrOrInt&& port)
		{
			return this->derived().template _do_connect<false>(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				condition_wrap<void>{});
		}

		/**
		 * @function : start the client, blocking connect to server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt>
		bool start(String&& host, StrOrInt&& port, use_kcp_t c)
		{
			return this->derived().template _do_connect<false>(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				condition_wrap<use_kcp_t>(c));
		}

		/**
		 * @function : start the client, blocking connect to server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt>
		bool async_start(String&& host, StrOrInt&& port)
		{
			return this->derived().template _do_connect<true>(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				condition_wrap<void>{});
		}

		/**
		 * @function : start the client, blocking connect to server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt>
		bool async_start(String&& host, StrOrInt&& port, use_kcp_t c)
		{
			return this->derived().template _do_connect<true>(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				condition_wrap<use_kcp_t>(c));
		}

		/**
		 * @function : stop the client
		 * You can call this function on the communication thread and anywhere to stop the client.
		 */
		inline void stop()
		{
			this->derived().post([this]() mutable
			{
				// first close reconnect timer
				this->_stop_reconnect_timer();
			});

			this->derived()._do_disconnect(asio::error::operation_aborted, std::make_shared<defer>([this]()
			{
				this->derived()._do_stop(asio::error::operation_aborted);
			}));

			this->iopool_.stop();
		}

	public:
		/**
		 * @function : get the kcp pointer, just used for kcp mode
		 * default mode : ikcp_nodelay(kcp, 0, 10, 0, 0);
		 * generic mode : ikcp_nodelay(kcp, 0, 10, 0, 1);
		 * fast    mode : ikcp_nodelay(kcp, 1, 10, 2, 1);
		 */
		inline kcp::ikcpcb* kcp()
		{
			return (this->kcp_ ? this->kcp_->kcp_ : nullptr);
		}

	public:
		/**
		 * @function : bind recv listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * Function signature : void(std::string_view s)
		 */
		template<class F, class ...C>
		inline derived_t & bind_recv(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::recv,
				observer_t<std::string_view>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

		/**
		 * @function : bind connect listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * This notification is called after the client connection completed, whether successful or unsuccessful
		 * Function signature : void(asio2::error_code ec)
		 */
		template<class F, class ...C>
		inline derived_t & bind_connect(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::connect,
				observer_t<error_code>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

		/**
		 * @function : bind disconnect listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * This notification is called before the client is ready to disconnect
		 * Function signature : void(asio2::error_code ec)
		 */
		template<class F, class ...C>
		inline derived_t & bind_disconnect(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::disconnect,
				observer_t<error_code>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

		/**
		 * @function : bind init listener,we should set socket options at here
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * Function signature : void()
		 */
		template<class F, class ...C>
		inline derived_t & bind_init(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::init,
				observer_t<>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

		/**
		 * @function : bind kcp handshake listener, just used fo kcp mode
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * Function signature : void(asio2::error_code ec)
		 */
		template<class F, class ...C>
		inline derived_t & bind_handshake(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::handshake,
				observer_t<error_code>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	protected:
		template<bool isAsync, typename String, typename StrOrInt, typename MatchCondition>
		bool _do_connect(String&& host, StrOrInt&& port, condition_wrap<MatchCondition> condition)
		{
			state_t expected = state_t::stopped;
			if (!this->state_.compare_exchange_strong(expected, state_t::starting))
			{
				set_last_error(asio::error::already_started);
				return false;
			}

			try
			{
				clear_last_error();

				this->iopool_.start();

				if (this->iopool_.is_stopped())
				{
					set_last_error(asio::error::shut_down);
					return false;
				}

				this->derived()._make_reconnect_timer(this->derived().selfptr(),
					[this, h = to_string(host), p = to_string(port), condition]() mutable
				{
					state_t expected = state_t::stopped;
					if (this->state_.compare_exchange_strong(expected, state_t::starting))
					{
						auto task = [this, h, p, condition](event_guard<derived_t>&& g) mutable
						{
							this->derived().template _start_connect<true>(h, p,
								this->derived().selfptr(), condition);
						};

						this->derived().push_event([this, t = std::move(task)]
						(event_guard<derived_t>&& g) mutable
						{
							auto task = [g = std::move(g), t = std::move(t)]() mutable
							{
								t(std::move(g));
							};
							this->derived().post(std::move(task));
							return true;
						});
					}
				});

				this->derived()._do_init(condition);

				super::start();

				return this->derived().template _start_connect<isAsync>(
					std::forward<String>(host), std::forward<StrOrInt>(port),
					this->derived().selfptr(), condition);
			}
			catch (system_error & e)
			{
				this->derived()._handle_connect(e.code(), this->derived().selfptr(), condition);
			}
			return false;
		}

		template<typename MatchCondition>
		inline void _do_init(condition_wrap<MatchCondition>)
		{
			if constexpr (std::is_same_v<MatchCondition, use_kcp_t>)
				this->kcp_ = std::make_unique<kcp_stream_cp<derived_t, false>>(this->derived(), this->io_);
			else
				this->kcp_.reset();
		}

		template<typename MatchCondition>
		inline void _handle_connect(const error_code & ec, std::shared_ptr<derived_t> this_ptr,
			condition_wrap<MatchCondition> condition)
		{
			set_last_error(ec);

			if (ec)
				return this->derived()._done_connect(ec, std::move(this_ptr), std::move(condition));

			if constexpr (std::is_same_v<MatchCondition, use_kcp_t>)
				this->kcp_->_post_handshake(std::move(this_ptr), std::move(condition));
			else
				this->derived()._done_connect(ec, std::move(this_ptr), std::move(condition));
		}

		template<typename MatchCondition>
		inline void _do_start(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			this->update_alive_time();
			this->reset_connect_time();

			this->derived()._start_recv(std::move(this_ptr), std::move(condition));
		}

		inline void _do_stop(const error_code& ec)
		{
			this->derived()._post_stop(ec, this->derived().selfptr());
		}

		inline void _post_stop(const error_code& ec, std::shared_ptr<derived_t> self_ptr)
		{
			// All pending sending events will be cancelled after enter the send strand below.
			auto task = [this, ec, this_ptr = std::move(self_ptr)](event_guard<derived_t>&& g) mutable
			{
				set_last_error(ec);

				// call the base class stop function
				super::stop();

				// call CRTP polymorphic stop
				this->derived()._handle_stop(ec, std::move(this_ptr));
			};

			this->derived().push_event([this, t = std::move(task)](event_guard<derived_t>&& g) mutable
			{
				auto task = [g = std::move(g), t = std::move(t)]() mutable
				{
					t(std::move(g));
				};
				this->derived().post(std::move(task));
				return true;
			});
		}

		inline void _handle_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			detail::ignore::unused(ec, this_ptr);

			if (this->kcp_)
				this->kcp_->_kcp_stop();

			// call socket's close function to notify the _handle_recv function response with 
			// error > 0 ,then the socket can get notify to exit
			// Call shutdown() to indicate that you will not write any more data to the socket.
			this->socket_.shutdown(asio::socket_base::shutdown_both, ec_ignore);
			// Call close,otherwise the _handle_recv will never return
			this->socket_.close(ec_ignore);
		}

		template<typename MatchCondition>
		inline void _start_recv(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			// Connect succeeded. post recv request.
			this->derived().post([this, this_ptr, condition]() mutable
			{
				this->derived().buffer().consume(this->derived().buffer().size());

				this->derived()._post_recv(std::move(this_ptr), std::move(condition));
			});
		}

		template<class Data, class Callback>
		inline bool _do_send(Data& data, Callback&& callback)
		{
			if (!this->kcp_)
				return this->derived()._udp_send(data, std::forward<Callback>(callback));
			return this->kcp_->_kcp_send(data, std::forward<Callback>(callback));
		}

	protected:
		template<typename MatchCondition>
		void _post_recv(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			if (!this->is_started())
				return;

			try
			{
				this->socket_.async_receive(this->buffer_.prepare(this->buffer_.pre_size()),
					asio::bind_executor(this->io_.strand(), make_allocator(this->rallocator_,
						[this, self_ptr = std::move(this_ptr), condition]
				(const error_code & ec, std::size_t bytes_recvd) mutable
				{
					this->derived()._handle_recv(ec, bytes_recvd, std::move(self_ptr), condition);
				})));
			}
			catch (system_error & e)
			{
				set_last_error(e);
				this->derived()._do_disconnect(e.code());
			}
		}

		template<typename MatchCondition>
		void _handle_recv(const error_code & ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			set_last_error(ec);

			if (ec == asio::error::operation_aborted || ec == asio::error::connection_refused)
			{
				this->derived()._do_disconnect(ec);
				return;
			}

			if (!this->is_started())
				return;

			this->buffer_.commit(bytes_recvd);

			if (!ec)
			{
				this->update_alive_time();

				std::string_view s = std::string_view(static_cast<std::string_view::const_pointer>
					(this->buffer_.data().data()), bytes_recvd);

				if constexpr (!std::is_same_v<MatchCondition, use_kcp_t>)
				{
					this->derived()._fire_recv(this_ptr, std::move(s));
				}
				else
				{
					if (s.size() == sizeof(kcp::kcphdr))
					{
						if /**/ (kcp::is_kcphdr_fin(s))
						{
							this->kcp_->send_fin_ = false;
							this->derived()._do_disconnect(asio::error::eof);
						}
						else if (kcp::is_kcphdr_synack(s, this->kcp_->seq_))
						{
							ASIO2_ASSERT(false);
						}
					}
					else
						this->kcp_->_kcp_recv(this_ptr, s, this->buffer_);
				}
			}

			this->buffer_.consume(this->buffer_.size());

			this->derived()._post_recv(std::move(this_ptr), condition);
		}

		inline void _fire_init()
		{
			this->listener_.notify(event::init);
		}

		inline void _fire_recv(detail::ignore, std::string_view s)
		{
			this->listener_.notify(event::recv, std::move(s));
		}

		inline void _fire_handshake(detail::ignore, error_code ec)
		{
			this->listener_.notify(event::handshake, ec);
		}

		inline void _fire_connect(detail::ignore, error_code ec)
		{
			this->listener_.notify(event::connect, ec);
		}

		inline void _fire_disconnect(detail::ignore, error_code ec)
		{
			this->listener_.notify(event::disconnect, ec);
		}

	protected:
		std::unique_ptr<kcp_stream_cp<derived_t, false>> kcp_;
	};
}

namespace asio2
{
	class udp_client : public detail::udp_client_impl_t<udp_client, asio::ip::udp::socket, asio2::linear_buffer>
	{
	public:
		using udp_client_impl_t<udp_client, asio::ip::udp::socket, asio2::linear_buffer>::udp_client_impl_t;
	};
}

#endif // !__ASIO2_UDP_CLIENT_HPP__
