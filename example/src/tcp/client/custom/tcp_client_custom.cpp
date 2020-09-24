// When compiling with vs under linux, you need to copy the "asio,beast,ceral,fmt" folders to
// the /usr/local/include directory first, and copy the "libcrypto.a,libssl.a" files to 
// /usr/local/lib directory first. "libcrypto.a,libssl.a" is in "asio2/lib/x64".

#include <asio2/asio2.hpp>

// how to use the match_role, see : https://blog.csdn.net/zhllxt/article/details/104772948

// the byte 1    head   (1 bytes) : #
// the byte 2    length (1 bytes) : the body length
// the byte 3... body   (n bytes) : the body content
using iterator = asio::buffers_iterator<asio::streambuf::const_buffers_type>;
std::pair<iterator, bool> match_role(iterator begin, iterator end)
{
	iterator i = begin;
	while (i != end)
	{
		if (*i != '#')
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

int main()
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	std::string_view host = "127.0.0.1";
	std::string_view port = "8026";

	asio2::tcp_client client;

	client.bind_connect([&](asio::error_code ec)
	{
		if (asio2::get_last_error())
			printf("connect failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
			printf("connect success : %s %u\n", client.local_address().c_str(), client.local_port());

		std::string s;
		s += '#';
		s += char(1);
		s += 'a';

		client.send(s);

	}).bind_disconnect([](asio::error_code ec)
	{
		printf("disconnect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	}).bind_recv([&](std::string_view sv)
	{
		printf("recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());

		std::string s;
		s += '#';
		uint8_t len = uint8_t(100 + (std::rand() % 100));
		s += char(len);
		for (uint8_t i = 0; i < len; i++)
		{
			s += (char)((std::rand() % 26) + 'a');
		}
		// demo of force a packet of data to be sent twice
		client.send(s.substr(0, s.size() / 2), []() {});
		//std::this_thread::sleep_for(std::chrono::milliseconds(10));
		client.send(s.substr(s.size() / 2), [](std::size_t bytes_sent) {});
	});

	client.async_start(host, port, match_role);

	while (std::getchar() != '\n');

	return 0;
}
