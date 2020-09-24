// When compiling with vs under linux, you need to copy the "asio,beast,ceral,fmt" folders to
// the /usr/local/include directory first, and copy the "libcrypto.a,libssl.a" files to 
// /usr/local/lib directory first. "libcrypto.a,libssl.a" is in "asio2/lib/x64".

#ifndef ASIO2_USE_SSL
#define ASIO2_USE_SSL
#endif

#include <asio2/asio2.hpp>

// how to use the match_role, see : https://blog.csdn.net/zhllxt/article/details/104772948

// the byte 1    head   (1 bytes) : #
// the byte 2    length (1 bytes) : the body length
// the byte 3... body   (n bytes) : the body content
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

			int length = std::uint8_t(*i); // get content length

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

namespace asio
{
	template <> struct is_match_condition<match_role> : public std::true_type {};
}

int main()
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	std::string_view host = "0.0.0.0";
	std::string_view port = "8001";

	asio2::tcps_server server;

	server.set_verify_mode(asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert);

	server.set_cert_file("../../../cert/ca.crt", "../../../cert/server.crt", "../../../cert/server.key", "server");
	server.set_dh_file("../../../cert/dh1024.pem");

	server.bind_recv([&](auto & session_ptr, std::string_view s)
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

		//session_ptr->send(s);

	}).bind_start([&](asio::error_code ec)
	{
		printf("start tcp server match role : %s %u %d %s\n", server.listen_address().c_str(), server.listen_port(),
			ec.value(), ec.message().c_str());
	}).bind_stop([&](asio::error_code ec)
	{
		printf("stop tcp server match role : %d %s\n", ec.value(), ec.message().c_str());
	});

	server.start(host, port, match_role('#'));

	while (std::getchar() != '\n');

	return 0;
}
