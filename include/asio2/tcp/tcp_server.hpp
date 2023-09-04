/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_TCP_SERVER_HPP__
#define __ASIO2_TCP_SERVER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <asio2/base/server.hpp>
#include <asio2/tcp/tcp_session.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;

	template<class derived_t, class session_t>
	class tcp_server_impl_t : public server_impl_t<derived_t, session_t>, public tcp_tag
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;

	public:
		using super = server_impl_t    <derived_t, session_t>;
		using self  = tcp_server_impl_t<derived_t, session_t>;

		using session_type = session_t;

	public:
		/**
		 * @brief constructor
		 */
		explicit tcp_server_impl_t(
			std::size_t init_buf_size = tcp_frame_size,
			std::size_t  max_buf_size = max_buffer_size,
			std::size_t   concurrency = default_concurrency() + 1 // The 1 is used for tcp acceptor
		)
			: super(concurrency)
			, acceptor_        (std::make_unique<asio::ip::tcp::acceptor>(this->io_->context()))
			, acceptor_timer_  (std::make_unique<asio::steady_timer>(this->io_->context()))
			, counter_timer_   (std::make_unique<asio::steady_timer>(this->io_->context()))
			, init_buffer_size_(init_buf_size)
			, max_buffer_size_ ( max_buf_size)
		{
		}

		template<class Scheduler, std::enable_if_t<!std::is_integral_v<detail::remove_cvref_t<Scheduler>>, int> = 0>
		explicit tcp_server_impl_t(
			std::size_t init_buf_size,
			std::size_t  max_buf_size,
			Scheduler&& scheduler
		)
			: super(std::forward<Scheduler>(scheduler))
			, acceptor_        (std::make_unique<asio::ip::tcp::acceptor>(this->io_->context()))
			, acceptor_timer_  (std::make_unique<asio::steady_timer>(this->io_->context()))
			, counter_timer_   (std::make_unique<asio::steady_timer>(this->io_->context()))
			, init_buffer_size_(init_buf_size)
			, max_buffer_size_ ( max_buf_size)
		{
		}

		template<class Scheduler, std::enable_if_t<!std::is_integral_v<detail::remove_cvref_t<Scheduler>>, int> = 0>
		explicit tcp_server_impl_t(Scheduler&& scheduler)
			: tcp_server_impl_t(tcp_frame_size, max_buffer_size, std::forward<Scheduler>(scheduler))
		{
		}

		// -- Support initializer_list causes the code of inherited classes to be not concised

		//template<class Scheduler, std::enable_if_t<!std::is_integral_v<detail::remove_cvref_t<Scheduler>>, int> = 0>
		//explicit tcp_server_impl_t(
		//	std::size_t init_buf_size,
		//	std::size_t  max_buf_size,
		//	std::initializer_list<Scheduler> scheduler
		//)
		//	: tcp_server_impl_t(init_buf_size, max_buf_size, std::vector<Scheduler>{std::move(scheduler)})
		//{
		//}

		//template<class Scheduler, std::enable_if_t<!std::is_integral_v<detail::remove_cvref_t<Scheduler>>, int> = 0>
		//explicit tcp_server_impl_t(std::initializer_list<Scheduler> scheduler)
		//	: tcp_server_impl_t(tcp_frame_size, max_buffer_size, std::move(scheduler))
		//{
		//}

		/**
		 * @brief destructor
		 */
		~tcp_server_impl_t()
		{
			this->stop();
		}

		/**
		 * @brief start the server
		 * @param host - A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param service - A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 * @param args - The delimiter condition.Valid value types include the following:
		 * char,std::string,std::string_view,
		 * function:std::pair<iterator, bool> match_condition(iterator begin, iterator end),
		 * asio::transfer_at_least,asio::transfer_exactly
		 * more details see asio::read_until
		 */
		template<typename String, typename StrOrInt, typename... Args>
		inline bool start(String&& host, StrOrInt&& service, Args&&... args)
		{
			return this->derived()._do_start(
				std::forward<String>(host), std::forward<StrOrInt>(service),
				ecs_helper::make_ecs(asio::transfer_at_least(1), std::forward<Args>(args)...));
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

			// asio bug , see : https://www.boost.org/users/history/version_1_72_0.html
			// Fixed a lost "outstanding work count" that can occur when an asynchronous 
			// accept operation is automatically restarted.
			// Using boost 1.72.0 or above version can avoid this problem (asio 1.16.0)
		}

		/**
		 * @brief destroy the content of all member variables, this is used for solve the memory leaks.
		 * After this function is called, this class object cannot be used again.
		 */
		inline void destroy()
		{
			derived_t& derive = this->derived();

			derive.counter_timer_.reset();
			derive.acceptor_timer_.reset();
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
		 * Function signature : void(std::shared_ptr<asio2::tcp_session>& session_ptr, std::string_view data)
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
		 * @brief bind accept listener
		 * @param fun - a user defined callback function.
		 * @param obj - a pointer or reference to a class object, this parameter can be none.
		 * @li if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or reference.
		 * This notification is invoked immediately when the server accept a new connection
		 * Function signature : void(std::shared_ptr<asio2::tcp_session>& session_ptr)
		 */
		template<class F, class ...C>
		inline derived_t & bind_accept(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::accept,
				observer_t<std::shared_ptr<session_t>&>(
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
		 * Function signature : void(std::shared_ptr<asio2::tcp_session>& session_ptr)
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
		 * Function signature : void(std::shared_ptr<asio2::tcp_session>& session_ptr)
		 * If is http or websocket server, when enter the disconnect callback, the socket of the
		 * session maybe closed already.
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

	public:
		/**
		 * @brief get the acceptor reference
		 */
		inline asio::ip::tcp::acceptor      & acceptor()       noexcept { return *(this->acceptor_); }

		/**
		 * @brief get the acceptor reference
		 */
		inline asio::ip::tcp::acceptor const& acceptor() const noexcept { return *(this->acceptor_); }

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

				this->counter_ptr_ = std::shared_ptr<void>((void*)1, [&derive](void*) mutable
				{
					derive._exec_stop(asio::error::operation_aborted, derive.selfptr());
				});

				error_code ec, ec_ignore;

				this->acceptor_->cancel(ec_ignore);
				this->acceptor_->close(ec_ignore);

				// parse address and port
				asio::ip::tcp::resolver resolver(this->io_->context());

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

				asio::ip::tcp::endpoint endpoint = *results.begin();

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
				// but the system not release the socket,util 2~4 seconds 
				// later,so we can use the SO_REUSEADDR option to avoid this
				// problem,like below
				
				// set port reuse
				this->acceptor_->set_option(asio::ip::tcp::acceptor::reuse_address(true), ec_ignore);

				clear_last_error();

				derive._fire_init();

				this->acceptor_->bind(endpoint, ec);
				if (ec)
				{
					derive._handle_start(ec, std::move(this_ptr), std::move(ecs));
					return;
				}

				this->acceptor_->listen(asio::socket_base::max_listen_connections, ec);
				if (ec)
				{
					derive._handle_start(ec, std::move(this_ptr), std::move(ecs));
					return;
				}

				// if the some error occured in the _fire_init notify function, the 
				// get_last_error maybe not zero, so if we use _handle_start(get_last_error()...
				// at here, the start will failed, and the user don't know what happend.
				// so we need use as this : _handle_start(error_code{}...
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

			this->derived()._post_accept(std::move(this_ptr), std::move(ecs));
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
			// must care for operate the socket. when need close the 
			// socket ,we use the io_context to post a event,make sure the
			// socket's close operation is in the same thread.
			asio::dispatch(this->derived().io_->context(), make_allocator(this->derived().wallocator(),
			[this, ec, old_state, this_ptr = std::move(this_ptr)]() mutable
			{
				detail::ignore_unused(this, ec, old_state, this_ptr);

				set_last_error(ec);

				ASIO2_ASSERT(this->state_ == state_t::stopping);

				// start timer to hold the acceptor io_context
				// should hold the server shared ptr too, if server is constructed with iopool, and 
				// server is a tmp local variable, then the server maybe destroyed before sessions.
				// so we need hold this ptr to ensure server must be destroyed after sessions.
				this->counter_timer_->expires_after((std::chrono::nanoseconds::max)());
				this->counter_timer_->async_wait([this_ptr](const error_code&)
				{
					detail::ignore_unused(this_ptr);
				});

				// stop all the sessions, the session::stop must be no blocking,
				// otherwise it may be cause loop lock.
				this->sessions_.quick_for_each([](std::shared_ptr<session_t> & session_ptr) mutable
				{
					session_ptr->stop();
				});

			#if defined(_DEBUG) || defined(DEBUG)
				// Check whether all sessions are evenly distributed in io threads
				std::vector<std::shared_ptr<io_t>> iots;
				std::vector<int> session_counter;

				iots.resize(this->iopool().size());
				session_counter.resize(this->iopool().size());

				for (std::size_t i = 0; i < iots.size(); ++i)
				{
					iots[i] = this->_get_io(i);
				}

				this->sessions_.quick_for_each([&iots, &session_counter](std::shared_ptr<session_t>& session_ptr) mutable
				{
					for (std::size_t i = 0; i < iots.size(); ++i)
					{
						if (session_ptr->io_ == iots[i])
						{
							session_counter[i]++;
							break;
						}
					}
				});

				if (iots.size() > std::size_t(2) && this->get_session_count() > ((iots.size() - 1) * 5))
				{
					ASIO2_ASSERT(session_counter[0] == 0);

					int count_diff = (std::max)(int(this->get_session_count() / (iots.size() - 1) / 10), 10);

					for (std::size_t i = 1; i < iots.size(); ++i)
					{
						ASIO2_ASSERT(std::abs(session_counter[1] - session_counter[i]) < count_diff);
					}
				}

				asio2::ignore_unused(iots, session_counter);
				asio2::ignore_unused(this->sessions_.empty()); // used to test ThreadSafetyAnalysis
			#endif

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
			}));
		}

		inline void _exec_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			// use asio::post to ensure this server's _handle_stop is called must be after 
			// all sessions _handle_stop has been called already.
			// if use asio::dispatch, session's _handle_stop maybe called first.
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

			detail::cancel_timer(*(this->acceptor_timer_));
			detail::cancel_timer(*(this->counter_timer_));

			// call the base class stop function
			super::stop();

			error_code ec_ignore{};

			// call acceptor's close function to notify the _handle_accept
			// function response with error > 0 , then the listen socket
			// can get notify to exit must ensure the close function has 
			// been called,otherwise the _handle_accept will never return
			this->acceptor_->cancel(ec_ignore);
			this->acceptor_->close(ec_ignore);

			ASIO2_ASSERT(this->state_ == state_t::stopped);
		}

		template<typename... Args>
		inline std::shared_ptr<session_t> _make_session(Args&&... args)
		{
			// skip zero io, the 0 io is used for acceptor.
			// but if the iopool size is 1, this io will be the zero io forever.
			std::shared_ptr<io_t> iot;

			if (this->iots_.size() > std::size_t(1))
			{
				iot = this->_get_io();

				if (iot == this->_get_io(0))
					iot = this->_get_io();
			}
			else
			{
				iot = this->_get_io();
			}

			return std::make_shared<session_t>(std::forward<Args>(args)...,
				this->sessions_, this->listener_, std::move(iot),
				this->init_buffer_size_, this->max_buffer_size_);
		}

		template<typename C>
		inline void _post_accept(std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			ASIO2_ASSERT(this->derived().io_->running_in_this_thread());

			if (!this->derived().is_started())
				return;

			std::shared_ptr<session_t> session_ptr = this->derived()._make_session();

		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(this->derived().post_recv_counter_.load() == 0);
			this->derived().post_recv_counter_++;
		#endif

			asio::io_context& ex = session_ptr->io_->context();

			this->acceptor_->async_accept(ex, make_allocator(this->rallocator_,
			[this, sptr = std::move(session_ptr), this_ptr = std::move(this_ptr), ecs = std::move(ecs)]
			(const error_code& ec, asio::ip::tcp::socket peer) mutable
			{
			#if defined(_DEBUG) || defined(DEBUG)
				this->derived().post_recv_counter_--;
			#endif

				sptr->socket().lowest_layer() = std::move(peer);

				this->derived()._handle_accept(ec, std::move(sptr), std::move(this_ptr), std::move(ecs));
			}));
		}

		template<typename C>
		inline void _handle_accept(
			const error_code& ec, std::shared_ptr<session_t> session_ptr,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			set_last_error(ec);

			// if the acceptor status is closed,don't call _post_accept again.
			if (ec == asio::error::operation_aborted)
				return;

			if (!this->derived().is_started())
				return;

			session_ptr->counter_ptr_ = this->counter_ptr_;
			session_ptr->start(detail::to_shared_ptr(ecs->clone()));

			// handle exception, may be is the exception "Too many open files" (exception code : 24)
			// asio::error::no_descriptors - Too many open files
			if (ec)
			{
				ASIO2_LOG_ERROR("Error occurred when accept:{} {}", ec.value(), ec.message());

				this->acceptor_timer_->expires_after(std::chrono::seconds(1));
				this->acceptor_timer_->async_wait(
				[this, this_ptr = std::move(this_ptr), ecs = std::move(ecs)]
				(const error_code& ec) mutable
				{
					if (ec == asio::error::operation_aborted)
						return;

					asio::post(this->io_->context(), make_allocator(this->wallocator(),
					[this, this_ptr = std::move(this_ptr), ecs = std::move(ecs)]() mutable
					{
						this->derived()._post_accept(std::move(this_ptr), std::move(ecs));
					}));
				});

				return;
			}

			this->derived()._post_accept(std::move(this_ptr), std::move(ecs));
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
		std::unique_ptr<asio::ip::tcp::acceptor> acceptor_;

		/// timer for acceptor exception, like the exception "Too many open files" (exception code : 24)
		std::unique_ptr<asio::steady_timer>      acceptor_timer_;

		/// used to hold the acceptor io_context util all sessions are closed already.
		std::unique_ptr<asio::steady_timer>      counter_timer_;

		std::size_t             init_buffer_size_ = tcp_frame_size;

		std::size_t             max_buffer_size_  = max_buffer_size;

	#if defined(_DEBUG) || defined(DEBUG)
		bool                    is_stop_called_  = false;
	#endif
	};
}

namespace asio2
{
	template<class derived_t, class session_t>
	using tcp_server_impl_t = detail::tcp_server_impl_t<derived_t, session_t>;

	/**
	 * @throws constructor maybe throw exception "Too many open files" (exception code : 24)
	 * asio::error::no_descriptors - Too many open files
	 */
	template<class session_t>
	class tcp_server_t : public detail::tcp_server_impl_t<tcp_server_t<session_t>, session_t>
	{
	public:
		using detail::tcp_server_impl_t<tcp_server_t<session_t>, session_t>::tcp_server_impl_t;
	};

	/**
	 * @brief tcp server
	 * If this object is created as a shared_ptr like std::shared_ptr<asio2::tcp_server> server;
	 * you must call the server->stop() manual when exit, otherwise maybe cause memory leaks.
	 * @throws constructor maybe throw exception "Too many open files" (exception code : 24)
	 * asio::error::no_descriptors - Too many open files
	 */
	using tcp_server = tcp_server_t<tcp_session>;
}

#if defined(ASIO2_INCLUDE_RATE_LIMIT)
#include <asio2/tcp/tcp_stream.hpp>
namespace asio2
{
	template<class session_t>
	class tcp_rate_server_t : public asio2::tcp_server_impl_t<tcp_rate_server_t<session_t>, session_t>
	{
	public:
		using asio2::tcp_server_impl_t<tcp_rate_server_t<session_t>, session_t>::tcp_server_impl_t;
	};

	using tcp_rate_server = tcp_rate_server_t<tcp_rate_session>;
}
#endif

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_TCP_SERVER_HPP__
