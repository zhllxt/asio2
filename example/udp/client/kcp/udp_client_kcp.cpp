#include <asio2/udp/udp_client.hpp>
#include <iostream>
#include <asio2/util/ini.hpp>

int main()
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	std::string_view host = "127.0.0.1";
	std::string_view port = "8036";

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
		client.async_send(std::string("1<abcdefghijklmnopqrstovuxyz0123456789>"));
	}).bind_disconnect([]()
	{
		printf("disconnect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	}).bind_recv([&](std::string_view sv)
	{
		printf("recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());

		//std::this_thread::sleep_for(std::chrono::milliseconds(100));

		std::string s;
		s += '2';
		s += '<';
		int len = 33 + std::rand() % (126 - 33);
		for (int i = 0; i < len; i++)
		{
			s += (char)((std::rand() % 26) + 'a');
		}
		s += '>';

		client.async_send(std::move(s));
		//client.async_send(sv, asio::use_future);
		//client.async_send(sv, [](std::size_t bytes_sent) {});

	}).bind_handshake([&]()
	{
		if (asio2::get_last_error())
			printf("handshake failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
			printf("handshake success : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());

		// this send will be failed, because connection is not fully completed
		client.async_send("abc", []()
		{
			ASIO2_ASSERT(asio2::get_last_error());
			std::cout << "send failed : " << asio2::last_error_msg() << std::endl;
		});
	});

	// to use kcp, the last param must be : asio2::use_kcp
	client.async_start(host, port, asio2::use_kcp);
	//auto * kp = client.kcp();

	std::string s;
	s += '#';

	client.send(s);
	client.send(std::move(s));
	client.send("abc");

	while (std::getchar() != '\n');

	client.stop();

	return 0;
}
