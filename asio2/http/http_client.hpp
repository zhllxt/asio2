/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef ASIO_STANDALONE

#ifndef __ASIO2_HTTP_CLIENT_HPP__
#define __ASIO2_HTTP_CLIENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/tcp/tcp_client.hpp>
#include <asio2/http/detail/http_util.hpp>
#include <asio2/http/component/http_send_cp.hpp>
#include <asio2/http/impl/http_send_op.hpp>
#include <asio2/http/impl/http_recv_op.hpp>

namespace asio2::detail
{
	template<class derived_t, class socket_t, class body_t, class buffer_t>
	class http_client_impl_t
		: public tcp_client_impl_t<derived_t, socket_t, buffer_t>
		, public http_send_cp<derived_t, body_t, buffer_t, false>
		, public http_send_op<derived_t, body_t, buffer_t, false>
		, public http_recv_op<derived_t, body_t, buffer_t, false>
	{
		template <class, bool>                friend class user_timer_cp;
		template <class, bool>                friend class reconnect_timer_cp;
		template <class, bool>                friend class connect_timeout_cp;
		template <class, class>               friend class connect_cp;
		template <class>                      friend class data_persistence_cp;
		template <class>                      friend class event_queue_cp;
		template <class, bool>                friend class send_cp;
		template <class, bool>                friend class tcp_send_op;
		template <class, bool>                friend class tcp_recv_op;
		template <class, class, class, bool>  friend class http_send_cp;
		template <class, class, class, bool>  friend class http_send_op;
		template <class, class, class, bool>  friend class http_recv_op;
		template <class, class, class>        friend class client_impl_t;
		template <class, class, class>        friend class tcp_client_impl_t;

	public:
		using self = http_client_impl_t<derived_t, socket_t, body_t, buffer_t>;
		using super = tcp_client_impl_t<derived_t, socket_t, buffer_t>;
		using body_type = body_t;
		using buffer_type = buffer_t;
		using super::send;
		using http_send_cp<derived_t, body_t, buffer_t, false>::send;
		using data_persistence_cp<derived_t>::_data_persistence;

	public:
		/**
		 * @constructor
		 */
		explicit http_client_impl_t(
			std::size_t init_buffer_size = tcp_frame_size,
			std::size_t max_buffer_size = (std::numeric_limits<std::size_t>::max)()
		)
			: super(init_buffer_size, max_buffer_size)
			, http_send_cp<derived_t, body_t, buffer_t, false>(this->io_)
			, http_send_op<derived_t, body_t, buffer_t, false>()
		{
		}

		/**
		 * @destructor
		 */
		~http_client_impl_t()
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
		bool start(String&& host, StrOrInt&& port, std::string_view target = {},
			http::verb method = http::verb::get, unsigned version = 11)
		{
			try
			{
				state_t expected = state_t::stopped;
				if (!this->state_.compare_exchange_strong(expected, state_t::stopped))
					asio::detail::throw_error(asio::error::already_started);

				std::string h = to_string(std::forward<String>(host));
				std::string p = to_string(std::forward<StrOrInt>(port));

				this->first_req_ = !target.empty();
				if (this->first_req_)
					this->req_ = http::make_request(h, p, target, method, version);

				return this->derived().template _do_connect<false>(std::move(h), std::move(p), condition_wrap<void>{});
			}
			catch (system_error & e)
			{
				set_last_error(e);
			}
			return false;
		}

		/**
		 * @function : start the client, blocking connect to server
		 */
		bool start(std::string_view url, http::verb method = http::verb::get, unsigned version = 11)
		{
			try
			{
				state_t expected = state_t::stopped;
				if (!this->state_.compare_exchange_strong(expected, state_t::stopped))
					asio::detail::throw_error(asio::error::already_started);

				std::string_view host = http::url_to_host(url);
				std::string_view port = http::url_to_port(url);

				if (host.empty())
					asio::detail::throw_error(asio::error::invalid_argument);

				this->first_req_ = true;
				this->req_ = http::make_request<body_t, buffer_t>(url);
				this->req_.method(method);
				this->req_.version(version);

				if (!port.empty() && port != "443" && port != "80")
					this->req_.set(http::field::host, std::string(host) + ":" + std::string(port));
				else
					this->req_.set(http::field::host, host);

				return this->derived().template _do_connect<false>(std::move(host), std::move(port),
					condition_wrap<void>{});
			}
			catch (system_error & e)
			{
				set_last_error(e);
			}
			return false;
		}

		/**
		 * @function : start the client, asynchronous connect to server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param port A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt>
		bool async_start(String&& host, StrOrInt&& port, String&& target = String{},
			http::verb method = http::verb::get, unsigned version = 11)
		{
			try
			{
				state_t expected = state_t::stopped;
				if (!this->state_.compare_exchange_strong(expected, state_t::stopped))
					asio::detail::throw_error(asio::error::already_started);

				std::string h = to_string(std::forward<String>(host));
				std::string p = to_string(std::forward<StrOrInt>(port));

				this->first_req_ = !target.empty();
				if (this->first_req_)
					this->req_ = http::make_request(h, p, target, method, version);

				return this->derived().template _do_connect<true>(std::move(h), std::move(p), condition_wrap<void>{});
			}
			catch (system_error & e)
			{
				set_last_error(e);
			}
			return false;
		}

		/**
		 * @function : start the client, asynchronous connect to server
		 */
		template<typename String>
		bool async_start(String&& url, http::verb method = http::verb::get, unsigned version = 11)
		{
			try
			{
				state_t expected = state_t::stopped;
				if (!this->state_.compare_exchange_strong(expected, state_t::stopped))
					asio::detail::throw_error(asio::error::already_started);

				std::string_view host = http::url_to_host(url);
				std::string_view port = http::url_to_port(url);

				if (host.empty())
					asio::detail::throw_error(asio::error::invalid_argument);

				this->first_req_ = true;
				this->req_ = http::make_request<body_t, buffer_t>(url);
				this->req_.method(method);
				this->req_.version(version);

				if (!port.empty() && port != "443" && port != "80")
					this->req_.set(http::field::host, std::string(host) + ":" + std::string(port));
				else
					this->req_.set(http::field::host, host);

				return this->derived().template _do_connect<true>(std::move(host), std::move(port),
					condition_wrap<void>{});
			}
			catch (system_error & e)
			{
				set_last_error(e);
			}
			return false;
		}

	public:
		template<class Body = http::string_body, class Fields = http::fields, class Buffer = beast::flat_buffer>
		static inline http::response<Body, Fields> execute(std::string_view host, std::string_view port,
			http::request<Body, Fields>& req, error_code& ec)
		{
			http::response<Body, Fields> rep;
			try
			{
				// set default result to unknown
				rep.result(http::status::unknown);

				// First assign default value timed_out to ec
				ec = asio::error::timed_out;

				// Get and Set the read timeout.
				std::chrono::steady_clock::duration timeout{ std::chrono::milliseconds(http_execute_timeout) };
				auto iter = req.find(http::field::timeout);
				if (iter != req.end())
				{
					auto sv = iter->value();
					std::string s(sv.data(), sv.size());
					timeout = std::chrono::milliseconds(std::stol(s));
				}

				// The io_context is required for all I/O
				asio::io_context ioc;

				// These objects perform our I/O
				asio::ip::tcp::resolver resolver{ ioc };
				asio::ip::tcp::socket socket{ ioc };

				// This buffer is used for reading and must be persisted
				Buffer buffer;

				// Look up the domain name
				resolver.async_resolve(host, port, [&](const error_code& ec1, const asio::ip::tcp::resolver::results_type& endpoints)
				{
					if (ec1) { ec = ec1; return; }

					// Make the connection on the IP address we get from a lookup
					asio::async_connect(socket, endpoints, [&](const error_code & ec2, const asio::ip::tcp::endpoint&)
					{
						if (ec2) { ec = ec2; return; }

						http::async_write(socket, req, [&](const error_code & ec3, std::size_t)
						{
							if (ec3) { ec = ec3; return; }

							// Then start asynchronous reading
							http::async_read(socket, buffer, rep, [&](const error_code & ec4, std::size_t)
							{
								// Reading completed, assign the read the result to ec
								// If the code does not execute into here, the ec value is the default value timed_out.
								ec = ec4;
							});
						});
					});
				});

				// timedout run
				ioc.run_for(timeout);

				// Gracefully close the socket
				socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec_ignore);
				socket.close(ec_ignore);
			}
			catch (system_error & e)
			{
				ec = e.code();
			}
			return rep;
		}

		template<class Body = http::string_body, class Fields = http::fields, class Buffer = beast::flat_buffer>
		static inline http::response<Body, Fields> execute(std::string_view host, std::string_view port, http::request<Body, Fields>& req)
		{
			error_code ec;
			http::response<Body, Fields> rep = execute<Body, Fields, Buffer>(host, port, req, ec);
			asio::detail::throw_error(ec);
			return rep;
		}

		static inline http::response<body_t> execute(std::string_view url, error_code& ec)
		{
			ec.clear();
			http::request<body_t> req = http::make_request<body_t>(url, ec);
			if (ec) return http::response<body_t>{};
			std::string_view host = http::url_to_host(url);
			std::string_view port = http::url_to_port(url);
			return execute<body_t>(host, port, req, ec);
		}

		static inline http::response<body_t> execute(std::string_view url)
		{
			error_code ec;
			http::response<body_t> rep = execute(url, ec);
			asio::detail::throw_error(ec);
			return rep;
		}

		static inline http::response<body_t> execute(std::string_view host, std::string_view port,
			std::string_view target, error_code& ec)
		{
			ec.clear();
			http::request<body_t> req = http::make_request<body_t>(host, port, target);
			return execute<body_t>(host, port, req, ec);
		}

		static inline http::response<body_t> execute(std::string_view host, std::string_view port,
			std::string_view target)
		{
			error_code ec;
			http::response<body_t> rep = execute(host, port, target, ec);
			asio::detail::throw_error(ec);
			return rep;
		}

	public:
		/**
		 * @function : bind recv listener
		 * @param    : fun - a user defined callback function
		 * Function signature : void(http::response<http::string_body>& rep)
		 */
		template<class F, class ...C>
		inline derived_t & bind_recv(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::recv,
				observer_t<http::response<body_t>&>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	protected:
		template<class T>
		inline auto _data_persistence(T&& data)
		{
			return this->derived()._data_persistence(asio::buffer(data));
		}

		template<class CharT, class SizeT>
		inline auto _data_persistence(CharT * s, SizeT count)
		{
			return this->derived()._data_persistence(asio::buffer((const void*)s, count * sizeof(CharT)));
		}

		template<typename = void>
		inline auto _data_persistence(asio::const_buffer&& data)
		{
			return copyable_wrapper(http::make_request<body_type>(std::string_view(
				reinterpret_cast<std::string_view::const_pointer>(data.data()), data.size())));
		}

		template<bool isRequest, class Body, class Fields = http::fields>
		inline auto _data_persistence(http::message<isRequest, Body, Fields>& msg)
		{
			return this->derived()._data_persistence(const_cast<const http::message<isRequest, Body, Fields>&>(msg));
		}

		template<bool isRequest, class Body, class Fields = http::fields>
		inline auto _data_persistence(const http::message<isRequest, Body, Fields>& msg)
		{
			return copyable_wrapper(std::move(msg));
		}

		template<bool isRequest, class Body, class Fields = http::fields>
		inline auto _data_persistence(http::message<isRequest, Body, Fields>&& msg)
		{
			// why use copyable_wrapper? beacuse http::message<isRequest, http::file_body> is moveable-only, but
			// std::function is copyable-only
			return copyable_wrapper(std::move(msg));
		}

		template<class Data, class Callback>
		inline bool _do_send(Data& data, Callback&& callback)
		{
			return this->derived().template _http_send<true>(this->derived().stream(),
				data, std::forward<Callback>(callback));
		}

	protected:
		template<typename MatchCondition>
		inline void _do_start(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			// Connect succeeded. post recv request.
			asio::post(this->io_.strand(), [this, this_ptr, condition]()
			{
				if (this->first_req_)
				{
					this->first_req_ = false;
					this->send(std::move(this->req_));
				}
			});
			super::_do_start(this_ptr, condition);
		}

		template<typename MatchCondition>
		inline void _post_recv(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			this->derived()._http_post_recv(std::move(this_ptr), condition);
		}

		template<typename MatchCondition>
		inline void _handle_recv(const error_code & ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			this->derived()._http_handle_recv(ec, bytes_recvd, std::move(this_ptr), condition);
		}

		inline void _fire_recv(detail::ignore, http::response<body_t>& rep)
		{
			this->listener_.notify(event::recv, rep);
		}

	protected:
		bool                   first_req_ = false;

		http::request<body_t>  req_;

		http::response<body_t> rep_;
	};
}

namespace asio2
{
	class http_client : public detail::http_client_impl_t<http_client, asio::ip::tcp::socket, http::string_body, beast::flat_buffer>
	{
	public:
		using http_client_impl_t<http_client, asio::ip::tcp::socket, http::string_body, beast::flat_buffer>::http_client_impl_t;
	};
}

#endif // !__ASIO2_HTTP_CLIENT_HPP__

#endif
