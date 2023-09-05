/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_UDP_SESSION_HPP__
#define __ASIO2_UDP_SESSION_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <asio2/base/session.hpp>
#include <asio2/base/detail/linear_buffer.hpp>

#include <asio2/udp/detail/kcp_util.hpp>

#include <asio2/udp/impl/udp_send_op.hpp>
#include <asio2/udp/impl/udp_recv_op.hpp>
#include <asio2/udp/impl/kcp_stream_cp.hpp>

namespace asio2::detail
{
	struct template_args_udp_session : public udp_tag
	{
		static constexpr bool is_session = true;
		static constexpr bool is_client  = false;
		static constexpr bool is_server  = false;

		using socket_t    = asio::ip::udp::socket;
		using buffer_t    = detail::proxy_buffer<asio2::linear_buffer>;
		using send_data_t = std::string_view;
		using recv_data_t = std::string_view;

		static constexpr std::size_t allocator_storage_size = 256;
	};

	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_UDP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_UDP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_UDP_SESSION;

	template<class derived_t, class args_t = template_args_udp_session>
	class udp_session_impl_t
		: public session_impl_t<derived_t, args_t>
		, public udp_send_op   <derived_t, args_t>
		, public udp_recv_op   <derived_t, args_t>
		, public udp_tag
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_UDP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_UDP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_UDP_SESSION;

	public:
		using super = session_impl_t    <derived_t, args_t>;
		using self  = udp_session_impl_t<derived_t, args_t>;

		using args_type = args_t;
		using key_type  = asio::ip::udp::endpoint;

		using buffer_type = typename args_t::buffer_t;
		using send_data_t = typename args_t::send_data_t;
		using recv_data_t = typename args_t::recv_data_t;

	public:
		/**
		 * @brief constructor
		 */
		explicit udp_session_impl_t(
			session_mgr_t<derived_t>                 & sessions,
			listener_t                               & listener,
			std::shared_ptr<io_t>                      rwio,
			std::size_t                                init_buf_size,
			std::size_t                                max_buf_size,
			asio2::linear_buffer                     & buffer,
			std::shared_ptr<typename args_t::socket_t> socket,
			asio::ip::udp::endpoint                  & endpoint
		)
			: super(sessions, listener, std::move(rwio), init_buf_size, max_buf_size, std::move(socket))
			, udp_send_op<derived_t, args_t>()
			, udp_recv_op<derived_t, args_t>()
			, wallocator_     ()
		{
			this->remote_endpoint_ = endpoint;

			this->buffer_.bind_buffer(&buffer);

			this->set_silence_timeout(std::chrono::milliseconds(udp_silence_timeout));
			this->set_connect_timeout(std::chrono::milliseconds(udp_connect_timeout));
		}

		/**
		 * @brief destructor
		 */
		~udp_session_impl_t()
		{
		}

	protected:
		/**
		 * @brief start this session for prepare to recv msg
		 */
		template<typename C>
		inline void start(std::shared_ptr<ecs_t<C>> ecs)
		{
			derived_t& derive = this->derived();

		#if defined(ASIO2_ENABLE_LOG)
		#if defined(ASIO2_ALLOCATOR_STORAGE_SIZE)
			static_assert(decltype(wallocator_)::storage_size == ASIO2_ALLOCATOR_STORAGE_SIZE);
		#else
			static_assert(decltype(wallocator_)::storage_size == args_t::allocator_storage_size);
		#endif
		#endif
			
			ASIO2_ASSERT(this->sessions_.io_->running_in_this_thread());
			ASIO2_ASSERT(this->io_->get_thread_id() != std::thread::id{});

		#if defined(_DEBUG) || defined(DEBUG)
			this->is_stop_silence_timer_called_ = false;
			this->is_stop_connect_timeout_timer_called_ = false;
			this->is_disconnect_called_ = false;
		#endif

			std::shared_ptr<derived_t> this_ptr = derive.selfptr();

			state_t expected = state_t::stopped;
			if (!this->state_.compare_exchange_strong(expected, state_t::starting))
			{
				derive._do_disconnect(asio::error::already_started, std::move(this_ptr));
				return;
			}

			// must read/write ecs in the io_context thread.
			derive.ecs_ = ecs;

			derive._do_init(this_ptr, ecs);

			// First call the base class start function
			super::start();

			// if the ecs has remote data call mode,do some thing.
			derive._rdc_init(ecs);

			derive.push_event(
			[&derive, this_ptr = std::move(this_ptr), ecs = std::move(ecs)]
			(event_queue_guard<derived_t> g) mutable
			{
				derive.sessions_.dispatch(
				[&derive, this_ptr, ecs = std::move(ecs), g = std::move(g)]
				() mutable
				{
					derive._handle_connect(
						error_code{}, std::move(this_ptr), std::move(ecs), defer_event(std::move(g)));
				});
			});
		}

	public:
		/**
		 * @brief stop session
		 * You can call this function in the communication thread and anywhere to stop the session.
		 * If this function is called in the communication thread, it will post a asynchronous
		 * event into the event queue, then return immediately.
		 * If this function is called not in the communication thread, it will blocking forever
		 * util the session is stopped completed.
		 * note : this function must be noblocking if it is called in the communication thread,
		 * otherwise if it's blocking, maybe cause circle lock.
		 * If the session stop is called in the server's bind connect callback, then the session
		 * will can't be added into the session manager, and the session's bind disconnect event
		 * can't be called also.
		 */
		inline void stop()
		{
			derived_t& derive = this->derived();

			state_t expected = state_t::stopped;
			if (this->state_.compare_exchange_strong(expected, state_t::stopped))
				return;

			expected = state_t::stopping;
			if (this->state_.compare_exchange_strong(expected, state_t::stopping))
				return;

			// use promise to get the result of stop
			std::promise<state_t> promise;
			std::future<state_t> future = promise.get_future();

			// use derfer to ensure the promise's value must be seted.
			detail::defer_event pg
			{
				[this, p = std::move(promise)]() mutable
				{
					p.set_value(this->state_.load());
				}
			};

			derive.post_event([&derive, this_ptr = derive.selfptr(), pg = std::move(pg)]
			(event_queue_guard<derived_t> g) mutable
			{
				derive._do_disconnect(asio::error::operation_aborted, derive.selfptr(), defer_event
				{
					[&derive, this_ptr = std::move(this_ptr), pg = std::move(pg)]
					(event_queue_guard<derived_t> g) mutable
					{
						detail::ignore_unused(derive, pg, g);

						// the "pg" should destroyed before the "g", otherwise if the "g"
						// is destroyed before "pg", the next event maybe called, then the
						// state maybe change to not stopped.
						{
							[[maybe_unused]] detail::defer_event t{ std::move(pg) };
						}
					}, std::move(g)
				});
			});

			// use this to ensure the client is stopped completed when the stop is called not in the io_context thread
			while (!derive.running_in_this_thread() && !derive.sessions_.io_->running_in_this_thread())
			{
				std::future_status status = future.wait_for(std::chrono::milliseconds(100));

				if (status == std::future_status::ready)
				{
					ASIO2_ASSERT(future.get() == state_t::stopped);
					break;
				}
				else
				{
					if (derive.get_thread_id() == std::thread::id{})
						break;

					if (derive.sessions_.io_->get_thread_id() == std::thread::id{})
						break;

					if (derive.io_->context().stopped())
						break;
				}
			}
		}

		/**
		 * @brief check whether the session is stopped
		 */
		inline bool is_stopped() const noexcept
		{
			return (this->state_ == state_t::stopped);
		}

	public:
		/**
		 * @brief get the remote address
		 */
		inline std::string get_remote_address() const noexcept
		{
			try
			{
				return this->remote_endpoint_.address().to_string();
			}
			catch (system_error & e) { set_last_error(e); }
			return std::string();
		}
		/**
		 * @brief get the remote address, same as get_remote_address
		 */
		inline std::string remote_address() const noexcept
		{
			return this->get_remote_address();
		}

		/**
		 * @brief get the remote port
		 */
		inline unsigned short get_remote_port() const noexcept
		{
			return this->remote_endpoint_.port();
		}
		/**
		 * @brief get the remote port, same as get_remote_port
		 */
		inline unsigned short remote_port() const noexcept
		{
			return this->get_remote_port();
		}

		/**
		 * @brief get this object hash key,used for session map
		 */
		inline key_type hash_key() const noexcept
		{
			// after test, there are a lot of hash collisions for asio::ip::udp::endpoint.
			// so the map key can't be the hash result of asio::ip::udp::endpoint, it must
			// be the asio::ip::udp::endpoint itself.
			return this->remote_endpoint_;
		}

		/**
		 * @brief get the kcp stream component
		 */
		inline kcp_stream_cp<derived_t, args_t>* get_kcp_stream() noexcept
		{
			return this->kcp_stream_.get();
		}

		/**
		 * @brief get the kcp stream component
		 */
		inline const kcp_stream_cp<derived_t, args_t>* get_kcp_stream() const noexcept
		{
			return this->kcp_stream_.get();
		}

		/**
		 * @brief get the kcp pointer, just used for kcp mode
		 * default mode : ikcp_nodelay(kcp, 0, 10, 0, 0);
		 * generic mode : ikcp_nodelay(kcp, 0, 10, 0, 1);
		 * fast    mode : ikcp_nodelay(kcp, 1, 10, 2, 1);
		 */
		inline kcp::ikcpcb* get_kcp() noexcept
		{
			return (this->kcp_stream_ ? this->kcp_stream_->kcp_ : nullptr);
		}

		/**
		 * @brief get the kcp pointer, just used for kcp mode
		 * default mode : ikcp_nodelay(kcp, 0, 10, 0, 0);
		 * generic mode : ikcp_nodelay(kcp, 0, 10, 0, 1);
		 * fast    mode : ikcp_nodelay(kcp, 1, 10, 2, 1);
		 */
		inline const kcp::ikcpcb* get_kcp() const noexcept
		{
			return (this->kcp_stream_ ? this->kcp_stream_->kcp_ : nullptr);
		}

		/**
		 * @brief get the kcp pointer, just used for kcp mode. same as get_kcp
		 * default mode : ikcp_nodelay(kcp, 0, 10, 0, 0);
		 * generic mode : ikcp_nodelay(kcp, 0, 10, 0, 1);
		 * fast    mode : ikcp_nodelay(kcp, 1, 10, 2, 1);
		 */
		inline kcp::ikcpcb* kcp() noexcept
		{
			return this->get_kcp();
		}

		/**
		 * @brief get the kcp pointer, just used for kcp mode. same as get_kcp
		 * default mode : ikcp_nodelay(kcp, 0, 10, 0, 0);
		 * generic mode : ikcp_nodelay(kcp, 0, 10, 0, 1);
		 * fast    mode : ikcp_nodelay(kcp, 1, 10, 2, 1);
		 */
		inline const kcp::ikcpcb* kcp() const noexcept
		{
			return this->get_kcp();
		}

	protected:
		template<typename C>
		inline void _do_init(std::shared_ptr<derived_t>& this_ptr, std::shared_ptr<ecs_t<C>>& ecs)
		{
			detail::ignore_unused(this_ptr, ecs);

			// reset the variable to default status
			this->derived().reset_connect_time();
			this->derived().update_alive_time();
		}

		template<typename C, typename DeferEvent>
		inline void _handle_connect(
			const error_code& ec, std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs,
			DeferEvent chain)
		{
			detail::ignore_unused(ec);

			ASIO2_ASSERT(!ec);
			ASIO2_ASSERT(this->derived().sessions_.io_->running_in_this_thread());

			if constexpr (std::is_same_v<typename ecs_t<C>::condition_lowest_type, use_kcp_t>)
			{
				std::string& data = *(this->first_data_);

				// step 3 : server recvd syn from client (the first data is syn)
				// Check whether the first data packet is SYN handshake
				if (!kcp::is_kcphdr_syn(data))
				{
					set_last_error(asio::error::address_family_not_supported);

					this->derived()._fire_handshake(this_ptr);
					this->derived()._do_disconnect(asio::error::address_family_not_supported,
						std::move(this_ptr), std::move(chain));

					return;
				}

				this->kcp_stream_ = std::make_unique<kcp_stream_cp<derived_t, args_t>>(
					this->derived(), this->io_->context());
				this->kcp_stream_->_post_handshake(std::move(this_ptr), std::move(ecs), std::move(chain));
			}
			else
			{
				this->kcp_stream_.reset();
				this->derived()._done_connect(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
			}
		}

		template<typename C, typename DeferEvent>
		inline void _do_start(
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			derived_t& derive = this->derived();

			if constexpr (std::is_same_v<typename ecs_t<C>::condition_lowest_type, use_kcp_t>)
			{
				ASIO2_ASSERT(this->kcp_stream_);

				if (this->kcp_stream_)
					this->kcp_stream_->send_fin_ = true;
			}
			else
			{
				ASIO2_ASSERT(!this->kcp_stream_);
			}

			derive.post_event(
			[&derive, this_ptr = std::move(this_ptr), ecs = std::move(ecs), e = chain.move_event()]
			(event_queue_guard<derived_t> g) mutable
			{
				defer_event chain(std::move(e), std::move(g));

				if (!derive.is_started())
				{
					derive._do_disconnect(asio::error::operation_aborted, std::move(this_ptr), std::move(chain));
					return;
				}

				derive._join_session(std::move(this_ptr), std::move(ecs), std::move(chain));
			});
		}

		template<typename DeferEvent>
		inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			ASIO2_ASSERT(this->derived().io_->running_in_this_thread());
			ASIO2_ASSERT(this->state_ == state_t::stopped);
			ASIO2_ASSERT(this->reading_ == false);

			set_last_error(ec);

			this->derived()._rdc_stop();

			super::_handle_disconnect(ec, std::move(this_ptr), std::move(chain));
		}

		template<typename DeferEvent>
		inline void _do_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			this->derived()._post_stop(ec, std::move(this_ptr), std::move(chain));
		}

		template<typename DeferEvent>
		inline void _post_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			// if we use asio::dispatch in server's _exec_stop, then we need:
			// put _kcp_stop in front of super::stop, othwise the super::stop will execute
			// "counter_ptr_.reset()", it will cause the udp server's _exec_stop is called,
			// and the _handle_stop is called, and the socket will be closed, then the 
			// _kcp_stop send kcphdr will failed.
			// but if we use asio::post in server's _exec_stop, there is no such problem.
			if (this->kcp_stream_)
				this->kcp_stream_->_kcp_stop();

			// call the base class stop function
			super::stop();

			// call CRTP polymorphic stop
			this->derived()._handle_stop(ec, std::move(this_ptr), std::move(chain));
		}

		template<typename DeferEvent>
		inline void _handle_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			detail::ignore_unused(ec, this_ptr, chain);

			ASIO2_ASSERT(this->state_ == state_t::stopped);
		}

		template<typename C, typename DeferEvent>
		inline void _join_session(
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			this->sessions_.emplace(this_ptr,
			[this, this_ptr, ecs = std::move(ecs), chain = std::move(chain)]
			(bool inserted) mutable
			{
				if (inserted)
					this->derived()._start_recv(std::move(this_ptr), std::move(ecs), std::move(chain));
				else
					this->derived()._do_disconnect(asio::error::address_in_use, std::move(this_ptr), std::move(chain));
			});
		}

		template<typename C, typename DeferEvent>
		inline void _start_recv(
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			// to avlid the user call stop in another thread,then it may be socket.async_read_some
			// and socket.close be called at the same time
			asio::dispatch(this->io_->context(), make_allocator(this->wallocator_,
			[this, this_ptr = std::move(this_ptr), ecs = std::move(ecs), chain = std::move(chain)]
			() mutable
			{
				using condition_lowest_type = typename ecs_t<C>::condition_lowest_type;

				detail::ignore_unused(chain);

				// start the timer of check silence timeout
				this->derived()._post_silence_timer(this->silence_timeout_, this_ptr);

				if constexpr (std::is_same_v<condition_lowest_type, asio2::detail::use_kcp_t>)
				{
					detail::ignore_unused(this_ptr, ecs);
				}
				else
				{
					std::string& data = *(this->first_data_);

					this->derived()._fire_recv(this_ptr, ecs, data);
				}

				this->first_data_.reset();

				ASIO2_ASSERT(!this->first_data_);
				ASIO2_ASSERT(this->reading_ == false);
			}));
		}

	protected:
		template<class Data, class Callback>
		inline bool _do_send(Data& data, Callback&& callback)
		{
			if (!this->kcp_stream_)
				return this->derived()._udp_send_to(
					this->remote_endpoint_, data, std::forward<Callback>(callback));
			return this->kcp_stream_->_kcp_send(data, std::forward<Callback>(callback));
		}

		template<class Data>
		inline send_data_t _rdc_convert_to_send_data(Data& data) noexcept
		{
			auto buffer = asio::buffer(data);
			return send_data_t{ reinterpret_cast<
				std::string_view::const_pointer>(buffer.data()),buffer.size() };
		}

		template<class Invoker>
		inline void _rdc_invoke_with_none(const error_code& ec, Invoker& invoker)
		{
			if (invoker)
				invoker(ec, send_data_t{}, recv_data_t{});
		}

		template<class Invoker>
		inline void _rdc_invoke_with_recv(const error_code& ec, Invoker& invoker, recv_data_t data)
		{
			if (invoker)
				invoker(ec, send_data_t{}, data);
		}

		template<class Invoker>
		inline void _rdc_invoke_with_send(const error_code& ec, Invoker& invoker, send_data_t data)
		{
			if (invoker)
				invoker(ec, data, recv_data_t{});
		}

	protected:
		// this function will can't be called forever
		template<typename C>
		inline void _post_recv(std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			ASIO2_ASSERT(false);
			this->derived()._udp_post_recv(std::move(this_ptr), std::move(ecs));
		}

		template<typename C>
		inline void _handle_recv(
			const error_code& ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t>& this_ptr, std::shared_ptr<ecs_t<C>>& ecs)
		{
			this->derived()._udp_handle_recv(ec, bytes_recvd, this_ptr, ecs);
		}

		template<typename C>
		inline void _fire_recv(
			std::shared_ptr<derived_t>& this_ptr, std::shared_ptr<ecs_t<C>>& ecs, std::string_view data)
		{
			data = detail::call_data_filter_before_recv(this->derived(), data);

			this->listener_.notify(event_type::recv, this_ptr, data);

			this->derived()._rdc_handle_recv(this_ptr, ecs, data);
		}

		inline void _fire_handshake(std::shared_ptr<derived_t>& this_ptr)
		{
			// the _fire_handshake must be executed in the thread 0.
			ASIO2_ASSERT(this->sessions_.io_->running_in_this_thread());

			this->listener_.notify(event_type::handshake, this_ptr);
		}

		template<typename C>
		inline void _fire_connect(std::shared_ptr<derived_t>& this_ptr, std::shared_ptr<ecs_t<C>>& ecs)
		{
			// the _fire_connect must be executed in the thread 0.
			ASIO2_ASSERT(this->sessions_.io_->running_in_this_thread());

		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(this->is_disconnect_called_ == false);
		#endif

			this->derived()._rdc_start(this_ptr, ecs);

			this->listener_.notify(event_type::connect, this_ptr);
		}

		inline void _fire_disconnect(std::shared_ptr<derived_t>& this_ptr)
		{
			// the _fire_disconnect must be executed in the thread 0.
			ASIO2_ASSERT(this->sessions_.io_->running_in_this_thread());

		#if defined(_DEBUG) || defined(DEBUG)
			this->is_disconnect_called_ = true;
		#endif

			this->listener_.notify(event_type::disconnect, this_ptr);
		}

	protected:
		/**
		 * @brief get the recv/read allocator object reference
		 */
		inline auto & rallocator() noexcept { return this->wallocator_; }
		/**
		 * @brief get the send/write allocator object reference
		 */
		inline auto & wallocator() noexcept { return this->wallocator_; }

	protected:
		/// The memory to use for handler-based custom memory allocation. used fo send/write.
		handler_memory<std::false_type, assizer<args_t>>  wallocator_;

		std::unique_ptr<kcp_stream_cp<derived_t, args_t>> kcp_stream_;

		std::uint32_t                                     kcp_conv_ = 0;

		/// first recvd data packet
		std::unique_ptr<std::string>                      first_data_;

	#if defined(_DEBUG) || defined(DEBUG)
		bool                                              is_disconnect_called_ = false;
	#endif
	};
}

namespace asio2
{
	using udp_session_args = detail::template_args_udp_session;

	template<class derived_t, class args_t>
	using udp_session_impl_t = detail::udp_session_impl_t<derived_t, args_t>;

	template<class derived_t>
	class udp_session_t : public detail::udp_session_impl_t<derived_t, detail::template_args_udp_session>
	{
	public:
		using detail::udp_session_impl_t<derived_t, detail::template_args_udp_session>::udp_session_impl_t;
	};

	class udp_session : public udp_session_t<udp_session>
	{
	public:
		using udp_session_t<udp_session>::udp_session_t;
	};
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_UDP_SESSION_HPP__
