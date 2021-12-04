//#ifndef ASIO2_USE_SSL
//#define ASIO2_USE_SSL
//#endif

#include <asio2/asio2.hpp>
#include <iostream>
#include <asio2/tcp/tcp_server.hpp>
#include <asio2/tcp/tcp_client.hpp>

int main()
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	// use a infinite loop for test
	for(;;)
	{
	std::string_view port = "8028";

	std::srand((unsigned int)time(nullptr));

	asio2::iopool iopool(4);

	// iopool must start first, othwise the server.start will blocked forever.
	iopool.start();

	//-----------------------------------------------------------------------------------

	asio2::tcp_server server(std::vector{ &iopool.get(0), &iopool.get(1), &iopool.get(2), &iopool.get(3) });

	int index = 0;

	// the server's io_context must be the user passed io_context.
	server.iopool().for_each([&iopool,&index](asio::io_context& ioc) mutable
	{
		asio2::ignore_unused(iopool, index, ioc);

		ASIO2_ASSERT(&ioc == &iopool.get(index++));
	});

	// just for test the constructor sfinae
	asio2::tcp_server server1(1024);
	asio2::tcp_server server2(1024, 65535);
	asio2::tcp_server server3(1024, 65535, 4);
	asio2::tcp_server server5(1024, 65535, std::vector{ &iopool.get(0), &iopool.get(1) });
	asio2::tcp_server server6(1024, 65535, std::list{ &iopool.get(0), &iopool.get(1) });
	asio2::rpc_server server7(1024, 65535, &iopool.get(0));
	asio2::http_server server8(1024, 65535, iopool.get(0));
	asio2::udp_server servera(std::vector{ &iopool.get(0), &iopool.get(1) });
	asio2::ws_server serverb(std::list{ &iopool.get(0), &iopool.get(1) });
	asio2::tcp_server serverc(&iopool.get(0));
	asio2::tcp_server serverd(iopool.get(0));

	server.start_timer(1, std::chrono::seconds(1), []() {}); // test timer
	server.bind_recv([&](std::shared_ptr<asio2::tcp_session> & session_ptr, std::string_view s)
	{
		ASIO2_ASSERT(session_ptr->io().strand().running_in_this_thread());

		printf("recv : %u %.*s\n", (unsigned)s.size(), (int)s.size(), s.data());

		session_ptr->async_send(s, [](std::size_t bytes_sent) {std::ignore = bytes_sent; });

	}).bind_connect([&](auto & session_ptr)
	{
		ASIO2_ASSERT(iopool.running_in_thread(0));

		session_ptr->no_delay(true);

		session_ptr->start_timer(2, std::chrono::seconds(1), []() {}); // test timer

		//session_ptr->stop(); // You can close the connection directly here.

		printf("client enter : %s %u %s %u\n",
			session_ptr->remote_address().c_str(), session_ptr->remote_port(),
			session_ptr->local_address().c_str(), session_ptr->local_port());

	}).bind_disconnect([&](auto & session_ptr)
	{
		ASIO2_ASSERT(iopool.running_in_thread(0));

		printf("client leave : %s %u %s\n",
			session_ptr->remote_address().c_str(),
			session_ptr->remote_port(), asio2::last_error_msg().c_str());
	}).bind_start([&](asio::error_code ec)
	{
		ASIO2_ASSERT(iopool.running_in_thread(0));

		printf("start tcp server : %s %u %d %s\n",
			server.listen_address().c_str(), server.listen_port(),
			ec.value(), ec.message().c_str());
	}).bind_stop([&](asio::error_code ec)
	{
		ASIO2_ASSERT(iopool.running_in_thread(0));

		printf("stop : %d %s\n", ec.value(), ec.message().c_str());
	});

	server.start("0.0.0.0", port);

	//-----------------------------------------------------------------------------------

	index = std::rand() % iopool.size();

	asio2::timer timer(&iopool.get(index));

	timer.start_timer(1, std::chrono::seconds(3), [&]()
	{
		ASIO2_ASSERT(iopool.running_in_thread(index));
	});

	asio2::tcp_client client(iopool.get(index));

	// just for test the constructor sfinae
	asio2::tcp_client client1(1024);
	asio2::tcp_client client2(1024, 65535);
	asio2::tcp_client client3(1024, 65535, 4);
	asio2::tcp_client client5(1024, 65535, std::vector{ &iopool.get(0) });
	asio2::tcp_client client6(1024, 65535, std::list{ &iopool.get(0) });
	asio2::tcp_client client7(1024, 65535, &iopool.get(0));
	asio2::tcp_client client8(1024, 65535, iopool.get(0));
	asio2::http_client clienta(std::vector{ &iopool.get(0) });
	asio2::ws_client clientb(std::list{ &iopool.get(0) });
	asio2::udp_client clientc(&iopool.get(0));
	asio2::rpc_client clientd(iopool.get(0));

	asio2::serial_port sp(iopool.get(1));
	asio2::udp_cast cast(&iopool.get(2));
	asio2::ping ping(&iopool.get(3));

	client.auto_reconnect(true, std::chrono::milliseconds(1000)); // enable auto reconnect and use custom delay

	client.start_timer(1, std::chrono::seconds(1), []() {}); // test timer

	client.bind_connect([&](asio::error_code ec)
	{
		asio2::detail::ignore_unused(ec);

		ASIO2_ASSERT(iopool.running_in_thread(index));

		if (asio2::get_last_error())
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
		s += '>';

		client.async_send(s);

	}).bind_disconnect([&](asio::error_code ec)
	{
		ASIO2_ASSERT(iopool.running_in_thread(index));

		printf("disconnect : %d %s\n", ec.value(), ec.message().c_str());
	}).bind_recv([&](std::string_view sv)
	{
		ASIO2_ASSERT(iopool.running_in_thread(index));

		printf("recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());

		std::string s;
		s += '#';
		uint8_t len = uint8_t(100 + (std::rand() % 100));
		s += char(len);
		for (uint8_t i = 0; i < len; i++)
		{
			s += (char)((std::rand() % 26) + 'a');
		}

		client.async_send(std::move(s), []() {});

	});

	client.async_start("127.0.0.1", port);

	//while (std::getchar() != '\n');  // press enter to exit this program

	std::this_thread::sleep_for(std::chrono::seconds(1 + std::rand() % 2));

	client.stop();

	server.stop();

	timer.stop_all_timers();

	// before call iopool.stop, all client's and server's and timer's stop must be
	// called alerady, othwise the iopool.stop will blocked forever.
	iopool.stop();
	}

	return 0;
}
