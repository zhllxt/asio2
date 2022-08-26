/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_HTTP_SESSION_HPP__
#define __ASIO2_HTTP_SESSION_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <asio2/tcp/tcp_session.hpp>
#include <asio2/http/component/ws_stream_cp.hpp>
#include <asio2/http/impl/http_send_op.hpp>
#include <asio2/http/impl/http_recv_op.hpp>
#include <asio2/http/impl/ws_send_op.hpp>
#include <asio2/http/detail/http_router.hpp>

namespace asio2::detail
{
	struct template_args_http_session : public template_args_tcp_session
	{
		using stream_t    = websocket::stream<asio::ip::tcp::socket&>;
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
		 * @constructor
		 */
		explicit http_session_impl_t(
			http_router_t<derived_t, args_t> & router,
			std::filesystem::path            & root_directory,
			bool                               is_arg0_session,
			bool                               support_websocket,
			session_mgr_t<derived_t>         & sessions,
			listener_t                       & listener,
			io_t                             & rwio,
			std::size_t                        init_buf_size,
			std::size_t                        max_buf_size
		)
			: super(sessions, listener, rwio, init_buf_size, max_buf_size)
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
		 * @destructor
		 */
		~http_session_impl_t()
		{
		}

	protected:
		/**
		 * @function : start the session for prepare to recv/send msg
		 */
		template<typename MatchCondition>
		inline void start(condition_wrap<MatchCondition> condition)
		{
			this->rep_.set_root_directory(this->root_directory_);
			this->rep_.session_ptr_ = this->selfptr();
			this->rep_.defer_callback_ = [this, condition, wptr = std::weak_ptr<derived_t>(this->selfptr())]
			() mutable
			{
				std::shared_ptr<derived_t> this_ptr = wptr.lock();
				ASIO2_ASSERT(this_ptr);
				if (this_ptr)
				{
					this->derived()._do_send_http_response(std::move(this_ptr), std::move(condition));
				}
			};

			super::start(std::move(condition));
		}

	public:
		/**
		 * @function : get this object hash key,used for session map
		 */
		inline key_type hash_key() const noexcept
		{
			return reinterpret_cast<key_type>(this);
		}

		inline bool is_websocket() noexcept { return (bool(this->websocket_router_)); }
		inline bool is_http     () noexcept { return (!(this->is_websocket()));       }

		/**
		 * @function : get the request object, same as get_request
		 */
		inline const http::web_request & request() noexcept { return this->req_; }

		/**
		 * @function : get the response object, same as get_response
		 */
		inline const http::web_response& response() noexcept { return this->rep_; }

		/**
		 * @function : get the request object
		 */
		inline const http::web_request & get_request() noexcept { return this->req_; }

		/**
		 * @function : get the response object
		 */
		inline const http::web_response& get_response() noexcept { return this->rep_; }

		/**
		 * @function : set how to send the http response in the bind_recv callback
		 * automatic - The framework automatically send the http response
		 * manual    - You need to send the http response youself
		 */
		inline derived_t& set_response_mode(asio2::response_mode mode)
		{
			this->response_mode_ = mode;
			return (static_cast<derived_t&>(*this));
		}

		/**
		 * @function : get the response mode
		 */
		inline asio2::response_mode get_response_mode() { return this->response_mode_; }

	protected:
		inline http_router_t<derived_t, args_t>& _router() noexcept
		{
			return (this->router_);
		}

		template<typename MatchCondition>
		inline void _do_init(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			super::_do_init(std::move(this_ptr), condition);

			if (this->support_websocket_)
			{
				this->derived()._ws_init(condition, this->derived().stream());
			}
		}

		template<typename DeferEvent>
		inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			this->derived()._rdc_stop();

			if (this->derived().is_http())
			{
				super::_handle_disconnect(ec, std::move(this_ptr), std::move(chain));
			}
			else
			{
				this->derived()._ws_stop(this_ptr,
					defer_event
					{
						[this, ec, this_ptr, e = chain.move_event()] (event_queue_guard<derived_t> g) mutable
						{
							super::_handle_disconnect(ec, std::move(this_ptr), defer_event(std::move(e), std::move(g)));
						}, chain.move_guard()
					}
				);
			}
		}

		template<typename DeferEvent>
		inline void _do_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			// call the base class _do_stop function
			super::_do_stop(ec, std::move(this_ptr), std::move(chain));

			// reset the callback shared_ptr, to avoid the callback owned this self shared_ptr.
			this->websocket_router_.reset();
		}

		template<typename MatchCondition>
		inline void _send_http_response(
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			if (this->rep_.defer_guard_)
			{
				this->rep_.defer_guard_.reset();
			}
			else
			{
				if (this->response_mode_ == asio2::response_mode::automatic)
				{
					this->derived()._do_send_http_response(std::move(this_ptr), std::move(condition));
				}
			}
		}

		template<typename MatchCondition>
		inline void _do_send_http_response(
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			ASIO2_ASSERT(this->is_http());

			asio::dispatch(this->derived().io().context(), make_allocator(this->derived().wallocator(),
			[this, this_ptr = std::move(this_ptr), condition = std::move(condition)]() mutable
			{
				if (this->derived().is_websocket())
					return;

				this->derived().async_send(std::move(this->rep_.base()),
				[this, this_ptr = std::move(this_ptr), condition = std::move(condition)]() mutable
				{
					// after send the response, we check if the client should be disconnect.
					if (this->derived().req_.need_eof())
					{
						// session maybe don't need check the state.
						//if (this->derived().state() == state_t::started)
						this->derived()._do_disconnect(asio::error::operation_aborted, std::move(this_ptr));
					}
					else
					{
						this->derived()._post_recv(std::move(this_ptr), std::move(condition));
					}
				});
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

		template<class Invoker, class FnData>
		inline void _rdc_invoke_with_send(const error_code& ec, Invoker& invoker, FnData& fn_data)
		{
			ASIO2_ASSERT(this->websocket_router_ && "Only available in websocket mode");
			if (this->derived().is_websocket() && invoker)
				invoker(ec, fn_data(), recv_data_t{});
		}

	protected:
		template<typename MatchCondition>
		inline void _post_recv(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			this->derived()._http_post_recv(std::move(this_ptr), std::move(condition));
		}

		template<typename MatchCondition>
		inline void _handle_recv(const error_code& ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			this->derived()._http_handle_recv(ec, bytes_recvd, std::move(this_ptr), std::move(condition));
		}

		template<typename MatchCondition, typename DeferEvent>
		inline void _handle_upgrade(const error_code & ec, std::shared_ptr<derived_t> self_ptr,
			condition_wrap<MatchCondition> condition, DeferEvent chain)
		{
			this->derived().sessions().post(
			[this, ec, this_ptr = std::move(self_ptr), condition = std::move(condition), chain = std::move(chain)]
			() mutable
			{
				try
				{
					set_last_error(ec);

					this->derived()._fire_upgrade(this_ptr);

					asio::detail::throw_error(ec);

					asio::post(this->derived().io().context(), make_allocator(this->derived().wallocator(),
					[this, this_ptr = std::move(this_ptr), condition = std::move(condition), chain = std::move(chain)]
					() mutable
					{
						detail::ignore_unused(chain);

						this->derived()._post_recv(std::move(this_ptr), std::move(condition));
					}));
				}
				catch (system_error & e)
				{
					set_last_error(e);

					this->derived()._do_disconnect(e.code(), this->derived().selfptr(), std::move(chain));
				}
			});
		}

		template<typename MatchCondition>
		inline bool _check_upgrade(std::shared_ptr<derived_t>& this_ptr, condition_wrap<MatchCondition>& condition)
		{
			if (this->support_websocket_ && this->derived().is_http() && this->req_.is_upgrade())
			{
				this->websocket_router_ = this->router_.template _find<false>(this->req_, this->rep_);

				if (this->websocket_router_)
				{
					this->req_.ws_frame_type_ = websocket::frame::open;
					this->req_.ws_frame_data_ = {};

					if (this->router_._route(this_ptr, this->req_, this->rep_))
					{
						this->derived().silence_timeout_ = std::chrono::milliseconds(tcp_silence_timeout);

						this->derived()._ws_start(this_ptr, condition, this->derived().stream());

						this->derived()._post_control_callback(this_ptr, condition);
						this->derived().push_event(
						[this, this_ptr = std::move(this_ptr), condition = std::move(condition)]
						(event_queue_guard<derived_t> g) mutable
						{
							this->derived()._post_upgrade(
								std::move(this_ptr), std::move(condition), this->req_.base(),
								defer_event([](event_queue_guard<derived_t>) {}, std::move(g)));
						});
					}
					else
					{
						this->req_.ws_frame_type_ = websocket::frame::unknown;
						this->req_.ws_frame_data_ = {};

						this->websocket_router_.reset();

						this->derived()._send_http_response(this_ptr, condition);
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

		template<typename MatchCondition>
		inline void _handle_control_ping(beast::string_view payload,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			detail::ignore_unused(payload, this_ptr, condition);

			this->req_.ws_frame_type_ = websocket::frame::ping;
			this->req_.ws_frame_data_ = payload;

			this->router_._route(this_ptr, this->req_, this->rep_);
		}

		template<typename MatchCondition>
		inline void _handle_control_pong(beast::string_view payload,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			detail::ignore_unused(payload, this_ptr, condition);

			this->req_.ws_frame_type_ = websocket::frame::pong;
			this->req_.ws_frame_data_ = payload;

			this->router_._route(this_ptr, this->req_, this->rep_);
		}

		template<typename MatchCondition>
		inline void _handle_control_close(beast::string_view payload,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			detail::ignore_unused(payload, this_ptr, condition);

			this->req_.ws_frame_type_ = websocket::frame::close;
			this->req_.ws_frame_data_ = payload;

			this->router_._route(this_ptr, this->req_, this->rep_);

			// session maybe don't need check the state.
			//if (this->derived().state() == state_t::started)
			//{
				this->derived()._do_disconnect(websocket::error::closed, std::move(this_ptr));
			//}
		}

		inline void _fire_upgrade(std::shared_ptr<derived_t>& this_ptr)
		{
			// the _fire_upgrade must be executed in the thread 0.
			ASIO2_ASSERT(this->sessions().io().running_in_this_thread());

			this->listener_.notify(event_type::upgrade, this_ptr);
		}

		template<typename MatchCondition>
		inline void _fire_recv(std::shared_ptr<derived_t>& this_ptr, condition_wrap<MatchCondition>& condition)
		{
			if (this->is_arg0_session_)
				this->listener_.notify(event_type::recv, this_ptr, this->req_, this->rep_);
			else
				this->listener_.notify(event_type::recv, this->req_, this->rep_);

			this->router_._route(this_ptr, this->req_, this->rep_);

			if (this->derived().is_websocket())
			{
				this->derived()._rdc_handle_recv(this_ptr, this->req_.ws_frame_data_, condition);
			}
			else
			{
				this->derived()._send_http_response(this_ptr, condition);
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

		std::shared_ptr<typename http_router_t<derived_t, args_t>::optype>   websocket_router_;
	};
}

namespace asio2
{
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

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_HTTP_SESSION_HPP__
