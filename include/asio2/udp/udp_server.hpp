/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_UDP_SERVER_HPP__
#define __ASIO2_UDP_SERVER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <asio2/base/server.hpp>
#include <asio2/udp/udp_session.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_UDP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_UDP_SERVER;

	template<class derived_t, class session_t>
	class udp_server_impl_t : public server_impl_t<derived_t, session_t>, public udp_tag
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_UDP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_UDP_SERVER;

	public:
		using super = server_impl_t    <derived_t, session_t>;
		using self  = udp_server_impl_t<derived_t, session_t>;

		using session_type = session_t;

	public:
		/**
		 * @brief constructor
		 */
		explicit udp_server_impl_t(
			std::size_t init_buf_size = udp_frame_size,
			std::size_t max_buf_size  = max_buffer_size,
			std::size_t concurrency   = 1
		)
			: super(concurrency)
			, acceptor_(std::make_shared<asio::ip::udp::socket>(this->io_->context()))
			, remote_endpoint_()
			, buffer_(init_buf_size, max_buf_size)
		{
		}

		template<class Scheduler, std::enable_if_t<!std::is_integral_v<detail::remove_cvref_t<Scheduler>>, int> = 0>
		explicit udp_server_impl_t(
			std::size_t init_buf_size,
			std::size_t max_buf_size,
			Scheduler&& scheduler
		)
			: super(std::forward<Scheduler>(scheduler))
			, acceptor_(std::make_shared<asio::ip::udp::socket>(this->io_->context()))
			, remote_endpoint_()
			, buffer_(init_buf_size, max_buf_size)
		{
		}

		template<class Scheduler, std::enable_if_t<!std::is_integral_v<detail::remove_cvref_t<Scheduler>>, int> = 0>
		explicit udp_server_impl_t(Scheduler&& scheduler)
			: udp_server_impl_t(udp_frame_size, max_buffer_size, std::forward<Scheduler>(scheduler))
		{
		}

		/**
		 * @brief destructor
		 */
		~udp_server_impl_t()
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
		inline bool start(String&& host, StrOrInt&& service, Args&&... args)
		{
			return this->derived()._do_start(
				std::forward<String>(host), std::forward<StrOrInt>(service),
				ecs_helper::make_ecs('0', std::forward<Args>(args)...));
		}

		/**
		 * @brief stop the server
		 * You can call this function on the communication thread and anywhere to stop the server.
		 */
		inline void stop()
		{
			if (this->is_iopool_stopped())
				return;

			derived_t& derive = this->derived();

			derive.io_->unregobj(&derive);

			derive.post([&derive]() mutable
			{
				derive._do_stop(asio::error::operation_aborted, derive.selfptr());
			});

			this->stop_iopool();
		}

		/**
		 * @brief destroy the content of all member variables, this is used for solve the memory leaks.
		 * After this function is called, this class object cannot be used again.
		 */
		inline void destroy()
		{
			derived_t& derive = this->derived();

			derive.acceptor_.reset();

			super::destroy();
		}

		/**
		 * @brief check whether the server is started
		 */
		inline bool is_started() const { return (super::is_started() && this->acceptor_->is_open()); }

		/**
		 * @brief check whether the server is stopped
		 */
		inline bool is_stopped() const
		{
			return (this->state_ == state_t::stopped && !this->acceptor_->is_open());
		}

	public:
		/**
		 * @brief bind recv listener
		 * @param fun - a user defined callback function.
		 * @param obj - a pointer or reference to a class object, this parameter can be none.
		 * @li if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or reference.
		 * Function signature : void(std::shared_ptr<asio2::udp_session>& session_ptr, std::string_view data)
		 */
		template<class F, class ...C>
		inline derived_t & bind_recv(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::recv,
				observer_t<std::shared_ptr<session_t>&, std::string_view>(
					std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

		/**
		 * @brief bind connect listener
		 * @param fun - a user defined callback function.
		 * @param obj - a pointer or reference to a class object, this parameter can be none.
		 * @li if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or reference.
		 * This notification is invoked after the connection is fully established,
		 * and only after the connect/handshake/upgrade are completed.
		 * Function signature : void(std::shared_ptr<asio2::udp_session>& session_ptr)
		 */
		template<class F, class ...C>
		inline derived_t & bind_connect(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::connect,
				observer_t<std::shared_ptr<session_t>&>(
					std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

		/**
		 * @brief bind disconnect listener
		 * @param fun - a user defined callback function.
		 * @param obj - a pointer or reference to a class object, this parameter can be none.
		 * @li if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or reference.
		 * This notification is invoked before the connection is disconnected, you can call
		 * get_last_error/last_error_msg, etc, to get the disconnected error information
		 * Function signature : void(std::shared_ptr<asio2::udp_session>& session_ptr)
		 */
		template<class F, class ...C>
		inline derived_t & bind_disconnect(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::disconnect,
				observer_t<std::shared_ptr<session_t>&>(
					std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

		/**
		 * @brief bind init listener,we should set socket options at here
		 * @param fun - a user defined callback function.
		 * @param obj - a pointer or reference to a class object, this parameter can be none.
		 * @li if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or reference.
		 * This notification is called after the socket is opened.
		 * You can set the socket option in this notification.
		 * Function signature : void()
		 */
		template<class F, class ...C>
		inline derived_t & bind_init(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::init, observer_t<>(
				std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

		/**
		 * @brief bind start listener
		 * @param fun - a user defined callback function.
		 * @param obj - a pointer or reference to a class object, this parameter can be none.
		 * @li if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or reference.
		 * This notification is called after the server starts up, whether successful or unsuccessful
		 * Function signature : void()
		 */
		template<class F, class ...C>
		inline derived_t & bind_start(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::start, observer_t<>(
				std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

		/**
		 * @brief bind stop listener
		 * @param fun - a user defined callback function.
		 * @param obj - a pointer or reference to a class object, this parameter can be none.
		 * @li if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or reference.
		 * This notification is called before the server is ready to stop
		 * Function signature : void()
		 */
		template<class F, class ...C>
		inline derived_t & bind_stop(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::stop, observer_t<>(
				std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

		/**
		 * @brief bind kcp handshake listener, just used fo kcp mode
		 * @param fun - a user defined callback function.
		 * @param obj - a pointer or reference to a class object, this parameter can be none.
		 * @li if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or reference.
		 * Function signature : void(std::shared_ptr<asio2::udp_session>& session_ptr)
		 */
		template<class F, class ...C>
		inline derived_t & bind_handshake(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::handshake,
				observer_t<std::shared_ptr<session_t>&>(
					std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	public:
		/**
		 * @brief get the acceptor reference
		 */
		inline asio::ip::udp::socket      & acceptor()       noexcept { return *(this->acceptor_); }
		/**
		 * @brief get the acceptor reference
		 */
		inline asio::ip::udp::socket const& acceptor() const noexcept { return *(this->acceptor_); }

	protected:
		template<typename String, typename StrOrInt, typename C>
		inline bool _do_start(String&& host, StrOrInt&& port, std::shared_ptr<ecs_t<C>> ecs)
		{
			derived_t& derive = this->derived();

			// if log is enabled, init the log first, otherwise when "Too many open files" error occurs,
			// the log file will be created failed too.
		#if defined(ASIO2_ENABLE_LOG)
			asio2::detail::get_logger();
		#endif

			this->start_iopool();

			if (!this->is_iopool_started())
			{
				set_last_error(asio::error::operation_aborted);
				return false;
			}

			asio::dispatch(derive.io_->context(), [&derive, this_ptr = derive.selfptr()]() mutable
			{
				detail::ignore_unused(this_ptr);

				// init the running thread id 
				derive.io_->init_thread_id();
			});

			// use promise to get the result of async accept
			std::promise<error_code> promise;
			std::future<error_code> future = promise.get_future();

			// use derfer to ensure the promise's value must be seted.
			detail::defer_event pg
			{
				[promise = std::move(promise)]() mutable
				{
					promise.set_value(get_last_error());
				}
			};

			derive.post(
			[this, this_ptr = derive.selfptr(), ecs = std::move(ecs), pg = std::move(pg),
				host = std::forward<String>(host), port = std::forward<StrOrInt>(port)]
			() mutable
			{
				derived_t& derive = this->derived();

				state_t expected = state_t::stopped;
				if (!this->state_.compare_exchange_strong(expected, state_t::starting))
				{
					// if the state is not stopped, set the last error to already_started
					set_last_error(asio::error::already_started);

					return;
				}

				// must read/write ecs in the io_context thread.
				derive.ecs_ = ecs;

				derive.io_->regobj(&derive);

			#if defined(_DEBUG) || defined(DEBUG)
				this->sessions_.is_all_session_stop_called_ = false;
				this->is_stop_called_ = false;
			#endif

				// convert to string maybe throw some exception.
				std::string h = detail::to_string(std::move(host));
				std::string p = detail::to_string(std::move(port));

				expected = state_t::starting;
				if (!this->state_.compare_exchange_strong(expected, state_t::starting))
				{
					ASIO2_ASSERT(false);
					derive._handle_start(asio::error::operation_aborted, std::move(this_ptr), std::move(ecs));
					return;
				}

				super::start();

				// should hold the server shared ptr too, if server is constructed with iopool, and 
				// server is a tmp local variable, then the server maybe destroyed before sessions.
				// so we need hold this ptr to ensure server must be destroyed after sessions.
				this->counter_ptr_ = std::shared_ptr<void>((void*)1, [&derive, this_ptr](void*) mutable
				{
					derive._exec_stop(asio::error::operation_aborted, std::move(this_ptr));
				});

				error_code ec, ec_ignore;

				// parse address and port
				asio::ip::udp::resolver resolver(this->io_->context());

				auto results = resolver.resolve(h, p,
					asio::ip::resolver_base::flags::passive |
					asio::ip::resolver_base::flags::address_configured, ec);
				if (ec)
				{
					derive._handle_start(ec, std::move(this_ptr), std::move(ecs));
					return;
				}
				if (results.empty())
				{
					derive._handle_start(asio::error::host_not_found, std::move(this_ptr), std::move(ecs));
					return;
				}

				asio::ip::udp::endpoint endpoint = *results.begin();

				this->acceptor_->cancel(ec_ignore);
				this->acceptor_->close(ec_ignore);
				this->acceptor_->open(endpoint.protocol(), ec);
				if (ec)
				{
					derive._handle_start(ec, std::move(this_ptr), std::move(ecs));
					return;
				}

				// when you close socket in linux system,and start socket
				// immediate,you will get like this "the address is in use",
				// and bind is failed,but i'm suer i close the socket correct
				// already before,why does this happen? the reasion is the
				// socket option "TIME_WAIT",although you close the socket,
				// but the system not release the socket,util 2~4 seconds later,
				// so we can use the SO_REUSEADDR option to avoid this problem,
				// like below

				// set port reuse
				this->acceptor_->set_option(asio::ip::udp::socket::reuse_address(true), ec_ignore);

				//// Join the multicast group. you can set this option in the on_init(_fire_init) function.
				//this->acceptor_->set_option(
				//	// for ipv6, the host must be a ipv6 address like 0::0
				//	asio::ip::multicast::join_group(asio::ip::make_address("ff31::8000:1234")));
				//	// for ipv4, the host must be a ipv4 address like 0.0.0.0
				//	//asio::ip::multicast::join_group(asio::ip::make_address("239.255.0.1")));

				clear_last_error();

				derive._fire_init();

				this->acceptor_->bind(endpoint, ec);
				if (ec)
				{
					derive._handle_start(ec, std::move(this_ptr), std::move(ecs));
					return;
				}

				derive._handle_start(ec, std::move(this_ptr), std::move(ecs));
			});

			if (!derive.io_->running_in_this_thread())
			{
				set_last_error(future.get());

				return static_cast<bool>(!get_last_error());
			}
			else
			{
				set_last_error(asio::error::in_progress);
			}

			// if the state is stopped , the return value is "is_started()".
			// if the state is stopping, the return value is false, the last error is already_started
			// if the state is starting, the return value is false, the last error is already_started
			// if the state is started , the return value is true , the last error is already_started
			return derive.is_started();
		}

		template<typename C>
		inline void _handle_start(error_code ec, std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			ASIO2_ASSERT(this->derived().io_->running_in_this_thread());

			// Whether the startup succeeds or fails, always call fire_start notification
			state_t expected = state_t::starting;
			if (!ec)
				if (!this->state_.compare_exchange_strong(expected, state_t::started))
					ec = asio::error::operation_aborted;

			set_last_error(ec);

			this->derived()._fire_start();

			expected = state_t::started;
			if (!ec)
				if (!this->state_.compare_exchange_strong(expected, state_t::started))
					ec = asio::error::operation_aborted;

			if (ec)
			{
				this->derived()._do_stop(ec, std::move(this_ptr));

				return;
			}

			this->buffer_.consume(this->buffer_.size());

			this->derived()._post_recv(std::move(this_ptr), std::move(ecs));
		}

		inline void _do_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			ASIO2_ASSERT(this->derived().io_->running_in_this_thread());

			state_t expected = state_t::starting;
			if (this->state_.compare_exchange_strong(expected, state_t::stopping))
				return this->derived()._post_stop(ec, std::move(this_ptr), expected);

			expected = state_t::started;
			if (this->state_.compare_exchange_strong(expected, state_t::stopping))
				return this->derived()._post_stop(ec, std::move(this_ptr), expected);
		}

		inline void _post_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr, state_t old_state)
		{
			// asio don't allow operate the same socket in multi thread,
			// if you close socket in one thread and another thread is 
			// calling socket's async_... function,it will crash.so we
			// must care for operate the socket.when need close the
			// socket ,we use the io_context to post a event,make sure the
			// socket's close operation is in the same thread.

			// psot a recv signal to ensure that all recv events has finished already.
			this->derived().post(
			[this, ec, old_state, this_ptr = std::move(this_ptr)]() mutable
			{
				detail::ignore_unused(this, ec, old_state, this_ptr);

				// When the code runs here,no new session can be emplace or erase to session_mgr.
				// stop all the sessions, the session::stop must be no blocking,
				// otherwise it may be cause loop lock.
				set_last_error(ec);

				ASIO2_ASSERT(this->state_ == state_t::stopping);

				// stop all the sessions, the session::stop must be no blocking,
				// otherwise it may be cause loop lock.
				this->sessions_.quick_for_each([](std::shared_ptr<session_t> & session_ptr) mutable
				{
					session_ptr->stop();
				});

			#if defined(_DEBUG) || defined(DEBUG)
				this->sessions_.is_all_session_stop_called_ = true;
			#endif

				if (this->counter_ptr_)
				{
					this->counter_ptr_.reset();
				}
				else
				{
					this->derived()._exec_stop(ec, std::move(this_ptr));
				}
			});
		}

		inline void _exec_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			// use asio::post to ensure this server's _handle_stop is called must be after 
			// all sessions _handle_stop has been called already.
			// is use asio::dispatch, session's _handle_stop maybe called first.
			asio::post(this->derived().io_->context(), make_allocator(this->derived().wallocator(),
			[this, ec, this_ptr = std::move(this_ptr)]() mutable
			{
				state_t expected = state_t::stopping;
				if (this->state_.compare_exchange_strong(expected, state_t::stopped))
				{
					this->derived()._handle_stop(ec, std::move(this_ptr));
				}
				else
				{
					ASIO2_ASSERT(false);
				}
			}));
		}

		inline void _handle_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			set_last_error(ec);

			this->derived()._fire_stop();

			// call the base class stop function
			super::stop();

			error_code ec_ignore{};

			// Call shutdown() to indicate that you will not write any more data to the socket.
			this->acceptor_->shutdown(asio::socket_base::shutdown_both, ec_ignore);

			this->acceptor_->cancel(ec_ignore);

			// Call close,otherwise the _handle_recv will never return
			this->acceptor_->close(ec_ignore);

			ASIO2_ASSERT(this->state_ == state_t::stopped);
		}

		template<typename C>
		inline void _post_recv(std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			if (!this->derived().is_started())
			{
				if (this->derived().state_ == state_t::started)
				{
					this->derived()._do_stop(asio2::get_last_error(), std::move(this_ptr));
				}
				return;
			}

		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(this->derived().post_recv_counter_.load() == 0);
			this->derived().post_recv_counter_++;
		#endif

			this->acceptor_->async_receive_from(
				this->buffer_.prepare(this->buffer_.pre_size()),
				this->remote_endpoint_,
				make_allocator(this->rallocator_, [this, this_ptr = std::move(this_ptr), ecs = std::move(ecs)]
			(const error_code& ec, std::size_t bytes_recvd) mutable
			{
			#if defined(_DEBUG) || defined(DEBUG)
				this->derived().post_recv_counter_--;
			#endif

				this->derived()._handle_recv(ec, bytes_recvd, std::move(this_ptr), std::move(ecs));
			}));
		}

		template<typename C>
		inline void _handle_recv(
			const error_code& ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			set_last_error(ec);

			if (!this->derived().is_started())
			{
				if (this->derived().state_ == state_t::started)
				{
					this->derived()._do_stop(ec, std::move(this_ptr));
				}
				return;
			}

			if (ec == asio::error::operation_aborted)
			{
				this->derived()._do_stop(ec, std::move(this_ptr));
				return;
			}

			this->buffer_.commit(bytes_recvd);

			if (!ec)
			{
				std::string_view data = std::string_view(static_cast<std::string_view::const_pointer>
					(this->buffer_.data().data()), bytes_recvd);

				// first we find whether the session is in the session_mgr pool already,if not ,
				// we new a session and put it into the session_mgr pool
				std::shared_ptr<session_t> session_ptr = this->sessions_.find(this->remote_endpoint_);
				if (!session_ptr)
				{
					this->derived()._handle_accept(ec, data, session_ptr, ecs);
				}
				else
				{
					session_ptr->_handle_recv(ec, bytes_recvd, session_ptr, ecs);
				}
			}
			else
			{
			#ifdef ASIO2_STOP_SESSION_WHEN_RECVD_0BYTES
				// has error, and bytes_recvd == 0
				if (bytes_recvd == 0)
				{
					std::shared_ptr<session_t> session_ptr = this->sessions_.find(this->remote_endpoint_);
					if (session_ptr)
					{
						ASIO2_LOG_INFOR("udp session stoped by recvd 0 bytes: {}",
							session_ptr->get_remote_address());

						session_ptr->stop();
					}
				}
			#endif
			}

			this->buffer_.consume(this->buffer_.size());

			if (bytes_recvd == this->buffer_.pre_size())
			{
				this->buffer_.pre_size((std::min)(this->buffer_.pre_size() * 2, this->buffer_.max_size()));
			}

			this->derived()._post_recv(std::move(this_ptr), std::move(ecs));
		}

		template<typename... Args>
		inline std::shared_ptr<session_t> _make_session(Args&&... args)
		{
			return std::make_shared<session_t>(
				std::forward<Args>(args)...,
				this->sessions_,
				this->listener_,
				this->io_,
				this->buffer_.pre_size(),
				this->buffer_.max_size(),
				this->buffer_,
				this->acceptor_,
				this->remote_endpoint_);
		}

		template<typename C>
		inline void _handle_accept(
			const error_code& ec, std::string_view first_data,
			std::shared_ptr<session_t> session_ptr, std::shared_ptr<ecs_t<C>>& ecs)
		{
			session_ptr = this->derived()._make_session();
			session_ptr->counter_ptr_ = this->counter_ptr_;
			session_ptr->first_data_ = std::make_unique<std::string>(first_data);
			session_ptr->kcp_conv_ = this->derived()._make_kcp_conv(first_data, ecs);
			session_ptr->start(detail::to_shared_ptr(ecs->clone()));
		}

		template<typename C>
		inline std::uint32_t _do_make_kcp_conv(std::string_view first_data, std::shared_ptr<ecs_t<C>>& ecs)
		{
			detail::ignore_unused(ecs);

			std::uint32_t conv = 0;

			if (kcp::is_kcphdr_syn(first_data))
			{
				kcp::kcphdr syn = kcp::to_kcphdr(first_data);

				// the syn.th_ack is the kcp conv
				if (syn.th_ack == 0)
				{
					conv = this->kcp_convs_.fetch_add(1);

					if (conv == 0)
						conv = this->kcp_convs_.fetch_add(1);
				}
				else
				{
					conv = syn.th_ack;
				}
			}

			return conv;
		}

		template<typename C>
		inline std::uint32_t _make_kcp_conv(std::string_view first_data, std::shared_ptr<ecs_t<C>>& ecs)
		{
			if constexpr (std::is_same_v<typename ecs_t<C>::condition_lowest_type, use_kcp_t>)
			{
				return this->_do_make_kcp_conv(first_data, ecs);
			}
			else
			{
				detail::ignore_unused(first_data, ecs);

				return 0;
			}
		}

		inline void _fire_init()
		{
			// the _fire_init must be executed in the thread 0.
			ASIO2_ASSERT(this->derived().io_->running_in_this_thread());
			ASIO2_ASSERT(!get_last_error());

			this->listener_.notify(event_type::init);
		}

		inline void _fire_start()
		{
			// the _fire_start must be executed in the thread 0.
			ASIO2_ASSERT(this->derived().io_->running_in_this_thread());

		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(this->is_stop_called_ == false);
		#endif

			this->listener_.notify(event_type::start);
		}

		inline void _fire_stop()
		{
			// the _fire_stop must be executed in the thread 0.
			ASIO2_ASSERT(this->derived().io_->running_in_this_thread());

		#if defined(_DEBUG) || defined(DEBUG)
			this->is_stop_called_ = true;
		#endif

			this->listener_.notify(event_type::stop);
		}

	protected:
		/// acceptor to accept client connection
		std::shared_ptr<asio::ip::udp::socket>   acceptor_;

		/// endpoint for udp 
		asio::ip::udp::endpoint                  remote_endpoint_;

		/// buffer
		asio2::buffer_wrap<asio2::linear_buffer> buffer_;

		std::atomic<std::uint32_t>               kcp_convs_ = 1;

	#if defined(_DEBUG) || defined(DEBUG)
		bool                    is_stop_called_  = false;
	#endif
	};
}

namespace asio2
{
	template<class derived_t, class session_t>
	using udp_server_impl_t = detail::udp_server_impl_t<derived_t, session_t>;

	/**
	 * @throws constructor maybe throw exception "Too many open files" (exception code : 24)
	 * asio::error::no_descriptors - Too many open files
	 */
	template<class session_t>
	class udp_server_t : public detail::udp_server_impl_t<udp_server_t<session_t>, session_t>
	{
	public:
		using detail::udp_server_impl_t<udp_server_t<session_t>, session_t>::udp_server_impl_t;
	};

	/**
	 * If this object is created as a shared_ptr like std::shared_ptr<asio2::udp_server> server;
	 * you must call the server->stop() manual when exit, otherwise maybe cause memory leaks.
	 * @throws constructor maybe throw exception "Too many open files" (exception code : 24)
	 * asio::error::no_descriptors - Too many open files
	 */
	using udp_server = udp_server_t<udp_session>;
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_UDP_SERVER_HPP__
