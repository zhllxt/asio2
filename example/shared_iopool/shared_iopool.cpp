//#ifndef ASIO2_USE_SSL
//#define ASIO2_USE_SSL
//#endif

#define ASIO2_ENABLE_TIMER_CALLBACK_WHEN_ERROR

#include <asio2/asio2.hpp>
#include <asio2/tcp/tcp_server.hpp>
#include <asio2/tcp/tcp_client.hpp>
#include <iostream>

int main()
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

#if defined(_DEBUG) || defined(DEBUG)
	printf("current is debug mode\n");
#else
	printf("current is release mode\n");
#endif

	std::this_thread::sleep_for(std::chrono::seconds(5));

	std::srand((unsigned int)time(nullptr));

	std::vector<std::shared_ptr<std::thread>> threads;

	// test timer
	for (;;)
	//for (int loop = 0; loop < 100; loop++)
	{
		printf("%d\n", std::rand());

		asio2::iopool iopool(4);

		// iopool must start first, othwise the server.start will blocked forever.
		iopool.start();

		//// --------------------------------------------------------------------------------

		// if you create a object and it will destroyed before iopool, you must do two things:
		// 1. create the object as shared_ptr<asio2::tcp_client>, can't be asio2::tcp_client.
		// 2. you must call stop() function manual.
		{
			// 1. create the object as shared_ptr<asio2::tcp_client>
			std::shared_ptr<asio2::tcp_client> tcp_client = std::make_shared<asio2::tcp_client>(iopool.get(0));

			tcp_client->start("127.0.0.1", 4567);

			tcp_client->async_send("i love you");

			// 2. call stop() function manual
			tcp_client->stop();
		}

		//// --------------------------------------------------------------------------------

		asio2::timer timer;

		int b1 = -1, b2 = -1, b3 = -1, b4 = -1;

		timer.stop_timer(1);
		timer.start_timer(1, std::chrono::milliseconds(100), [&]()
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
		timer.start_timer(1, std::chrono::milliseconds(100), [&]()
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
			timer.start_timer(1, std::chrono::milliseconds(100), [&]()
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
			timer.start_timer(1, std::chrono::milliseconds(100), [&]()
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

		asio2::timer t1;
		t1.post([&]()
		{
			t1.start_timer(1, std::chrono::seconds(1), []() {});
		});

		asio2::timer t2;
		t2.start_timer("2011", std::chrono::milliseconds(10), []() {});
		t2.start_timer("2012", std::chrono::milliseconds(10), std::chrono::milliseconds(10), []() {});
		t2.start_timer("2013", std::chrono::milliseconds(10), 3, []() {});
		t2.start_timer("2014", std::chrono::milliseconds(10), 3, std::chrono::milliseconds(10), []() {});
		t2.post([&]()
		{
			t2.start_timer("2021", std::chrono::milliseconds(10), []() {});
			t2.start_timer("2022", std::chrono::milliseconds(10), std::chrono::milliseconds(10), []() {});
			t2.start_timer("2023", std::chrono::milliseconds(10), 3, []() {});
			t2.start_timer("2024", std::chrono::milliseconds(10), 3, std::chrono::milliseconds(10), []() {});
		});

		std::thread([&t2]()
		{
			t2.start_timer("2031", std::chrono::milliseconds(10), []() {});
			t2.start_timer("2032", std::chrono::milliseconds(10), std::chrono::milliseconds(10), []() {});
			t2.start_timer("2033", std::chrono::milliseconds(10), 3, []() {});
			t2.start_timer("2034", std::chrono::milliseconds(10), 3, std::chrono::milliseconds(10), []() {});
			t2.post([&]()
			{
				t2.start_timer("2041", std::chrono::milliseconds(10), []() {});
				t2.start_timer("2042", std::chrono::milliseconds(10), std::chrono::milliseconds(10), []() {});
				t2.start_timer("2043", std::chrono::milliseconds(10), 3, []() {});
				t2.start_timer("2044", std::chrono::milliseconds(10), 3, std::chrono::milliseconds(10), []() {});
			});
		}).join();

		asio2::timer t3(iopool.get(std::rand() % iopool.size()));
		t3.start_timer("3011", std::chrono::milliseconds(10), []() {});
		t3.start_timer("3012", std::chrono::milliseconds(10), std::chrono::milliseconds(10), []() {});
		t3.start_timer("3013", std::chrono::milliseconds(10), 3, []() {});
		t3.start_timer("3014", std::chrono::milliseconds(10), 3, std::chrono::milliseconds(10), []() {});
		t3.post([&]()
		{
			t3.start_timer("3021", std::chrono::milliseconds(10), []() {});
			t3.start_timer("3022", std::chrono::milliseconds(10), std::chrono::milliseconds(10), []() {});
			t3.start_timer("3023", std::chrono::milliseconds(10), 3, []() {});
			t3.start_timer("3024", std::chrono::milliseconds(10), 3, std::chrono::milliseconds(10), []() {});
		});

		std::thread([&t3]()
		{
			t3.start_timer("3031", std::chrono::milliseconds(10), []() {});
			t3.start_timer("3032", std::chrono::milliseconds(10), std::chrono::milliseconds(10), []() {});
			t3.start_timer("3033", std::chrono::milliseconds(10), 3, []() {});
			t3.start_timer("3034", std::chrono::milliseconds(10), 3, std::chrono::milliseconds(10), []() {});
			t3.post([&]()
			{
				t3.start_timer("3041", std::chrono::milliseconds(10), []() {});
				t3.start_timer("3042", std::chrono::milliseconds(10), std::chrono::milliseconds(10), []() {});
				t3.start_timer("3043", std::chrono::milliseconds(10), 3, []() {});
				t3.start_timer("3044", std::chrono::milliseconds(10), 3, std::chrono::milliseconds(10), []() {});
			});
		}).join();

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

		threads.emplace_back(std::make_shared<std::thread>([ptr]() mutable
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(130 + std::rand() % 2));
			ptr->notify();
		}));

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

		threads.emplace_back(std::make_shared<std::thread>([ptr]() mutable
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(130 + std::rand() % 2));
			ptr->notify();
		}));

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
		}).bind_start([&]()
		{
			ASIO2_ASSERT(iopool.running_in_thread(0));

			printf("start tcp server : %s %u %d %s\n",
				server.listen_address().c_str(), server.listen_port(),
				asio2::last_error_val(), asio2::last_error_msg().c_str());
		}).bind_stop([&]()
		{
			ASIO2_ASSERT(iopool.running_in_thread(0));

			printf("stop : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
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

		client.bind_connect([&]()
		{
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

		}).bind_disconnect([&]()
		{
			ASIO2_ASSERT(iopool.running_in_thread(index));

			printf("disconnect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		}).bind_recv([&](std::string_view sv)
		{
			ASIO2_ASSERT(iopool.running_in_thread(index));
			asio2::ignore_unused(sv);
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

		std::this_thread::sleep_for(std::chrono::milliseconds(20 + std::rand() % 80));

		client.stop();

		server.stop();

		//timer2.stop_all_timers();

		// before call iopool.stop, all client's and server's and timer's stop must be
		// called alerady, othwise the iopool.stop will blocked forever.
		iopool.stop();


		// iopool must start first, othwise the server.start will blocked forever.
		iopool.start();

		b1 = -1; b2 = -1; b3 = -1; b4 = -1;

		timer.stop_timer(1);
		timer.start_timer(1, std::chrono::milliseconds(100), [&]()
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
		timer.start_timer(1, std::chrono::milliseconds(100), [&]()
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
			timer.start_timer(1, std::chrono::milliseconds(100), [&]()
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
			timer.start_timer(1, std::chrono::milliseconds(100), [&]()
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
			std::this_thread::sleep_for(std::chrono::milliseconds(1));

		t1.post([&]()
		{
			t1.start_timer(1, std::chrono::seconds(1), []() {});
		});

		//while (std::getchar() != '\n');

	// --------------------------------------------------------------------------------
	// user io pool
	// --------------------------------------------------------------------------------

	// use a infinite loop for test


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

		ptr = udp.post_event([]()
		{
			std::ignore = true;
		});

		udp.post([]()
		{
			std::ignore = true;
		}, std::chrono::milliseconds(std::rand() % 50));

		threads.emplace_back(std::make_shared<std::thread>([ptr]() mutable
		{std::this_thread::sleep_for(std::chrono::milliseconds(130 + std::rand() % 10));
		ptr->notify();
		}));

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

		threads.emplace_back(std::make_shared<std::thread>([ptr]() mutable
		{std::this_thread::sleep_for(std::chrono::milliseconds(130 + std::rand() % 10));
		ptr->notify();
		}));

		ptr.reset();

		ptr = udp.post_event([]()
		{
			std::ignore = true;
		});

		//-----------------------------------------------------------------------------------


		// the server's io_context must be the user passed io_context.
		for (std::size_t i = 0; i < server.iopool().size(); i++)
		{
			asio::io_context& ioc1 = iopool.get(i).context();
			asio::io_context& ioc2 = server.iopool().get(i).context();

			ASIO2_ASSERT(&ioc1 == &ioc2);
		}

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
		}).bind_start([&]()
		{
			ASIO2_ASSERT(iopool.running_in_thread(0));

			printf("start tcp server : %s %u %d %s\n",
				server.listen_address().c_str(), server.listen_port(),
				asio2::last_error_val(), asio2::last_error_msg().c_str());
		}).bind_stop([&]()
		{
			ASIO2_ASSERT(iopool.running_in_thread(0));

			printf("stop : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		});

		server.start("0.0.0.0", port);

		//-----------------------------------------------------------------------------------

		timer2.start_timer(1, std::chrono::seconds(3), [&]()
		{
			ASIO2_ASSERT(iopool.running_in_thread(index));
		});

		client.auto_reconnect(true, std::chrono::milliseconds(1000)); // enable auto reconnect and use custom delay

		client.start_timer(1, std::chrono::seconds(1), []() {}); // test timer

		client.bind_connect([&]()
		{
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

		}).bind_disconnect([&]()
		{
			ASIO2_ASSERT(iopool.running_in_thread(index));

			printf("disconnect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		}).bind_recv([&](std::string_view sv)
		{
			ASIO2_ASSERT(iopool.running_in_thread(index));
			asio2::ignore_unused(sv);
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

		std::this_thread::sleep_for(std::chrono::milliseconds(20 + std::rand() % 80));

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

			b1 = -1; b2 = -1; b3 = -1; b4 = -1;

			timer.stop_timer(1);
			timer.start_timer(1, std::chrono::milliseconds(100), [&]()
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
			timer.start_timer(1, std::chrono::milliseconds(100), [&]()
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
				timer.start_timer(1, std::chrono::milliseconds(100), [&]()
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
				timer.start_timer(1, std::chrono::milliseconds(100), [&]()
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

			asio2::udp_cast ucast(iopool.get(std::rand() % iopool.size()));

			ucast.start_timer(1, std::chrono::seconds(1), []()
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

			ptr = ucast.post_event([]()
			{
				std::ignore = true;
			});

			ucast.post([]()
			{
				std::ignore = true;
			}, std::chrono::milliseconds(std::rand() % 50));

			threads.emplace_back(std::make_shared<std::thread>([ptr]() mutable
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(130 + std::rand() % 2));
				ptr->notify();
			}));

			std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 2));

			ptr->notify();

			ucast.start("0.0.0.0", 4444);

			std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 10));

			ucast.stop();

			ucast.start_timer(1, std::chrono::seconds(1), []()
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

			ucast.post([]()
			{
				std::ignore = true;
			}, std::chrono::milliseconds(std::rand() % 50));

			ptr = ucast.post_event([]()
			{
				std::ignore = true;
			});

			threads.emplace_back(std::make_shared<std::thread>([ptr]() mutable
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(130 + std::rand() % 2));
				ptr->notify();
			}));

			ptr.reset();

			ptr = ucast.post_event([]()
			{
				std::ignore = true;
			});

			//-----------------------------------------------------------------------------------

			asio2::tcp_server server(std::vector<asio2::io_t*>{
				&iopool.get(0), & iopool.get(1), & iopool.get(2), & iopool.get(3) });

			index = 0;

			// the server's io_context must be the user passed io_context.
			for (std::size_t i = 0; i < server.iopool().size(); i++)
			{
				asio::io_context& ioc1 = iopool.get(i).context();
				asio::io_context& ioc2 = server.iopool().get(i).context();

				ASIO2_ASSERT(&ioc1 == &ioc2);
			}

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
			}).bind_start([&]()
			{
				ASIO2_ASSERT(iopool.running_in_thread(0));

				printf("start tcp server : %s %u %d %s\n",
					server.listen_address().c_str(), server.listen_port(),
					asio2::last_error_val(), asio2::last_error_msg().c_str());
			}).bind_stop([&]()
			{
				ASIO2_ASSERT(iopool.running_in_thread(0));

				printf("stop : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
			});

			server.start("0.0.0.0", port);

			//-----------------------------------------------------------------------------------

			index = std::rand() % iopool.size();

			asio2::timer timer23(&iopool.get(index));

			timer23.start_timer(1, std::chrono::seconds(3), [&]()
			{
				ASIO2_ASSERT(iopool.running_in_thread(index));
			});

			asio2::tcp_client client(iopool.get(index));

			client.auto_reconnect(true, std::chrono::milliseconds(1000)); // enable auto reconnect and use custom delay

			client.start_timer(1, std::chrono::seconds(1), []() {}); // test timer

			client.bind_connect([&]()
			{
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

			}).bind_disconnect([&]()
			{
				ASIO2_ASSERT(iopool.running_in_thread(index));

				printf("disconnect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
			}).bind_recv([&](std::string_view sv)
			{
				ASIO2_ASSERT(iopool.running_in_thread(index));
				asio2::ignore_unused(sv);
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

			std::this_thread::sleep_for(std::chrono::milliseconds(20 + std::rand() % 80));

			client.stop();

			server.stop();

			//timer23.stop_all_timers();

			// before call iopool.stop, all client's and server's and timer's stop must be
			// called alerady, othwise the iopool.stop will blocked forever.
			iopool.stop();
		}

		for (auto& thread : threads)
		{
			thread->join();
			//thread->detach(); // will cause crash
		}

		threads.clear();
	}

	return 0;
}
