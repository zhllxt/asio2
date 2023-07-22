#include <asio2/tcp/tcp_client.hpp>

// how to use the match_role, see : https://blog.csdn.net/zhllxt/article/details/127670983

// the byte 1    head   (1 bytes) : #
// the byte 2    length (1 bytes) : the body length
// the byte 3... body   (n bytes) : the body content
using buffer_iterator = asio::buffers_iterator<asio::streambuf::const_buffers_type>;
std::pair<buffer_iterator, bool> match_role(buffer_iterator begin, buffer_iterator end)
{
	buffer_iterator p = begin;
	while (p != end)
	{
		// how to convert the Iterator to char* 
		[[maybe_unused]] const char * buf = &(*p);

		if (*p != '#')
			return std::pair(begin, true); // head character is not #, return and kill the client

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

int main()
{
	std::string_view host = "127.0.0.1";
	std::string_view port = "8026";

	// shared_ptr client
	std::shared_ptr<asio2::tcp_client> client_ptr = std::make_shared<asio2::tcp_client>();

	// note [1]: the callback has holds the client_ptr.
	client_ptr->bind_connect([client_ptr]()
	{
		if (asio2::get_last_error())
			printf("connect failure : %d %s\n",
				asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
			printf("connect success : %s %u\n",
				client_ptr->local_address().c_str(), client_ptr->local_port());

		std::string str;
		str += '#';
		str += char(3);
		str += "abc";

		client_ptr->async_send(str);

	}).bind_recv([client_ptr](std::string_view data)
	{
		printf("recv : %zu %.*s\n", data.size(), (int)data.size(), data.data());

		std::string str;
		str += '#';
		uint8_t len = uint8_t(100 + (std::rand() % 100));
		str += char(len);
		for (uint8_t i = 0; i < len; i++)
		{
			str += (char)((std::rand() % 26) + 'a');
		}

		// this is just a demo to show :
		// even if we force one packet data to be sent twice,
		// but the server must recvd whole packet once
		client_ptr->async_send(str.substr(0, str.size() / 2));
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		client_ptr->async_send(str.substr(str.size() / 2));

		// of course you can sent the whole data once
		//client_ptr->async_send(std::move(str));
	});

	client_ptr->async_start(host, port, match_role);

	while (std::getchar() != '\n');

	// beacuse the client is a shared_ptr, so you must call stop() manual, otherwise
	// the client shared_ptr will can't be destroyed due to a circular reference.
	client_ptr->stop();

	// after the client is stopped, we call async_send(or other functions) again, this
	// will cause circular reference again.
	client_ptr->async_send("abc");

	// and the callback has holds the client shared_ptr(see [1] above), it also cause
	// circular reference .

	// So, we can call destroy to clear the all circular references.
	client_ptr->destroy();

	return 0;
}
