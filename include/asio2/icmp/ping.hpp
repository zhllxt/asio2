/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_PING_HPP__
#define __ASIO2_PING_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <cstdint>
#include <memory>
#include <chrono>
#include <functional>
#include <atomic>
#include <string>
#include <string_view>
#include <queue>
#include <any>
#include <future>
#include <tuple>

#include <asio2/base/iopool.hpp>
#include <asio2/base/listener.hpp>
#include <asio2/base/define.hpp>

#include <asio2/base/detail/object.hpp>
#include <asio2/base/detail/allocator.hpp>
#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/buffer_wrap.hpp>
#include <asio2/base/detail/condition_wrap.hpp>

#include <asio2/base/component/thread_id_cp.hpp>
#include <asio2/base/component/user_data_cp.hpp>
#include <asio2/base/component/socket_cp.hpp>
#include <asio2/base/component/user_timer_cp.hpp>
#include <asio2/base/component/post_cp.hpp>
#include <asio2/base/component/event_queue_cp.hpp>
#include <asio2/base/component/condition_event_cp.hpp>

#include <asio2/icmp/detail/icmp_header.hpp>
#include <asio2/icmp/detail/ipv4_header.hpp>
#include <asio2/icmp/detail/ipv6_header.hpp>

namespace asio2
{
	class icmp_rep : public detail::ipv4_header, /*public detail::ipv6_header,*/ public detail::icmp_header
	{
	public:
		std::chrono::steady_clock::duration lag{ std::chrono::steady_clock::duration(-1) };

		inline bool is_timeout() noexcept { return (this->lag.count() == -1); }

		inline auto get_milliseconds() noexcept
		{
			return this->lag.count() == -1 ? -1 :
				std::chrono::duration_cast<std::chrono::milliseconds>(this->lag).count();
		}

		inline auto milliseconds() noexcept
		{
			return this->get_milliseconds();
		}

		detail::ipv4_header& base_ipv4() noexcept { return static_cast<detail::ipv4_header&>(*this); }
		//detail::ipv6_header& base_ipv6() noexcept { return static_cast<detail::ipv6_header&>(*this); }
		detail::icmp_header& base_icmp() noexcept { return static_cast<detail::icmp_header&>(*this); }

	protected:

	};
}

namespace asio2::detail
{
	struct template_args_icmp
	{
		using socket_t = asio::ip::icmp::socket;
		using buffer_t = asio::streambuf;
	};

	ASIO2_CLASS_FORWARD_DECLARE_BASE;

	template<class derived_t, class args_t = template_args_icmp>
	class ping_impl_t
		: public object_t          <derived_t        >
		, public iopool_cp         <derived_t, args_t>
		, public thread_id_cp      <derived_t, args_t>
		, public user_data_cp      <derived_t, args_t>
		, public user_timer_cp     <derived_t, args_t>
		, public post_cp           <derived_t, args_t>
		, public condition_event_cp<derived_t, args_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;

	public:
		using super = object_t   <derived_t        >;
		using self  = ping_impl_t<derived_t, args_t>;

		using iopoolcp = iopool_cp<derived_t, args_t>;

		using args_type   = args_t;
		using socket_type = typename args_t::socket_t;
		using buffer_type = typename args_t::buffer_t;

		/**
		 * @constructor
		 * @param send_count Total number of echo packets you want to send
		 * send_count equals -1 for infinite send
		 * Other parameters should use default values.
		 */
		explicit ping_impl_t(
			std::size_t send_count       = -1,
			std::size_t init_buf_size = 64 * 1024, // We prepare the buffer to receive up to 64KB.
			std::size_t max_buf_size  = max_buffer_size,
			std::size_t concurrency   = 1
		)
			: super()
			, iopool_cp          <derived_t, args_t>(concurrency)
			, user_data_cp       <derived_t, args_t>()
			, user_timer_cp      <derived_t, args_t>()
			, post_cp            <derived_t, args_t>()
			, condition_event_cp <derived_t, args_t>()
			, socket_    (iopoolcp::_get_io(0).context())
			, rallocator_()
			, wallocator_()
			, listener_  ()
			, io_        (iopoolcp::_get_io(0))
			, buffer_    (init_buf_size, max_buf_size)
			, timer_     (iopoolcp::_get_io(0).context())
			, ncount_    (send_count)
		{
		}

		template<class Scheduler, std::enable_if_t<!std::is_integral_v<detail::remove_cvref_t<Scheduler>>, int> = 0>
		explicit ping_impl_t(
			std::size_t send_count,
			std::size_t init_buf_size,
			std::size_t max_buf_size,
			Scheduler&& scheduler
		)
			: super()
			, iopool_cp          <derived_t, args_t>(std::forward<Scheduler>(scheduler))
			, user_data_cp       <derived_t, args_t>()
			, user_timer_cp      <derived_t, args_t>()
			, post_cp            <derived_t, args_t>()
			, condition_event_cp <derived_t, args_t>()
			, socket_    (iopoolcp::_get_io(0).context())
			, rallocator_()
			, wallocator_()
			, listener_  ()
			, io_        (iopoolcp::_get_io(0))
			, buffer_    (init_buf_size, max_buf_size)
			, timer_     (iopoolcp::_get_io(0).context())
			, ncount_    (send_count)
		{
		}

		template<class Scheduler, std::enable_if_t<!std::is_integral_v<detail::remove_cvref_t<Scheduler>>, int> = 0>
		explicit ping_impl_t(Scheduler&& scheduler)
			: ping_impl_t(std::size_t(-1), 64 * 1024, max_buffer_size, std::forward<Scheduler>(scheduler))
		{
		}

		/**
		 * @destructor
		 */
		~ping_impl_t()
		{
			this->stop();
		}

		/**
		 * @function : start
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.  example : "151.101.193.69" or "www.google.com"
		 */
		template<typename String>
		inline bool start(String&& host)
		{
			return this->derived()._do_start(std::forward<String>(host));
		}

		/**
		 * @function : stop
		 */
		inline void stop()
		{
			if (this->is_iopool_stopped())
				return;

			derived_t& derive = this->derived();

			derive.io().unregobj(&derive);

			derive.dispatch([&derive]() mutable
			{
				derive._do_stop(asio::error::operation_aborted, derive.selfptr());
			});

			this->stop_iopool();
		}

		/**
		 * @function : check whether the client is started
		 */
		inline bool is_started() const
		{
			return (this->state_ == state_t::started && this->socket_.lowest_layer().is_open());
		}

		/**
		 * @function : check whether the client is stopped
		 */
		inline bool is_stopped() const
		{
			return (this->state_ == state_t::stopped && !this->socket_.lowest_layer().is_open() && this->is_iopool_stopped());
		}

	public:
		/**
		 * @function : sync ping the host, and return the response directly.
		 * if some error occurs, call asio2::get_last_error(); to get the error info.
		 */
		template<class Rep, class Period>
		static inline icmp_rep execute(
			std::string_view host, std::chrono::duration<Rep, Period> timeout, std::string body)
		{
			icmp_rep rep;
			try
			{
				// First assign default value timed_out to last error
				set_last_error(asio::error::timed_out);

				// The io_context is required for all I/O
				asio::io_context ioc;

				// These objects perform our I/O
				asio::ip::icmp::resolver resolver{ ioc };
				asio::ip::icmp::socket socket{ ioc };

				asio::streambuf request_buffer;
				asio::streambuf reply_buffer;

				std::ostream os(std::addressof(request_buffer));
				std::istream is(std::addressof(reply_buffer));

				icmp_header echo_request;

				unsigned short id = static_cast<unsigned short>(0);
				unsigned short sequence_number = static_cast<unsigned short>(0);

				decltype(std::chrono::steady_clock::now()) time_sent;

				// Look up the domain name
				resolver.async_resolve(host, "",
				[&](const error_code& ec1, const asio::ip::icmp::resolver::results_type& endpoints) mutable
				{
					if (ec1) { set_last_error(ec1); return; }

					for (auto& dest : endpoints)
					{
						struct socket_guard
						{
							socket_guard(
								asio::ip::icmp::socket& s,
								const asio::ip::basic_resolver_entry<asio::ip::icmp>& d
							) : socket(s), dest(d)
							{
								error_code ec_ignore{};
								socket.close(ec_ignore);
								socket.open(dest.endpoint().protocol());
							}
							~socket_guard()
							{
								error_code ec_ignore{};
								// Gracefully close the socket
								socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec_ignore);
								socket.close(ec_ignore);
							}
							asio::ip::icmp::socket& socket;
							const asio::ip::basic_resolver_entry<asio::ip::icmp>& dest;
						};

						std::unique_ptr<socket_guard> guarder = std::make_unique<socket_guard>(socket, dest);

						// Create an ICMP header for an echo request.
						echo_request.type(icmp_header::echo_request);
						echo_request.code(0);
						id = (unsigned short)(std::size_t(guarder.get()));
						echo_request.identifier(id);
						auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
							std::chrono::steady_clock::now().time_since_epoch()).count();
						sequence_number = static_cast<unsigned short>(ms % (std::numeric_limits<unsigned short>::max)());
						echo_request.sequence_number(sequence_number);
						compute_checksum(echo_request, body.begin(), body.end());

						// Encode the request packet.
						os << echo_request << body;

						// Send the request.
						time_sent = std::chrono::steady_clock::now();

						socket.async_send_to(request_buffer.data(), dest, [&, guarder = std::move(guarder)]
						(const error_code& ec2, std::size_t) mutable
						{
							if (ec2) { set_last_error(ec2); return; }

							// Discard any data already in the buffer.
							reply_buffer.consume(reply_buffer.size());

							std::size_t length = sizeof(ipv4_header) + sizeof(icmp_header) + body.size();

							// Wait for a reply. We prepare the buffer to receive up to 64KB.
							socket.async_receive(reply_buffer.prepare(length), [&, guarder = std::move(guarder)]
							(const error_code& ec3, std::size_t bytes_recvd) mutable
							{
								set_last_error(ec3);

								// The actual number of bytes received is committed to the buffer so that we
								// can extract it using a std::istream object.
								reply_buffer.commit(bytes_recvd);

								// Decode the reply packet.
								ipv4_header& ipv4_hdr = rep.base_ipv4();
								icmp_header& icmp_hdr = rep.base_icmp();
								is >> ipv4_hdr >> icmp_hdr;

								ASIO2_ASSERT(ipv4_hdr.total_length() == bytes_recvd);

								// We can receive all ICMP packets received by the host, so we need to
								// filter out only the echo replies that match the our identifier and
								// expected sequence number.
								if (is && icmp_hdr.type() == icmp_header::echo_reply
									&& icmp_hdr.identifier() == id
									&& icmp_hdr.sequence_number() == sequence_number)
								{
									// Print out some information about the reply packet.
									rep.lag = std::chrono::steady_clock::now() - time_sent;
								}
							});
						});

						break;
					}
				});

				// timedout run
				ioc.run_for(timeout);
			}
			catch (system_error & e)
			{
				set_last_error(e);
			}

			return rep;
		}

		/**
		 * @function : sync ping the host, and return the response directly.
		 * if some error occurs, call asio2::get_last_error(); to get the error info.
		 */
		template<class Rep, class Period>
		static inline icmp_rep execute(std::string_view host, std::chrono::duration<Rep, Period> timeout)
		{
			return derived_t::execute(host, timeout, R"("Hello!" from Asio ping.)");
		}

		/**
		 * @function : sync ping the host, and return the response directly.
		 * if some error occurs, call asio2::get_last_error(); to get the error info.
		 */
		static inline icmp_rep execute(std::string_view host)
		{
			return derived_t::execute(host, std::chrono::milliseconds(icmp_execute_timeout), R"("Hello!" from Asio ping.)");
		}

	public:
		/**
		 * @function : bind recv listener
		 * @param    : fun - a user defined callback function
		 * void on_recv(asio2::icmp_rep& rep){...}
		 * or a lumbda function like this :
		 * [&](asio2::icmp_rep& rep){...}
		 */
		template<class F, class ...C>
		inline derived_t & bind_recv(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::recv,
				observer_t<icmp_rep&>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

		/**
		 * @function : bind init listener,we should set socket options at here
		 * @param    : fun - a user defined callback function
		 * This notification is called after the socket is opened.
		 * You can set the socket option in this notification.
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
		 * @function : bind start listener
		 * @param    : fun - a user defined callback function
		 * This notification is called after the server starts up, whether successful or unsuccessful
		 * Function signature : void()
		 */
		template<class F, class ...C>
		inline derived_t & bind_start(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::start,
				observer_t<>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

		/**
		 * @function : bind stop listener
		 * @param    : fun - a user defined callback function
		 * This notification is called before the server is ready to stop
		 * Function signature : void()
		 */
		template<class F, class ...C>
		inline derived_t & bind_stop(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::stop,
				observer_t<>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	public:
		/**
		 * @function : get the socket object refrence
		 */
		inline socket_type & socket() noexcept { return this->socket_; }

		/**
		 * @function : get the stream object refrence
		 */
		inline socket_type & stream() noexcept { return this->socket_; }

	public:
		/**
		 * @function : set icmp protocol identifier
		 */
		template<class Integer>
		inline derived_t & set_identifier(Integer id) noexcept
		{
			this->identifier_ = (unsigned short)(std::size_t(id));
			return (this->derived());
		}
		/**
		 * @function : get icmp protocol identifier
		 */
		inline unsigned short get_identifier() noexcept
		{
			return this->identifier_;
		}

		/**
		 * @function : set reply timeout duration value
		 */
		template<class Rep, class Period>
		inline derived_t & set_timeout(std::chrono::duration<Rep, Period> duration) noexcept
		{
			this->timeout_ = duration;
			return (this->derived());
		}
		/**
		 * @function : set reply timeout duration value, same as set_timeout
		 */
		template<class Rep, class Period>
		inline derived_t & timeout(std::chrono::duration<Rep, Period> duration) noexcept
		{
			return this->set_timeout(std::move(duration));
		}

		/**
		 * @function : get reply timeout duration value
		 */
		inline std::chrono::steady_clock::duration get_timeout() noexcept
		{
			return this->timeout_;
		}
		/**
		 * @function : get reply timeout duration value, same as get_timeout
		 */
		inline std::chrono::steady_clock::duration timeout() noexcept
		{
			return this->get_timeout();
		}

		/**
		 * @function : set send interval duration value
		 */
		template<class Rep, class Period>
		inline derived_t & set_interval(std::chrono::duration<Rep, Period> duration) noexcept
		{
			this->interval_ = duration;
			return (this->derived());
		}

		/**
		 * @function : set send interval duration value, same as set_interval
		 */
		template<class Rep, class Period>
		inline derived_t & interval(std::chrono::duration<Rep, Period> duration) noexcept
		{
			return this->set_interval(std::move(duration));
		}

		/**
		 * @function : get send interval duration value
		 */
		inline std::chrono::steady_clock::duration get_interval() noexcept
		{
			return this->interval_;
		}

		/**
		 * @function : get send interval duration value, same as get_interval
		 */
		inline std::chrono::steady_clock::duration interval() noexcept
		{
			return this->interval_;
		}

		/**
		 * @function : set icmp payload body
		 * This function is the same as the "payload()" function
		 */
		inline derived_t & set_body(std::string_view body)
		{
			this->body_ = body;
			if (this->body_.size() > 65500)
				this->body_.resize(65500);
			return (this->derived());
		}

		/**
		 * @function : set icmp payload body, same as set_body
		 * This function is the same as the "payload()" function
		 */
		inline derived_t & body(std::string_view body)
		{
			return this->set_body(std::move(body));
		}

		/**
		 * @function : set icmp payload body
		 * This function is the same as the "body()" function
		 */
		inline derived_t & set_payload(std::string_view body)
		{
			return this->derived().body(std::move(body));
		}

		/**
		 * @function : set icmp payload body, same as set_payload
		 * This function is the same as the "body()" function
		 */
		inline derived_t & payload(std::string_view body)
		{
			return this->set_payload(std::move(body));
		}

		/**
		 * @function : get the resolved host ip
		 */
		inline std::string get_host_ip() { return this->destination_.address().to_string(); }

		/**
		 * @function : get the resolved host ip, same as get_host_ip
		 */
		inline std::string host_ip() { return this->get_host_ip(); }

		/**
		 * @function : Set the total number of echo packets you want to send
		 */
		inline derived_t & set_ncount(std::size_t send_count) noexcept
		{
			this->ncount_ = send_count;
			return (this->derived());
		}

		/**
		 * @function : Set the total number of echo packets you want to send, same as set_ncount
		 */
		inline derived_t & ncount(std::size_t send_count) noexcept
		{
			return this->set_ncount(send_count);
		}

		/**
		 * @function : Get the total number of echo packets has sent, same as get_total_send
		 */
		inline std::size_t total_send() noexcept { return this->total_send_; }

		/**
		 * @function : Get the total number of reply packets has recved, same as get_total_recv
		 */
		inline std::size_t total_recv() noexcept { return this->total_recv_; }

		/**
		 * @function : Get the total number of echo packets has sent
		 */
		inline std::size_t get_total_send() noexcept { return this->total_send_; }

		/**
		 * @function : Get the total number of reply packets has recved
		 */
		inline std::size_t get_total_recv() noexcept { return this->total_recv_; }

		/**
		 * @function : Get the packet loss probability (loss rate)
		 */
		inline double get_plp() noexcept
		{
			if (this->total_send_ == static_cast<std::size_t>(0))
				return 0.0;
			return (((double)(total_send_ - total_recv_)) / (double)total_send_ * 100.0);
		}

		/**
		 * @function : Get the packet loss probability (loss rate), same as get_plp
		 */
		inline double plp() noexcept
		{
			return this->get_plp();
		}

		/**
		 * @function : Get the average duration of elapsed when recved reply packets
		 */
		inline std::chrono::steady_clock::duration get_avg_lag() noexcept
		{
			if (this->total_recv_ == static_cast<std::size_t>(0))
				return std::chrono::steady_clock::duration(0);
			return std::chrono::steady_clock::duration(
				long((double)this->total_time_.count() / (double)this->total_recv_));
		}

		/**
		 * @function : Get the average duration of elapsed when recved reply packets, same as get_avg_lag
		 */
		inline std::chrono::steady_clock::duration avg_lag() noexcept
		{
			return this->get_avg_lag();
		}

	protected:
		template<typename String>
		bool _do_start(String&& host)
		{
			derived_t& derive = this->derived();

			this->start_iopool();

			if (this->is_iopool_stopped())
			{
				set_last_error(asio::error::operation_aborted);
				return false;
			}

			asio::dispatch(derive.io().context(), [&derive, this_ptr = derive.selfptr()]() mutable
			{
				detail::ignore_unused(this_ptr);

				// init the running thread id 
				if (derive.io().get_thread_id() == std::thread::id{})
					derive.io().init_thread_id();
			});

			// use promise to get the result of async accept
			std::promise<error_code> promise;
			std::future<error_code> future = promise.get_future();

			// use derfer to ensure the promise's value must be seted.
			detail::defer_event pg
			{
				[promise = std::move(promise)]() mutable { promise.set_value(get_last_error()); }
			};

			derive.post(
			[this, this_ptr = derive.selfptr(), host = std::forward<String>(host), pg = std::move(pg)]
			() mutable
			{
				derived_t& derive = this->derived();

				state_t expected = state_t::stopped;
				if (!this->state_.compare_exchange_strong(expected, state_t::starting))
				{
					// if the state is not stopped, set the last error to already_started
					set_last_error(asio::error::already_started);

					return;
				}

				try
				{
					clear_last_error();

					derive.io().regobj(&derive);

				#if defined(_DEBUG) || defined(DEBUG)
					this->is_stop_called_ = false;
				#endif

					expected = state_t::starting;
					if (!this->state_.compare_exchange_strong(expected, state_t::starting))
					{
						ASIO2_ASSERT(false);
						asio::detail::throw_error(asio::error::operation_aborted);
					}

					this->seq_ = 0;
					this->total_send_ = 0;
					this->total_recv_ = 0;
					this->total_time_ = std::chrono::steady_clock::duration{ 0 };
					asio::ip::icmp::resolver resolver(this->io_.context());
					this->destination_ = *resolver.resolve(host, "").begin();
				
					error_code ec_ignore{};

					this->socket_.close(ec_ignore);
					this->socket_.open(this->destination_.protocol());

					derive._fire_init();

					derive._handle_start(error_code{}, std::move(this_ptr));

					return;
				}
				catch (system_error const& e)
				{
					set_last_error(e.code());
				}
				catch (std::exception const&)
				{
					set_last_error(asio::error::invalid_argument);
				}

				derive._handle_start(get_last_error(), std::move(this_ptr));
			});

			if (!derive.io().running_in_this_thread())
			{
				set_last_error(future.get());

				return static_cast<bool>(!get_last_error());
			}
			else
			{
				set_last_error(asio::error::in_progress);
			}

			// if the state is stopped , the return value is "is_started()".
			// if the state is stopping, the return value is false, the last error is already_started
			// if the state is starting, the return value is false, the last error is already_started
			// if the state is started , the return value is true , the last error is already_started
			return derive.is_started();
		}

		void _handle_start(error_code ec, std::shared_ptr<derived_t> this_ptr)
		{
			ASIO2_ASSERT(this->derived().io().running_in_this_thread());

			try
			{
				// Whether the startup succeeds or fails, always call fire_start notification
				state_t expected = state_t::starting;
				if (!ec)
					if (!this->state_.compare_exchange_strong(expected, state_t::started))
						ec = asio::error::operation_aborted;

				set_last_error(ec);

				this->derived()._fire_start();

				expected = state_t::started;
				if (!ec)
					if (!this->state_.compare_exchange_strong(expected, state_t::started))
						ec = asio::error::operation_aborted;

				asio::detail::throw_error(ec);

				this->buffer_.consume(this->buffer_.size());

				this->derived()._post_send(this_ptr);
				this->derived()._post_recv(std::move(this_ptr));
			}
			catch (system_error & e)
			{
				set_last_error(e);

				this->derived()._do_stop(e.code(), this->derived().selfptr());
			}
		}

		inline void _do_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			ASIO2_ASSERT(this->derived().io().running_in_this_thread());

			state_t expected = state_t::starting;
			if (this->state_.compare_exchange_strong(expected, state_t::stopping))
				return this->derived()._post_stop(ec, std::move(this_ptr), expected);

			expected = state_t::started;
			if (this->state_.compare_exchange_strong(expected, state_t::stopping))
				return this->derived()._post_stop(ec, std::move(this_ptr), expected);
		}

		inline void _post_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr, state_t old_state)
		{
			// asio don't allow operate the same socket in multi thread,
			// if you close socket in one thread and another thread is 
			// calling socket's async_... function,it will crash.so we
			// must care for operate the socket.when need close the 
			// socket ,we use the io_context to post a event,make sure the
			// socket's close operation is in the same thread.
			asio::dispatch(this->io().context(), make_allocator(this->derived().wallocator(),
			[this, ec, this_ptr = std::move(this_ptr), old_state]() mutable
			{
				detail::ignore_unused(old_state);

				set_last_error(ec);

				state_t expected = state_t::stopping;
				if (this->state_.compare_exchange_strong(expected, state_t::stopped))
				{
					this->derived()._fire_stop();

					// call CRTP polymorphic stop
					this->derived()._handle_stop(ec, std::move(this_ptr));
				}
				else
				{
					ASIO2_ASSERT(false);
				}
			}));
		}

		inline void _handle_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr)
		{
			ASIO2_ASSERT(this->derived().io().running_in_this_thread());

			detail::ignore_unused(ec, this_ptr);

			error_code ec_ignore{};

			// close user custom timers
			this->stop_all_timers();

			// close all posted timed tasks
			this->stop_all_timed_tasks();

			// close all async_events
			this->notify_all_condition_events();

			// destroy user data, maybe the user data is self shared_ptr,
			// if don't destroy it, will cause loop refrence.
			this->user_data_.reset();

			try
			{
				this->timer_.cancel();
			}
			catch (system_error const&)
			{
			}

			// Call close,otherwise the _handle_recv will never return
			this->socket_.close(ec_ignore);
		}

		void _post_send(std::shared_ptr<derived_t> this_ptr)
		{
			// if ncount_ is equal to max, infinite send
			if (this->ncount_ != std::size_t(-1))
			{
				if (this->total_send_ >= this->ncount_)
				{
					this->derived()._do_stop(asio::error::eof, std::move(this_ptr));
					return;
				}
			}

			// Create an ICMP header for an echo request.
			icmp_header req;
			req.type(icmp_header::echo_request);
			req.code(0);
			req.identifier(this->identifier_);
			req.sequence_number(++seq_);
			compute_checksum(req, this->body_.begin(), this->body_.end());

			// Encode the request packet.
			asio::streambuf buffer;
			std::ostream os(std::addressof(buffer));
			os << req << this->body_;

			// Send the request.
			error_code ec;
			this->time_sent_ = std::chrono::steady_clock::now();
			this->socket_.send_to(buffer.data(), this->destination_, 0, ec);
			set_last_error(ec);
			if (!ec)
				this->total_send_++;

			// Wait up to five seconds for a reply.
			this->replies_ = 0;
			if (this->is_started())
			{
				this->timer_.expires_after(this->timeout_);
				this->timer_.async_wait(
				[this, this_ptr = std::move(this_ptr)](const error_code & ec) mutable
				{
					this->derived()._handle_timer(ec, std::move(this_ptr));
				});
			}
		}

		void _handle_timer(const error_code & ec, std::shared_ptr<derived_t> this_ptr)
		{
			detail::ignore_unused(ec);

			if (this->replies_ == 0)
			{
				this->rep_.lag = std::chrono::steady_clock::duration(-1);

				if (!ec && this->is_started())
				{
					detail::condition_wrap<void> dummy{};
					this->derived()._fire_recv(this->rep_, dummy);
				}
			}

			// Requests must be sent no less than one second apart.
			if (this->is_started())
			{
				this->timer_.expires_after(this->interval_);
				this->timer_.async_wait(
				[this, this_ptr = std::move(this_ptr)](const error_code & ec) mutable
				{
					detail::ignore_unused(ec);

					this->derived()._post_send(std::move(this_ptr));
				});
			}
		}

		void _post_recv(std::shared_ptr<derived_t> this_ptr)
		{
			if (!this->is_started())
			{
				if (this->derived().state() == state_t::started)
				{
					this->derived()._do_stop(asio2::get_last_error(), std::move(this_ptr));
				}
				return;
			}

			try
			{
				// Wait for a reply. We prepare the buffer to receive up to 64KB.
				this->socket_.async_receive(this->buffer_.prepare(this->buffer_.pre_size()),
					make_allocator(this->rallocator_,
						[this, this_ptr = std::move(this_ptr)]
				(const error_code& ec, std::size_t bytes_recvd) mutable
				{
					this->derived()._handle_recv(ec, bytes_recvd, std::move(this_ptr));
				}));
			}
			catch (system_error & e)
			{
				set_last_error(e);

				this->derived()._do_stop(e.code(), this->derived().selfptr());
			}
		}

		void _handle_recv(const error_code& ec, std::size_t bytes_recvd, std::shared_ptr<derived_t> this_ptr)
		{
			set_last_error(ec);

			if (!this->is_started())
			{
				if (this->derived().state() == state_t::started)
				{
					this->derived()._do_stop(ec, std::move(this_ptr));
				}
				return;
			}

			if (ec == asio::error::operation_aborted)
			{
				this->derived()._do_stop(ec, std::move(this_ptr));
				return;
			}

			if (ec && bytes_recvd == 0)
			{
				this->derived()._do_stop(ec, std::move(this_ptr));
				return;
			}

			// The actual number of bytes received is committed to the buffer so that we
			// can extract it using a std::istream object.
			this->buffer_.commit(bytes_recvd);

			// Decode the reply packet.
			std::istream is(std::addressof(this->buffer_));
			ipv4_header& ipv4_hdr = this->rep_.base_ipv4();
			icmp_header& icmp_hdr = this->rep_.base_icmp();
			is >> ipv4_hdr >> icmp_hdr;

			ASIO2_ASSERT(ipv4_hdr.total_length() == bytes_recvd);

			// We can receive all ICMP packets received by the host, so we need to
			// filter out only the echo replies that match the our identifier and
			// expected sequence number.
			if (is
				&& icmp_hdr.type() == icmp_header::echo_reply
				&& icmp_hdr.identifier() == this->identifier_
				&& icmp_hdr.sequence_number() == this->seq_)
			{
				// If this is the first reply, interrupt the five second timeout.
				if (this->replies_++ == 0)
				{
					try
					{
						this->timer_.cancel();
					}
					catch (system_error const&)
					{
					}
				}

				this->total_recv_++;
				this->rep_.lag = std::chrono::steady_clock::now() - this->time_sent_;
				this->total_time_ += this->rep_.lag;

				detail::condition_wrap<void> dummy{};
				this->derived()._fire_recv(this->rep_, dummy);
			}

			// Discard any data already in the buffer.
			this->buffer_.consume(this->buffer_.size());

			this->derived()._post_recv(std::move(this_ptr));
		}

		inline void _fire_init()
		{
			// the _fire_init must be executed in the thread 0.
			ASIO2_ASSERT(this->derived().io().running_in_this_thread());
			ASIO2_ASSERT(!get_last_error());

			this->listener_.notify(event_type::init);
		}

		inline void _fire_start()
		{
			// the _fire_start must be executed in the thread 0.
			ASIO2_ASSERT(this->derived().io().running_in_this_thread());

		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(this->is_stop_called_ == false);
		#endif

			this->listener_.notify(event_type::start);
		}

		inline void _fire_stop()
		{
			// the _fire_stop must be executed in the thread 0.
			ASIO2_ASSERT(this->derived().io().running_in_this_thread());

		#if defined(_DEBUG) || defined(DEBUG)
			this->is_stop_called_ = true;
		#endif

			this->listener_.notify(event_type::stop);
		}

		template<typename MatchCondition>
		inline void _fire_recv(icmp_rep& rep, condition_wrap<MatchCondition>& condition)
		{
			detail::ignore_unused(condition);

			this->listener_.notify(event_type::recv, rep);
		}

	public:
		/**
		 * @function : get the buffer object refrence
		 */
		inline buffer_wrap<buffer_type> & buffer() noexcept { return this->buffer_; }
		/**
		 * @function : get the io object refrence
		 */
		inline io_t & io() noexcept { return this->io_; }

	protected:
		/**
		 * @function : get the recv/read allocator object refrence
		 */
		inline auto & rallocator() noexcept { return this->rallocator_; }
		/**
		 * @function : get the timer/post allocator object refrence
		 */
		inline auto & wallocator() noexcept { return this->wallocator_; }

		inline listener_t                 & listener() noexcept { return this->listener_; }
		inline std::atomic<state_t>       & state   () noexcept { return this->state_;    }

	protected:
		/// socket 
		socket_type                                 socket_;

		/// The memory to use for handler-based custom memory allocation. used fo recv/read.
		handler_memory<>                            rallocator_;

		/// The memory to use for handler-based custom memory allocation. used fo timer/post.
		handler_memory<size_op<>, std::true_type>   wallocator_;

		/// listener
		listener_t                                  listener_;

		/// The io_context wrapper used to handle the accept event.
		io_t                                      & io_;

		/// buffer
		buffer_wrap<buffer_type>                    buffer_;

		/// state
		std::atomic<state_t>                        state_ = state_t::stopped;

		asio::steady_timer                          timer_;
		std::string                                 body_{ R"("Hello!" from Asio ping.)" };
		unsigned short                              seq_ = 0;
		std::size_t                                 replies_ = 0;
		icmp_rep                                    rep_;
		asio::ip::icmp::endpoint                    destination_;
		unsigned short                              identifier_ = (unsigned short)(std::size_t(this));

		std::size_t                                 ncount_    { std::size_t(-1) };
		std::size_t                                 total_send_{ 0 };
		std::size_t                                 total_recv_{ 0 };
		std::chrono::steady_clock::duration         total_time_{ 0 };

		std::chrono::steady_clock::duration         timeout_  = std::chrono::milliseconds(icmp_execute_timeout);
		std::chrono::steady_clock::duration         interval_ = std::chrono::milliseconds(1000);
		std::chrono::steady_clock::time_point       time_sent_;

	#if defined(_DEBUG) || defined(DEBUG)
		bool                                        is_stop_called_  = false;
	#endif
	};
}

namespace asio2
{
	template<class derived_t>
	class ping_t : public detail::ping_impl_t<derived_t, detail::template_args_icmp>
	{
	public:
		using detail::ping_impl_t<derived_t, detail::template_args_icmp>::ping_impl_t;
	};

	/**
	 * @constructor Parameter description
	 * @param send_count Total number of echo packets you want to send
	 * send_count equals -1 for infinite send
	 * Other parameters should use default values.
	 */
	class ping : public ping_t<ping>
	{
	public:
		using ping_t<ping>::ping_t;
	};
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_PING_HPP__
