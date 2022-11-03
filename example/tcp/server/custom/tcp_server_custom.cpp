#include <asio2/tcp/tcp_server.hpp>

// how to use the match_role, see : https://blog.csdn.net/zhllxt/article/details/127670983

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
		Iterator p = begin;
		while (p != end)
		{
			// how to convert the Iterator to char* 
			[[maybe_unused]] const char * buf = &(*p);

			// eg : How to close illegal clients
			if (*p != c_)
			{
				// method 1:
				// call the session stop function directly, you need add the init function, see below.
				session_ptr_->stop();
				break;

				// method 2:
				// return the matching success here and then determine the number of bytes received
				// in the on_recv callback function, if it is 0, we close the connection in on_recv.
				//return std::pair(begin, true); // head character is not #, return and kill the client
			}

			p++;
			if (p == end) break;

			int length = std::uint8_t(*p); // get content length

			p++;
			if (p == end) break;

			if (end - p >= length)
				return std::pair(p + length, true);

			break;
		}
		return std::pair(begin, false);
	}

	// the asio2 framework will call this function immediately after the session is created, 
	// you can save the session pointer into a member variable, or do something else.
	void init(std::shared_ptr<asio2::tcp_session>& session_ptr)
	{
		session_ptr_ = session_ptr;
	}

private:
	char c_;

	// note : use a shared_ptr to save the session does not cause circular reference.
	std::shared_ptr<asio2::tcp_session> session_ptr_;
};

#ifdef ASIO_STANDALONE
namespace asio
#else
namespace boost::asio
#endif
{
	template <> struct is_match_condition<match_role> : public std::true_type {};
}

int main()
{
	std::string_view host = "0.0.0.0";
	std::string_view port = "8026";

	asio2::tcp_server server;

	server.bind_recv([&](auto & session_ptr, std::string_view data)
	{
		// how to close the illegal client, see : https://blog.csdn.net/zhllxt/article/details/127670983
		if (data.size() == 0)
		{
			printf("close illegal client : %s %u\n",
				session_ptr->remote_address().c_str(), session_ptr->remote_port());
			session_ptr->stop();
			return;
		}

		printf("recv : %zu %.*s\n", data.size(), (int)data.size(), data.data());

		// this is just a demo to show :
		// even if we force one packet data to be sent twice,
		// but the client must recvd whole packet once
		session_ptr->async_send(data.substr(0, data.size() / 2));
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		session_ptr->async_send(data.substr(data.size() / 2));

		// of course you can sent the whole data once
		//session_ptr->async_send(data);

	}).bind_start([&]()
	{
		printf("start tcp server match role : %s %u %d %s\n",
			server.listen_address().c_str(), server.listen_port(),
			asio2::last_error_val(), asio2::last_error_msg().c_str());
	}).bind_stop([&]()
	{
		printf("stop tcp server match role : %d %s\n",
			asio2::last_error_val(), asio2::last_error_msg().c_str());
	});

	server.start(host, port, match_role('#'));

	while (std::getchar() != '\n');

	server.stop();

	return 0;
}
