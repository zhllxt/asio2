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
	std::string_view port = "8027";

	asio2::tcp_client client;

	client.bind_connect([&](asio::error_code ec)
	{
		if (ec)
			printf("connect failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
			printf("connect success : %s %u\n", client.local_address().c_str(), client.local_port());

		std::string s;
		s += '#';
		s += char(1);
		s += 'a';

		// Beacuse the server specify the "max recv buffer size" to 1024, so if we
		// send a too long packet, then this client will be disconnect .
		//s.resize(1500);

		client.async_send(s);

	}).bind_disconnect([](asio::error_code ec)
	{
		printf("disconnect : %d %s\n", ec.value(), ec.message().c_str());
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

		client.async_send(std::move(s));

	});

	asio2::rdc::option rdc_option
	{
		[](std::string_view data)
		{
			int id = std::strtol(data.substr(0,4).data(), nullptr, 10);
			return id;
		}
	};

	asio2::socks5::option<asio2::socks5::method::anonymous, asio2::socks5::method::password>
		sock5_option{ "s5.doudouip.cn",1088,"zjww-1","aaa123" };

	client.start(host, port, asio2::use_dgram, std::move(rdc_option), std::move(sock5_option));

	while (std::getchar() != '\n');

	return 0;
}
