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

	std::string_view host = "0.0.0.0";
	std::string_view port = "8020";

	//while (1)
	{
		printf("\n");
		asio2::udp_cast sender;
		sender.bind_recv([&](asio::ip::udp::endpoint& endpoint, std::string_view sv)
		{
			printf("recv : %s %u %u %.*s\n", endpoint.address().to_string().c_str(),
				endpoint.port(), (unsigned)sv.size(), (int)sv.size(), sv.data());
		}).bind_start([&](asio::error_code ec)
		{
			printf("start : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		}).bind_stop([&](asio::error_code ec)
		{
			printf("stop : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		}).bind_init([&]()
		{
			//// Join the multicast group.
			//sender.socket().set_option(
			//	asio::ip::multicast::join_group(asio::ip::make_address("239.255.0.1")));
		});

		sender.start(host, port);

		std::string s("<0123456789abcdefghijklmnopqrstowvxyz>");
		asio::ip::udp::endpoint ep1(asio::ip::make_address("::FFFF:127.0.0.1"), 18080); // ipv6 address
		sender.send(ep1, s, [](std::size_t bytes_sent)
		{
			printf("send : %u %s\n", (unsigned)bytes_sent, asio2::last_error_msg().c_str());
		});
		sender.send("239.255.0.1", "8080", s, []() // this is a multicast address
		{
			printf("send : %s\n", asio2::last_error_msg().c_str());
		});

		std::string shost("239.255.0.1"), sport("8080");

		int narys[2] = { 1,2 };
		sender.send(shost, sport, s);
		sender.send(shost, sport, s, []() {});
		sender.send(shost, sport, (uint8_t*)"<abcdefghijklmnopqrstovuxyz0123456789>", 10);
		sender.send(shost, sport, (const uint8_t*)"<abcdefghijklmnopqrstovuxyz0123456789>", 10);
		sender.send(shost, sport, s.data(), 5);
		sender.send(shost, sport, s.data(), []() {});
		sender.send(shost, sport, s.c_str(), size_t(5));
		sender.send(shost, sport, s, asio::use_future);
		sender.send(shost, sport, "<abcdefghijklmnopqrstovuxyz0123456789>", asio::use_future);
		sender.send(shost, sport, s.data(), asio::use_future);
		sender.send(shost, sport, s.c_str(), asio::use_future);
		sender.send(shost, sport, narys);
		sender.send(shost, sport, narys, []() {});
		sender.send(shost, sport, narys, [](std::size_t bytes) {});
		std::future<std::pair<asio::error_code, std::size_t>> fu1 = 
			sender.send(shost, sport, narys, asio::use_future);
		auto[ec, size] = fu1.get();
		std::cout << "send result : " << ec.message() << " " << size << std::endl;

		sender.send(ep1, s);
		sender.send(ep1, s, []() {});
		sender.send(ep1, (uint8_t*)"<abcdefghijklmnopqrstovuxyz0123456789>", 10);
		sender.send(ep1, (const uint8_t*)"<abcdefghijklmnopqrstovuxyz0123456789>", 10);
		sender.send(ep1, s.data(), 5);
		sender.send(ep1, s.data(), []() {});
		sender.send(ep1, s.c_str(), size_t(5));
		sender.send(ep1, s, asio::use_future);
		sender.send(ep1, "<abcdefghijklmnopqrstovuxyz0123456789>", asio::use_future);
		sender.send(ep1, s.data(), asio::use_future);
		sender.send(ep1, s.c_str(), asio::use_future);
		sender.send(ep1, std::move(s));
		sender.send(ep1, narys);
		sender.send(ep1, narys, []() {});
		sender.send(ep1, narys, [](std::size_t bytes) {});
		sender.send(ep1, narys, asio::use_future);

		// the resolve function is a time-consuming operation
		//asio::ip::udp::resolver resolver(sender.io().context());
		//asio::ip::udp::resolver::query query("www.baidu.com", "18080");
		//asio::ip::udp::endpoint ep2 = *resolver.resolve(query);
		//sender.send(ep2, std::move(s));

		while (std::getchar() != '\n');
		//std::this_thread::sleep_for(std::chrono::seconds(2));

		sender.stop();
	}

	return 0;
}
