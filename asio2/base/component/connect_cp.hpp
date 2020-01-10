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

namespace asio2::detail
{
	template<class derived_t, class socket_t>
	class connect_cp
	{
	public:
		using self = connect_cp<derived_t, socket_t>;
		using resolver_type = typename asio::ip::basic_resolver<typename socket_t::protocol_type>;
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

				this->fire_connect_flag_.clear();

				auto & socket = derive.socket().lowest_layer();

				socket.close(ec_ignore);

				socket.open(derive.local_endpoint().protocol());

				// Connect succeeded. set the keeplive values
				socket.set_option(typename socket_t::reuse_address(true)); // set port reuse

				if constexpr (std::is_same_v<typename socket_t::protocol_type, asio::ip::tcp>)
					derive.keep_alive_options();
				else
					std::ignore = true;

				derive._fire_init();

				socket.bind(derive.local_endpoint());

				// start the timeout timer
				std::future<error_code> future = derive._post_timeout_timer(derive.connect_timeout(),
					this_ptr, [this, this_ptr, condition](error_code ec) mutable
				{
					if (!ec)
					{
						ec = asio::error::timed_out;
						derive.template _handle_connect<isAsync>(ec, std::move(this_ptr), std::move(condition));
					}
				});

				std::unique_ptr<resolver_type> resolver_ptr = std::make_unique<resolver_type>(derive.io().context());

				// Before async_resolve execution is complete, we must hold the resolver object.
				// so we captured the resolver_ptr into the lambda callback function.
				resolver_type * resolver_pointer = resolver_ptr.get();
				resolver_pointer->async_resolve(this->host_, this->port_, asio::bind_executor(derive.io().strand(),
					[this, this_ptr, condition, resolver_ptr = std::move(resolver_ptr)]
				(const error_code& ec, const endpoints_type& endpoints)
				{
					set_last_error(ec);

					this->endpoints_ = endpoints;

					if (ec)
						derive.template _handle_connect<isAsync>(ec, std::move(this_ptr), std::move(condition));
					else
						derive.template _post_connect<isAsync>(ec, this->endpoints_.begin(), std::move(this_ptr), std::move(condition));
				}));

				if constexpr (isAsync)
					return true;
				else
				{
					derive._wait_timeout_timer(future);
					return derive.is_started();
				}
			}
			catch (system_error & e)
			{
				set_last_error(e);
				derive.template _handle_connect<isAsync>(e.code(), std::move(this_ptr), std::move(condition));
			}
			return false;
		}

		template<bool isAsync, typename MatchCondition>
		inline void _post_connect(error_code ec, endpoints_iterator iter,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			try
			{
				if (iter == this->endpoints_.end())
				{
					// There are no more endpoints to try. Shut down the client.
					derive.template _handle_connect<isAsync>(ec ? ec : asio::error::host_unreachable,
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
						derive.template _post_connect<isAsync>(ec, ++iter, std::move(this_ptr), std::move(condition));
					else
						derive.template _handle_connect<isAsync>(ec, std::move(this_ptr), std::move(condition));
				})));
			}
			catch (system_error & e)
			{
				set_last_error(e);

				derive.template _handle_connect<isAsync>(e.code(), std::move(this_ptr), std::move(condition));
			}
		}

		template<bool isAsync, typename MatchCondition>
		inline void _handle_connect(const error_code & ec, std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			set_last_error(ec);

			derive._done_connect(ec, std::move(this_ptr), std::move(condition));
		}

		template<typename MatchCondition>
		inline void _done_connect(error_code ec, std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			try
			{
				// Whatever of connection success or failure or timeout, cancel the timeout timer.
				derive._stop_timeout_timer();

				// Whether the connection succeeds or fails, always call fire_connect notification

				// Set the state to started before fire_connect because the user may send data in
				// fire_connect and fail if the state is not set to started.
				if (!ec)
				{
					state_t expected = state_t::starting;
					if (!derive.state().compare_exchange_strong(expected, state_t::started))
						ec = asio::error::operation_aborted;
				}

				set_last_error(ec);

				if (!this->fire_connect_flag_.test_and_set())
				{
					derive._fire_connect(this_ptr, ec);

					if (ec)
					{
						derive._wake_reconnect_timer();
					}
				}

				if (!ec)
				{
					state_t expected = state_t::started;
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

		inline void _do_disconnect(const error_code& ec)
		{
			state_t expected = state_t::started;
			if (derive.state().compare_exchange_strong(expected, state_t::stopping))
				return derive._post_disconnect(ec, std::shared_ptr<derived_t>{}, expected);

			expected = state_t::starting;
			if (derive.state().compare_exchange_strong(expected, state_t::stopping))
				return derive._post_disconnect(ec, std::shared_ptr<derived_t>{}, expected);
		}

		inline void _post_disconnect(const error_code& ec, std::shared_ptr<derived_t> self_ptr, state_t old_state)
		{
			auto task = [this, ec, this_ptr = std::move(self_ptr), old_state]()
			{
				set_last_error(ec);

				state_t expected = state_t::stopping;
				if (derive.state().compare_exchange_strong(expected, state_t::stopped))
				{
					if (old_state == state_t::started)
					{
						derive._fire_disconnect(this_ptr, ec);

						if (ec)
						{
							derive._wake_reconnect_timer();
						}
					}

					derive._handle_disconnect(ec, std::move(this_ptr));
				}
				else
				{
					ASIO2_ASSERT(false);
				}
			};
			// All pending sending events will be cancelled after enter the send strand below.
#if defined(ASIO2_SEND_CORE_ASYNC)
			derive.push_event([this, t = std::move(task)]() mutable
			{
				auto task = [this, t = std::move(t)]() mutable
				{
					t();
					derive.next_event();
				};
				asio::post(derive.io().strand(), std::move(task));
				return true;
			});
#else
			asio::post(derive.io().strand(), std::move(task));
#endif
		}

		inline void _handle_disconnect(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			detail::ignore::unused(ec, this_ptr);
		}

	protected:
		derived_t                     & derive;

		/// Save the host and port of the server
		std::string                     host_, port_;

		/// the endpoints which parsed from host and port
		endpoints_type                  endpoints_;

		/// Used to avoid multiple calls to fire_connect due to connection timeout
		std::atomic_flag                fire_connect_flag_;
	};
}

#endif // !__ASIO2_CONNECT_COMPONENT_HPP__
