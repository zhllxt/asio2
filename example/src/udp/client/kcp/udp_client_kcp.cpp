// When compiling with vs under linux, you need to copy the "asio,beast,ceral,fmt" folders to
// the /usr/local/include directory first, and copy the "libcrypto.a,libssl.a" files to 
// /usr/local/lib directory first. "libcrypto.a,libssl.a" is in "asio2/lib/x64".

#include <asio2/asio2.hpp>

int main()
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	std::string_view host = "127.0.0.1";
	std::string_view port = "8036";

	asio2::udp_client client;

	//client.local_endpoint(asio::ip::address_v4::from_string("127.0.0.1"), 9876);
	client.local_endpoint(asio::ip::udp::v4(), 8679);

	//std::string msg;
	//msg.resize(15000, 'a');

	client.bind_connect([&](asio::error_code ec)
	{
		if (asio2::get_last_error())
			printf("connect failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
			printf("connect success : %s %u\n", client.local_address().c_str(), client.local_port());

		//client.send(msg);
		client.send(std::string("<abcdefghijklmnopqrstovuxyz0123456789>"));

	}).bind_disconnect([](asio::error_code ec)
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

		client.send(std::move(s));
		//client.post([&client, s = std::string(sv)]()
		//{
		//	client.stream().send(asio::buffer(std::move(s)));
		//});
		//client.send(sv, asio::use_future);
		//client.send(sv, [](std::size_t bytes_sent) {});
		//asio::write(client.get_socket(), asio::buffer(s));

	}).bind_handshake([&](asio::error_code ec)
	{
		if (asio2::get_last_error())
			printf("handshake failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
			printf("handshake success : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());

		//client.send(msg);
		if (!asio2::get_last_error())
			client.send(std::string("<abcdefghijklmnopqrstovuxyz0123456789abcdefghijklmnopqrstovuxyz0123456789>"));
	});

	// to use kcp, the last param must be : asio2::use_kcp
	client.async_start(host, port, asio2::use_kcp);
	//auto * kp = client.kcp();

	while (std::getchar() != '\n');

	client.stop();

	return 0;
}
