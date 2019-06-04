/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 */

#ifndef ASIO_STANDALONE

#ifndef __ASIO2_HTTPWS_SESSION_HPP__
#define __ASIO2_HTTPWS_SESSION_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/http/http_session.hpp>
#include <asio2/http/component/ws_stream_cp.hpp>
#include <asio2/http/impl/ws_send_op.hpp>

namespace asio2::detail
{
	template<class derived_t, class socket_t, class stream_t, class body_t, class buffer_t>
	class httpws_session_impl_t
		: public http_session_impl_t<derived_t, socket_t, body_t, buffer_t>
		, public ws_stream_cp<derived_t, stream_t, true>
		, public ws_send_op<derived_t, true>
	{
		template <class, bool>  friend class user_timer_cp;
		template <class, bool>  friend class send_cp;
		template <class, bool>  friend class silence_timer_cp;
		template <class, bool>  friend class connect_timeout_cp;
		template <class, bool>         friend class tcp_send_op;
		template <class, bool>         friend class tcp_recv_op;
		template <class, class, class, bool>  friend class http_send_cp;
		template <class, class, class, bool>  friend class http_send_op;
		template <class, class, class, bool>  friend class http_recv_op;
		template <class, class, bool>         friend class ws_stream_cp;
		template <class, bool>                friend class ws_send_op;
		template <class>                      friend class session_mgr_t;
		template <class, class, class>        friend class session_impl_t;
		template <class, class, class>        friend class tcp_session_impl_t;
		template <class, class, class, class> friend class http_session_impl_t;
		template <class, class>               friend class tcp_server_impl_t;
		template <class, class>               friend class httpws_server_impl_t;

	public:
		using self = httpws_session_impl_t<derived_t, socket_t, stream_t, body_t, buffer_t>;
		using super = http_session_impl_t<derived_t, socket_t, body_t, buffer_t>;
		using key_type = std::size_t;
		using body_type = body_t;
		using buffer_type = buffer_t;
		using ws_stream_comp = ws_stream_cp<derived_t, stream_t, true>;
		using super::send;

		/**
		 * @constructor
		 */
		explicit httpws_session_impl_t(
			session_mgr_t<derived_t> & sessions,
			listener_t & listener,
			io_t & rwio,
			std::size_t init_buffer_size,
			std::size_t max_buffer_size
		)
			: super(sessions, listener, rwio, init_buffer_size, max_buffer_size)
			, ws_stream_comp(this->socket_)
			, ws_send_op<derived_t, true>()
		{
		}

		/**
		 * @destructor
		 */
		~httpws_session_impl_t()
		{
		}

	public:
		/**
		 * @function : get this object hash key,used for session map
		 */
		inline const key_type hash_key() const
		{
			return reinterpret_cast<key_type>(this);
		}

		/**
		 * @function : get the stream object refrence
		 */
		inline typename ws_stream_comp::stream_type & stream()
		{
			return this->ws_stream_;
		}

		inline bool is_websocket() { return (this->is_ws_); }
		inline bool is_http() { return (!this->is_ws_); }

	protected:
		inline void _handle_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			if (this->is_http())
				return super::_handle_stop(ec, std::move(this_ptr));

			this->derived()._ws_stop(this_ptr, [this, ec, this_ptr]()
			{
				super::_handle_stop(ec, std::move(this_ptr));
			});
		}

		template<class ConstBufferSequence>
		inline bool _do_send(ConstBufferSequence buffer)
		{
			if (this->is_ws_)
				return this->derived()._ws_send(buffer);

			return this->derived().template _http_send<false>(this->socket_, buffer);
		}

		template<class ConstBufferSequence, class Callback>
		inline bool _do_send(ConstBufferSequence buffer, Callback& fn)
		{
			if (this->is_ws_)
				return this->derived()._ws_send(buffer, fn);

			return this->derived().template _http_send<false>(this->socket_, buffer, fn);
		}

		template<class ConstBufferSequence>
		inline bool _do_send(ConstBufferSequence buffer, std::promise<std::pair<error_code, std::size_t>>& promise)
		{
			if (this->is_ws_)
				return this->derived()._ws_send(buffer, promise);

			return this->derived().template _http_send<false>(this->socket_, buffer, promise);
		}

		template<bool isRequest, class Body, class Fields>
		inline bool _do_send(http::message<isRequest, Body, Fields>& msg)
		{
			if (this->is_ws_)
			{
				std::ostringstream oss;
				oss << msg;
				std::string str = oss.str();
				return this->derived()._ws_send(asio::buffer(str));
			}

			return this->derived()._http_send(this->socket_, msg);
		}

		template<bool isRequest, class Body, class Fields, class Callback>
		inline bool _do_send(http::message<isRequest, Body, Fields>& msg, Callback& fn)
		{
			if (this->is_ws_)
			{
				std::ostringstream oss;
				oss << msg;
				std::string str = oss.str();
				return this->derived()._ws_send(asio::buffer(str), fn);
			}

			return this->derived()._http_send(this->socket_, msg, fn);
		}

		template<bool isRequest, class Body, class Fields>
		inline bool _do_send(http::message<isRequest, Body, Fields>& msg, std::promise<std::pair<error_code, std::size_t>>& promise)
		{
			if (this->is_ws_)
			{
				std::ostringstream oss;
				oss << msg;
				std::string str = oss.str();
				return this->derived()._ws_send(asio::buffer(str), promise);
			}

			return this->derived()._http_send(this->socket_, msg, promise);
		}

	protected:
		template<typename MatchCondition>
		void _post_recv(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			if (!this->is_started())
				return;

			try
			{
				if (this->is_http())
				{
					// Make the request empty before reading,
					// otherwise the operation behavior is undefined.
					this->req_ = {};

					// Read a request
					http::async_read(this->socket_, this->buffer_.base(), this->req_,
						asio::bind_executor(this->io_.strand(), make_allocator(this->rallocator_,
							[this, self_ptr = std::move(this_ptr), condition](const error_code & ec, std::size_t bytes_recvd)
					{
						this->derived()._handle_recv(ec, bytes_recvd, std::move(self_ptr), condition);
					})));
				}
				else
				{
					// Read a message into our buffer
					this->ws_stream_.async_read(this->buffer_.base(),
						asio::bind_executor(this->io_.strand(), make_allocator(this->rallocator_,
							[this, self_ptr = std::move(this_ptr), condition](const error_code & ec, std::size_t bytes_recvd)
					{
						this->derived()._handle_recv(ec, bytes_recvd, std::move(self_ptr), condition);
					})));
				}
			}
			catch (system_error & e)
			{
				set_last_error(e);
				this->derived()._do_stop(e.code());
			}
		}

		template<typename MatchCondition>
		void _handle_recv(const error_code & ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			set_last_error(ec);

			if (!ec)
			{
				// every times recv data,we update the last active time.
				this->reset_active_time();

				if (this->is_http() && websocket::is_upgrade(this->req_))
				{
					this->is_ws_ = true;
					this->silence_timeout_ = std::chrono::milliseconds(tcp_silence_timeout);
					this->derived()._post_control_callback(this_ptr, condition);
					this->derived()._post_upgrade(std::move(this_ptr), condition, this->req_);
					return;
				}

				this->derived()._fire_recv(this_ptr, this->req_, std::string_view(reinterpret_cast<
					std::string_view::const_pointer>(this->buffer_.data().data()), bytes_recvd));

				if (this->is_ws_)
					this->buffer_.consume(this->buffer_.size());

				this->derived()._post_recv(std::move(this_ptr), condition);
			}
			else
			{
				// This means they closed the connection
				//if (ec == http::error::end_of_stream)
				this->derived()._do_stop(ec);
			}
		}

		template<typename MatchCondition>
		inline void _handle_upgrade(const error_code & ec, std::shared_ptr<derived_t> self_ptr, condition_wrap<MatchCondition> condition)
		{
			this->derived().sessions().post([this, ec, this_ptr = std::move(self_ptr), condition]() mutable
			{
				try
				{
					set_last_error(ec);

					this->derived()._fire_upgrade(this_ptr, ec);

					asio::detail::throw_error(ec);

					this->derived()._post_recv(std::move(this_ptr), condition);
				}
				catch (system_error & e)
				{
					set_last_error(e);
					this->derived()._do_stop(e.code());
				}
			});
		}

		inline void _fire_recv(std::shared_ptr<derived_t> & this_ptr, http::request<body_t> & req, std::string_view s)
		{
			this->listener_.notify(event::recv, this_ptr, req, s);
		}

		inline void _fire_upgrade(std::shared_ptr<derived_t> & this_ptr, error_code ec)
		{
			this->listener_.notify(event::upgrade, this_ptr, ec);
		}

	protected:
		http::response<body_t> rep_;

		bool                   is_ws_ = false;
	};
}

namespace asio2
{
	class httpws_session : public detail::httpws_session_impl_t<httpws_session,
		asio::ip::tcp::socket, websocket::stream<asio::ip::tcp::socket&>, http::string_body, beast::flat_buffer>
	{
	public:
		using httpws_session_impl_t<httpws_session, asio::ip::tcp::socket, websocket::stream<asio::ip::tcp::socket&>,
			http::string_body, beast::flat_buffer>::httpws_session_impl_t;
	};
}

#endif // !__ASIO2_HTTPWS_SESSION_HPP__

#endif
