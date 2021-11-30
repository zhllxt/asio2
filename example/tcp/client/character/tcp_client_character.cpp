//#include <asio2/asio2.hpp>
#include <iostream>
#include <asio2/tcp/tcp_client.hpp>

int main()
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	std::string_view host = "127.0.0.1";
	std::string_view port = "8025";

	asio2::tcp_client client;

	client.start_timer(1, std::chrono::seconds(1), []()
	{
		printf("timer\n");
	});

	client.bind_connect([&](asio::error_code ec)
	{
		if (ec)
			printf("connect failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
			printf("connect success : %s %u\n", client.local_address().c_str(), client.local_port());

		std::string s;
		s += '<';
		int len = 128 + std::rand() % (300);
		for (int i = 0; i < len; i++)
		{
			s += (char)((std::rand() % 26) + 'a');
		}
		//s += '>';
		s += "\r\n";

		client.async_send(s);

	}).bind_disconnect([](asio::error_code ec)
	{
		asio2::detail::ignore_unused(ec);
		printf("disconnect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	}).bind_recv([&](std::string_view sv)
	{
		printf("recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());

		std::string s;
		s += '<';
		int len = 128 + std::rand() % (300);
		for (int i = 0; i < len; i++)
		{
			s += (char)((std::rand() % 26) + 'a');
		}
		//s += '>';
		s += "\r\n";

		// demo of force a packet of data to be sent twice 
		client.async_send(s.substr(0, s.size() / 2), []() {});
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		client.async_send(s.substr(s.size() / 2), [](std::size_t bytes_sent) {std::ignore = bytes_sent; });
	});

	//client.start(host, port, '>');
	client.async_start(host, port, "\r\n");

	while (std::getchar() != '\n');

	return 0;
}
