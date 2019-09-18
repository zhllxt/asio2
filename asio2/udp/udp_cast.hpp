/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_UDP_CAST_HPP__
#define __ASIO2_UDP_CAST_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstdint>
#include <memory>
#include <chrono>
#include <functional>
#include <atomic>
#include <string>
#include <string_view>
#include <queue>
#include <any>
#include <future>
#include <tuple>
#include <unordered_map>

#include <asio2/base/selector.hpp>
#include <asio2/base/iopool.hpp>
#include <asio2/base/error.hpp>
#include <asio2/base/listener.hpp>

#include <asio2/base/detail/object.hpp>
#include <asio2/base/detail/allocator.hpp>
#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/buffer_wrap.hpp>

#include <asio2/base/component/active_time_cp.hpp>
#include <asio2/base/component/user_data_cp.hpp>
#include <asio2/base/component/socket_cp.hpp>
#include <asio2/base/component/user_timer_cp.hpp>
#include <asio2/base/component/post_cp.hpp>

#include <asio2/base/detail/linear_buffer.hpp>
#include <asio2/udp/component/udp_send_cp.hpp>
#include <asio2/udp/impl/udp_send_op.hpp>

namespace asio2::detail
{
	template<class derived_t, class socket_t, class buffer_t>
	class udp_cast_impl_t
		: public object_t<derived_t>
		, public iopool_cp
		, public user_data_cp<derived_t>
		, public active_time_cp<derived_t>
		, public socket_cp<derived_t, socket_t>
		, public user_timer_cp<derived_t, false>
		, public post_cp<derived_t>
		, public udp_send_cp<derived_t, false>
		, public udp_send_op<derived_t, false>
	{
		template <class, bool> friend class user_timer_cp;
		template <class, bool> friend class udp_send_cp;
		template <class, bool> friend class udp_send_op;
		template <class> friend class post_cp;

	public:
		using self = udp_cast_impl_t<derived_t, socket_t, buffer_t>;
		using super = object_t<derived_t>;
		using buffer_type = buffer_t;

		/**
		 * @constructor
		 */
		explicit udp_cast_impl_t(
			std::size_t init_buffer_size = udp_frame_size,
			std::size_t max_buffer_size = (std::numeric_limits<std::size_t>::max)()
		)
			: super()
			, iopool_cp(1)
			, user_data_cp<derived_t>()
			, active_time_cp<derived_t>()
			, socket_cp<derived_t, socket_t>(iopool_.get(0).context())
			, user_timer_cp<derived_t, false>(iopool_.get(0))
			, post_cp<derived_t>()
			, udp_send_cp<derived_t, false>(iopool_.get(0))
			, udp_send_op<derived_t, false>()
			, rallocator_()
			, wallocator_()
			, listener_()
			, io_(iopool_.get(0))
			, buffer_(init_buffer_size, max_buffer_size)
		{
			this->iopool_.start();
		}

		/**
		 * @destructor
		 */
		~udp_cast_impl_t()
		{
			this->stop();
			this->iopool_.stop();
		}

		/**
		 * @function : start the server
		 * @param service A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename StringOrInt>
		inline bool start(StringOrInt&& service)
		{
			return this->derived()._do_start(std::string_view{},
				to_string_port(std::forward<StringOrInt>(service)), condition_wrap<void>{});
		}

		/**
		 * @function : start the server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param service A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename StringOrInt>
		inline bool start(std::string_view host, StringOrInt&& service)
		{
			return this->derived()._do_start(host,
				to_string_port(std::forward<StringOrInt>(service)), condition_wrap<void>{});
		}

		/**
		 * @function : stop the client
		 * You can call this function on the communication thread and anywhere to stop the client.
		 */
		inline void stop()
		{
			this->derived()._do_stop(asio::error::operation_aborted);
		}

		/**
		 * @function : check whether the client is started
		 */
		inline bool is_started() const
		{
			return (this->state_ == state_t::started && this->socket_.lowest_layer().is_open());
		}

		/**
		 * @function : check whether the client is stopped
		 */
		inline bool is_stopped() const
		{
			return (this->state_ == state_t::stopped && !this->socket_.lowest_layer().is_open());
		}

	public:
		/**
		 * @function : bind recv listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * Function signature : void(asio::ip::udp::endpoint& endpoint, std::string_view s)
		 */
		template<class F, class ...C>
		inline derived_t & bind_recv(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::recv,
				observer_t<asio::ip::udp::endpoint&, std::string_view>(std::forward<F>(fun), std::forward<C>(obj)...));
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
			this->listener_.bind(event::init, observer_t<>(std::forward<F>(fun), std::forward<C>(obj)...));
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
			this->listener_.bind(event::start, observer_t<error_code>(std::forward<F>(fun), std::forward<C>(obj)...));
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
			this->listener_.bind(event::stop, observer_t<error_code>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	protected:
		template<typename MatchCondition>
		bool _do_start(std::string_view host, std::string_view service, condition_wrap<MatchCondition> condition)
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

				this->socket_.close(ec_ignore);

				// parse address and port
				asio::ip::udp::resolver resolver(this->socket_.get_io_context());
				//asio::ip::udp::resolver::query query(host, service,
				//	asio::ip::resolver_base::flags::passive | asio::ip::resolver_base::flags::address_configured);
				asio::ip::udp::endpoint endpoint = *resolver.resolve(host, service,
					asio::ip::resolver_base::flags::passive | asio::ip::resolver_base::flags::address_configured).begin();

				this->socket_.open(endpoint.protocol());

				// when you close socket in linux system,and start socket immediate,you will get like this "the address is in use",
				// and bind is failed,but i'm suer i close the socket correct already before,why does this happen? the reasion is 
				// the socket option "TIME_WAIT",although you close the socket,but the system not release the socket,util 2~4 
				// seconds later,so we can use the SO_REUSEADDR option to avoid this problem,like below
				this->socket_.set_option(asio::ip::udp::socket::reuse_address(true)); // set port reuse

				this->derived()._fire_init();

				this->socket_.bind(endpoint);

				this->derived()._handle_start(error_code{}, std::move(condition));

				return (this->is_started());
			}
			catch (system_error & e)
			{
				set_last_error(e);
				this->derived()._handle_start(e.code(), std::move(condition));
				this->derived()._do_stop(e.code());
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

				asio::post(this->io_.strand(), [this, condition]()
				{
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
				return this->derived()._post_stop(ec, std::shared_ptr<derived_t>{}, expected);

			expected = state_t::started;
			if (this->state_.compare_exchange_strong(expected, state_t::stopping))
				return this->derived()._post_stop(ec, std::shared_ptr<derived_t>{}, expected);
		}

		inline void _post_stop(const error_code& ec, std::shared_ptr<derived_t> self_ptr, state_t old_state)
		{
			// psot a recv signal to ensure that all recv events has finished already.
			asio::post(this->io_.strand(), [this, ec, this_ptr = std::move(self_ptr), old_state]()
			{
				// When the code runs here,no new session can be emplace or erase to session_mgr.
				// stop all the sessions, the session::stop must be no blocking,otherwise it may be cause loop lock.
				set_last_error(ec);

				state_t expected = state_t::stopping;
				if (this->state_.compare_exchange_strong(expected, state_t::stopped))
				{
					this->derived()._fire_stop(ec);

					// call CRTP polymorphic stop
					this->derived()._handle_stop(ec, std::move(this_ptr));
				}
				else
				{
					ASIO2_ASSERT(false);
				}
			});
		}

		inline void _handle_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			detail::ignore::unused(ec, this_ptr);

			// close user custom timers
			this->stop_all_timers();

			// destroy user data, maybe the user data is self shared_ptr, if don't destroy it, will cause loop refrence.
			this->user_data_.reset();

			// call socket's close function to notify the _handle_recv function response with error > 0 ,then the socket 
			// can get notify to exit
			// Call shutdown() to indicate that you will not write any more data to the socket.
			this->socket_.shutdown(asio::socket_base::shutdown_both, ec_ignore);
			// Call close,otherwise the _handle_recv will never return
			this->socket_.close(ec_ignore);
		}

	protected:
		inline asio::ip::udp::endpoint to_endpoint(const std::string& host, const std::string& port)
		{
			// the resolve function is a time-consuming operation
			asio::ip::udp::resolver resolver(this->io_.context());
			asio::ip::udp::resolver::query query(host, port);
			asio::ip::udp::endpoint endpoint = *resolver.resolve(query);
			return endpoint;
		}

		template<class ConstBufferSequence>
		inline bool _do_send(const std::string& host, const std::string& port, ConstBufferSequence buffer)
		{
			try
			{
				return this->derived()._do_send(to_endpoint(host, port), buffer);
			}
			catch (system_error & e) { set_last_error(e); }
			return false;
		}

		template<class ConstBufferSequence, class Callback>
		inline bool _do_send(const std::string& host, const std::string& port, ConstBufferSequence buffer, Callback& fn)
		{
			try
			{
				return this->derived()._do_send(to_endpoint(host, port), buffer, fn);
			}
			catch (system_error & e) { set_last_error(e); }
			return false;
		}

		template<class ConstBufferSequence>
		inline bool _do_send(const std::string& host, const std::string& port,
			ConstBufferSequence buffer, std::promise<std::pair<error_code, std::size_t>>& promise)
		{
			try
			{
				return this->derived()._do_send(to_endpoint(host, port), buffer, promise);
			}
			catch (system_error & e) { set_last_error(e); }
			return false;
		}

		template<class ConstBufferSequence>
		inline bool _do_send(const asio::ip::udp::endpoint& endpoint, ConstBufferSequence buffer)
		{
			return this->derived()._udp_send_to(endpoint, buffer);
		}

		template<class ConstBufferSequence, class Callback>
		inline bool _do_send(const asio::ip::udp::endpoint& endpoint, ConstBufferSequence buffer, Callback& fn)
		{
			return this->derived()._udp_send_to(endpoint, buffer, fn);
		}

		template<class ConstBufferSequence>
		inline bool _do_send(const asio::ip::udp::endpoint& endpoint,
			ConstBufferSequence buffer, std::promise<std::pair<error_code, std::size_t>>& promise)
		{
			return this->derived()._udp_send_to(endpoint, buffer, promise);
		}

	protected:
		template<typename MatchCondition>
		void _post_recv(condition_wrap<MatchCondition> condition)
		{
			if (!this->is_started())
				return;

			try
			{
				this->socket_.async_receive_from(
					this->buffer_.prepare(this->buffer_.pre_size()), this->remote_endpoint_,
					asio::bind_executor(this->io_.strand(), make_allocator(this->rallocator_,
						[this, condition](const error_code& ec, std::size_t bytes_recvd)
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
		void _handle_recv(const error_code& ec, std::size_t bytes_recvd, condition_wrap<MatchCondition> condition)
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
				this->derived()._fire_recv(std::shared_ptr<derived_t>{}, std::string_view(
					static_cast<std::string_view::const_pointer>(this->buffer_.data().data()), bytes_recvd));
			}

			this->buffer_.consume(this->buffer_.size());

			this->derived()._post_recv(condition);
		}

		inline void _fire_init()
		{
			this->listener_.notify(event::init);
		}

		inline void _fire_recv(std::shared_ptr<derived_t>, std::string_view s)
		{
			this->listener_.notify(event::recv, this->remote_endpoint_, s);
		}

		inline void _fire_start(error_code ec)
		{
			this->listener_.notify(event::start, ec);
		}

		inline void _fire_stop(error_code ec)
		{
			this->listener_.notify(event::stop, ec);
		}

	public:
		/**
		 * @function : get the buffer object refrence
		 */
		inline buffer_wrap<buffer_t> & buffer() { return this->buffer_; }

		/**
		 * @function : get the io object refrence
		 */
		inline io_t & io() { return this->io_; }

	protected:
		/**
		 * @function : get the recv/read allocator object refrence
		 */
		inline auto & rallocator() { return this->rallocator_; }
		/**
		 * @function : get the send/write allocator object refrence
		 */
		inline auto & wallocator() { return this->wallocator_; }

		inline listener_t                 & listener() { return this->listener_; }
		inline std::atomic<state_t>       & state() { return this->state_; }

	protected:
		/// The memory to use for handler-based custom memory allocation. used fo recv/read.
		handler_memory<>                            rallocator_;

		/// The memory to use for handler-based custom memory allocation. used fo send/write.
		handler_memory<size_op<>, std::true_type>   wallocator_;

		/// listener 
		listener_t                                  listener_;

		/// The io (include io_context and strand) used to handle the connect/recv/send event.
		io_t                                      & io_;

		/// buffer
		buffer_wrap<buffer_t>                       buffer_;

		/// state
		std::atomic<state_t>                        state_ = state_t::stopped;

		/// endpoint for udp 
		asio::ip::udp::endpoint                     remote_endpoint_;
	};
}

namespace asio2
{
	/*
	 * udp unicast/multicast/broadcast
	 */
	class udp_cast : public detail::udp_cast_impl_t<udp_cast, asio::ip::udp::socket, asio2::linear_buffer>
	{
	public:
		using udp_cast_impl_t<udp_cast, asio::ip::udp::socket, asio2::linear_buffer>::udp_cast_impl_t;
	};
}

#endif // !__ASIO2_UDP_CAST_HPP__
