/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
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
	class udp_server_impl_t : public server_impl_t<derived_t, session_t>
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
		 * @constructor
		 */
		explicit udp_server_impl_t(
			std::size_t init_buf_size = udp_frame_size,
			std::size_t max_buf_size  = max_buffer_size,
			std::size_t concurrency   = 1
		)
			: super(concurrency)
			, acceptor_(this->io_.context())
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
			, acceptor_(this->io_.context())
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
		 * @destructor
		 */
		~udp_server_impl_t()
		{
			this->stop();
		}

		/**
		 * @function : start the server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param service A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt, typename... Args>
		inline bool start(String&& host, StrOrInt&& service, Args&&... args)
		{
			return this->derived()._do_start(
				std::forward<String>(host), std::forward<StrOrInt>(service),
				condition_helper::make_condition('0', std::forward<Args>(args)...));
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
		 * @function : bind connect listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
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
		 * @function : bind disconnect listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
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

		/**
		 * @function : bind kcp handshake listener, just used fo kcp mode
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
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
		 * @function : get the acceptor refrence
		 */
		inline asio::ip::udp::socket & acceptor() noexcept { return this->acceptor_; }

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
					asio::ip::udp::resolver resolver(this->io_.context());
					asio::ip::udp::endpoint endpoint = *resolver.resolve(h, p,
						asio::ip::resolver_base::flags::passive |
						asio::ip::resolver_base::flags::address_configured).begin();

					this->acceptor_.open(endpoint.protocol());

					// when you close socket in linux system,and start socket
					// immediate,you will get like this "the address is in use",
					// and bind is failed,but i'm suer i close the socket correct
					// already before,why does this happen? the reasion is the
					// socket option "TIME_WAIT",although you close the socket,
					// but the system not release the socket,util 2~4 seconds later,
					// so we can use the SO_REUSEADDR option to avoid this problem,
					// like below

					// set port reuse
					this->acceptor_.set_option(asio::ip::udp::socket::reuse_address(true));

					//// Join the multicast group. you can set this option in the on_init(_fire_init) function.
					//this->acceptor_.set_option(
					//	// for ipv6, the host must be a ipv6 address like 0::0
					//	asio::ip::multicast::join_group(asio::ip::make_address("ff31::8000:1234")));
					//	// for ipv4, the host must be a ipv4 address like 0.0.0.0
					//	//asio::ip::multicast::join_group(asio::ip::make_address("239.255.0.1")));

					derive._fire_init();

					this->acceptor_.bind(endpoint);

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

				this->buffer_.consume(this->buffer_.size());

				this->derived()._post_recv(std::move(this_ptr), std::move(condition));
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
			});
		}

		inline void _exec_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			// use asio::post to ensure this server's _handle_stop is called must be after 
			// all sessions _handle_stop has been called already.
			// is use asio::dispatch, session's _handle_stop maybe called first.
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

			// call the base class stop function
			super::stop();

			// Call shutdown() to indicate that you will not write any more data to the socket.
			this->acceptor_.shutdown(asio::socket_base::shutdown_both, ec_ignore);
			// Call close,otherwise the _handle_recv will never return
			this->acceptor_.close(ec_ignore);

			ASIO2_ASSERT(this->state_ == state_t::stopped);
		}

		template<typename MatchCondition>
		inline void _post_recv(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			if (!this->derived().is_started())
			{
				if (this->derived().state() == state_t::started)
				{
					this->derived()._do_stop(asio2::get_last_error(), std::move(this_ptr));
				}
				return;
			}

			try
			{
				this->acceptor_.async_receive_from(
					this->buffer_.prepare(this->buffer_.pre_size()), this->remote_endpoint_,
					make_allocator(this->rallocator_,
					[this, this_ptr = std::move(this_ptr), condition = std::move(condition)]
				(const error_code& ec, std::size_t bytes_recvd) mutable
				{
					this->derived()._handle_recv(ec, bytes_recvd, std::move(this_ptr), std::move(condition));
				}));
			}
			catch (system_error & e)
			{
				set_last_error(e);

				this->derived()._do_stop(e.code(), this->derived().selfptr());
			}
		}

		template<typename MatchCondition>
		inline void _handle_recv(const error_code& ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			set_last_error(ec);

			if (!this->derived().is_started())
			{
				if (this->derived().state() == state_t::started)
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
				std::size_t key = asio2::hash<asio::ip::udp::endpoint>{}(this->remote_endpoint_);
				std::shared_ptr<session_t> session_ptr = this->sessions_.find(key);
				if (!session_ptr)
				{
					this->derived()._handle_accept(ec, data, session_ptr, condition);
				}
				else
				{
					if constexpr (std::is_same_v<typename condition_wrap<MatchCondition>::condition_type, use_kcp_t>)
					{
						if (data.size() == kcp::kcphdr::required_size())
						{
							// the client is disconnect without send a "fin" or the server has't recvd the 
							// "fin", and then the client connect again a later, at this time, the client
							// is in the session map already, so we need check whether the first message is fin
							if /**/ (kcp::is_kcphdr_syn(data))
							{
								if (session_ptr->kcp_)
									session_ptr->kcp_->send_fin_ = false;
								session_ptr->stop();

								session_t* session = session_ptr.get();

								session->push_event([this, ec, session_ptr = std::move(session_ptr),
									condition = std::move(condition), syn = std::string{ data.data(),data.size() }]
								(event_queue_guard<session_t> g) mutable
								{
									detail::ignore_unused(g);

									this->derived()._handle_accept(ec,
										std::string_view{ syn }, std::move(session_ptr), std::move(condition));
								});
							}
							else
							{
								session_ptr->_handle_recv(ec, data, session_ptr, condition);
							}
						}
						else
						{
							session_ptr->_handle_recv(ec, data, session_ptr, condition);
						}
					}
					else
					{
						session_ptr->_handle_recv(ec, data, session_ptr, condition);
					}
				}
			}

			this->buffer_.consume(this->buffer_.size());

			if (bytes_recvd == this->buffer_.pre_size())
			{
				this->buffer_.pre_size((std::min)(this->buffer_.pre_size() * 2, this->buffer_.max_size()));
			}

			this->derived()._post_recv(std::move(this_ptr), std::move(condition));
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

		template<typename MatchCondition>
		inline void _handle_accept(const error_code & ec, std::string_view first,
			std::shared_ptr<session_t> session_ptr, condition_wrap<MatchCondition> condition)
		{
			set_last_error(ec);

			if (!ec)
			{
				if (this->derived().is_started())
				{
					session_ptr = this->derived()._make_session();
					session_ptr->counter_ptr_ = this->counter_ptr_;
					session_ptr->first_ = first;
					session_ptr->start(condition.clone());
				}
			}
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
		asio::ip::udp::socket    acceptor_;

		/// endpoint for udp 
		asio::ip::udp::endpoint  remote_endpoint_;

		/// buffer
		asio2::buffer_wrap<asio2::linear_buffer> buffer_;

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
	class udp_server_t : public detail::udp_server_impl_t<udp_server_t<session_t>, session_t>
	{
	public:
		using detail::udp_server_impl_t<udp_server_t<session_t>, session_t>::udp_server_impl_t;
	};

	/**
	 * constructor maybe throw exception "Too many open files" (exception code : 24)
	 * asio::error::no_descriptors - Too many open files
	 */
	using udp_server = udp_server_t<udp_session>;
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_UDP_SERVER_HPP__
