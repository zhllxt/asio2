/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_ICMP_SENDER_IMPL_HPP__
#define __ASIO2_ICMP_SENDER_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <thread>
#include <atomic>

#include <boost/asio.hpp>

#include <asio2/util/helper.hpp>

#include <asio2/base/sender_impl.hpp>
#include <asio2/base/io_service_pool.hpp>
#include <asio2/base/listener_mgr.hpp>

#include <asio2/icmp/icmp_header.hpp>
#include <asio2/icmp/ipv4_header.hpp>

namespace asio2
{

	//class icmp_sender_impl : public sender_impl
	//{
	//public:

	//	/**
	//	 * @construct
	//	 */
	//	explicit icmp_sender_impl(
	//		std::shared_ptr<listener_mgr> listener_mgr_ptr,
	//		std::shared_ptr<url_parser> url_parser_ptr
	//	)
	//		: sender_impl(listener_mgr_ptr, url_parser_ptr)
	//	{
	//	}

	//	/**
	//	 * @destruct
	//	 */
	//	virtual ~icmp_sender_impl()
	//	{
	//	}

	//	/**
	//	 * @function : start the sender.you must call the stop function before application exit,otherwise will cause crash.
	//	 * @return   : true  - start successed 
	//	 *             false - start failed
	//	 */
	//	virtual bool start() override
	//	{
	//		try
	//		{
	//			// check if started and not stoped
	//			if (is_start())
	//			{
	//				assert(false);
	//				return false;
	//			}

	//			// first call base class start function
	//			if (!sender_impl::start())
	//				return false;

	//			// init recv buf pool 
	//			m_recv_buf_pool_ptr = std::make_shared<pool<uint8_t>>(_get_pool_buffer_size());

	//			// init service pool size and startup the io service thread 
	//			m_ioservice_pool_ptr = std::make_shared<io_service_pool>(_get_io_service_pool_size());
	//			m_ioservice_thread_ptr = std::make_shared<std::thread>(std::bind(&io_service_pool::run, m_ioservice_pool_ptr));

	//			// start send
	//			return _start_send();
	//		}
	//		catch (boost::system::system_error & e)
	//		{
	//			set_last_error(e.code().value());
	//		}
	//		catch (std::exception &)
	//		{
	//		}

	//		return false;
	//	}

	//	/**
	//	 * @function : stop the client
	//	 */
	//	virtual void stop() override
	//	{
	//		bool is_start = this->is_start();

	//		m_stop_is_called = true;

	//		// call listen socket's close function to notify the _handle_accept function response with error > 0 ,then the listen socket 
	//		// can get notify to exit
	//		if (m_socket_ptr && m_socket_ptr->is_open())
	//		{
	//			// close the socket by post a event
	//			if (is_start)
	//			{
	//				// use promise and future to wait for the async post finished.
	//				std::promise<void> promise_send;

	//				// first wait for all send pending complete
	//				m_send_strand_ptr->post([this, &promise_send]()
	//				{
	//					// do nothing ,just make sure the send pending is last executed
	//					auto_promise<void> ap(promise_send);
	//				});

	//				promise_send.get_future().wait();

	//				// use promise and future to wait for the async post finished.
	//				std::promise<void> promise_recv;

	//				// asio don't allow operate the same socket in multi thread,if you close socket in one thread and another thread is 
	//				// calling socket's async_read... function,it will crash.so we must care for operate the socket.when need close the
	//				// socket ,we use the strand to post a event,make sure the socket's close operation is in the same thread.
	//				m_recv_strand_ptr->post([this, &promise_recv]()
	//				{
	//					boost::system::error_code ec;

	//					m_socket_ptr->shutdown(boost::asio::socket_base::shutdown_both, ec);
	//					if (ec)
	//						set_last_error(ec.value());

	//					m_socket_ptr->close(ec);
	//					if (ec)
	//						set_last_error(ec.value());

	//					auto_promise<void> ap(promise_recv);
	//				});

	//				// wait util the socket is closed completed
	//				// must wait for the socket closed completed,otherwise when use m_evt_strand_ptr post a event to close the socket,
	//				// before socket closed ,the event thread join is returned already,and it will cause memory leaks

	//				// wait for the async task finish,when finished,the socket must be closed already.
	//				promise_recv.get_future().wait();
	//			}
	//			// close the socket directly
	//			else
	//			{
	//				boost::system::error_code ec;

	//				m_socket_ptr->shutdown(boost::asio::socket_base::shutdown_both, ec);
	//				if (ec)
	//					set_last_error(ec.value());

	//				m_socket_ptr->close(ec);
	//				if (ec)
	//					set_last_error(ec.value());
	//			}
	//		}

	//		if (m_ioservice_pool_ptr)
	//			m_ioservice_pool_ptr->stop();

	//		if (m_ioservice_thread_ptr && m_ioservice_thread_ptr->joinable())
	//			m_ioservice_thread_ptr->join();

	//		// release the buffer pool malloced memory 
	//		if (m_recv_buf_pool_ptr)
	//			m_recv_buf_pool_ptr->destroy();
	//		//if (m_send_buf_pool_ptr)
	//		//	m_send_buf_pool_ptr->destroy();

	//		// reset the resource to default status
	//		reset();
	//	}

	//	/**
	//	 * @function : reset the resource to default status
	//	 */
	//	virtual void reset() override
	//	{
	//		m_stop_is_called = false;

	//		m_fire_close_is_called.clear(std::memory_order_release);
	//	}

	//	/**
	//	 * @function : whether the client is started
	//	 */
	//	virtual bool is_start()
	//	{
	//		return (
	//			!m_stop_is_called &&
	//			(m_socket_ptr && m_socket_ptr->is_open()) &&
	//			(m_ioservice_thread_ptr && m_ioservice_thread_ptr->joinable())
	//			);
	//	}

	//	/**
	//	 * @function : no use here 
	//	 */
	//	virtual void send(std::string ip, unsigned short port, std::shared_ptr<uint8_t> send_buf_ptr, std::size_t len)
	//	{
	//		assert(false);
	//	}

	//public:

	//	/**
	//	 * @function : get the socket shared_ptr
	//	 */
	//	inline std::shared_ptr<boost::asio::ip::icmp::socket> get_socket_ptr()
	//	{
	//		return m_socket_ptr;
	//	}

	//	/**
	//	 * @function : get the local address
	//	 */
	//	virtual std::string get_local_address() override
	//	{
	//		try
	//		{
	//			if (m_socket_ptr && m_socket_ptr->is_open())
	//			{
	//				auto endpoint = m_socket_ptr->local_endpoint();
	//				return endpoint.address().to_string();
	//			}
	//		}
	//		catch (boost::system::system_error & e)
	//		{
	//			set_last_error(e.code().value());
	//		}
	//		return "";
	//	}

	//	/**
	//	 * @function : get the local port
	//	 */
	//	virtual unsigned short get_local_port() override
	//	{
	//		try
	//		{
	//			if (m_socket_ptr && m_socket_ptr->is_open())
	//			{
	//				auto endpoint = m_socket_ptr->local_endpoint();
	//				return endpoint.port();
	//			}
	//		}
	//		catch (boost::system::system_error & e)
	//		{
	//			set_last_error(e.code().value());
	//		}
	//		return 0;
	//	}

	//	/**
	//	 * @function : get the remote address
	//	 */
	//	virtual std::string get_remote_address() override
	//	{
	//		try
	//		{
	//			if (m_socket_ptr && m_socket_ptr->is_open())
	//			{
	//				auto endpoint = m_socket_ptr->remote_endpoint();
	//				return endpoint.address().to_string();
	//			}
	//		}
	//		catch (boost::system::system_error & e)
	//		{
	//			set_last_error(e.code().value());
	//		}
	//		return "";
	//	}

	//	/**
	//	 * @function : get the remote port
	//	 */
	//	virtual unsigned short get_remote_port() override
	//	{
	//		try
	//		{
	//			if (m_socket_ptr && m_socket_ptr->is_open())
	//			{
	//				auto endpoint = m_socket_ptr->remote_endpoint();
	//				return endpoint.port();
	//			}
	//		}
	//		catch (boost::system::system_error & e)
	//		{
	//			set_last_error(e.code().value());
	//		}
	//		return 0;
	//	}

	//	/**
	//	 * @function : get socket's recv buffer size
	//	 */
	//	virtual int get_recv_buffer_size() override
	//	{
	//		try
	//		{
	//			if (m_socket_ptr && m_socket_ptr->is_open())
	//			{
	//				boost::asio::socket_base::receive_buffer_size option;
	//				m_socket_ptr->get_option(option);
	//				return option.value();
	//			}
	//		}
	//		catch (boost::system::system_error & e)
	//		{
	//			set_last_error(e.code().value());
	//		}
	//		return 0;
	//	}

	//	/**
	//	 * @function : set socket's recv buffer size.
	//	 *             when packet lost rate is high,you can set the recv buffer size to a big value to avoid it.
	//	 */
	//	virtual bool set_recv_buffer_size(int size) override
	//	{
	//		try
	//		{
	//			if (m_socket_ptr && m_socket_ptr->is_open())
	//			{
	//				boost::asio::socket_base::receive_buffer_size option(size);
	//				m_socket_ptr->set_option(option);
	//				return true;
	//			}
	//		}
	//		catch (boost::system::system_error & e)
	//		{
	//			set_last_error(e.code().value());
	//		}
	//		return false;
	//	}

	//	/**
	//	 * @function : get socket's send buffer size
	//	 */
	//	virtual int get_send_buffer_size() override
	//	{
	//		try
	//		{
	//			if (m_socket_ptr && m_socket_ptr->is_open())
	//			{
	//				boost::asio::socket_base::send_buffer_size option;
	//				m_socket_ptr->get_option(option);
	//				return option.value();
	//			}
	//		}
	//		catch (boost::system::system_error & e)
	//		{
	//			set_last_error(e.code().value());
	//		}
	//		return 0;
	//	}

	//	/**
	//	 * @function : set socket's send buffer size
	//	 */
	//	virtual bool set_send_buffer_size(int size) override
	//	{
	//		try
	//		{
	//			if (m_socket_ptr && m_socket_ptr->is_open())
	//			{
	//				boost::asio::socket_base::send_buffer_size option(size);
	//				m_socket_ptr->set_option(option);
	//				return true;
	//			}
	//		}
	//		catch (boost::system::system_error & e)
	//		{
	//			set_last_error(e.code().value());
	//		}
	//		return false;
	//	}

	//protected:

	//	virtual void _set_reply_timeout_from_url()
	//	{
	//		// set reply_timeout from the url
	//		std::string str_reply_timeout = m_url_parser_ptr->get_param_value("reply_timeout");
	//		if (!str_reply_timeout.empty())
	//		{
	//			long reply_timeout = static_cast<long>(std::atoi(str_reply_timeout.c_str()));
	//			if (str_reply_timeout.find_last_of("ms") != std::string::npos)
	//				reply_timeout *= 1;
	//			else if (str_reply_timeout.find_last_of('s') != std::string::npos)
	//				reply_timeout *= 1000;
	//			if (reply_timeout < 1)
	//				reply_timeout = DEFAULT_REPLY_TIMEOUT;

	//			m_reply_timeout = reply_timeout;
	//		}
	//	}

	//	virtual std::size_t _get_io_service_pool_size() override
	//	{
	//		// get io_service_pool_size from the url
	//		std::size_t io_service_pool_size = 2;
	//		std::string str_io_service_pool_size = m_url_parser_ptr->get_param_value("io_service_pool_size");
	//		if (!str_io_service_pool_size.empty())
	//			io_service_pool_size = static_cast<std::size_t>(std::atoi(str_io_service_pool_size.c_str()));
	//		if (io_service_pool_size == 0)
	//			io_service_pool_size = 2;
	//		return io_service_pool_size;
	//	}

	//	virtual std::size_t _get_pool_buffer_size() override
	//	{
	//		// get pool_buffer_size from the url
	//		std::size_t pool_buffer_size = 576; // udp packet size best not more than 576,that is, MTU size
	//		std::string str_pool_buffer_size = m_url_parser_ptr->get_param_value("pool_buffer_size");
	//		if (!str_pool_buffer_size.empty())
	//		{
	//			pool_buffer_size = static_cast<std::size_t>(std::atoi(str_pool_buffer_size.c_str()));
	//			if (str_pool_buffer_size.find_last_of('k') != std::string::npos)
	//				pool_buffer_size *= 1024;
	//			else if (str_pool_buffer_size.find_last_of('m') != std::string::npos)
	//				pool_buffer_size *= 1024 * 1024;
	//		}
	//		if (pool_buffer_size < 16)
	//			pool_buffer_size = 576;
	//		return pool_buffer_size;
	//	}

	//	unsigned short _get_identifier()
	//	{
	//		return static_cast<unsigned short>(std::hash<std::thread::id>()(std::this_thread::get_id()));
	//	}

	//	virtual bool _start_send()
	//	{
	//		try
	//		{
	//			_set_reply_timeout_from_url();

	//			m_send_ioservice_ptr = m_ioservice_pool_ptr->get_io_service_ptr();
	//			m_recv_ioservice_ptr = m_ioservice_pool_ptr->get_io_service_ptr();

	//			m_send_strand_ptr = std::make_shared <boost::asio::io_service::strand>(*m_send_ioservice_ptr);
	//			m_recv_strand_ptr = std::make_shared <boost::asio::io_service::strand>(*m_recv_ioservice_ptr);

	//			// parse address and port
	//			boost::asio::ip::icmp::resolver resolver(*m_recv_ioservice_ptr);
	//			boost::asio::ip::icmp::resolver::query query(boost::asio::ip::icmp::v4(), m_url_parser_ptr->get_ip(), "");
	//			m_dest_endpoint = *resolver.resolve(query);

	//			m_socket_ptr = std::make_shared<boost::asio::ip::icmp::socket>(*m_recv_ioservice_ptr, boost::asio::ip::icmp::v4());

	//			_post_send();

	//			return (m_socket_ptr && m_socket_ptr->is_open());
	//		}
	//		catch (boost::system::system_error & e)
	//		{
	//			set_last_error(e.code().value());
	//		}

	//		return false;
	//	}

	//	virtual void _post_recv()
	//	{
	//		if (is_start())
	//		{
	//			// every times post recv event,we get the recv buffer from the buffer pool
	//			std::shared_ptr<buffer<uint8_t>> recv_buf_ptr = m_recv_buf_pool_ptr->get(0);

	//			try
	//			{
	//				// Wait for a reply. We prepare the buffer to receive up to 64KB.
	//				m_socket_ptr->async_receive(
	//					boost::asio::buffer(recv_buf_ptr->data(), recv_buf_ptr->capacity()),
	//					m_recv_strand_ptr->wrap(std::bind(&icmp_sender_impl::_handle_recv, 
	//						std::dynamic_pointer_cast<icmp_sender_impl>(shared_from_this()),
	//						std::placeholders::_1,
	//						std::placeholders::_2,
	//						recv_buf_ptr
	//					)));
	//			}
	//			catch (std::exception &)
	//			{
	//			}
	//		}
	//	}

	//	virtual void _handle_recv(const boost::system::error_code& ec, std::size_t bytes_recvd, std::shared_ptr<uint8_t> recv_buf_ptr)
	//	{
	//		set_last_error(ec.value());

	//		if (!ec)
	//		{
	//			// The actual number of bytes received is committed to the buffer so that we
	//			// can extract it using a std::istream object.
	//			reply_buffer_.commit(bytes_recvd);

	//			// Decode the reply packet.
	//			std::istream is(&reply_buffer_);
	//			ipv4_header ipv4_hdr;
	//			icmp_header icmp_hdr;
	//			is >> ipv4_hdr >> icmp_hdr;

	//			// We can receive all ICMP packets received by the host, so we need to
	//			// filter out only the echo replies that match the our identifier and
	//			// expected sequence number.
	//			if (is 
	//				&& icmp_hdr.type() == icmp_header::echo_reply
	//				&& icmp_hdr.identifier() == _get_identifier()
	//				&& icmp_hdr.sequence_number() == m_sequence_number
	//				)
	//			{
	//				// If this is the first reply, interrupt the five second timeout.
	//				if (m_num_replies++ == 0)
	//					m_timer_ptr->cancel();

	//				// Print out some information about the reply packet.
	//				posix_time::ptime now = posix_time::microsec_clock::universal_time();
	//				std::cout << length - ipv4_hdr.header_length()
	//					<< " bytes from " << ipv4_hdr.source_address()
	//					<< ": icmp_seq=" << icmp_hdr.sequence_number()
	//					<< ", ttl=" << ipv4_hdr.time_to_live()
	//					<< ", time=" << (now - time_sent_).total_milliseconds() << " ms"
	//					<< std::endl;
	//			}

	//			_fire_recv(m_sender_endpoint, recv_buf_ptr, bytes_recvd);

	//			_post_recv();
	//		}
	//		else
	//		{
	//			// close this client
	//			_fire_close(ec.value());

	//			//// may be user pressed the CTRL + C to exit application.
	//			//if (ec == boost::asio::error::operation_aborted)
	//			//	return;

	//			//// if user call stop to stop client,the socket is closed,then _handle_recv will be called,and with error,so when appear error,we check 
	//			//// the socket status,if closed,don't call _post_recv again.
	//			//if (!m_socket_ptr || !m_socket_ptr->is_open())
	//			//	return;
	//		}
	//	}

	//	virtual void _post_send()
	//	{
	//		if (is_start())
	//		{
	//			std::string body = m_url_parser_ptr->get_param_value("msg");

	//			// Create an ICMP header for an echo request.
	//			icmp_header echo_request;
	//			echo_request.type(icmp_header::echo_request);
	//			echo_request.code(0);
	//			echo_request.identifier(_get_identifier());
	//			echo_request.sequence_number(++m_sequence_number);

	//			// compute the checksum
	//			compute_checksum(echo_request, body.begin(), body.end());

	//			// Encode the request packet.
	//			boost::asio::streambuf request_buffer;
	//			std::ostream os(&request_buffer);
	//			os << echo_request << body;

	//			// Send the request.
	//			//time_sent_ = posix_time::microsec_clock::universal_time();
	//			m_socket_ptr->send_to(request_buffer.data(), m_dest_endpoint);

	//			// Wait up to five seconds for a reply.
	//			m_num_replies = 0;
	//			//timer_.expires_at(time_sent_ + posix_time::seconds(5));
	//			//timer_.async_wait(boost::bind(&pinger::handle_timeout, this));

	//			m_timer_ptr->expires_from_now(boost::posix_time::seconds(m_reply_timeout));
	//			m_timer_ptr->async_wait(
	//				m_timer_strand_ptr->wrap(std::bind(&icmp_sender_impl::_handle_timeout,
	//					std::dynamic_pointer_cast<icmp_sender_impl>(shared_from_this()),
	//					std::placeholders::_1 // error_code
	//				)));

	//			//boost::system::error_code ec;
	//			//m_socket_ptr->send_to(boost::asio::buffer(send_buf_ptr.get(), len), recver_endpoint, 0, ec);
	//			//set_last_error(ec.value());

	//			//_fire_send(recver_endpoint, send_buf_ptr, bytes_sent, ec.value());

	//			//if (ec)
	//			//{
	//			//	PRINT_EXCEPTION;

	//			//	_fire_close(ec.value());
	//			//}
	//		}
	//	}

	//	virtual void _handle_timeout()
	//	{
	//		if (m_num_replies == 0)
	//			std::cout << "request timed out" << std::endl;

	//		// Requests must be sent no less than one second apart.
	//		m_timer_ptr->expires_from_now(boost::posix_time::seconds(1));
	//		m_timer_ptr->async_wait(
	//			m_timer_strand_ptr->wrap(std::bind(&icmp_sender_impl::_post_send,
	//				std::dynamic_pointer_cast<icmp_sender_impl>(shared_from_this())
	//			)));
	//	}

	//	virtual void _fire_recv(boost::asio::ip::udp::endpoint sender_endpoint, std::shared_ptr<uint8_t> recv_buf_ptr, std::size_t bytes_recvd)
	//	{
	//	}

	//	virtual void _do_fire_recv(boost::asio::ip::udp::endpoint & sender_endpoint, std::shared_ptr<uint8_t> recv_buf_ptr, std::size_t bytes_recvd)
	//	{
	//		if (m_listener_mgr_ptr)
	//		{
	//			m_listener_mgr_ptr->notify_recv(sender_endpoint.address().to_string(), sender_endpoint.port(), recv_buf_ptr, bytes_recvd);
	//		}
	//	}

	//	virtual void _fire_send(boost::asio::ip::udp::endpoint recver_endpoint, std::shared_ptr<uint8_t> send_buf_ptr, std::size_t bytes_sent, int error)
	//	{
	//	}

	//	virtual void _do_fire_send(boost::asio::ip::udp::endpoint & recver_endpoint, std::shared_ptr<uint8_t> send_buf_ptr, std::size_t bytes_sent, int error)
	//	{
	//		if (m_listener_mgr_ptr)
	//		{
	//			m_listener_mgr_ptr->notify_send(recver_endpoint.address().to_string(), recver_endpoint.port(), send_buf_ptr, bytes_sent, error);
	//		}
	//	}

	//	virtual void _fire_close(int error)
	//	{
	//		if (!m_fire_close_is_called.test_and_set(std::memory_order_acquire))
	//			_do_fire_close(error);
	//	}

	//	virtual void _do_fire_close(int error)
	//	{
	//		if (m_listener_mgr_ptr)
	//		{
	//			m_listener_mgr_ptr->notify_close(error);
	//		}
	//	}

	//protected:

	//	std::shared_ptr<boost::asio::ip::icmp::socket> m_socket_ptr;

	//	/// the m_io_service_pool_evt thread for socket event
	//	std::shared_ptr<std::thread> m_ioservice_thread_ptr;

	//	/// the io_service_pool for socket event
	//	io_service_pool_ptr m_ioservice_pool_ptr;

	//	/// recv buffer pool for every session
	//	std::shared_ptr<pool<uint8_t>> m_recv_buf_pool_ptr;

	//	/// use to check whether the user call session stop function
	//	volatile bool m_stop_is_called = false;

	//	/// use to avoid call _fire_close twice
	//	std::atomic_flag m_fire_close_is_called = ATOMIC_FLAG_INIT;

	//	/// endpoint for remote destination 
	//	boost::asio::ip::icmp::endpoint m_dest_endpoint;

	//	/// timer for session time out
	//	std::shared_ptr<boost::asio::deadline_timer> m_timer_ptr;

	//	/// strand for timer to insure multi thread safe
	//	std::shared_ptr<boost::asio::io_service::strand> m_timer_strand_ptr;

	//	std::chrono::time_point<std::chrono::steady_clock> m_sent_time = std::chrono::steady_clock::now();

	//	/// reply timeout value,unit : milliseconds
	//	long m_reply_timeout = DEFAULT_REPLY_TIMEOUT;

	//	unsigned short m_sequence_number = 0;

	//	std::size_t m_num_replies = 0;

	//	boost::asio::streambuf reply_buffer_;
	//};
}

#endif // !__ASIO2_ICMP_SENDER_IMPL_HPP__
