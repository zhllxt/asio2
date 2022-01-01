//#ifndef ASIO2_USE_SSL
//#define ASIO2_USE_SSL
//#endif

#include <asio2/asio2.hpp>
#include <asio2/tcp/tcp_server.hpp>
#include <asio2/tcp/tcp_client.hpp>
#include <iostream>

class my_tcp_session : public asio2::tcp_session_t<my_tcp_session>
{
public:
	//using asio2::tcp_session_t<my_tcp_session>::tcp_session_t;

	template<class... Args>
	my_tcp_session(Args&&... args) : asio2::tcp_session_t<my_tcp_session>(std::forward<Args>(args)...)
	{
		uuid = "custom string";
	}

	// ... user custom properties and functions
	std::string uuid;
};

using my_tcp_server1 = asio2::tcp_server_t<my_tcp_session>;

class my_tcp_server2 : public asio2::tcp_server_t<my_tcp_session>
{
public:
	using asio2::tcp_server_t<my_tcp_session>::tcp_server_t;
};

class my_tcp_client1 : public asio2::tcp_client_t<my_tcp_client1>
{
public:
	// ... user custom properties and functions
	std::string uuid;
};

class my_tcp_client2 : public asio2::tcp_client
{
public:
	// ... user custom properties and functions
	std::string uuid;
};

int main()
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	std::srand((unsigned int)time(nullptr));

	//my_tcp_server1 my_server1;

	//my_server1.post([]()
	//{
	//	std::ignore = true;
	//});
	//my_server1.post([]()
	//{
	//	std::ignore = true; 
	//}, std::chrono::seconds(3));
	//auto future1 = my_server1.post([]()
	//{
	//	std::ignore = true; 
	//}, asio::use_future);
	//auto future2 = my_server1.post([]()
	//{
	//	std::ignore = true;
	//}, std::chrono::seconds(3), asio::use_future);
	//my_server1.dispatch([]()
	//{
	//	std::ignore = true;
	//});
	//auto future3 = my_server1.dispatch([]()
	//{
	//	std::ignore = true;
	//}, asio::use_future);

	//my_server1.bind_connect([&](std::shared_ptr<my_tcp_session>& session_ptr)
	//{
	//	session_ptr->uuid = std::to_string(session_ptr->hash_key());

	//	session_ptr->post([]() {}, std::chrono::seconds(3));

	//}).bind_recv([&](std::shared_ptr<my_tcp_session>& session_ptr, std::string_view sv)
	//{
	//	asio2::ignore_unused(session_ptr, sv);

	//	ASIO2_ASSERT(session_ptr->uuid == std::to_string(session_ptr->hash_key()));

	//	printf("recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());

	//	session_ptr->async_send(sv);
	//});

	//my_server1.start("0.0.0.0", 9981);

	//// --------------------------------------------------------------------------------

	//my_tcp_client1 my_client1;

	//my_client1.bind_connect([&](asio::error_code ec)
	//{
	//	if (!ec)
	//		my_client1.async_send("<0123456789abcdefghijklmnopqrstovwxyz>");

	//}).bind_recv([&](std::string_view sv)
	//{
	//	printf("recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());

	//	my_client1.async_send(sv);
	//});

	//my_client1.async_start("127.0.0.1", 9981);

	//// --------------------------------------------------------------------------------

	// test timer
	for (;;)
	{
		printf("%d\n", std::rand());

		asio2::iopool iopool(4);

		// iopool must start first, othwise the server.start will blocked forever.
		iopool.start();

		asio2::timer timer;

		int b1 = -1, b2 = -1, b3 = -1, b4 = -1;

		timer.stop_timer(1);
		timer.start_timer(1, std::chrono::milliseconds(50), [&]()
		{
			if (asio2::get_last_error())
			{
				ASIO2_ASSERT(true); // must run to here
				b1 = 0;
				return;
			}
			ASIO2_ASSERT(false); // can't run to here
			b1 = 1;
			timer.stop_timer(1);
		});

		timer.stop_timer(1);
		timer.start_timer(1, std::chrono::milliseconds(50), [&]()
		{
			if (asio2::get_last_error())
			{
				ASIO2_ASSERT(true); // must run to here
				b2 = 0;
				ASIO2_ASSERT(b1 == 0);
				return;
			}
			ASIO2_ASSERT(false); // can't run to here
			b2 = 1;
			timer.stop_timer(1);
		});

		timer.post([&]()
		{
			timer.stop_timer(1);
			timer.start_timer(1, std::chrono::milliseconds(50), [&]()
			{
				if (asio2::get_last_error())
				{
					ASIO2_ASSERT(true); // must run to here
					b3 = 0;
					ASIO2_ASSERT(b1 == 0 && b2 == 0);
					return;
				}
				ASIO2_ASSERT(false); // can't run to here
				b3 = 1;
				timer.stop_timer(1);
			});

			timer.stop_timer(1);
			timer.start_timer(1, std::chrono::milliseconds(50), [&]()
			{
				if (asio2::get_last_error())
				{
					ASIO2_ASSERT(b1 == 0 && b2 == 0 && b3 == 0 && b4 == 1); // must run to here second
					b4 = 0;
					return;
				}
				ASIO2_ASSERT(b1 == 0 && b2 == 0 && b3 == 0 && b4 == -1); // must run to here first
				b4 = 1;
				timer.stop_timer(1);
			});
		});

		while (b4 != 0)
			std::this_thread::sleep_for(std::chrono::milliseconds(0));

		asio2::timer t;
		t.post([&]()
		{
			t.start_timer(1, std::chrono::seconds(1), []() {});
		});

		//while (std::getchar() != '\n');

	// --------------------------------------------------------------------------------
	// user io pool
	// --------------------------------------------------------------------------------

	// use a infinite loop for test

	std::string_view port = "8028";


	asio2::udp_cast udp(iopool.get(std::rand() % iopool.size()));

	udp.start_timer(1, std::chrono::seconds(1), []()
	{
		if (asio2::get_last_error())
		{
			printf("timer with error\n");
			std::ignore = true;
			return;
		}
		printf("timer with no error\n");
		std::ignore = true;
	});

	auto ptr = udp.post_event([]()
	{
		std::ignore = true;
	});

	udp.post([]()
	{
		std::ignore = true;
	}, std::chrono::milliseconds(std::rand() % 50));

	std::thread([ptr]() mutable
	{
		ptr->notify();
	}).detach();

	std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 2));

	ptr->notify();

	udp.start("0.0.0.0", 4444);

	std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 10));

	udp.stop();

	udp.start_timer(1, std::chrono::seconds(1), []()
	{
		if (asio2::get_last_error())
		{
			printf("timer with error 2 \n");
			std::ignore = true;
			return;
		}
		printf("timer with no error 2\n");
		std::ignore = true;
	});

	udp.post([]()
	{
		std::ignore = true;
	}, std::chrono::milliseconds(std::rand() % 50));

	ptr = udp.post_event([]()
	{
		std::ignore = true;
	});

	std::thread([ptr]() mutable
	{
		ptr->notify();
	}).detach();

	ptr.reset();

	ptr = udp.post_event([]()
	{
		std::ignore = true;
	});

	//-----------------------------------------------------------------------------------

	asio2::tcp_server server(std::vector<asio2::io_t*>{ 
		&iopool.get(0), &iopool.get(1), &iopool.get(2), &iopool.get(3) });

	int index = 0;

	// the server's io_context must be the user passed io_context.
	for (std::size_t i = 0; i < server.iopool().size(); i++)
	{
		asio::io_context& ioc1 = iopool.get(i).context();
		asio::io_context& ioc2 = server.iopool().get(i).context();

		ASIO2_ASSERT(&ioc1 == &ioc2);
	}

	// just for test the constructor sfinae
	asio2::tcp_server server1(1024);
	asio2::tcp_server server2(1024, 65535);
	asio2::tcp_server server3(1024, 65535, 4);
	asio2::tcp_server server5(1024, 65535, std::vector{ &iopool.get(0), &iopool.get(1) });
	asio2::tcp_server server6(1024, 65535, std::list{ &iopool.get(0), &iopool.get(1) });
	asio2::rpc_server server7(1024, 65535, &iopool.get(0));
	asio2::http_server server8(1024, 65535, iopool.get(0));
	asio2::udp_server servera(std::vector<asio2::io_t*>{ &iopool.get(0), &iopool.get(1) });
	asio2::ws_server  serverb(std::list  <asio2::io_t*>{ &iopool.get(0), &iopool.get(1) });
	asio2::tcp_server serverc(&iopool.get(0));
	asio2::tcp_server serverd(iopool.get(0));

	server.start_timer(1, std::chrono::seconds(1), []() {}); // test timer
	server.bind_recv([&](std::shared_ptr<asio2::tcp_session> & session_ptr, std::string_view s)
	{
		ASIO2_ASSERT(session_ptr->io().strand().running_in_this_thread());

		//printf("recv : %u %.*s\n", (unsigned)s.size(), (int)s.size(), s.data());

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

	asio2::timer timer2(&iopool.get(index));

	timer2.start_timer(1, std::chrono::seconds(3), [&]()
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
	asio2::http_client clienta(std::vector<asio2::io_t*>{ &iopool.get(0) });
	asio2::ws_client   clientb(std::list  <asio2::io_t*>{ &iopool.get(0) });
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

		//printf("recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());

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

	std::this_thread::sleep_for(std::chrono::milliseconds(20 + std::rand() % 200));

	client.stop();

	server.stop();

	//timer2.stop_all_timers();

	// before call iopool.stop, all client's and server's and timer's stop must be
	// called alerady, othwise the iopool.stop will blocked forever.
	iopool.stop();

	{
		// iopool must start first, othwise the server.start will blocked forever.
		iopool.start();

		asio2::timer timer(iopool.get(std::rand() % iopool.size()));

		int b1 = -1, b2 = -1, b3 = -1, b4 = -1;

		timer.stop_timer(1);
		timer.start_timer(1, std::chrono::milliseconds(50), [&]()
		{
			if (asio2::get_last_error())
			{
				ASIO2_ASSERT(true); // must run to here
				b1 = 0;
				return;
			}
			ASIO2_ASSERT(false); // can't run to here
			b1 = 1;
			timer.stop_timer(1);
		});

		timer.stop_timer(1);
		timer.start_timer(1, std::chrono::milliseconds(50), [&]()
		{
			if (asio2::get_last_error())
			{
				ASIO2_ASSERT(true); // must run to here
				b2 = 0;
				ASIO2_ASSERT(b1 == 0);
				return;
			}
			ASIO2_ASSERT(false); // can't run to here
			b2 = 1;
			timer.stop_timer(1);
		});

		timer.post([&]()
		{
			timer.stop_timer(1);
			timer.start_timer(1, std::chrono::milliseconds(50), [&]()
			{
				if (asio2::get_last_error())
				{
					ASIO2_ASSERT(true); // must run to here
					b3 = 0;
					ASIO2_ASSERT(b1 == 0 && b2 == 0);
					return;
				}
				ASIO2_ASSERT(false); // can't run to here
				b3 = 1;
				timer.stop_timer(1);
			});

			timer.stop_timer(1);
			timer.start_timer(1, std::chrono::milliseconds(50), [&]()
			{
				if (asio2::get_last_error())
				{
					ASIO2_ASSERT(b1 == 0 && b2 == 0 && b3 == 0 && b4 == 1); // must run to here second
					b4 = 0;
					return;
				}
				ASIO2_ASSERT(b1 == 0 && b2 == 0 && b3 == 0 && b4 == -1); // must run to here first
				b4 = 1;
				timer.stop_timer(1);
			});
		});

		while (b4 != 0)
			std::this_thread::sleep_for(std::chrono::milliseconds(0));

		asio2::timer t(iopool.get(std::rand() % iopool.size()));
		t.post([&]()
		{
			t.start_timer(1, std::chrono::seconds(1), []() {});
		});

		//while (std::getchar() != '\n');

	// --------------------------------------------------------------------------------
	// user io pool
	// --------------------------------------------------------------------------------

	// use a infinite loop for test

		std::string_view port = "8028";


		asio2::udp_cast udp(iopool.get(std::rand() % iopool.size()));

		udp.start_timer(1, std::chrono::seconds(1), []()
		{
			if (asio2::get_last_error())
			{
				printf("timer with error\n");
				std::ignore = true;
				return;
			}
			printf("timer with no error\n");
			std::ignore = true;
		});

		auto ptr = udp.post_event([]()
		{
			std::ignore = true;
		});

		udp.post([]()
		{
			std::ignore = true;
		}, std::chrono::milliseconds(std::rand() % 50));

		std::thread([ptr]() mutable
		{
			ptr->notify();
		}).detach();

		std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 2));

		ptr->notify();

		udp.start("0.0.0.0", 4444);

		std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 10));

		udp.stop();

		udp.start_timer(1, std::chrono::seconds(1), []()
		{
			if (asio2::get_last_error())
			{
				printf("timer with error 2 \n");
				std::ignore = true;
				return;
			}
			printf("timer with no error 2\n");
			std::ignore = true;
		});

		udp.post([]()
		{
			std::ignore = true;
		}, std::chrono::milliseconds(std::rand() % 50));

		ptr = udp.post_event([]()
		{
			std::ignore = true;
		});

		std::thread([ptr]() mutable
		{
			ptr->notify();
		}).detach();

		ptr.reset();

		ptr = udp.post_event([]()
		{
			std::ignore = true;
		});

		//-----------------------------------------------------------------------------------

		asio2::tcp_server server(std::vector<asio2::io_t*>{
			&iopool.get(0), & iopool.get(1), & iopool.get(2), & iopool.get(3) });

		int index = 0;

		// the server's io_context must be the user passed io_context.
		for (std::size_t i = 0; i < server.iopool().size(); i++)
		{
			asio::io_context& ioc1 = iopool.get(i).context();
			asio::io_context& ioc2 = server.iopool().get(i).context();

			ASIO2_ASSERT(&ioc1 == &ioc2);
		}

		// just for test the constructor sfinae
		asio2::tcp_server server1(1024);
		asio2::tcp_server server2(1024, 65535);
		asio2::tcp_server server3(1024, 65535, 4);
		asio2::tcp_server server5(1024, 65535, std::vector{ &iopool.get(0), &iopool.get(1) });
		asio2::tcp_server server6(1024, 65535, std::list{ &iopool.get(0), &iopool.get(1) });
		asio2::rpc_server server7(1024, 65535, &iopool.get(0));
		asio2::http_server server8(1024, 65535, iopool.get(0));
		asio2::udp_server servera(std::vector<asio2::io_t*>{ &iopool.get(0), & iopool.get(1) });
		asio2::ws_server  serverb(std::list  <asio2::io_t*>{ &iopool.get(0), & iopool.get(1) });
		asio2::tcp_server serverc(&iopool.get(0));
		asio2::tcp_server serverd(iopool.get(0));

		server.start_timer(1, std::chrono::seconds(1), []() {}); // test timer
		server.bind_recv([&](std::shared_ptr<asio2::tcp_session>& session_ptr, std::string_view s)
		{
			ASIO2_ASSERT(session_ptr->io().strand().running_in_this_thread());

			//printf("recv : %u %.*s\n", (unsigned)s.size(), (int)s.size(), s.data());

			session_ptr->async_send(s, [](std::size_t bytes_sent) {std::ignore = bytes_sent; });

		}).bind_connect([&](auto& session_ptr)
		{
			ASIO2_ASSERT(iopool.running_in_thread(0));

			session_ptr->no_delay(true);

			session_ptr->start_timer(2, std::chrono::seconds(1), []() {}); // test timer

			//session_ptr->stop(); // You can close the connection directly here.

			printf("client enter : %s %u %s %u\n",
				session_ptr->remote_address().c_str(), session_ptr->remote_port(),
				session_ptr->local_address().c_str(), session_ptr->local_port());

		}).bind_disconnect([&](auto& session_ptr)
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

		asio2::timer timer2(&iopool.get(index));

		timer2.start_timer(1, std::chrono::seconds(3), [&]()
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
		asio2::http_client clienta(std::vector<asio2::io_t*>{ &iopool.get(0) });
		asio2::ws_client   clientb(std::list  <asio2::io_t*>{ &iopool.get(0) });
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

			//printf("recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());

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

		std::this_thread::sleep_for(std::chrono::milliseconds(20 + std::rand() % 200));

		client.stop();

		server.stop();

		//timer2.stop_all_timers();

		// before call iopool.stop, all client's and server's and timer's stop must be
		// called alerady, othwise the iopool.stop will blocked forever.
		iopool.stop();
	}
	}

	return 0;
}
