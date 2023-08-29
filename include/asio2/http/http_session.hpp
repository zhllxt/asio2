/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_HTTP_SESSION_HPP__
#define __ASIO2_HTTP_SESSION_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <asio2/tcp/tcp_session.hpp>
#include <asio2/http/impl/ws_stream_cp.hpp>
#include <asio2/http/impl/http_send_op.hpp>
#include <asio2/http/impl/http_recv_op.hpp>
#include <asio2/http/impl/ws_send_op.hpp>
#include <asio2/http/detail/http_router.hpp>

namespace asio2::detail
{
	struct template_args_http_session : public template_args_tcp_session
	{
		using stream_t    = websocket::stream<typename template_args_tcp_session::socket_t&>;
		using body_t      = http::string_body;
		using buffer_t    = beast::flat_buffer;
		using send_data_t = std::string_view;
		using recv_data_t = std::string_view;
	};

	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SESSION;

	template<class derived_t, class args_t = template_args_http_session>
	class http_session_impl_t
		: public tcp_session_impl_t<derived_t, args_t>
		, public http_send_op      <derived_t, args_t>
		, public http_recv_op      <derived_t, args_t>
		, public ws_stream_cp      <derived_t, args_t>
		, public ws_send_op        <derived_t, args_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION;

	public:
		using super = tcp_session_impl_t <derived_t, args_t>;
		using self  = http_session_impl_t<derived_t, args_t>;

		using args_type   = args_t;
		using key_type    = std::size_t;
		using body_type   = typename args_t::body_t;
		using buffer_type = typename args_t::buffer_t;
		using send_data_t = typename args_t::send_data_t;
		using recv_data_t = typename args_t::recv_data_t;

		using ws_stream_comp = ws_stream_cp<derived_t, args_t>;

		using super::send;
		using super::async_send;

	public:
		/**
		 * @brief constructor
		 */
		explicit http_session_impl_t(
			http_router_t<derived_t, args_t> & router,
			std::filesystem::path            & root_directory,
			bool                               is_arg0_session,
			bool                               support_websocket,
			session_mgr_t<derived_t>         & sessions,
			listener_t                       & listener,
			std::shared_ptr<io_t>              rwio,
			std::size_t                        init_buf_size,
			std::size_t                        max_buf_size
		)
			: super(sessions, listener, std::move(rwio), init_buf_size, max_buf_size)
			, http_send_op<derived_t, args_t>()
			, ws_stream_cp<derived_t, args_t>()
			, ws_send_op  <derived_t, args_t>()
			, req_()
			, rep_()
			, router_(router)
			, root_directory_   (root_directory)
			, is_arg0_session_  (is_arg0_session)
			, support_websocket_(support_websocket)
		{
			this->silence_timeout_ = std::chrono::milliseconds(http_silence_timeout);
		}

		/**
		 * @brief destructor
		 */
		~http_session_impl_t()
		{
		}

	protected:
		/**
		 * @brief start the session for prepare to recv/send msg
		 */
		template<typename C>
		inline void start(std::shared_ptr<ecs_t<C>> ecs)
		{
			this->rep_.set_root_directory(this->root_directory_);
			this->rep_.session_ptr_ = this->derived().selfptr();
			this->rep_.defer_callback_ = [this, ecs, wptr = std::weak_ptr<derived_t>(this->derived().selfptr())]
			() mutable
			{
				std::shared_ptr<derived_t> this_ptr = wptr.lock();
				ASIO2_ASSERT(this_ptr);
				if (this_ptr)
				{
					this->derived()._do_send_http_response(std::move(this_ptr), std::move(ecs), this->rep_.base());
				}
			};

			super::start(std::move(ecs));
		}

	public:
		/**
		 * @brief destroy the content of all member variables, this is used for solve the memory leaks.
		 * After this function is called, this class object cannot be used again.
		 */
		inline void destroy()
		{
			derived_t& derive = this->derived();

			derive.ws_stream_.reset();

			super::destroy();
		}

	public:
		/**
		 * @brief get this object hash key,used for session map
		 */
		inline key_type hash_key() const noexcept
		{
			return reinterpret_cast<key_type>(this);
		}

		inline bool is_websocket() const noexcept { return (bool(this->websocket_router_)); }
		inline bool is_http     () const noexcept { return (!(this->is_websocket()));       }

		/**
		 * @brief get the request object, same as get_request
		 */
		inline       http::web_request & request()       noexcept { return this->req_; }

		/**
		 * @brief get the request object, same as get_request
		 */
		inline const http::web_request & request() const noexcept { return this->req_; }

		/**
		 * @brief get the response object, same as get_response
		 */
		inline       http::web_response& response()       noexcept { return this->rep_; }

		/**
		 * @brief get the response object, same as get_response
		 */
		inline const http::web_response& response() const noexcept { return this->rep_; }

		/**
		 * @brief get the request object
		 */
		inline       http::web_request & get_request()       noexcept { return this->req_; }

		/**
		 * @brief get the request object
		 */
		inline const http::web_request & get_request() const noexcept { return this->req_; }

		/**
		 * @brief get the response object
		 */
		inline       http::web_response& get_response()       noexcept { return this->rep_; }

		/**
		 * @brief get the response object
		 */
		inline const http::web_response& get_response() const noexcept { return this->rep_; }

		/**
		 * @brief set how to send the http response in the bind_recv callback
		 * automatic - The framework automatically send the http response
		 * manual    - You need to send the http response youself
		 */
		inline derived_t& set_response_mode(asio2::response_mode mode)
		{
			this->response_mode_ = mode;
			return (static_cast<derived_t&>(*this));
		}

		/**
		 * @brief get the response mode
		 */
		inline asio2::response_mode get_response_mode() const { return this->response_mode_; }

	protected:
		inline http_router_t<derived_t, args_t>& _router() noexcept
		{
			return (this->router_);
		}

		template<typename C>
		inline void _do_init(std::shared_ptr<derived_t>& this_ptr, std::shared_ptr<ecs_t<C>>& ecs)
		{
			super::_do_init(this_ptr, ecs);

			if (this->support_websocket_)
			{
				this->derived()._ws_init(ecs, this->derived().stream());
			}
		}

		template<typename DeferEvent>
		inline void _post_shutdown(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			ASIO2_LOG_DEBUG("http_session::_post_shutdown: {} {}", ec.value(), ec.message());

			if (this->derived().is_http())
			{
				super::_post_shutdown(ec, std::move(this_ptr), std::move(chain));
			}
			else
			{
				this->derived()._ws_stop(this_ptr, defer_event
				{
					[this, ec, this_ptr, e = chain.move_event()] (event_queue_guard<derived_t> g) mutable
					{
						super::_post_shutdown(ec, std::move(this_ptr), defer_event(std::move(e), std::move(g)));
					}, chain.move_guard()
				});
			}
		}

		template<typename DeferEvent>
		inline void _handle_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			// can not use std::move(this_ptr), beacuse after handle stop with std::move(this_ptr),
			// this object maybe destroyed, then call "this" will crash.
			super::_handle_stop(ec, this_ptr, std::move(chain));

			// reset the callback shared_ptr, to avoid the callback owned this self shared_ptr.
			this->websocket_router_.reset();
		}

		template<typename C, class MessageT>
		inline void _send_http_response(
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, MessageT& msg)
		{
			ASIO2_ASSERT(this->derived().io_->running_in_this_thread());

			if (this->rep_.defer_guard_)
			{
				this->rep_.defer_guard_.reset();
			}
			else
			{
				if (this->response_mode_ == asio2::response_mode::automatic)
				{
					this->derived()._do_send_http_response(std::move(this_ptr), std::move(ecs), msg);
				}
				// if the manual mode is used, then the user maybe use async send to send the 
				// response by self, at this time, the post recv will can't be called automaticly,
				// so we need to call it manualy.
				// But there may be some drawbacks in this situation:
				// we can't call the post recv inside the async send callback, beacuse the 
				// user maybe call async send with a string, and the string is just half of
				// http protocol data, and we don't know when the all completed http protocol
				// data will send finished.
				// so we have to call the post recv directly at here.
				// So the user should be best to send a completed http protocol packet at once.
				else
				{
					this->derived()._post_recv(std::move(this_ptr), std::move(ecs));
				}
			}
		}

		template<typename C, class MessageT>
		inline void _do_send_http_response_impl(
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, MessageT& msg)
		{
			ASIO2_ASSERT(this->is_http());
			ASIO2_ASSERT(this->io_->running_in_this_thread());

			derived_t& derive = this->derived();

			if (derive.is_websocket())
				return;

			if (!derive.is_started())
				return;

			// be careful: here we pushed the reference of the msg into the queue, so the msg object
			// must can't be destroyed or modifyed.
			derive.push_event([&derive, this_ptr = std::move(this_ptr), ecs = std::move(ecs), &msg]
			(event_queue_guard<derived_t> g) mutable
			{
				// use this_ptr to instead of std::move(this_ptr) in the lambda capture has better safety.
				derive._do_send(msg,
				[&derive, this_ptr, ecs = std::move(ecs), g = std::move(g)]
				(const error_code&, std::size_t) mutable
				{
					ASIO2_ASSERT(!g.is_empty());

				#if defined(ASIO2_ENABLE_LOG)
					auto now = std::chrono::system_clock::now();
					auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
					ASIO2_LOG_TRACE("http_session::send {:%Y-%m-%d %H:%M:%S}.{:03d} {}",
						now, ms % 1000, derive.req_.target());
				#endif

					// after send the response, we check if the client should be disconnect.
					if (derive.req_.need_eof())
					{
						ASIO2_LOG_DEBUG("http_session send response need_eof");

						// session maybe don't need check the state.
						//if (derive.state_ == state_t::started)
						derive._do_disconnect(asio::error::operation_aborted, std::move(this_ptr));
					}
					else
					{
						derive._post_recv(std::move(this_ptr), std::move(ecs));
					}
				});
			});
		}

		template<typename C, class MessageT>
		inline void _do_send_http_response(
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, MessageT& msg)
		{
			derived_t& derive = this->derived();

			if (derive.io_->running_in_this_thread())
			{
				derive._do_send_http_response_impl(std::move(this_ptr), std::move(ecs), msg);
				return;
			}

			asio::post(derive.io_->context(), make_allocator(derive.wallocator(),
			[&derive, this_ptr = std::move(this_ptr), ecs = std::move(ecs), &msg]() mutable
			{
				derive._do_send_http_response_impl(std::move(this_ptr), std::move(ecs), msg);
			}));
		}

	protected:
		template<class Data, class Callback>
		inline bool _do_send(Data& data, Callback&& callback)
		{
			if (this->derived().is_websocket())
				return this->derived()._ws_send(data, std::forward<Callback>(callback));

			return this->derived()._http_send(data, std::forward<Callback>(callback));
		}

		template<class Data>
		inline send_data_t _rdc_convert_to_send_data(Data& data) noexcept
		{
			ASIO2_ASSERT(this->websocket_router_ && "Only available in websocket mode");

			set_last_error(asio::error::operation_not_supported);

			auto buffer = asio::buffer(data);
			return send_data_t{ reinterpret_cast<
				std::string_view::const_pointer>(buffer.data()),buffer.size() };
		}

		template<class Invoker>
		inline void _rdc_invoke_with_none(const error_code& ec, Invoker& invoker)
		{
			ASIO2_ASSERT(this->websocket_router_ && "Only available in websocket mode");
			if (this->derived().is_websocket() && invoker)
				invoker(ec, send_data_t{}, recv_data_t{});
		}

		template<class Invoker>
		inline void _rdc_invoke_with_recv(const error_code& ec, Invoker& invoker, recv_data_t data)
		{
			ASIO2_ASSERT(this->websocket_router_ && "Only available in websocket mode");
			if (this->derived().is_websocket() && invoker)
				invoker(ec, send_data_t{}, data);
		}

		template<class Invoker>
		inline void _rdc_invoke_with_send(const error_code& ec, Invoker& invoker, send_data_t data)
		{
			ASIO2_ASSERT(this->websocket_router_ && "Only available in websocket mode");
			if (this->derived().is_websocket() && invoker)
				invoker(ec, data, recv_data_t{});
		}

	protected:
		template<typename C>
		inline void _post_recv(std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			this->derived()._http_post_recv(std::move(this_ptr), std::move(ecs));
		}

		template<typename C>
		inline void _handle_recv(
			const error_code& ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			this->derived()._http_handle_recv(ec, bytes_recvd, std::move(this_ptr), std::move(ecs));
		}

		template<typename C, typename DeferEvent>
		inline void _handle_upgrade(
			const error_code& ec,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			this->derived().sessions_.post(
			[this, ec, this_ptr = std::move(this_ptr), ecs = std::move(ecs), chain = std::move(chain)]
			() mutable
			{
				set_last_error(ec);

				this->derived()._fire_upgrade(this_ptr);

				if (ec)
				{
					this->derived()._do_disconnect(ec, std::move(this_ptr), std::move(chain));

					return;
				}

				asio::post(this->derived().io_->context(), make_allocator(this->derived().wallocator(),
				[this, this_ptr = std::move(this_ptr), ecs = std::move(ecs), chain = std::move(chain)]
				() mutable
				{
					detail::ignore_unused(chain);

					this->derived()._post_recv(std::move(this_ptr), std::move(ecs));
				}));
			});
		}

		template<typename C>
		inline bool _check_upgrade(std::shared_ptr<derived_t>& this_ptr, std::shared_ptr<ecs_t<C>>& ecs)
		{
			derived_t& derive = this->derived();

			if (this->support_websocket_ && derive.is_http() && this->req_.is_upgrade())
			{
				this->websocket_router_ = this->router_.template _find<false>(this->req_, this->rep_);

				if (this->websocket_router_)
				{
					this->req_.ws_frame_type_ = websocket::frame::open;
					this->req_.ws_frame_data_ = {};

					if (this->router_._route(this_ptr, this->req_, this->rep_))
					{
						derive.silence_timeout_ = std::chrono::milliseconds(tcp_silence_timeout);

						derive._ws_start(this_ptr, ecs, derive.stream());

						derive._post_control_callback(this_ptr, ecs);
						derive.push_event(
						[&derive, this_ptr, ecs](event_queue_guard<derived_t> g) mutable
						{
							derive._post_upgrade(
								this_ptr, std::move(ecs), derive.req_.base(),
								defer_event([](event_queue_guard<derived_t>) {}, std::move(g)));
						});
					}
					else
					{
						this->req_.ws_frame_type_ = websocket::frame::unknown;
						this->req_.ws_frame_data_ = {};

						this->websocket_router_.reset();

						derive._send_http_response(this_ptr, ecs, this->rep_.base());
					}

					// If find websocket router, the router callback must has been called, 
					// so as long as we find the websocket router, we return true
					// If don't do this, the fire recv will be called, then the router callback
					// will be called again.
					return true;
				}
			}
			return false;
		}

		template<typename C>
		inline void _handle_control_ping(
			beast::string_view payload, std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			detail::ignore_unused(payload, this_ptr, ecs);

			this->req_.ws_frame_type_ = websocket::frame::ping;
			this->req_.ws_frame_data_ = payload;

			this->router_._route(this_ptr, this->req_, this->rep_);
		}

		template<typename C>
		inline void _handle_control_pong(
			beast::string_view payload, std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			detail::ignore_unused(payload, this_ptr, ecs);

			this->req_.ws_frame_type_ = websocket::frame::pong;
			this->req_.ws_frame_data_ = payload;

			this->router_._route(this_ptr, this->req_, this->rep_);
		}

		template<typename C>
		inline void _handle_control_close(
			beast::string_view payload, std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			detail::ignore_unused(payload, this_ptr, ecs);

			this->req_.ws_frame_type_ = websocket::frame::close;
			this->req_.ws_frame_data_ = payload;

			this->router_._route(this_ptr, this->req_, this->rep_);

			// session maybe don't need check the state.
			//if (this->derived().state_ == state_t::started)
			{
				ASIO2_LOG_DEBUG("http_session::_handle_control_close _do_disconnect");

				this->derived()._do_disconnect(websocket::error::closed, std::move(this_ptr));
			}
		}

		inline void _fire_upgrade(std::shared_ptr<derived_t>& this_ptr)
		{
			// the _fire_upgrade must be executed in the thread 0.
			ASIO2_ASSERT(this->sessions_.io_->running_in_this_thread());

			this->listener_.notify(event_type::upgrade, this_ptr);
		}

		template<typename C>
		inline void _fire_recv(std::shared_ptr<derived_t>& this_ptr, std::shared_ptr<ecs_t<C>>& ecs)
		{
			if (this->is_arg0_session_)
				this->listener_.notify(event_type::recv, this_ptr, this->req_, this->rep_);
			else
				this->listener_.notify(event_type::recv, this->req_, this->rep_);

		#if defined(ASIO2_ENABLE_LOG)
			auto now = std::chrono::system_clock::now();
			auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
			ASIO2_LOG_TRACE("http_session::recv {:%Y-%m-%d %H:%M:%S}.{:03d} {}",
				now, ms % 1000, this->req_.target());
		#endif

			auto* prep = this->router_._route(this_ptr, this->req_, this->rep_);

			if (this->derived().is_websocket())
			{
				this->derived()._rdc_handle_recv(this_ptr, ecs, this->req_.ws_frame_data_);
			}
			else
			{
				this->derived()._send_http_response(this_ptr, ecs, *prep);
			}
		}

	protected:
		http::web_request                   req_;

		http::web_response                  rep_;

		http_router_t<derived_t, args_t>  & router_;

		std::filesystem::path             & root_directory_;

		bool                                is_arg0_session_    = false;

		bool                                support_websocket_  = false;

		asio2::response_mode                response_mode_      = asio2::response_mode::automatic;

		std::shared_ptr<typename http_router_t<derived_t, args_t>::opfun>   websocket_router_;
	};
}

namespace asio2
{
	using http_session_args = detail::template_args_http_session;

	template<class derived_t, class args_t>
	using http_session_impl_t = detail::http_session_impl_t<derived_t, args_t>;

	template<class derived_t>
	class http_session_t : public detail::http_session_impl_t<derived_t, detail::template_args_http_session>
	{
	public:
		using detail::http_session_impl_t<derived_t, detail::template_args_http_session>::http_session_impl_t;
	};

	class http_session : public http_session_t<http_session>
	{
	public:
		using http_session_t<http_session>::http_session_t;
	};
}

#if defined(ASIO2_INCLUDE_RATE_LIMIT)
#include <asio2/tcp/tcp_stream.hpp>
namespace asio2
{
	struct http_rate_session_args : public http_session_args
	{
		using socket_t = asio2::tcp_stream<asio2::simple_rate_policy>;
		using stream_t = websocket::stream<socket_t&>;
	};

	template<class derived_t>
	class http_rate_session_t : public asio2::http_session_impl_t<derived_t, http_rate_session_args>
	{
	public:
		using asio2::http_session_impl_t<derived_t, http_rate_session_args>::http_session_impl_t;
	};

	class http_rate_session : public asio2::http_rate_session_t<http_rate_session>
	{
	public:
		using asio2::http_rate_session_t<http_rate_session>::http_rate_session_t;
	};
}
#endif

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_HTTP_SESSION_HPP__
