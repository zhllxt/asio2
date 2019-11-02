#pragma once

#include <asio2/asio2.hpp>

// byte 1    head   : #
// byte 2    length : the body length
// byte 3... body   : the body content
class match_role
{
public:
	explicit match_role(char c) : c_(c) {}

	// The first member of the
	// return value is an iterator marking one-past-the-end of the bytes that have
	// been consumed by the match function.This iterator is used to calculate the
	// begin parameter for any subsequent invocation of the match condition.The
	// second member of the return value is true if a match has been found, false
	// otherwise.
	template <typename Iterator>
	std::pair<Iterator, bool> operator()(Iterator begin, Iterator end) const
	{
		Iterator i = begin;
		while (i != end)
		{
			// eg : How to close illegal clients
			// If the first byte is not # indicating that the client is illegal, we return
			// the matching success here and then determine the number of bytes received
			// in the on_recv callback function, if it is 0, we close the connection in on_recv.
			if (*i != c_)
				return std::pair(begin, true); // head character is not #, return and kill the client

			i++;
			if (i == end) break;

			int length = int(*i); // get content length

			i++;
			if (i == end) break;

			if (end - i >= length)
				return std::pair(i + length, true);

			break;
		}
		return std::pair(begin, false);
	}

private:
	char c_;
};

#ifdef ASIO_STANDALONE
namespace asio {
#else
namespace boost::asio {
#endif
	template <> struct is_match_condition<match_role> : public std::true_type {};
} // namespace asio

void run_tcp_server_match_role(std::string_view host, std::string_view port)
{
	asio2::tcp_server server;
	//while (1) // use infinite loop and sleep 2 seconds to test start and stop
	{
		printf("\n");
		server.start_timer(1, std::chrono::seconds(1), []() {});
		server.bind_recv([&server](auto & session_ptr, std::string_view s)
		{
			if (s.size() == 0)
			{
				printf("close illegal client : %s %u\n",
					session_ptr->remote_address().c_str(), session_ptr->remote_port());
				session_ptr->stop();
				return;
			}

			printf("recv : %u %.*s\n", (unsigned)s.size(), (int)s.size(), s.data());

			// force one packet data to be sent twice, and the client will recvd compeleted packet
			session_ptr->send(s.substr(0, s.size() / 2), []() {});
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			session_ptr->send(s.substr(s.size() / 2), [](std::size_t bytes_sent) {});

			//session_ptr->send(s, [](std::size_t bytes_sent) {});

		}).bind_connect([&server](auto & session_ptr)
		{
			session_ptr->no_delay(true);
			//session_ptr->start_timer(1, std::chrono::seconds(1), []() {});
			//session_ptr->stop(); // You can close the connection directly here.
			printf("client enter : %s %u %s %u\n",
				session_ptr->remote_address().c_str(), session_ptr->remote_port(),
				session_ptr->local_address().c_str(), session_ptr->local_port());
		}).bind_disconnect([&server](auto & session_ptr)
		{
			printf("client leave : %s %u %s\n",
				session_ptr->remote_address().c_str(),
				session_ptr->remote_port(), asio2::last_error_msg().c_str());
		}).bind_start([&server](asio::error_code ec)
		{
			printf("start tcp server match role : %s %u %d %s\n", server.listen_address().c_str(), server.listen_port(),
				ec.value(), ec.message().c_str());
		}).bind_stop([&server](asio::error_code ec)
		{
			printf("stop : %d %s\n", ec.value(), ec.message().c_str());
		});
		server.start(host, port, match_role('#'));

		while (std::getchar() != '\n');
		//std::this_thread::sleep_for(std::chrono::seconds(1));

		server.stop();
	}
}
