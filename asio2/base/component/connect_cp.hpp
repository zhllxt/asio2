/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_CONNECT_COMPONENT_HPP__
#define __ASIO2_CONNECT_COMPONENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <future>
#include <utility>
#include <string_view>

#include <asio2/base/selector.hpp>
#include <asio2/base/iopool.hpp>
#include <asio2/base/error.hpp>
#include <asio2/base/listener.hpp>
#include <asio2/base/detail/condition_wrap.hpp>

#include <asio2/base/component/event_queue_cp.hpp>

namespace asio2::detail
{
	template<class derived_t, class socket_t, bool isSession>
	class connect_cp;

	template<class derived_t, class socket_t>
	class connect_cp<derived_t, socket_t, false>
	{
	public:
		using self = connect_cp<derived_t, socket_t, false>;
		using raw_socket_t = typename std::remove_reference_t<socket_t>;
		using resolver_type = typename asio::ip::basic_resolver<typename raw_socket_t::protocol_type>;
		using endpoints_type = typename resolver_type::results_type;
		using endpoints_iterator = typename endpoints_type::iterator;

	public:
		/**
		 * @constructor
		 */
		connect_cp() : derive(static_cast<derived_t&>(*this)) {}

		/**
		 * @destructor
		 */
		~connect_cp() = default;

	protected:
		template<bool isAsync, typename String, typename StrOrInt, typename MatchCondition>
		inline bool _start_connect(String&& host, StrOrInt&& port,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			try
			{
				if (derive.iopool_.is_stopped())
				{
					set_last_error(asio::error::shut_down);
					return false;
				}

				this->host_ = to_string(std::forward<String>(host));
				this->port_ = to_string(std::forward<StrOrInt>(port));

				auto & socket = derive.socket().lowest_layer();

				socket.close(ec_ignore);

				socket.open(derive.local_endpoint().protocol());

				// Connect succeeded. set the keeplive values
				socket.set_option(typename raw_socket_t::reuse_address(true)); // set port reuse

				if constexpr (std::is_same_v<typename raw_socket_t::protocol_type, asio::ip::tcp>)
				{
					derive.keep_alive_options();
				}
				else
				{
					std::ignore = true;
				}

				derive._fire_init();

				socket.bind(derive.local_endpoint());

				if constexpr (isAsync)
				{
					// start the timeout timer
					derive.post([this, this_ptr, condition]() mutable
					{
						derive._post_connect_timeout_timer(derive.connect_timeout(), this_ptr,
							[this, this_ptr, condition]
						(const error_code& ec) mutable
						{
							detail::ignore_unused(this_ptr, condition);

							// no errors indicating that the connection timed out
							if (!ec)
							{
								// we close the socket, so the async_connect will returned 
								// with operation_aborted.
								derive.socket().lowest_layer().close(ec_ignore);
							}
						});
					});

					derive._post_resolve(std::move(this_ptr), std::move(condition));

					return true;
				}
				else
				{
					std::promise<error_code> promise;
					std::future<error_code> future = promise.get_future();

					// start the timeout timer
					derive.post([this, this_ptr, condition, promise = std::move(promise)]() mutable
					{
						derive._post_connect_timeout_timer(derive.connect_timeout(), this_ptr,
							[this, this_ptr, condition, promise = std::move(promise)]
						(const error_code& ec) mutable
						{
							detail::ignore_unused(this_ptr, condition);

							// no errors indicating that the connection timed out
							if (!ec)
							{
								// we close the socket, so the async_connect will returned 
								// with operation_aborted.
								derive.socket().lowest_layer().close(ec_ignore);
							}

							promise.set_value(derive._connect_error_code() ?
								derive._connect_error_code() : (ec ? ec : asio::error::timed_out));
						});
					});

					derive._post_resolve(std::move(this_ptr), std::move(condition));

					if (!derive.io().strand().running_in_this_thread())
					{
						set_last_error(future.get());
					}
					else
					{
						ASIO2_ASSERT(false);
					}

					return derive.is_started();
				}
			}
			catch (system_error & e)
			{
				set_last_error(e);

				derive._handle_connect(e.code(), std::move(this_ptr), std::move(condition));
			}

			return false;
		}

		template<typename MatchCondition>
		inline void _post_resolve(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			derive.post([this, this_ptr = std::move(this_ptr), condition = std::move(condition)]() mutable
			{
				// resolve the server address.
				std::unique_ptr<resolver_type> resolver_ptr = std::make_unique<resolver_type>(
					derive.io().context());

				resolver_type* resolver_rptr = resolver_ptr.get();

				// Before async_resolve execution is complete, we must hold the resolver object.
				// so we captured the resolver_ptr into the lambda callback function.
				resolver_rptr->async_resolve(this->host_, this->port_,
					asio::bind_executor(derive.io().strand(),
						[this, this_ptr = std::move(this_ptr), condition = std::move(condition),
						resolver_ptr = std::move(resolver_ptr)]
				(const error_code& ec, const endpoints_type& endpoints) mutable
				{
					set_last_error(ec);

					this->endpoints_ = endpoints;

					if (ec)
						derive._handle_connect(ec, std::move(this_ptr), std::move(condition));
					else
						derive._post_connect(ec, this->endpoints_.begin(),
							std::move(this_ptr), std::move(condition));
				}));
			});
		}

		template<typename MatchCondition>
		inline void _post_connect(error_code ec, endpoints_iterator iter,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			try
			{
				if (iter == this->endpoints_.end())
				{
					// There are no more endpoints to try. Shut down the client.
					derive._handle_connect(ec ? ec : asio::error::host_unreachable,
						std::move(this_ptr), std::move(condition));

					return;
				}

				// Start the asynchronous connect operation.
				derive.socket().lowest_layer().async_connect(iter->endpoint(),
					asio::bind_executor(derive.io().strand(), make_allocator(derive.rallocator(),
						[this, iter, this_ptr, condition](const error_code & ec) mutable
				{
					set_last_error(ec);

					if (ec && ec != asio::error::operation_aborted)
						derive._post_connect(ec, ++iter, std::move(this_ptr), std::move(condition));
					else
						derive._handle_connect(ec, std::move(this_ptr), std::move(condition));
				})));
			}
			catch (system_error & e)
			{
				set_last_error(e);

				derive._handle_connect(e.code(), std::move(this_ptr), std::move(condition));
			}
		}

		template<typename MatchCondition>
		inline void _handle_connect(const error_code& ec, std::shared_ptr<derived_t> this_ptr,
			condition_wrap<MatchCondition> condition)
		{
			set_last_error(ec);

			derive._done_connect(ec, std::move(this_ptr), std::move(condition));
		}

		template<typename MatchCondition>
		inline void _done_connect(error_code ec, std::shared_ptr<derived_t> this_ptr,
			condition_wrap<MatchCondition> condition)
		{
			try
			{
				// if connect timeout is true, reset the error to timed_out.
				if (derive._is_connect_timeout())
				{
					ec = asio::error::timed_out;
				}

				// Whatever of connection success or failure or timeout, cancel the timeout timer.
				derive._stop_connect_timeout_timer(ec);

				state_t expected;

				// Set the state to started before fire_connect because the user may send data in
				// fire_connect and fail if the state is not set to started.
				if (!ec)
				{
					expected = state_t::starting;
					if (!derive.state().compare_exchange_strong(expected, state_t::started))
						ec = asio::error::operation_aborted;
				}

				set_last_error(ec);

				// Whether the connection succeeds or fails, always call fire_connect notification
				{
					expected = state_t::starting;
					if (derive.state().compare_exchange_strong(expected, state_t::starting))
					{
						ASIO2_ASSERT(ec);
						derive._fire_connect(this_ptr, ec);
					}
					else
					{
						expected = state_t::started;
						if (derive.state().compare_exchange_strong(expected, state_t::started))
						{
							ASIO2_ASSERT(!ec);
							derive._fire_connect(this_ptr, ec);
						}
					}
				}

				if (!ec)
				{
					expected = state_t::started;
					if (!derive.state().compare_exchange_strong(expected, state_t::started))
						asio::detail::throw_error(asio::error::operation_aborted);
				}

				asio::detail::throw_error(ec);

				derive._do_start(std::move(this_ptr), std::move(condition));
			}
			catch (system_error & e)
			{
				set_last_error(e);

				derive._do_disconnect(e.code());
			}
		}

	protected:
		derived_t                     & derive;

		/// Save the host and port of the server
		std::string                     host_, port_;

		/// the endpoints which parsed from host and port
		endpoints_type                  endpoints_;
	};

	template<class derived_t, class socket_t>
	class connect_cp<derived_t, socket_t, true>
	{
	public:
		using self = connect_cp<derived_t, socket_t, true>;
		using raw_socket_t = typename std::remove_reference_t<socket_t>;
		using resolver_type = typename asio::ip::basic_resolver<typename raw_socket_t::protocol_type>;
		using endpoints_type = typename resolver_type::results_type;
		using endpoints_iterator = typename endpoints_type::iterator;

	public:
		/**
		 * @constructor
		 */
		connect_cp() : derive(static_cast<derived_t&>(*this)) {}

		/**
		 * @destructor
		 */
		~connect_cp() = default;

	protected:
		template<typename MatchCondition>
		inline void _handle_connect(const error_code& ec, std::shared_ptr<derived_t> this_ptr,
			condition_wrap<MatchCondition> condition)
		{
			set_last_error(ec);

			derive._done_connect(ec, std::move(this_ptr), std::move(condition));
		}

		template<typename MatchCondition>
		inline void _done_connect(error_code ec, std::shared_ptr<derived_t> this_ptr,
			condition_wrap<MatchCondition> condition)
		{
			try
			{
				// if connect timeout is true, reset the error to timed_out.
				if (derive._is_connect_timeout())
				{
					ec = asio::error::timed_out;
				}

				// Whatever of connection success or failure or timeout, cancel the timeout timer.
				derive._stop_connect_timeout_timer(ec);

				state_t expected;

				// Set the state to started before fire_connect because the user may send data in
				// fire_connect and fail if the state is not set to started.
				if (!ec)
				{
					expected = state_t::starting;
					if (!derive.state().compare_exchange_strong(expected, state_t::started))
						ec = asio::error::operation_aborted;
				}

				set_last_error(ec);

				// Whether the connection succeeds or fails, always call fire_connect notification
				{
					expected = state_t::starting;
					if (derive.state().compare_exchange_strong(expected, state_t::starting))
					{
						ASIO2_ASSERT(ec);
						derive._fire_connect(this_ptr);
					}
					else
					{
						expected = state_t::started;
						if (derive.state().compare_exchange_strong(expected, state_t::started))
						{
							ASIO2_ASSERT(!ec);
							derive._fire_connect(this_ptr);
						}
					}
				}

				if (!ec)
				{
					expected = state_t::started;
					if (!derive.state().compare_exchange_strong(expected, state_t::started))
						asio::detail::throw_error(asio::error::operation_aborted);
				}

				asio::detail::throw_error(ec);

				derive._do_start(std::move(this_ptr), std::move(condition));
			}
			catch (system_error & e)
			{
				set_last_error(e);

				derive._do_disconnect(e.code());
			}
		}

	protected:
		derived_t                     & derive;
	};
}

#endif // !__ASIO2_CONNECT_COMPONENT_HPP__
