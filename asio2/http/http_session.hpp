/*
 * COPYRIGHT (C) 2017-2019, zhllxt
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

#include <asio2/tcp/tcp_session.hpp>
#include <asio2/http/component/http_send_cp.hpp>
#include <asio2/http/component/ws_stream_cp.hpp>
#include <asio2/http/impl/http_send_op.hpp>
#include <asio2/http/impl/http_recv_op.hpp>
#include <asio2/http/impl/ws_send_op.hpp>
#include <asio2/http/detail/router.hpp>

namespace asio2::detail
{
	template <class>                      class session_mgr_t;
	template <class, class>               class tcp_server_impl_t;
	template <class, class>               class http_server_impl_t;

	template<class derived_t, class socket_t, class stream_t, class body_t, class buffer_t>
	class http_session_impl_t
		: public tcp_session_impl_t<derived_t, socket_t, buffer_t>
		, public http_send_cp<derived_t, body_t, buffer_t, true>
		, public http_send_op<derived_t, body_t, buffer_t, true>
		, public http_recv_op<derived_t, body_t, buffer_t, true>
		, public ws_stream_cp<derived_t, stream_t, true>
		, public ws_send_op<derived_t, true>
	{
		template <class, bool>                friend class user_timer_cp;
		template <class>                      friend class post_cp;
		template <class, class, bool>         friend class connect_cp;
		template <class, class, bool>         friend class disconnect_cp;
		template <class>                      friend class data_persistence_cp;
		template <class>                      friend class event_queue_cp;
		template <class, bool>                friend class send_cp;
		template <class, bool>                friend class silence_timer_cp;
		template <class, bool>                friend class connect_timeout_cp;
		template <class, bool>                friend class tcp_send_op;
		template <class, bool>                friend class tcp_recv_op;
		template <class, class, class, bool>  friend class http_send_cp;
		template <class, class, class, bool>  friend class http_send_op;
		template <class, class, class, bool>  friend class http_recv_op;
		template <class, class, bool>         friend class ws_stream_cp;
		template <class, bool>                friend class ws_send_op;
		template <class>                      friend class session_mgr_t;
		template <class, class, class>        friend class session_impl_t;
		template <class, class, class>        friend class tcp_session_impl_t;
		template <class, class>               friend class tcp_server_impl_t;
		template <class, class>               friend class http_server_impl_t;
		template <class>                      friend class http_router_t;

	public:
		using self = http_session_impl_t<derived_t, socket_t, stream_t, body_t, buffer_t>;
		using super = tcp_session_impl_t<derived_t, socket_t, buffer_t>;
		using key_type = std::size_t;
		using body_type = body_t;
		using buffer_type = buffer_t;
		using ws_stream_comp = ws_stream_cp<derived_t, stream_t, true>;
		using super::send;
		using http_send_cp<derived_t, body_t, buffer_t, true>::send;
		using data_persistence_cp<derived_t>::_data_persistence;

		/**
		 * @constructor
		 */
		explicit http_session_impl_t(
			http_router_t<derived_t> & router,
			std::filesystem::path    & root_directory,
			bool                       is_arg0_session,
			bool                       support_websocket,
			session_mgr_t<derived_t> & sessions,
			listener_t               & listener,
			io_t                     & rwio,
			std::size_t                init_buffer_size,
			std::size_t                max_buffer_size
		)
			: super(sessions, listener, rwio, init_buffer_size, max_buffer_size)
			, http_send_cp<derived_t, body_t, buffer_t, true>(rwio)
			, http_send_op<derived_t, body_t, buffer_t, true>()
			, ws_stream_comp()
			, ws_send_op<derived_t, true>()
			, req_()
			, rep_()
			, router_(router)
			, root_directory_(root_directory)
			, is_arg0_session_(is_arg0_session)
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
			this->rep_.root_directory(this->root_directory_);
			this->rep_.defer_callback_ = [this, condition,
				wptr = std::weak_ptr<derived_t>(this->selfptr())]() mutable
			{
				std::shared_ptr<derived_t> this_ptr = wptr.lock();
				if (this_ptr)
				{
					this->derived()._send_response(std::move(this_ptr), condition);
				}
			};

			super::start(std::move(condition));
		}

	public:
		/**
		 * @function : get this object hash key,used for session map
		 */
		inline const key_type hash_key() const
		{
			return reinterpret_cast<key_type>(this);
		}

		inline bool is_websocket() { return (bool(this->websocket_router_)); }
		inline bool is_http()      { return (!(this->is_websocket()));       }

		/**
		 * @function : get the request object
		 */
		inline const http::request & request()  { return this->req_; }

		/**
		 * @function : get the response object
		 */
		inline const http::response& response() { return this->rep_; }

	protected:
		inline http_router_t<derived_t>& _router()
		{
			return (this->router_);
		}

		inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			if (this->is_http())
			{
				super::_handle_disconnect(ec, std::move(this_ptr));
			}
			else
			{
				this->derived()._ws_stop(this_ptr, [this, ec, this_ptr]() mutable
				{
					super::_handle_disconnect(ec, std::move(this_ptr));
				});
			}
		}

		inline void _do_stop(const error_code& ec)
		{
			// call the base class _do_stop function
			super::_do_stop(ec);

			// reset the callback shared_ptr, to avoid the callback owned this self shared_ptr.
			this->websocket_router_.reset();
		}

		template<typename MatchCondition>
		inline void _send_response(std::shared_ptr<derived_t> this_ptr,
			condition_wrap<MatchCondition> condition)
		{
			ASIO2_ASSERT(this->is_http());

			if (!this->derived().io().strand().running_in_this_thread())
			{
				this->derived().post([this, this_ptr = std::move(this_ptr), condition]() mutable
				{
					this->derived()._send_response(std::move(this_ptr), std::move(condition));
				});
				return;
			}

			if (this->is_websocket())
				return;

			this->derived().send(std::move(this->rep_.base()),
				[this, this_ptr = std::move(this_ptr), condition]() mutable
			{
				this->derived()._post_recv(std::move(this_ptr), std::move(condition));
			});
		}

	protected:
		template<bool isRequest, class Body, class Fields = http::fields>
		inline auto _data_persistence(http::message<isRequest, Body, Fields>& msg)
		{
			return this->derived()._data_persistence(
				const_cast<const http::message<isRequest, Body, Fields>&>(msg));
		}

		template<bool isRequest, class Body, class Fields = http::fields>
		inline auto _data_persistence(const http::message<isRequest, Body, Fields>& msg)
		{
			return copyable_wrapper(std::move(msg));
		}

		template<bool isRequest, class Body, class Fields = http::fields>
		inline auto _data_persistence(http::message<isRequest, Body, Fields>&& msg)
		{
			return copyable_wrapper(std::move(msg));
		}

		template<class Data, class Callback>
		inline bool _do_send(Data& data, Callback&& callback)
		{
			if (this->is_websocket())
				return this->derived()._ws_send(data, std::forward<Callback>(callback));

			return this->derived()._http_send(data, std::forward<Callback>(callback));
		}

	protected:
		template<typename MatchCondition>
		inline void _post_recv(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			this->derived()._http_post_recv(std::move(this_ptr), condition);
		}

		template<typename MatchCondition>
		inline void _handle_recv(const error_code& ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			this->derived()._http_handle_recv(ec, bytes_recvd, std::move(this_ptr), condition);
		}

		template<typename MatchCondition>
		inline void _handle_upgrade(const error_code & ec, std::shared_ptr<derived_t> self_ptr,
			condition_wrap<MatchCondition> condition)
		{
			this->derived().sessions().post(
				[this, ec, this_ptr = std::move(self_ptr), condition]() mutable
			{
				try
				{
					set_last_error(ec);

					this->derived()._fire_upgrade(this_ptr, ec);

					asio::detail::throw_error(ec);

					this->derived().post([this, this_ptr = std::move(this_ptr), condition]() mutable
					{
						this->derived()._post_recv(std::move(this_ptr), condition);
					});
				}
				catch (system_error & e)
				{
					set_last_error(e);

					this->derived()._do_disconnect(e.code());
				}
			});
		}

		template<typename MatchCondition>
		inline bool _check_upgrade(std::shared_ptr<derived_t>& this_ptr,
			condition_wrap<MatchCondition>& condition)
		{
			if (this->support_websocket_ && this->derived().is_http() && this->req_.is_upgrade())
			{
				this->websocket_router_ = this->router_.template _find<false>(this->req_, this->rep_);

				if (this->websocket_router_)
				{
					this->req_.ws_frame_type_ = websocket::frame::open;
					this->req_.ws_frame_data_ = {};

					this->derived().silence_timeout_ = std::chrono::milliseconds(tcp_silence_timeout);
					this->derived()._ws_init(condition, this->derived().stream());
					this->derived()._ws_start(this_ptr, condition, this->derived().stream());

					if (this->router_._route(this_ptr, this->req_, this->rep_))
					{
						this->derived()._post_control_callback(this_ptr, condition);
						this->derived()._post_upgrade(this_ptr, condition, this->req_.base());
						return true;
					}
					else
					{
						ASIO2_ASSERT(false);
					}
				}
			}
			return false;
		}

		template<typename MatchCondition>
		inline void _handle_control_ping(beast::string_view payload,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			detail::ignore::unused(payload, this_ptr, condition);

			this->req_.ws_frame_type_ = websocket::frame::ping;
			this->req_.ws_frame_data_ = payload;

			this->router_._route(this_ptr, this->req_, this->rep_);
		}

		template<typename MatchCondition>
		inline void _handle_control_pong(beast::string_view payload,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			detail::ignore::unused(payload, this_ptr, condition);

			this->req_.ws_frame_type_ = websocket::frame::pong;
			this->req_.ws_frame_data_ = payload;

			this->router_._route(this_ptr, this->req_, this->rep_);
		}

		template<typename MatchCondition>
		inline void _handle_control_close(beast::string_view payload,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			detail::ignore::unused(payload, this_ptr, condition);

			this->req_.ws_frame_type_ = websocket::frame::close;
			this->req_.ws_frame_data_ = payload;

			this->router_._route(this_ptr, this->req_, this->rep_);

			this->derived()._do_disconnect(websocket::error::closed);
		}

		inline void _fire_upgrade(std::shared_ptr<derived_t>& this_ptr, error_code ec)
		{
			this->listener_.notify(event::upgrade, this_ptr, ec);
		}

		template<typename MatchCondition>
		inline void _fire_recv(std::shared_ptr<derived_t>& this_ptr,
			condition_wrap<MatchCondition>& condition)
		{
			if (this->is_arg0_session_)
				this->listener_.notify(event::recv, this_ptr, this->req_, this->rep_);
			else
				this->listener_.notify(event::recv, this->req_, this->rep_);

			this->router_._route(this_ptr, this->req_, this->rep_);

			if (this->rep_.defer_guard_)
				this->rep_.defer_guard_.reset();
			else
			{
				if (this->is_http())
					this->derived()._send_response(this_ptr, condition);
			}
		}

	protected:
		http::request             req_;

		http::response            rep_;

		http_router_t<derived_t>& router_;

		std::filesystem::path   & root_directory_;

		bool                      is_arg0_session_    = false;

		bool                      support_websocket_  = false;

		std::shared_ptr<typename http_router_t<derived_t>::optype>   websocket_router_;
	};
}

namespace asio2
{
	class http_session : public detail::http_session_impl_t<http_session, asio::ip::tcp::socket,
		websocket::stream<asio::ip::tcp::socket&>, http::string_body, beast::flat_buffer>
	{
	public:
		using http_session_impl_t<http_session, asio::ip::tcp::socket,
			websocket::stream<asio::ip::tcp::socket&>, http::string_body,
			beast::flat_buffer>::http_session_impl_t;
	};
}

#endif // !__ASIO2_HTTP_SESSION_HPP__
