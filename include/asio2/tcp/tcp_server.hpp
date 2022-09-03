/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
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
	class tcp_server_impl_t : public server_impl_t<derived_t, session_t>
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
		 * @constructor
		 */
		explicit tcp_server_impl_t(
			std::size_t init_buf_size = tcp_frame_size,
			std::size_t  max_buf_size = max_buffer_size,
			std::size_t   concurrency = default_concurrency()
		)
			: super(concurrency)
			, acceptor_        (this->io_.context())
			, acceptor_timer_  (this->io_.context())
			, counter_timer_   (this->io_.context())
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
			, acceptor_        (this->io_.context())
			, acceptor_timer_  (this->io_.context())
			, counter_timer_   (this->io_.context())
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
		 * @destructor
		 */
		~tcp_server_impl_t()
		{
			this->stop();
		}

		/**
		 * @function : start the server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param service A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 * @param args The delimiter condition.Valid value types include the following:
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
				condition_helper::make_condition(asio::transfer_at_least(1), std::forward<Args>(args)...));
		}

		/**
		 * @function : stop the server
		 * You can call this function on the communication thread and anywhere to stop the server.
		 */
		inline void stop()
		{
			if (this->is_iopool_stopped())
				return;

			derived_t& derive = this->derived();

			derive.io().unregobj(&derive);

			derive.dispatch([&derive]() mutable
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
		 * @function : check whether the server is started
		 */
		inline bool is_started() const { return (super::is_started() && this->acceptor_.is_open()); }

		/**
		 * @function : check whether the server is stopped
		 */
		inline bool is_stopped() const
		{
			return (this->state_ == state_t::stopped && !this->acceptor_.is_open() && this->is_iopool_stopped());
		}

	public:
		/**
		 * @function : bind recv listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
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
		 * @function : bind accept listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
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
		 * @function : bind connect listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
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
		 * @function : bind disconnect listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * This notification is invoked before the connection is disconnected, you can call
		 * get_last_error/last_error_msg, etc, to get the disconnected error information
		 * Function signature : void(std::shared_ptr<asio2::tcp_session>& session_ptr)
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
		 * @function : bind init listener,we should set socket options at here
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
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
		 * @function : bind start listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
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
		 * @function : bind stop listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
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
		 * @function : get the acceptor refrence
		 */
		inline asio::ip::tcp::acceptor & acceptor() noexcept { return this->acceptor_; }

	protected:
		template<typename String, typename StrOrInt, typename MatchCondition>
		inline bool _do_start(String&& host, StrOrInt&& port, condition_wrap<MatchCondition> condition)
		{
			derived_t& derive = this->derived();

			this->start_iopool();

			if (this->is_iopool_stopped())
			{
				set_last_error(asio::error::operation_aborted);
				return false;
			}

			asio::dispatch(derive.io().context(), [&derive, this_ptr = derive.selfptr()]() mutable
			{
				detail::ignore_unused(this_ptr);

				// init the running thread id 
				if (derive.io().get_thread_id() == std::thread::id{})
					derive.io().init_thread_id();
			});

			// use promise to get the result of async accept
			std::promise<error_code> promise;
			std::future<error_code> future = promise.get_future();

			// use derfer to ensure the promise's value must be seted.
			detail::defer_event pg
			{
				[promise = std::move(promise)]() mutable { promise.set_value(get_last_error()); }
			};

			derive.post(
			[this, this_ptr = derive.selfptr(),
				host = std::forward<String>(host), port = std::forward<StrOrInt>(port),
				condition = std::move(condition), pg = std::move(pg)]
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

				try
				{
					clear_last_error();

					derive.io().regobj(&derive);

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
						asio::detail::throw_error(asio::error::operation_aborted);
					}

					super::start();

					this->counter_ptr_ = std::shared_ptr<void>((void*)1, [&derive](void*) mutable
					{
						derive._exec_stop(asio::error::operation_aborted, derive.selfptr());
					});

					error_code ec_ignore{};

					this->acceptor_.close(ec_ignore);

					// parse address and port
					asio::ip::tcp::resolver resolver(this->io_.context());
					asio::ip::tcp::endpoint endpoint = *resolver.resolve(h, p,
						asio::ip::resolver_base::flags::passive |
						asio::ip::resolver_base::flags::address_configured).begin();

					this->acceptor_.open(endpoint.protocol());

					// when you close socket in linux system,and start socket
					// immediate,you will get like this "the address is in use",
					// and bind is failed,but i'm suer i close the socket correct
					// already before,why does this happen? the reasion is the 
					// socket option "TIME_WAIT",although you close the socket,
					// but the system not release the socket,util 2~4 seconds 
					// later,so we can use the SO_REUSEADDR option to avoid this
					// problem,like below
				
					// set port reuse
					this->acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));

					derive._fire_init();

					this->acceptor_.bind(endpoint);
					this->acceptor_.listen();

					// if the some error occured in the _fire_init notify function, the 
					// get_last_error maybe not zero, so if we use _handle_start(get_last_error()...
					// at here, the start will failed, and the user don't know what happend.
					// so we need use as this : _handle_start(error_code{}...
					derive._handle_start(error_code{}, std::move(this_ptr), std::move(condition));

					return;
				}
				catch (system_error const& e)
				{
					set_last_error(e.code());
				}
				catch (std::exception const&)
				{
					set_last_error(asio::error::invalid_argument);
				}

				derive._handle_start(get_last_error(), std::move(this_ptr), std::move(condition));
			});

			if (!derive.io().running_in_this_thread())
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

		template<typename MatchCondition>
		inline void _handle_start(
			error_code ec, std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			ASIO2_ASSERT(this->derived().io().running_in_this_thread());

			try
			{
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

				asio::detail::throw_error(ec);

				this->derived()._post_accept(std::move(this_ptr), std::move(condition));
			}
			catch (system_error & e)
			{
				set_last_error(e);

				this->derived()._do_stop(e.code(), this->derived().selfptr());
			}
		}

		inline void _do_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			ASIO2_ASSERT(this->derived().io().running_in_this_thread());

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
			asio::dispatch(this->derived().io().context(), make_allocator(this->derived().wallocator(),
			[this, ec, old_state, this_ptr = std::move(this_ptr)]() mutable
			{
				detail::ignore_unused(this, ec, old_state, this_ptr);

				set_last_error(ec);

				ASIO2_ASSERT(this->state_ == state_t::stopping);

				// start timer to hold the acceptor io_context
				this->counter_timer_.expires_after((std::chrono::nanoseconds::max)());
				this->counter_timer_.async_wait([](const error_code&) {});

				// stop all the sessions, the session::stop must be no blocking,
				// otherwise it may be cause loop lock.
				this->sessions_.for_each([](std::shared_ptr<session_t> & session_ptr) mutable
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
			}));
		}

		inline void _exec_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			// use asio::post to ensure this server's _handle_stop is called must be after 
			// all sessions _handle_stop has been called already.
			// if use asio::dispatch, session's _handle_stop maybe called first.
			asio::post(this->derived().io().context(), make_allocator(this->derived().wallocator(),
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
			detail::ignore_unused(this_ptr);

			set_last_error(ec);

			this->derived()._fire_stop();

			error_code ec_ignore{};

			try
			{
				this->acceptor_timer_.cancel();
				this->counter_timer_.cancel();
			}
			catch (system_error const&)
			{
			}

			// call the base class stop function
			super::stop();

			// call acceptor's close function to notify the _handle_accept
			// function response with error > 0 , then the listen socket
			// can get notify to exit must ensure the close function has 
			// been called,otherwise the _handle_accept will never return
			this->acceptor_.close(ec_ignore);

			ASIO2_ASSERT(this->state_ == state_t::stopped);
		}

		template<typename... Args>
		inline std::shared_ptr<session_t> _make_session(Args&&... args)
		{
			return std::make_shared<session_t>(std::forward<Args>(args)...,
				this->sessions_, this->listener_, this->_get_io(),
				this->init_buffer_size_, this->max_buffer_size_);
		}

		template<typename MatchCondition>
		inline void _post_accept(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			ASIO2_ASSERT(this->derived().io().running_in_this_thread());

			if (!this->derived().is_started())
				return;

			try
			{
				std::shared_ptr<session_t> session_ptr = this->derived()._make_session();

				auto& socket = session_ptr->socket().lowest_layer();
				this->acceptor_.async_accept(socket, make_allocator(this->rallocator_,
				[this, sptr = std::move(session_ptr), this_ptr = std::move(this_ptr), condition]
				(const error_code& ec) mutable
				{
					this->derived()._handle_accept(ec, std::move(sptr), std::move(this_ptr), std::move(condition));
				}));
			}
			// handle exception, may be is the exception "Too many open files" (exception code : 24)
			// asio::error::no_descriptors - Too many open files
			catch (system_error & e)
			{
				set_last_error(e);

				std::shared_ptr<derived_t> self_ptr = this->derived().selfptr();

				this->acceptor_timer_.expires_after(std::chrono::seconds(1));
				this->acceptor_timer_.async_wait(
				[this, self_ptr = std::move(self_ptr), condition = std::move(condition)]
				(const error_code& ec) mutable
				{
					set_last_error(ec);
					if (ec) return;
					this->derived().post(
					[this, self_ptr = std::move(self_ptr), condition = std::move(condition)]() mutable
					{
						this->derived()._post_accept(std::move(self_ptr), std::move(condition));
					});
				});
			}
		}

		template<typename MatchCondition>
		inline void _handle_accept(const error_code & ec, std::shared_ptr<session_t> session_ptr,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			set_last_error(ec);

			// if the acceptor status is closed,don't call _post_accept again.
			if (ec == asio::error::operation_aborted)
				return;

			if (!ec)
			{
				if (this->derived().is_started())
				{
					session_ptr->counter_ptr_ = this->counter_ptr_;
					session_ptr->start(condition.clone());
				}
			}

			this->derived()._post_accept(std::move(this_ptr), std::move(condition));
		}

		inline void _fire_init()
		{
			// the _fire_init must be executed in the thread 0.
			ASIO2_ASSERT(this->derived().io().running_in_this_thread());
			ASIO2_ASSERT(!get_last_error());

			this->listener_.notify(event_type::init);
		}

		inline void _fire_start()
		{
			// the _fire_start must be executed in the thread 0.
			ASIO2_ASSERT(this->derived().io().running_in_this_thread());

		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(this->is_stop_called_ == false);
		#endif

			this->listener_.notify(event_type::start);
		}

		inline void _fire_stop()
		{
			// the _fire_stop must be executed in the thread 0.
			ASIO2_ASSERT(this->derived().io().running_in_this_thread());

		#if defined(_DEBUG) || defined(DEBUG)
			this->is_stop_called_ = true;
		#endif

			this->listener_.notify(event_type::stop);
		}

	protected:
		/// acceptor to accept client connection
		asio::ip::tcp::acceptor acceptor_;

		/// timer for acceptor exception, like the exception "Too many open files" (exception code : 24)
		asio::steady_timer      acceptor_timer_;

		/// used to hold the acceptor io_context util all sessions are closed already.
		asio::steady_timer      counter_timer_;

		std::size_t             init_buffer_size_ = tcp_frame_size;

		std::size_t             max_buffer_size_  = max_buffer_size;

	#if defined(_DEBUG) || defined(DEBUG)
		bool                    is_stop_called_  = false;
	#endif
	};
}

namespace asio2
{
	/**
	 * constructor maybe throw exception "Too many open files" (exception code : 24)
	 * asio::error::no_descriptors - Too many open files
	 */
	template<class session_t>
	class tcp_server_t : public detail::tcp_server_impl_t<tcp_server_t<session_t>, session_t>
	{
	public:
		using detail::tcp_server_impl_t<tcp_server_t<session_t>, session_t>::tcp_server_impl_t;
	};

	/**
	 * constructor maybe throw exception "Too many open files" (exception code : 24)
	 * asio::error::no_descriptors - Too many open files
	 */
	using tcp_server = tcp_server_t<tcp_session>;
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_TCP_SERVER_HPP__
