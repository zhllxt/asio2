//#include <asio2/asio2.hpp>
#include <iostream>
#include <asio2/udp/udp_client.hpp>

int main()
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	std::string_view host = "127.0.0.1";
	std::string_view port = "8035";

	asio2::udp_client client;

	//std::string msg;
	//msg.resize(15000, 'a');

	client.bind_connect([&]()
	{
		if (asio2::get_last_error())
			printf("connect failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
			printf("connect success : %s %u\n", client.local_address().c_str(), client.local_port());

		//client.async_send(msg);
		//client.async_send(std::string("<abcdefghijklmnopqrstovuxyz0123456789>"));

	}).bind_disconnect([]()
	{
		printf("disconnect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	}).bind_recv([&](std::string_view sv)
	{
		printf("recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());

		//std::this_thread::sleep_for(std::chrono::milliseconds(100));

		std::string s;
		s += '<';
		int len = 33 + std::rand() % (126 - 33);
		for (int i = 0; i < len; i++)
		{
			s += (char)((std::rand() % 26) + 'a');
		}
		s += '>';

		//client.async_send(std::move(s));

		//client.async_send(sv, asio::use_future);
		//client.async_send(sv, [](std::size_t bytes_sent) {});

	});

	if (!client.start(host, port))
		printf("start failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());

	std::string s;
	//s += '#';
	s.resize(2000);

	// Example for Synchronous send data. The return value is the sent bytes.
	// You can use asio2::get_last_error() to check whether some error occured.
	client.send(s);
	//client.send(std::move(s));
	//client.send("abc");

	while (std::getchar() != '\n');

	client.send(s);

	while (std::getchar() != '\n');

	client.stop();

	return 0;
}
