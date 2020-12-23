/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_TCP_CLIENT_HPP__
#define __ASIO2_TCP_CLIENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/client.hpp>

#include <asio2/base/detail/condition_wrap.hpp>
#include <asio2/tcp/component/tcp_keepalive_cp.hpp>
#include <asio2/tcp/impl/tcp_send_op.hpp>
#include <asio2/tcp/impl/tcp_recv_op.hpp>

namespace asio2::detail
{
	struct template_args_tcp_client
	{
		static constexpr bool is_session = false;
		static constexpr bool is_client  = true;

		using socket_t    = asio::ip::tcp::socket;
		using buffer_t    = asio::streambuf;
		using send_data_t = std::string_view;
		using recv_data_t = std::string_view;
	};

	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_CLIENT;

	template<class derived_t, class args_t>
	class tcp_client_impl_t
		: public client_impl_t         <derived_t, args_t>
		, public tcp_keepalive_cp      <derived_t, args_t>
		, public tcp_send_op           <derived_t, args_t>
		, public tcp_recv_op           <derived_t, args_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT;

	public:
		using super = client_impl_t    <derived_t, args_t>;
		using self  = tcp_client_impl_t<derived_t, args_t>;

		using buffer_type = typename args_t::buffer_t;
		using send_data_t = typename args_t::send_data_t;
		using recv_data_t = typename args_t::recv_data_t;

		using super::send;

	public:
		/**
		 * @constructor
		 */
		explicit tcp_client_impl_t(
			std::size_t init_buffer_size = tcp_frame_size,
			std::size_t max_buffer_size = (std::numeric_limits<std::size_t>::max)()
		)
			: super(1, init_buffer_size, max_buffer_size)
			, tcp_keepalive_cp<derived_t, args_t>(this->socket_)
			, tcp_send_op<derived_t, args_t>()
			, tcp_recv_op<derived_t, args_t>()
		{
			this->connect_timeout(std::chrono::milliseconds(tcp_connect_timeout));
		}

		/**
		 * @destructor
		 */
		~tcp_client_impl_t()
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
				condition_wrap<asio::detail::transfer_at_least_t>{asio::transfer_at_least(1)});
		}

		/**
		 * @function : start the client, blocking connect to server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 * @param condition The delimiter condition.Valid value types include the following:
		 * char,std::string,std::string_view,
		 * function:std::pair<iterator, bool> match_condition(iterator begin, iterator end),
		 * asio::transfer_at_least,asio::transfer_exactly
		 * more details see asio::read_until
		 */
		template<typename String, typename StrOrInt, typename MatchCondition>
		bool start(String&& host, StrOrInt&& port, MatchCondition condition)
		{
			return this->derived().template _do_connect<false>(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				condition_wrap<MatchCondition>(condition));
		}

		/**
		 * @function : start the client, blocking connect to server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 * @param condition The delimiter condition.Valid value types include the following:
		 * char,std::string,std::string_view,
		 * function:std::pair<iterator, bool> match_condition(iterator begin, iterator end),
		 * asio::transfer_at_least,asio::transfer_exactly
		 * more details see asio::read_until
		 */
		template<typename String, typename StrOrInt, typename MatchCondition, typename ParserFun>
		bool start(String&& host, StrOrInt&& port, MatchCondition condition, ParserFun&& parser)
		{
			using fun_traits_type = function_traits<std::remove_cv_t<std::remove_reference_t<ParserFun>>>;
			using IdT = typename fun_traits_type::return_type;
			using SendDataT = typename fun_traits_type::template args<0>::type;
			using RecvDataT = typename fun_traits_type::template args<0>::type;

			return this->derived().template _do_connect<false>(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				condition_wrap<use_rdc_t<MatchCondition, IdT, SendDataT, RecvDataT>>(
					std::in_place,
					std::move(condition),
					std::forward<ParserFun>(parser)));
		}

		/**
		 * @function : start the client, blocking connect to server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 * @param condition The delimiter condition.Valid value types include the following:
		 * char,std::string,std::string_view,
		 * function:std::pair<iterator, bool> match_condition(iterator begin, iterator end),
		 * asio::transfer_at_least,asio::transfer_exactly
		 * more details see asio::read_until
		 */
		template<typename String, typename StrOrInt, typename MatchCondition,
			typename SendParserFun, typename RecvParserFun>
		bool start(String&& host, StrOrInt&& port, MatchCondition condition,
			SendParserFun&& send_parser, RecvParserFun&& recv_parser)
		{
			using send_fun_traits_type = function_traits<std::remove_cv_t<std::remove_reference_t<SendParserFun>>>;
			using recv_fun_traits_type = function_traits<std::remove_cv_t<std::remove_reference_t<RecvParserFun>>>;
			using SendIdT = typename send_fun_traits_type::return_type;
			using RecvIdT = typename recv_fun_traits_type::return_type;
			using SendDataT = typename send_fun_traits_type::template args<0>::type;
			using RecvDataT = typename recv_fun_traits_type::template args<0>::type;

			static_assert(std::is_same_v<SendIdT, RecvIdT>);

			return this->derived().template _do_connect<false>(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				condition_wrap<use_rdc_t<MatchCondition, SendIdT, SendDataT, RecvDataT>>(
					std::in_place,
					std::move(condition),
					std::forward<SendParserFun>(send_parser),
					std::forward<RecvParserFun>(recv_parser)));
		}

		/**
		 * @function : start the client, asynchronous connect to server
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
				condition_wrap<asio::detail::transfer_at_least_t>{asio::transfer_at_least(1)});
		}

		/**
		 * @function : start the client, asynchronous connect to server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 * @param condition The delimiter condition.Valid value types include the following:
		 * char,std::string,std::string_view,
		 * function:std::pair<iterator, bool> match_condition(iterator begin, iterator end),
		 * asio::transfer_at_least,asio::transfer_exactly
		 * more details see asio::read_until
		 */
		template<typename String, typename StrOrInt, typename MatchCondition>
		bool async_start(String&& host, StrOrInt&& port, MatchCondition condition)
		{
			return this->derived().template _do_connect<true>(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				condition_wrap<MatchCondition>(condition));
		}

		/**
		 * @function : start the client, asynchronous connect to server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 * @param condition The delimiter condition.Valid value types include the following:
		 * char,std::string,std::string_view,
		 * function:std::pair<iterator, bool> match_condition(iterator begin, iterator end),
		 * asio::transfer_at_least,asio::transfer_exactly
		 * more details see asio::read_until
		 */
		template<typename String, typename StrOrInt, typename MatchCondition, typename ParserFun>
		bool async_start(String&& host, StrOrInt&& port, MatchCondition condition, ParserFun&& parser)
		{
			using fun_traits_type = function_traits<std::remove_cv_t<std::remove_reference_t<ParserFun>>>;
			using IdT = typename fun_traits_type::return_type;
			using SendDataT = typename fun_traits_type::template args<0>::type;
			using RecvDataT = typename fun_traits_type::template args<0>::type;

			return this->derived().template _do_connect<true>(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				condition_wrap<use_rdc_t<MatchCondition, IdT, SendDataT, RecvDataT>>(
					std::in_place,
					std::move(condition),
					std::forward<ParserFun>(parser)));
		}

		/**
		 * @function : start the client, asynchronous connect to server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 * @param condition The delimiter condition.Valid value types include the following:
		 * char,std::string,std::string_view,
		 * function:std::pair<iterator, bool> match_condition(iterator begin, iterator end),
		 * asio::transfer_at_least,asio::transfer_exactly
		 * more details see asio::read_until
		 */
		template<typename String, typename StrOrInt, typename MatchCondition,
			typename SendParserFun, typename RecvParserFun>
		bool async_start(String&& host, StrOrInt&& port, MatchCondition condition,
			SendParserFun&& send_parser, RecvParserFun&& recv_parser)
		{
			using send_fun_traits_type = function_traits<std::remove_cv_t<std::remove_reference_t<SendParserFun>>>;
			using recv_fun_traits_type = function_traits<std::remove_cv_t<std::remove_reference_t<RecvParserFun>>>;
			using SendIdT = typename send_fun_traits_type::return_type;
			using RecvIdT = typename recv_fun_traits_type::return_type;
			using SendDataT = typename send_fun_traits_type::template args<0>::type;
			using RecvDataT = typename recv_fun_traits_type::template args<0>::type;

			static_assert(std::is_same_v<SendIdT, RecvIdT>);

			return this->derived().template _do_connect<true>(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				condition_wrap<use_rdc_t<MatchCondition, SendIdT, SendDataT, RecvDataT>>(
					std::in_place,
					std::move(condition),
					std::forward<SendParserFun>(send_parser),
					std::forward<RecvParserFun>(recv_parser)));
		}

		/**
		 * @function : stop the client
		 * You can call this function on the communication thread and anywhere to stop the client.
		 */
		inline void stop()
		{
			// Asio end socket functions: cancel, shutdown, close, release :
			// https://stackoverflow.com/questions/51468848/asio-end-socket-functions-cancel-shutdown-close-release
			// The proper steps are:
			// 1.Call shutdown() to indicate that you will not write any more data to the socket.
			// 2.Continue to (async-) read from the socket until you get either an error or the connection is closed.
			// 3.Now close() the socket (in the async read handler).
			// If you don't do this, you may end up closing the connection while the other side is still sending data.
			// This will result in an ungraceful close.

			this->derived().post([this]() mutable
			{
				// first close the reconnect timer
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
			this->listener_.bind(event_type::recv,
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
			this->listener_.bind(event_type::connect,
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
			this->listener_.bind(event_type::disconnect,
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
			this->listener_.bind(event_type::init,
				observer_t<>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	protected:
		template<bool isAsync, typename String, typename StrOrInt, typename MatchCondition>
		bool _do_connect(String&& host, StrOrInt&& port, condition_wrap<MatchCondition> condition)
		{
			// Used to test whether the behavior of different compilers is consistent
			static_assert(tcp_send_op<derived_t, args_t>::template has_member_dgram<self>::value,
				"The behavior of different compilers is not consistent");

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

				this->derived()._load_reconnect_timer(host, port, condition);

				this->derived()._do_init(condition);

				super::start();

				// if the match condition is remote data call mode,do some thing.
				this->derived()._rdc_init(condition);

				return this->derived().template _start_connect<isAsync>(
					std::forward<String>(host), std::forward<StrOrInt>(port),
					this->derived().selfptr(), std::move(condition));
			}
			catch (system_error & e)
			{
				this->derived()._handle_connect(e.code(), this->derived().selfptr(), condition);
			}
			return false;
		}

		template<typename String, typename StrOrInt, typename MatchCondition>
		void _load_reconnect_timer(String&& host, StrOrInt&& port, condition_wrap<MatchCondition> condition)
		{
			this->derived()._make_reconnect_timer(this->derived().selfptr(),
				[this, h = to_string(host), p = to_string(port), condition]() mutable
			{
				state_t expected = state_t::stopped;
				if (this->state_.compare_exchange_strong(expected, state_t::starting))
				{
					// can't use h = std::move(h), p = std::move(p); Otherwise, the value of h,p will
					// be empty the next time the code goto here.
					auto task = [this, h, p, condition](event_queue_guard<derived_t>&& g) mutable
					{
						this->derived().template _start_connect<true>(std::move(h), std::move(p),
							this->derived().selfptr(), std::move(condition));
					};

					this->derived().push_event([this, t = std::move(task)]
					(event_queue_guard<derived_t>&& g) mutable
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
		}

		template<typename MatchCondition>
		inline void _do_init(condition_wrap<MatchCondition>)
		{
			if constexpr (std::is_same_v<MatchCondition, use_dgram_t>)
				this->dgram_ = true;
			else
				this->dgram_ = false;
		}

		template<typename MatchCondition>
		inline void _do_start(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			this->update_alive_time();
			this->reset_connect_time();

			this->derived()._start_recv(std::move(this_ptr), std::move(condition));
		}

		inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			detail::ignore_unused(ec, this_ptr);

			this->derived()._rdc_stop();

			// should we close the socket in _handle_disconnect function? otherwise when send
			// data failed, will cause the _do_disconnect function be called, then cause the
			// auto reconnect executed, and then the _post_recv will be return with some error,
			// and the _post_recv will cause the auto reconnect executed again.

			// call socket's close function to notify the _handle_recv function response with 
			// error > 0 ,then the socket can get notify to exit
			// Call shutdown() to indicate that you will not write any more data to the socket.
			this->socket_.lowest_layer().shutdown(asio::socket_base::shutdown_both, ec_ignore);
			// Call close,otherwise the _handle_recv will never return
			this->socket_.lowest_layer().close(ec_ignore);
		}

		inline void _do_stop(const error_code& ec)
		{
			this->derived()._post_stop(ec, this->derived().selfptr());
		}

		inline void _post_stop(const error_code& ec, std::shared_ptr<derived_t> self_ptr)
		{
			// All pending sending events will be cancelled after enter the send strand below.
			auto task = [this, ec, this_ptr = std::move(self_ptr)](event_queue_guard<derived_t>&& g) mutable
			{
				set_last_error(ec);

				// call the base class stop function
				super::stop();

				// call CRTP polymorphic stop
				this->derived()._handle_stop(ec, std::move(this_ptr));
			};

			this->derived().push_event([this, t = std::move(task)](event_queue_guard<derived_t>&& g) mutable
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
			detail::ignore_unused(ec, this_ptr);
		}

		template<typename MatchCondition>
		inline void _start_recv(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			// Connect succeeded. post recv request.
			this->derived().post([this, self_ptr = std::move(this_ptr), condition = std::move(condition)]() mutable
			{
				if constexpr (!std::is_same_v<MatchCondition, asio2::detail::hook_buffer_t>)
				{
					this->derived().buffer().consume(this->derived().buffer().size());
				}
				else
				{
					std::ignore = true;
				}

				this->derived()._post_recv(std::move(self_ptr), std::move(condition));
			});
		}

		template<class Data, class Callback>
		inline bool _do_send(Data& data, Callback&& callback)
		{
			return this->derived()._tcp_send(data, std::forward<Callback>(callback));
		}

		template<class Data>
		inline send_data_t _rdc_convert_to_send_data(Data& data)
		{
			auto buffer = asio::buffer(data);
			return send_data_t{ reinterpret_cast<
				std::string_view::const_pointer>(buffer.data()),buffer.size() };
		}

		template<class Invoker>
		inline void _rdc_invoke_with_none(const error_code& ec, Invoker& invoker)
		{
			invoker(ec, send_data_t{}, recv_data_t{});
		}

		template<class Invoker>
		inline void _rdc_invoke_with_recv(const error_code& ec, Invoker& invoker, recv_data_t data)
		{
			invoker(ec, send_data_t{}, data);
		}

		template<class Invoker, class FnData>
		inline void _rdc_invoke_with_send(const error_code& ec, Invoker& invoker, FnData& fn_data)
		{
			invoker(ec, fn_data(), recv_data_t{});
		}

	protected:
		template<typename MatchCondition>
		inline void _post_recv(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			this->derived()._tcp_post_recv(std::move(this_ptr), std::move(condition));
		}

		template<typename MatchCondition>
		inline void _handle_recv(const error_code & ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			this->derived()._tcp_handle_recv(ec, bytes_recvd, std::move(this_ptr), std::move(condition));
		}

		inline void _fire_init()
		{
			this->listener_.notify(event_type::init);
		}

		template<typename MatchCondition>
		inline void _fire_recv(std::shared_ptr<derived_t>& this_ptr, std::string_view s,
			condition_wrap<MatchCondition>& condition)
		{
			this->listener_.notify(event_type::recv, s);

			this->derived()._rdc_handle_recv(this_ptr, s, condition);
		}

		template<typename MatchCondition>
		inline void _fire_connect(std::shared_ptr<derived_t>& this_ptr, error_code ec,
			condition_wrap<MatchCondition>& condition)
		{
			if constexpr (is_template_instance_of_v<use_rdc_t, MatchCondition>)
			{
				if (!ec)
				{
					this->derived()._rdc_start();
					this->derived()._rdc_post_wait(this_ptr, condition);
				}
			}
			else
			{
				std::ignore = true;
			}

			this->listener_.notify(event_type::connect, ec);
		}

		inline void _fire_disconnect(std::shared_ptr<derived_t>& this_ptr, error_code ec)
		{
			detail::ignore_unused(this_ptr);

			this->listener_.notify(event_type::disconnect, ec);
		}

	protected:
		bool dgram_ = false;
	};
}

namespace asio2
{
	class tcp_client : public detail::tcp_client_impl_t<tcp_client, detail::template_args_tcp_client>
	{
	public:
		using tcp_client_impl_t<tcp_client, detail::template_args_tcp_client>::tcp_client_impl_t;
	};
}

#endif // !__ASIO2_TCP_CLIENT_HPP__
