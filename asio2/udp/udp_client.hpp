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
	struct template_args_udp_client
	{
		static constexpr bool is_session = false;
		static constexpr bool is_client  = true;

		using socket_t    = asio::ip::udp::socket;
		using buffer_t    = asio2::linear_buffer;
		using send_data_t = std::string_view;
		using recv_data_t = std::string_view;
	};

	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_UDP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_UDP_CLIENT;

	template<class derived_t, class args_t>
	class udp_client_impl_t
		: public client_impl_t<derived_t, args_t>
		, public udp_send_op  <derived_t, args_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_UDP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_UDP_CLIENT;

	public:
		using super = client_impl_t    <derived_t, args_t>;
		using self  = udp_client_impl_t<derived_t, args_t>;

		using buffer_type = typename args_t::buffer_t;
		using send_data_t = typename args_t::send_data_t;
		using recv_data_t = typename args_t::recv_data_t;

		using super::send;

		/**
		 * @constructor
		 */
		explicit udp_client_impl_t(
			std::size_t init_buffer_size = udp_frame_size,
			std::size_t max_buffer_size  = (std::numeric_limits<std::size_t>::max)()
		)
			: super(1, init_buffer_size, max_buffer_size)
			, udp_send_op<derived_t, args_t>()
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
		template<typename String, typename StrOrInt, typename ParserFun>
		bool start(String&& host, StrOrInt&& port, ParserFun&& parser)
		{
			using fun_traits_type = function_traits<std::remove_cv_t<std::remove_reference_t<ParserFun>>>;
			using IdT = typename fun_traits_type::return_type;
			using SendDataT = typename fun_traits_type::template args<0>::type;
			using RecvDataT = typename fun_traits_type::template args<0>::type;

			return this->derived().template _do_connect<false>(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				condition_wrap<use_rdc_t<void, IdT, SendDataT, RecvDataT>>(
					std::in_place,
					std::forward<ParserFun>(parser)));
		}

		/**
		 * @function : start the client, blocking connect to server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt, typename SendParserFun, typename RecvParserFun>
		bool start(String&& host, StrOrInt&& port, SendParserFun&& send_parser, RecvParserFun&& recv_parser)
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
				condition_wrap<use_rdc_t<void, SendIdT, SendDataT, RecvDataT>>(
					std::in_place,
					std::forward<SendParserFun>(send_parser),
					std::forward<RecvParserFun>(recv_parser)));
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
		template<typename String, typename StrOrInt, typename ParserFun>
		bool start(String&& host, StrOrInt&& port, use_kcp_t c, ParserFun&& parser)
		{
			using fun_traits_type = function_traits<std::remove_cv_t<std::remove_reference_t<ParserFun>>>;
			using IdT = typename fun_traits_type::return_type;
			using SendDataT = typename fun_traits_type::template args<0>::type;
			using RecvDataT = typename fun_traits_type::template args<0>::type;

			return this->derived().template _do_connect<false>(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				condition_wrap<use_rdc_t<use_kcp_t, IdT, SendDataT, RecvDataT>>(
					std::in_place,
					std::move(c),
					std::forward<ParserFun>(parser)));
		}

		/**
		 * @function : start the client, blocking connect to server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt, typename SendParserFun, typename RecvParserFun>
		bool start(String&& host, StrOrInt&& port, use_kcp_t c, SendParserFun&& send_parser, RecvParserFun&& recv_parser)
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
				condition_wrap<use_rdc_t<use_kcp_t, SendIdT, SendDataT, RecvDataT>>(
					std::in_place,
					std::move(c),
					std::forward<SendParserFun>(send_parser),
					std::forward<RecvParserFun>(recv_parser)));
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
		template<typename String, typename StrOrInt, typename ParserFun>
		bool async_start(String&& host, StrOrInt&& port, ParserFun&& parser)
		{
			using fun_traits_type = function_traits<std::remove_cv_t<std::remove_reference_t<ParserFun>>>;
			using IdT = typename fun_traits_type::return_type;
			using SendDataT = typename fun_traits_type::template args<0>::type;
			using RecvDataT = typename fun_traits_type::template args<0>::type;

			return this->derived().template _do_connect<true>(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				condition_wrap<use_rdc_t<void, IdT, SendDataT, RecvDataT>>(
					std::in_place,
					std::forward<ParserFun>(parser)));
		}

		/**
		 * @function : start the client, blocking connect to server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt, typename SendParserFun, typename RecvParserFun>
		bool async_start(String&& host, StrOrInt&& port, SendParserFun&& send_parser, RecvParserFun&& recv_parser)
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
				condition_wrap<use_rdc_t<void, SendIdT, SendDataT, RecvDataT>>(
					std::in_place,
					std::forward<SendParserFun>(send_parser),
					std::forward<RecvParserFun>(recv_parser)));
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
		 * @function : start the client, blocking connect to server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt, typename ParserFun>
		bool async_start(String&& host, StrOrInt&& port, use_kcp_t c, ParserFun&& parser)
		{
			using fun_traits_type = function_traits<std::remove_cv_t<std::remove_reference_t<ParserFun>>>;
			using IdT = typename fun_traits_type::return_type;
			using SendDataT = typename fun_traits_type::template args<0>::type;
			using RecvDataT = typename fun_traits_type::template args<0>::type;

			return this->derived().template _do_connect<true>(
				std::forward<String>(host), std::forward<StrOrInt>(port),
				condition_wrap<use_rdc_t<use_kcp_t, IdT, SendDataT, RecvDataT>>(
					std::in_place,
					std::move(c),
					std::forward<ParserFun>(parser)));
		}

		/**
		 * @function : start the client, blocking connect to server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt, typename SendParserFun, typename RecvParserFun>
		bool async_start(String&& host, StrOrInt&& port, use_kcp_t c, SendParserFun&& send_parser, RecvParserFun&& recv_parser)
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
				condition_wrap<use_rdc_t<use_kcp_t, SendIdT, SendDataT, RecvDataT>>(
					std::in_place,
					std::move(c),
					std::forward<SendParserFun>(send_parser),
					std::forward<RecvParserFun>(recv_parser)));
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
			this->listener_.bind(event_type::handshake,
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
				this->derived()._handle_connect(e.code(), this->derived().selfptr(), std::move(condition));
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
			if constexpr (std::is_same_v<MatchCondition, use_kcp_t>)
				this->kcp_ = std::make_unique<kcp_stream_cp<derived_t, args_t>>(this->derived(), this->io_);
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

		inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			detail::ignore_unused(ec, this_ptr);

			this->derived()._rdc_stop();
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
			this->derived().post([this, this_ptr, condition = std::move(condition)]() mutable
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
		void _post_recv(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			if (!this->is_started())
				return;

			try
			{
				this->socket_.async_receive(this->buffer_.prepare(this->buffer_.pre_size()),
					asio::bind_executor(this->io_.strand(), make_allocator(this->rallocator_,
						[this, self_ptr = std::move(this_ptr), condition = std::move(condition)]
				(const error_code & ec, std::size_t bytes_recvd) mutable
				{
					this->derived()._handle_recv(ec, bytes_recvd, std::move(self_ptr), std::move(condition));
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
					this->derived()._fire_recv(this_ptr, std::move(s), condition);
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
						this->kcp_->_kcp_recv(this_ptr, s, this->buffer_, condition);
				}
			}

			this->buffer_.consume(this->buffer_.size());

			this->derived()._post_recv(std::move(this_ptr), std::move(condition));
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

		inline void _fire_handshake(std::shared_ptr<derived_t>& this_ptr, error_code ec)
		{
			detail::ignore_unused(this_ptr);

			this->listener_.notify(event_type::handshake, ec);
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
		std::unique_ptr<kcp_stream_cp<derived_t, args_t>> kcp_;
	};
}

namespace asio2
{
	class udp_client : public detail::udp_client_impl_t<udp_client, detail::template_args_udp_client>
	{
	public:
		using udp_client_impl_t<udp_client, detail::template_args_udp_client>::udp_client_impl_t;
	};
}

#endif // !__ASIO2_UDP_CLIENT_HPP__
