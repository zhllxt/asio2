/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
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
		template<bool isAsync, typename MatchCondition>
		inline bool _start_connect(std::string_view host, std::string_view port,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			try
			{
				this->host_ = host;
				this->port_ = port;

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
						derive._done_connect(ec, std::move(this_ptr), std::move(condition));
					}
				});

				std::shared_ptr<resolver_type> resolver_ptr = std::make_shared<resolver_type>(socket.get_executor().context());
				//typename resolver_type::query query(std::string{ host }, std::string{ port });

				// Before async_resolve execution is complete, we must hold the resolver object.
				// so we captured the resolver_ptr into the lambda callback function.
				resolver_ptr->async_resolve(host, port, asio::bind_executor(derive.io().strand(),
					[this, this_ptr, condition, resolver_ptr](const error_code& ec, const endpoints_type& endpoints)
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
				state_t expected = state_t::starting;
				if (!ec)
					if (!derive.state().compare_exchange_strong(expected, state_t::started))
						ec = asio::error::operation_aborted;

				set_last_error(ec);

				std::call_once(*this->once_flag_, [this, this_ptr, ec]()
				{
					derive._fire_connect(this_ptr, ec);
				});

				expected = state_t::started;
				if (!ec)
					if (!derive.state().compare_exchange_strong(expected, state_t::started))
						asio::detail::throw_error(asio::error::operation_aborted);

				asio::detail::throw_error(ec);

				derive._do_start(std::move(this_ptr), std::move(condition));
			}
			catch (system_error & e)
			{
				set_last_error(e);
				derive._do_stop(e.code());
			}
		}

	protected:
		derived_t                     & derive;

		/// Save the host and port of the server
		std::string                     host_, port_;

		/// the endpoints which parsed from host and port
		endpoints_type                  endpoints_;

		/// Used to avoid multiple calls to fire_connect due to connection timeout
		std::unique_ptr<std::once_flag> once_flag_;
	};
}

#endif // !__ASIO2_CONNECT_COMPONENT_HPP__
