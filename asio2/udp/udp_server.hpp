/*
 * COPYRIGHT (C) 2017-2019, zhllxt
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

#include <asio2/base/server.hpp>
#include <asio2/udp/udp_session.hpp>
#include <asio2/base/detail/linear_buffer.hpp>

namespace asio2::detail
{
	template<class derived_t, class session_t>
	class udp_server_impl_t : public server_impl_t<derived_t, session_t>
	{
		template <class, bool>  friend class user_timer_cp;
		template <class>        friend class post_cp;
		template <class, class> friend class server_impl_t;

	public:
		using self = udp_server_impl_t<derived_t, session_t>;
		using super = server_impl_t<derived_t, session_t>;
		using session_type = session_t;

		/**
		 * @constructor
		 */
		explicit udp_server_impl_t(
			std::size_t init_buffer_size = udp_frame_size,
			std::size_t max_buffer_size = (std::numeric_limits<std::size_t>::max)()
		)
			: super(1)
			, acceptor_(this->io_.context())
			, remote_endpoint_()
			, buffer_(init_buffer_size, max_buffer_size)
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
		 * @param service A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename StrOrInt>
		inline bool start(StrOrInt&& service)
		{
			return this->start(std::string_view{}, std::forward<StrOrInt>(service));
		}

		/**
		 * @function : start the server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param service A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt>
		inline bool start(String&& host, StrOrInt&& service)
		{
			return this->derived()._do_start(
				std::forward<String>(host), std::forward<StrOrInt>(service),
				condition_wrap<void>{});
		}

		/**
		 * @function : start the server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param service A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt>
		inline bool start(String&& host, StrOrInt&& service, use_kcp_t c)
		{
			return this->derived()._do_start(
				std::forward<String>(host), std::forward<StrOrInt>(service),
				condition_wrap<use_kcp_t>(c));
		}

		/**
		 * @function : stop acceptor
		 */
		inline void stop()
		{
			this->derived()._do_stop(asio::error::operation_aborted);

			this->iopool_.stop();
		}

		/**
		 * @function : check whether the server is started
		 */
		inline bool is_started() { return (super::is_started() && this->acceptor_.is_open()); }

		/**
		 * @function : check whether the server is stopped
		 */
		inline bool is_stopped() { return (super::is_stopped() && !this->acceptor_.is_open()); }

	public:
		/**
		 * @function : bind recv listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * Function signature : void(std::shared_ptr<asio2::udp_session>& session_ptr, std::string_view s)
		 */
		template<class F, class ...C>
		inline derived_t & bind_recv(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::recv,
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
			this->listener_.bind(event::connect,
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
			this->listener_.bind(event::disconnect,
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
		 * Function signature : void()
		 */
		template<class F, class ...C>
		inline derived_t & bind_init(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::init, observer_t<>(
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
		 * Function signature : void(asio2::error_code ec)
		 */
		template<class F, class ...C>
		inline derived_t & bind_start(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::start, observer_t<error_code>(
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
		 * Function signature : void(asio2::error_code ec)
		 */
		template<class F, class ...C>
		inline derived_t & bind_stop(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::stop, observer_t<error_code>(
				std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

		/**
		 * @function : bind kcp handshake listener, just used fo kcp mode
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * Function signature : void(std::shared_ptr<asio2::udp_session>& session_ptr, asio::error_code ec)
		 */
		template<class F, class ...C>
		inline derived_t & bind_handshake(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::handshake,
				observer_t<std::shared_ptr<session_t>&, error_code>(
					std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	public:
		/**
		 * @function : get the acceptor refrence
		 */
		inline asio::ip::udp::socket & acceptor() { return this->acceptor_; }

	protected:
		template<typename String, typename StrOrInt, typename MatchCondition>
		bool _do_start(String&& host, StrOrInt&& service, condition_wrap<MatchCondition> condition)
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

				this->counter_ptr_ = std::shared_ptr<void>((void*)1, [this](void*)
				{
					this->derived().post([this]() mutable
					{
						state_t expected = state_t::stopping;
						if (this->state_.compare_exchange_strong(expected, state_t::stopped))
							this->derived()._handle_stop(asio::error::operation_aborted,
								this->derived().selfptr());
						else
							ASIO2_ASSERT(false);
					});
				});

				this->acceptor_.close(ec_ignore);

				std::string h = to_string(std::forward<String>(host));
				std::string p = to_string(std::forward<StrOrInt>(service));

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

				this->derived()._fire_init();

				this->acceptor_.bind(endpoint);

				this->derived()._handle_start(error_code{}, std::move(condition));

				return (this->is_started());
			}
			catch (system_error & e)
			{
				this->derived()._handle_start(e.code(), std::move(condition));
			}
			return false;
		}

		template<typename MatchCondition>
		void _handle_start(error_code ec, condition_wrap<MatchCondition> condition)
		{
			try
			{
				// Whether the startup succeeds or fails, always call fire_start notification
				state_t expected = state_t::starting;
				if (!ec)
					if (!this->state_.compare_exchange_strong(expected, state_t::started))
						ec = asio::error::operation_aborted;

				set_last_error(ec);

				this->derived()._fire_start(ec);

				expected = state_t::started;
				if (!ec)
					if (!this->state_.compare_exchange_strong(expected, state_t::started))
						asio::detail::throw_error(asio::error::operation_aborted);

				asio::detail::throw_error(ec);

				this->derived().post([this, condition]() mutable
				{
					this->buffer_.consume(this->buffer_.size());

					this->derived()._post_recv(std::move(condition));
				});
			}
			catch (system_error & e)
			{
				set_last_error(e);
				this->derived()._do_stop(e.code());
			}
		}

		inline void _do_stop(const error_code& ec)
		{
			state_t expected = state_t::starting;
			if (this->state_.compare_exchange_strong(expected, state_t::stopping))
				return this->derived()._post_stop(ec, this->derived().selfptr(), expected);

			expected = state_t::started;
			if (this->state_.compare_exchange_strong(expected, state_t::stopping))
				return this->derived()._post_stop(ec, this->derived().selfptr(), expected);
		}

		inline void _post_stop(const error_code& ec, std::shared_ptr<derived_t> self_ptr, state_t old_state)
		{
			// asio don't allow operate the same socket in multi thread,
			// if you close socket in one thread and another thread is 
			// calling socket's async_... function,it maybe crash.so we
			// must care for operate the socket.when need close the
			// socket ,we use the strand to post a event,make sure the
			// socket's close operation is in the same thread.

			// psot a recv signal to ensure that all recv events has finished already.
			this->derived().post([this, ec, this_ptr = std::move(self_ptr), old_state]() mutable
			{
				detail::ignore::unused(this, old_state);

				// When the code runs here,no new session can be emplace or erase to session_mgr.
				// stop all the sessions, the session::stop must be no blocking,
				// otherwise it may be cause loop lock.
				set_last_error(ec);

				// stop all the sessions, the session::stop must be no blocking,
				// otherwise it may be cause loop lock.
				this->sessions_.foreach([](std::shared_ptr<session_t> & session_ptr) mutable
				{
					session_ptr->stop();
				});

				this->counter_ptr_.reset();
			});
		}

		inline void _handle_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			detail::ignore::unused(ec, this_ptr);

			this->derived()._fire_stop(ec);

			// call the base class stop function
			super::stop();

			// Call shutdown() to indicate that you will not write any more data to the socket.
			this->acceptor_.shutdown(asio::socket_base::shutdown_both, ec_ignore);
			// Call close,otherwise the _handle_recv will never return
			this->acceptor_.close(ec_ignore);
		}

		template<typename MatchCondition>
		inline void _post_recv(condition_wrap<MatchCondition> condition)
		{
			if (!this->is_started())
				return;

			try
			{
				this->acceptor_.async_receive_from(
					this->buffer_.prepare(this->buffer_.pre_size()), this->remote_endpoint_,
					asio::bind_executor(this->io_.strand(), make_allocator(this->rallocator_,
						[this, condition](const error_code& ec, std::size_t bytes_recvd) mutable
				{
					this->derived()._handle_recv(ec, bytes_recvd, condition);
				})));
			}
			catch (system_error & e)
			{
				set_last_error(e);
				this->derived()._do_stop(e.code());
			}
		}

		template<typename MatchCondition>
		inline void _handle_recv(const error_code& ec, std::size_t bytes_recvd,
			condition_wrap<MatchCondition> condition)
		{
			set_last_error(ec);

			if (ec == asio::error::operation_aborted)
			{
				this->derived()._do_stop(ec);
				return;
			}

			if (!this->is_started())
				return;

			this->buffer_.commit(bytes_recvd);

			if (!ec)
			{
				std::string_view s = std::string_view(static_cast<std::string_view::const_pointer>
					(this->buffer_.data().data()), bytes_recvd);

				// first we find whether the session is in the session_mgr pool already,if not ,
				// we new a session and put it into the session_mgr pool
				std::shared_ptr<session_t> session_ptr = this->sessions_.find(this->remote_endpoint_);
				if (!session_ptr)
				{
					this->derived()._handle_accept(ec, s, session_ptr, condition);
				}
				else
				{
					if constexpr (std::is_same_v<MatchCondition, use_kcp_t>)
					{
						if (s.size() == sizeof(kcp::kcphdr))
						{
							if /**/ (kcp::is_kcphdr_syn(s))
							{
								if (session_ptr->kcp_)
									session_ptr->kcp_->send_fin_ = false;
								session_ptr->stop();

								auto task = [this, ec, condition, session_ptr,
									syn = std::string{ s.data(),s.size() }]
									(event_guard<session_t>&& g) mutable
								{
									this->derived()._handle_accept(ec,
										std::string_view{ syn }, session_ptr, condition);
								};

								session_ptr->push_event([this, t = std::move(task)]
								(event_guard<session_t>&& g) mutable
								{
									auto task = [g = std::move(g), t = std::move(t)]() mutable
									{
										t(std::move(g));
									};
									this->derived().post(std::move(task));
									return true;
								});
							}
							else
							{
								std::ignore = true;
							}
						}
						else
						{
							session_ptr->_handle_recv(ec, s, session_ptr, condition);
						}
					}
					else
					{
						session_ptr->_handle_recv(ec, s, session_ptr, condition);
					}
				}
			}

			this->buffer_.consume(this->buffer_.size());

			this->derived()._post_recv(condition);
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
			session_ptr = this->derived()._make_session();
			session_ptr->counter_ptr_ = this->counter_ptr_;
			session_ptr->first_ = first;
			session_ptr->start(condition);
		}

		inline void _fire_init()
		{
			this->listener_.notify(event::init);
		}

		inline void _fire_start(error_code ec)
		{
			this->listener_.notify(event::start, ec);
		}

		inline void _fire_stop(error_code ec)
		{
			this->listener_.notify(event::stop, ec);
		}

	protected:
		/// acceptor to accept client connection
		asio::ip::udp::socket    acceptor_;

		/// endpoint for udp 
		asio::ip::udp::endpoint  remote_endpoint_;

		/// buffer
		asio2::buffer_wrap<asio2::linear_buffer> buffer_;
	};
}

namespace asio2
{
	template<class session_t>
	class udp_server_t : public detail::udp_server_impl_t<udp_server_t<session_t>, session_t>
	{
	public:
		using detail::udp_server_impl_t<udp_server_t<session_t>, session_t>::udp_server_impl_t;
	};

	using udp_server = udp_server_t<udp_session>;
}


#endif // !__ASIO2_UDP_SERVER_HPP__
