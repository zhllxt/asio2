#include "unit_test.hpp"
#include <iostream>
#include <asio2/external/asio.hpp>
#include <fmt/format.h>

struct userinfo
{
	int id;
	char name[20];
	int8_t age;
};

#ifdef ASIO_STANDALONE
namespace asio
#else
namespace boost::asio
#endif
{
	inline asio::const_buffer buffer(const userinfo& u) noexcept
	{
		return asio::const_buffer(&u, sizeof(userinfo));
	}
}

#include <asio2/tcp/tcp_server.hpp>
#include <asio2/tcp/tcp_client.hpp>

static std::string_view chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

void tcp_general_test()
{
	ASIO2_TEST_BEGIN_LOOP(test_loop_times);

	{
		asio2::tcp_server server;

		std::atomic<int> server_recv_counter = 0;
		std::atomic<std::size_t> server_recv_size = 0;
		server.bind_recv([&](std::shared_ptr<asio2::tcp_session> & session_ptr, std::string_view data)
		{
			server_recv_counter++;
			if (server.iopool().size() > 1)
			{
				ASIO2_CHECK(std::addressof(session_ptr->io()) != std::addressof(server.io()));
			}
			server_recv_size += data.size();

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(session_ptr->is_started());

			session_ptr->async_send(data);

			ASIO2_CHECK(session_ptr->io().running_in_this_thread());
		});
		std::atomic<int> server_accept_counter = 0;
		server.bind_accept([&](auto & session_ptr)
		{
			if (!asio2::get_last_error())
			{
				session_ptr->no_delay(true);

				server_accept_counter++;

				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
				ASIO2_CHECK(server.get_listen_port() == 18028);
				ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_port() == 18028);
				ASIO2_CHECK(server.io().running_in_this_thread());
				ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());

				//// You can close the connection directly here.
				//if (session_ptr->remote_address() == "192.168.0.254")
				//	session_ptr->stop();
			}
		});
		std::atomic<int> server_connect_counter = 0;
		server.bind_connect([&](auto & session_ptr)
		{
			server_connect_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
			ASIO2_CHECK(session_ptr->is_keep_alive());
			ASIO2_CHECK(session_ptr->is_no_delay());
		});
		std::atomic<int> server_disconnect_counter = 0;
		server.bind_disconnect([&](auto & session_ptr)
		{
			server_disconnect_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			// after test, on linux, when disconnect is called, and there has some data 
			// is transmiting(by async_send), the remote_address maybe empty.
			ASIO2_CHECK(session_ptr->socket().is_open());
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_init_counter = 0;
		server.bind_init([&]()
		{
			server_init_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());

			asio::socket_base::reuse_address option;
			server.acceptor().get_option(option);
			ASIO2_CHECK(option.value());
		});
		std::atomic<int> server_start_counter = 0;
		server.bind_start([&]()
		{
			server_start_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});

		ASIO2_CHECK(server.find_session(0) == nullptr);
		ASIO2_CHECK(server.find_session_if([](std::shared_ptr<asio2::tcp_session>& session_ptr)
		{
			return session_ptr->get_remote_port() == 0;
		}) == nullptr);
		server.foreach_session([](std::shared_ptr<asio2::tcp_session>&) {});
		ASIO2_CHECK(server.get_session_count() == 0);

		bool server_start_ret = server.start("127.0.0.1", 18028);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		asio2::tcp_client client;

		// disable auto reconnect, default reconnect option is "enable"
		//client.set_auto_reconnect(false);

		// enable auto reconnect and use custom delay, default delay is 1 seconds
		client.set_auto_reconnect(true, std::chrono::milliseconds(2000));
		ASIO2_CHECK(client.is_auto_reconnect());
		ASIO2_CHECK(client.get_auto_reconnect_delay() == std::chrono::milliseconds(2000));

		std::atomic<std::size_t> client_send_size = 0;
		std::atomic<std::size_t> client_recv_size = 0;
		std::atomic<int> client_init_counter = 0;
		client.bind_init([&]()
		{
			client_init_counter++;

			client.set_no_delay(true);

			ASIO2_CHECK(client.io().running_in_this_thread());
			ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(client.is_keep_alive());
			ASIO2_CHECK(client.is_reuse_address());
			ASIO2_CHECK(client.is_no_delay());

			ASIO2_CHECK(client.get_sndbuf_size() > 0); // just used for test "const function"
			ASIO2_CHECK(client.get_rcvbuf_size() > 0); // just used for test "const function"
			ASIO2_CHECK(int(client.get_linger().enabled()) != 100); // just used for test "const function"
		});
		std::atomic<int> client_connect_counter = 0;
		client.bind_connect([&]()
		{
			ASIO2_CHECK(client.io().running_in_this_thread());
			ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(client.get_local_address() == "127.0.0.1");
			ASIO2_CHECK(client.get_remote_address() == "127.0.0.1");
			ASIO2_CHECK(client.get_remote_port() == 18028);

			client_connect_counter++;

			const char * p1 = "abc";
			char buf[10] = "1234";
			char * p2 = buf;

			client_send_size += std::strlen(p1);
			client.async_send(p1, [p1](std::size_t bytes)
			{
				ASIO2_CHECK(bytes == 3);
				ASIO2_CHECK(bytes == std::strlen(p1));
				ASIO2_CHECK(!asio2::get_last_error());
			});

			client_send_size += std::strlen(buf);
			client.async_send(buf, [buf](std::size_t bytes)
			{
				ASIO2_CHECK(bytes == 4);
				ASIO2_CHECK(bytes == std::strlen(buf));
				ASIO2_CHECK(!asio2::get_last_error());
			});

			client_send_size += std::strlen(p2);
			client.async_send(p2, [p2, buf](std::size_t bytes) mutable
			{
				p2 = buf;
				ASIO2_CHECK(bytes == 4);
				ASIO2_CHECK(bytes == std::strlen(p2));
				ASIO2_CHECK(!asio2::get_last_error());
			});

			client_send_size += std::strlen("<abc>");
			client.async_send("<abc>", [](std::size_t bytes)
			{
				ASIO2_CHECK(bytes == 5);
				ASIO2_CHECK(bytes == std::strlen("<abc>"));
				ASIO2_CHECK(!asio2::get_last_error());
			});

			std::string str;
			str += '<';
			int len = 128 + std::rand() % (300);
			for (int i = 0; i < len; i++)
			{
				str += (char)((std::rand() % 26) + 'a');
			}
			str += '>';

			client_send_size += str.size();
			client.async_send(str);

			client_send_size += str.size();
			client.async_send(str, [str](std::size_t bytes)
			{
				ASIO2_CHECK(bytes == str.size());
				ASIO2_CHECK(!asio2::get_last_error());
			});

			client_send_size += 10;
			client.async_send((uint8_t*)"<abcdefghijklmnopqrstovuxyz0123456789>", 10, [](std::size_t bytes)
			{
				ASIO2_CHECK(bytes == 10);
				ASIO2_CHECK(!asio2::get_last_error());
			});
		
			client_send_size += 10;
			client.async_send((const uint8_t*)"<abcdefghijklmnopqrstovuxyz0123456789>", 10, [](std::size_t bytes)
			{
				ASIO2_CHECK(bytes == 10);
				ASIO2_CHECK(!asio2::get_last_error());
			});
			
			client_send_size += str.size();
			client.async_send(str.data(), int(str.size()));
			
			client_send_size += str.size();
			client.async_send(str.data(), []() { ASIO2_CHECK(!asio2::get_last_error()); });
			
			client_send_size += str.size();
			client.async_send(str.c_str(), size_t(str.size()));
			
			client_send_size += str.size();
			client.async_send(str);
			
			int narys[2] = { 1,2 };
			
			client_send_size += 2 * sizeof(int);
			client.async_send(narys);
			
			client_send_size += 2 * sizeof(int);
			client.async_send(narys, []() { ASIO2_CHECK(!asio2::get_last_error()); });
			
			client_send_size += 2 * sizeof(int);
			client.async_send(narys, [](std::size_t bytes)
			{
				ASIO2_CHECK(bytes == 2 * sizeof(int));
				ASIO2_CHECK(!asio2::get_last_error());
			});

			std::vector<uint8_t> vec;
			len = 128 + std::rand() % (300);
			for (int i = 0; i < len; i++)
			{
				vec.emplace_back((uint8_t)((std::rand() % 255)));
			}

			client_send_size += vec.size();
			client.async_send(vec, [vec](std::size_t bytes)
			{
				ASIO2_CHECK(bytes == vec.size());
				ASIO2_CHECK(!asio2::get_last_error());
			});

			std::array<uint8_t, 99> ary;

			client_send_size += ary.size();
			client.async_send(ary, [ary](std::size_t bytes)
			{
				ASIO2_CHECK(bytes == ary.size());
				ASIO2_CHECK(!asio2::get_last_error());
			});

			const char * msg = "<abcdefghijklmnopqrstovuxyz0123456789>";
			asio::const_buffer buffer = asio::buffer(msg);
			client_send_size += buffer.size();
			client.async_send(buffer, [buffer](std::size_t bytes)
			{
				ASIO2_CHECK(bytes == buffer.size());
				ASIO2_CHECK(!asio2::get_last_error());
			});

			userinfo u;
			u.id = 11;
			memset(u.name, 0, sizeof(u.name));
			memcpy(u.name, "abc", 3);
			u.age = 20;
			client_send_size += sizeof(u);
			client.async_send(u, [](std::size_t bytes)
			{
				ASIO2_CHECK(bytes == sizeof(userinfo));
				ASIO2_CHECK(!asio2::get_last_error());
			});

			double scores[5] = { 0.1,0.2,0.3 };
			client_send_size += sizeof(double) * 3;
			client.async_send(scores, 3, [](std::size_t bytes)
			{
				ASIO2_CHECK(bytes == sizeof(double) * 3);
				ASIO2_CHECK(!asio2::get_last_error());
			});

			std::size_t sent_bytes;

			sent_bytes = client.send(p1);
			ASIO2_CHECK(sent_bytes == 0);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
			client_send_size += std::strlen(p1);

			sent_bytes = client.send(buf);
			ASIO2_CHECK(sent_bytes == 0);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
			client_send_size += std::strlen(buf);

			sent_bytes = client.send(p2);
			ASIO2_CHECK(sent_bytes == 0);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
			client_send_size += std::strlen(p2);

			sent_bytes = client.send("<abcdefghijklmnopqrstovuxyz0123456789>");
			ASIO2_CHECK(sent_bytes == 0);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
			client_send_size += std::strlen("<abcdefghijklmnopqrstovuxyz0123456789>");

			sent_bytes = client.send(str);
			ASIO2_CHECK(sent_bytes == 0);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
			client_send_size += str.size();

			sent_bytes = client.send((uint8_t*)"<abcdefghijklmnopqrstovuxyz0123456789>", 10);
			ASIO2_CHECK(sent_bytes == 0);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
			client_send_size += 10;
		
			sent_bytes = client.send((const uint8_t*)"<abcdefghijklmnopqrstovuxyz0123456789>", 10);
			ASIO2_CHECK(sent_bytes == 0);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
			client_send_size += 10;
			
			sent_bytes = client.send(str.data(), int(str.size()));
			ASIO2_CHECK(sent_bytes == 0);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
			client_send_size += str.size();
			
			sent_bytes = client.send(str.data());
			ASIO2_CHECK(sent_bytes == 0);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
			client_send_size += str.size();
			
			sent_bytes = client.send(str.c_str(), size_t(str.size()));
			ASIO2_CHECK(sent_bytes == 0);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
			client_send_size += str.size();
			
			sent_bytes = client.send(str);
			ASIO2_CHECK(sent_bytes == 0);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
			client_send_size += str.size();
			
			sent_bytes = client.send(narys);
			ASIO2_CHECK(sent_bytes == 0);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
			client_send_size += 2 * sizeof(int);

			sent_bytes = client.send(vec);
			ASIO2_CHECK(sent_bytes == 0);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
			client_send_size += vec.size();

			sent_bytes = client.send(ary);
			ASIO2_CHECK(sent_bytes == 0);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
			client_send_size += ary.size();

			sent_bytes = client.send(buffer);
			ASIO2_CHECK(sent_bytes == 0);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
			client_send_size += buffer.size();

			sent_bytes = client.send(u);
			ASIO2_CHECK(sent_bytes == 0);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
			client_send_size += sizeof(u);

			sent_bytes = client.send(scores, 3);
			ASIO2_CHECK(sent_bytes == 0);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
			client_send_size += sizeof(double) * 3;
		});
		std::atomic<int> client_disconnect_counter = 0;
		client.bind_disconnect([&]()
		{
			client_disconnect_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(client.io().running_in_this_thread());
			ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
		});
		client.bind_recv([&](std::string_view data)
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(client.io().running_in_this_thread());
			ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
			ASIO2_CHECK(client.is_started());
			client_recv_size += data.size();
		});

		bool client_start_ret = client.start("127.0.0.1", 18028);

		ASIO2_CHECK(client_start_ret);
		ASIO2_CHECK(client.is_started());

		ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == 1);
		ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == 1);

		// use this to ensure the ASIO2_CHECK(server.get_session_count() == std::size_t(1));
		while (server_recv_counter < 1)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK(server.get_session_count() == std::size_t(1));

		ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == 1);
		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == 1);

		//-----------------------------------------------------------------------------------------
		const char * p1 = "defg";
		char buf[10] = "123456";
		char * p2 = buf;

		client_send_size += std::strlen(p1);
		client.async_send(p1, [p1](std::size_t bytes)
		{
			ASIO2_CHECK(bytes == 4);
			ASIO2_CHECK(bytes == std::strlen(p1));
			ASIO2_CHECK(!asio2::get_last_error());
		});

		client_send_size += std::strlen(buf);
		client.async_send(buf, [buf](std::size_t bytes)
		{
			ASIO2_CHECK(bytes == 6);
			ASIO2_CHECK(bytes == std::strlen(buf));
			ASIO2_CHECK(!asio2::get_last_error());
		});

		client_send_size += std::strlen(p2);
		client.async_send(p2, [p2](std::size_t bytes)
		{
			ASIO2_CHECK(bytes == 6);
			ASIO2_CHECK(bytes == std::strlen(p2));
			ASIO2_CHECK(!asio2::get_last_error());
		});

		client_send_size += std::strlen("<123>");
		client.async_send("<123>", [](std::size_t bytes)
		{
			ASIO2_CHECK(bytes == 5);
			ASIO2_CHECK(bytes == std::strlen("<123>"));
			ASIO2_CHECK(!asio2::get_last_error());
		});

		std::string str;
		str += '<';
		int len = 128 + std::rand() % (300);
		for (int i = 0; i < len; i++)
		{
			str += (char)((std::rand() % 26) + 'a');
		}
		str += '>';

		client_send_size += str.size();
		client.async_send(str);

		client_send_size += str.size();
		client.async_send(str, [str](std::size_t bytes)
		{
			ASIO2_CHECK(bytes == str.size());
			ASIO2_CHECK(!asio2::get_last_error());
		});

		client_send_size += 10;
		client.async_send((uint8_t*)"<abcdefghijklmnopqrstovuxyz0123456789>", 10);
		
		client_send_size += 10;
		client.async_send((const uint8_t*)"<abcdefghijklmnopqrstovuxyz0123456789>", 10);
			
		client_send_size += str.size();
		client.async_send(str.data(), int(str.size()));
			
		client_send_size += str.size();
		client.async_send(str.data(), []() { ASIO2_CHECK(!asio2::get_last_error()); });
			
		client_send_size += str.size();
		client.async_send(str.c_str(), size_t(str.size()));
			
		client_send_size += str.size();
		client.async_send(str);
			
		int narys[2] = { 1,2 };
			
		client_send_size += 2 * sizeof(int);
		client.async_send(narys);
			
		client_send_size += 2 * sizeof(int);
		client.async_send(narys, []() { ASIO2_CHECK(!asio2::get_last_error()); });
			
		client_send_size += 2 * sizeof(int);
		client.async_send(narys, [](std::size_t bytes)
		{
			ASIO2_CHECK(bytes == 2 * sizeof(int));
			ASIO2_CHECK(!asio2::get_last_error());
		});

		std::vector<uint8_t> vec;
		len = 128 + std::rand() % (300);
		for (int i = 0; i < len; i++)
		{
			vec.emplace_back((uint8_t)((std::rand() % 255)));
		}

		client_send_size += vec.size();
		client.async_send(vec, [vec](std::size_t bytes)
		{
			ASIO2_CHECK(bytes == vec.size());
			ASIO2_CHECK(!asio2::get_last_error());
		});

		std::array<uint8_t, 99> ary;

		client_send_size += ary.size();
		client.async_send(ary, [ary](std::size_t bytes)
		{
			ASIO2_CHECK(bytes == ary.size());
			ASIO2_CHECK(!asio2::get_last_error());
		});

		const char * msg = "<abcdefghijklmnopqrstovuxyz0123456789>";
		asio::const_buffer buffer = asio::buffer(msg);
		client_send_size += buffer.size();
		client.async_send(buffer, [buffer](std::size_t bytes)
		{
			ASIO2_CHECK(bytes == buffer.size());
			ASIO2_CHECK(!asio2::get_last_error());
		});

		userinfo u;
		u.id = 11;
		memset(u.name, 0, sizeof(u.name));
		memcpy(u.name, "abc", 3);
		u.age = 20;
		client_send_size += sizeof(u);
		client.async_send(u, [](std::size_t bytes)
		{
			ASIO2_CHECK(bytes == sizeof(userinfo));
			ASIO2_CHECK(!asio2::get_last_error());
		});

		double scores[5] = { 0.1,0.2,0.3 };
		client_send_size += sizeof(double) * 3;
		client.async_send(scores, 3, [](std::size_t bytes)
		{
			ASIO2_CHECK(bytes == sizeof(double) * 3);
			ASIO2_CHECK(!asio2::get_last_error());
		});

		std::size_t sent_bytes;

		sent_bytes = client.send(p1);
		ASIO2_CHECK(sent_bytes == std::strlen(p1));
		ASIO2_CHECK(sent_bytes == 4);
		ASIO2_CHECK(!asio2::get_last_error());
		client_send_size += std::strlen(p1);

		sent_bytes = client.send(buf);
		ASIO2_CHECK(sent_bytes == std::strlen(buf));
		ASIO2_CHECK(sent_bytes == 6);
		ASIO2_CHECK(!asio2::get_last_error());
		client_send_size += std::strlen(buf);

		sent_bytes = client.send(p2);
		ASIO2_CHECK(sent_bytes == std::strlen(p2));
		ASIO2_CHECK(sent_bytes == 6);
		ASIO2_CHECK(!asio2::get_last_error());
		client_send_size += std::strlen(p2);

		sent_bytes = client.send("<456>");
		ASIO2_CHECK(sent_bytes == std::strlen("<456>"));
		ASIO2_CHECK(sent_bytes == 5);
		ASIO2_CHECK(!asio2::get_last_error());
		client_send_size += std::strlen("<456>");

		sent_bytes = client.send(str);
		ASIO2_CHECK(sent_bytes == str.size());
		ASIO2_CHECK(!asio2::get_last_error());
		client_send_size += str.size();

		sent_bytes = client.send((uint8_t*)"<abcdefghijklmnopqrstovuxyz0123456789>", 10);
		ASIO2_CHECK(sent_bytes == 10);
		ASIO2_CHECK(!asio2::get_last_error());
		client_send_size += 10;
		
		sent_bytes = client.send((const uint8_t*)"<abcdefghijklmnopqrstovuxyz0123456789>", 10);
		ASIO2_CHECK(sent_bytes == 10);
		ASIO2_CHECK(!asio2::get_last_error());
		client_send_size += 10;
			
		sent_bytes = client.send(str.data(), int(str.size()));
		ASIO2_CHECK(sent_bytes == str.size());
		ASIO2_CHECK(!asio2::get_last_error());
		client_send_size += str.size();
			
		sent_bytes = client.send(str.data());
		ASIO2_CHECK(sent_bytes == str.size());
		ASIO2_CHECK(!asio2::get_last_error());
		client_send_size += str.size();
			
		sent_bytes = client.send(str.c_str(), size_t(str.size()));
		ASIO2_CHECK(sent_bytes == str.size());
		ASIO2_CHECK(!asio2::get_last_error());
		client_send_size += str.size();
			
		sent_bytes = client.send(str);
		ASIO2_CHECK(sent_bytes == str.size());
		ASIO2_CHECK(!asio2::get_last_error());
		client_send_size += str.size();
			
		sent_bytes = client.send(narys);
		ASIO2_CHECK(sent_bytes == 2 * sizeof(int));
		ASIO2_CHECK(!asio2::get_last_error());
		client_send_size += 2 * sizeof(int);

		sent_bytes = client.send(vec);
		ASIO2_CHECK(sent_bytes == vec.size());
		ASIO2_CHECK(!asio2::get_last_error());
		client_send_size += vec.size();

		sent_bytes = client.send(ary);
		ASIO2_CHECK(sent_bytes == ary.size());
		ASIO2_CHECK(!asio2::get_last_error());
		client_send_size += ary.size();

		sent_bytes = client.send(buffer);
		ASIO2_CHECK(sent_bytes == buffer.size());
		ASIO2_CHECK(!asio2::get_last_error());
		client_send_size += buffer.size();

		sent_bytes = client.send(u);
		ASIO2_CHECK(sent_bytes == sizeof(u));
		ASIO2_CHECK(!asio2::get_last_error());
		client_send_size += sizeof(u);

		sent_bytes = client.send(scores, 3);
		ASIO2_CHECK(sent_bytes == sizeof(double) * 3);
		ASIO2_CHECK(!asio2::get_last_error());
		client_send_size += sizeof(double) * 3;

		client_send_size += str.size();
		client.async_send(str, asio::use_future);

		client_send_size += 5;
		client.async_send("<789>", asio::use_future);

		client_send_size += str.size();
		client.async_send(str.data(), asio::use_future);

		client_send_size += str.size();
		client.async_send(str.data(), str.size(), asio::use_future);

		client_send_size += str.size();
		client.async_send(str.c_str(), asio::use_future);

		client_send_size += str.size();
		client.async_send(str.c_str(), str.size(), asio::use_future);

		client_send_size += vec.size();
		client.async_send(vec, asio::use_future);

		client_send_size += ary.size();
		client.async_send(ary, asio::use_future);

		client_send_size += 2 * sizeof(int);
		auto future1 = client.async_send(narys, asio::use_future);
		auto[ec1, bytes1] = future1.get();
		ASIO2_CHECK(!ec1);
		ASIO2_CHECK(bytes1 == 2 * sizeof(int));

		client_send_size += 5;
		auto future2 = client.async_send("0123456789", 5, asio::use_future);
		auto[ec2, bytes2] = future2.get();
		ASIO2_CHECK(!ec2);
		ASIO2_CHECK(bytes2 == 5);

		while (server_recv_size != client_send_size)
		{
			ASIO2_TEST_WAIT_CHECK();
		}
		while (server_recv_size != client_recv_size)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK(server_recv_size == client_send_size);
		ASIO2_CHECK(server_recv_size == client_recv_size);

		client.stop();
		ASIO2_CHECK(client.is_stopped());

		ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == 1);

		// use this to ensure the ASIO2_CHECK(session_ptr->is_started());
		while (server_disconnect_counter != 1)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		server.stop();
		ASIO2_CHECK(server.is_stopped());

		ASIO2_CHECK_VALUE(server_disconnect_counter.load(), server_disconnect_counter == 1);
		ASIO2_CHECK_VALUE(server_stop_counter      .load(), server_stop_counter       == 1);
	}

	{
		asio2::tcp_server server;

		std::atomic<int> server_recv_counter = 0;
		server.bind_recv([&](std::shared_ptr<asio2::tcp_session> & session_ptr, std::string_view data)
		{
			server_recv_counter++;
			if (server.iopool().size() > 1)
			{
				ASIO2_CHECK(std::addressof(session_ptr->io()) != std::addressof(server.io()));
			}
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(session_ptr->is_started());
			session_ptr->async_send(data);

			ASIO2_CHECK(session_ptr->io().running_in_this_thread());
		});
		std::atomic<int> server_accept_counter = 0;
		server.bind_accept([&](auto & session_ptr)
		{
			if (!asio2::get_last_error())
			{
				session_ptr->no_delay(true);

				server_accept_counter++;

				ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
				ASIO2_CHECK(server.get_listen_port() == 18028);
				ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_port() == 18028);
				ASIO2_CHECK(server.io().running_in_this_thread());
				ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());

				//// You can close the connection directly here.
				//if (session_ptr->remote_address() == "192.168.0.254")
				//	session_ptr->stop();
			}
		});
		std::atomic<int> server_connect_counter = 0;
		server.bind_connect([&](auto & session_ptr)
		{
			server_connect_counter++;

			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
			ASIO2_CHECK(session_ptr->is_keep_alive());
			ASIO2_CHECK(session_ptr->is_no_delay());
		});
		std::atomic<int> server_disconnect_counter = 0;
		server.bind_disconnect([&](auto & session_ptr)
		{
			server_disconnect_counter++;
			ASIO2_CHECK(session_ptr->socket().is_open());
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_init_counter = 0;
		server.bind_init([&]()
		{
			server_init_counter++;
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());

			asio::socket_base::reuse_address option;
			server.acceptor().get_option(option);
			ASIO2_CHECK(option.value());
		});
		std::atomic<int> server_start_counter = 0;
		server.bind_start([&]()
		{
			server_start_counter++;
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});

		bool server_start_ret = server.start("127.0.0.1", 18028);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		asio2::tcp_client client;

		// disable auto reconnect, default reconnect option is "enable"
		//client.set_auto_reconnect(false);

		// enable auto reconnect and use custom delay, default delay is 1 seconds
		client.set_auto_reconnect(true, std::chrono::milliseconds(2000));

		std::atomic<int> client_init_counter = 0;
		client.bind_init([&]()
		{
			client_init_counter++;

			client.set_no_delay(true);

			ASIO2_CHECK(client.io().running_in_this_thread());
			ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(client.is_keep_alive());
			ASIO2_CHECK(client.is_reuse_address());
			ASIO2_CHECK(client.is_no_delay());
		});
		std::atomic<int> client_connect_counter = 0;
		client.bind_connect([&]()
		{
			ASIO2_CHECK(client.io().running_in_this_thread());
			ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(client.get_local_address() == "127.0.0.1");
			ASIO2_CHECK(client.get_remote_address() == "127.0.0.1");
			ASIO2_CHECK(client.get_remote_port() == 18028);

			client_connect_counter++;

			client.async_send("<abcdefghijklmnopqrstovuxyz0123456789>");
		});
		std::atomic<int> client_disconnect_counter = 0;
		client.bind_disconnect([&]()
		{
			client_disconnect_counter++;
			ASIO2_CHECK(client.io().running_in_this_thread());
			ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
		});
		client.bind_recv([&](std::string_view data)
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(client.io().running_in_this_thread());
			ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
			ASIO2_CHECK(client.is_started());
			client.async_send(data);
		});

		bool client_start_ret = client.start("127.0.0.1", 18028);

		ASIO2_CHECK(client_start_ret);
		ASIO2_CHECK(client.is_started());

		ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == 1);
		ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == 1);

		std::string str = "[abcdefghijklmnopqrstovuxyz0123456789]";

		if (client.is_started())
		{
			// ## All of the following ways of send operation are correct.
			client.async_send(str, [str](std::size_t sent_bytes)
			{
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(sent_bytes == str.size());
			});

			client.async_send(str.data(), str.size() / 2, [str](std::size_t sent_bytes)
			{
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(sent_bytes == str.size() / 2);
			});

			int intarray[2] = { 1, 2 };

			// callback with no params
			client.async_send(intarray, [](std::size_t sent_bytes)
			{
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(sent_bytes == 2 * sizeof(int));
			});

			// callback with param
			client.async_send(intarray, [](std::size_t sent_bytes)
			{
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(sent_bytes == 2 * sizeof(int));
			});

			// use future to wait util the send is finished.
			std::future<std::pair<asio::error_code, std::size_t>> future =
				client.async_send(str, asio::use_future);
			auto[ec, bytes] = future.get();
			ASIO2_CHECK(!ec);
			ASIO2_CHECK(bytes == str.size());

			// use asio::buffer to avoid memory allocation, the underlying
			// buffer must be persistent, like the static pointer "msg" below
			const char * msg = "<abcdefghijklmnopqrstovuxyz0123456789>";
			asio::const_buffer buffer = asio::buffer(msg);
			client.async_send(buffer, [msg](std::size_t sent_bytes)
			{
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(sent_bytes == std::strlen(msg));
			});

			// Example for Synchronous send data. The return value is the sent bytes.
			// You can use asio2::get_last_error() to check whether some error occured.
			std::size_t sent_bytes = client.send(str);
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(sent_bytes == str.size());

			// ##Example how to send a struct directly:
			userinfo u;
			u.id = 11;
			memset(u.name, 0, sizeof(u.name));
			memcpy(u.name, "abc", 3);
			u.age = 20;
			client.async_send(u, [](std::size_t sent_bytes)
			{
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(sent_bytes == sizeof(userinfo));
			});

			// send vector, array, .... and others
			std::vector<uint8_t> data{ 1,2,3 };
			sent_bytes = client.send(data);
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(sent_bytes == data.size());
		}

		// use this to ensure the ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == 1);
		while (server_recv_counter < 1)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == 1);
		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == 1);

		client.stop();
		ASIO2_CHECK(client.is_stopped());

		ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == 1);

		// use this to ensure the ASIO2_CHECK(session_ptr->is_started());
		while (server_disconnect_counter != 1)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		server.stop();
		ASIO2_CHECK(server.is_stopped());

		ASIO2_CHECK_VALUE(server_disconnect_counter.load(), server_disconnect_counter == 1);
		ASIO2_CHECK_VALUE(server_stop_counter      .load(), server_stop_counter       == 1);
	}

	{
		asio2::tcp_server server;
		std::size_t session_key = std::size_t(std::rand() % test_client_count);
		std::atomic<int> server_recv_counter = 0;
		server.bind_recv([&](std::shared_ptr<asio2::tcp_session> & session_ptr, std::string_view data)
		{
			server_recv_counter++;
			if (server.iopool().size() > 1)
			{
				ASIO2_CHECK(std::addressof(session_ptr->io()) != std::addressof(server.io()));
			}
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(session_ptr->is_started());
			session_ptr->async_send(data);

			ASIO2_CHECK(session_ptr->io().running_in_this_thread());
		});
		std::atomic<int> server_accept_counter = 0;
		server.bind_accept([&](auto & session_ptr)
		{
			if (!asio2::get_last_error())
			{
				session_ptr->no_delay(true);

				server_accept_counter++;

				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
				ASIO2_CHECK(server.get_listen_port() == 18028);
				ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_port() == 18028);
				ASIO2_CHECK(server.io().running_in_this_thread());
				ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());

				//// You can close the connection directly here.
				//if (session_ptr->remote_address() == "192.168.0.254")
				//	session_ptr->stop();
			}
		});
		std::atomic<int> server_connect_counter = 0;
		server.bind_connect([&](auto & session_ptr)
		{
			if (session_key == std::size_t(server_connect_counter.load()))
				session_key = session_ptr->hash_key();

			server_connect_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(!server.find_session(session_ptr->hash_key()));
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
			ASIO2_CHECK(session_ptr->is_keep_alive());
			ASIO2_CHECK(session_ptr->is_no_delay());
		});
		std::atomic<int> server_disconnect_counter = 0;
		server.bind_disconnect([&](auto & session_ptr)
		{
			server_disconnect_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(!server.find_session(session_ptr->hash_key()));
			ASIO2_CHECK(session_ptr->socket().is_open());
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_init_counter = 0;
		server.bind_init([&]()
		{
			server_init_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());

			asio::socket_base::reuse_address option;
			server.acceptor().get_option(option);
			ASIO2_CHECK(option.value());
		});
		std::atomic<int> server_start_counter = 0;
		server.bind_start([&]()
		{
			server_start_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});

		bool server_start_ret = server.start("127.0.0.1", 18028);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		std::vector<std::shared_ptr<asio2::tcp_client>> clients;
		std::atomic<int> client_init_counter = 0;
		std::atomic<int> client_connect_counter = 0;
		std::atomic<int> client_disconnect_counter = 0;
		for (int i = 0; i < test_client_count; i++)
		{
			auto iter = clients.emplace_back(std::make_shared<asio2::tcp_client>());

			asio2::tcp_client& client = *iter;

			// disable auto reconnect, default reconnect option is "enable"
			//client.set_auto_reconnect(false);

			// enable auto reconnect and use custom delay, default delay is 1 seconds
			client.set_auto_reconnect(true, std::chrono::milliseconds(2000));

			client.bind_init([&]()
			{
				client_init_counter++;

				client.set_no_delay(true);

				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.is_keep_alive());
				ASIO2_CHECK(client.is_reuse_address());
				ASIO2_CHECK(client.is_no_delay());
			});
			client.bind_connect([&]()
			{
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.get_local_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_port() == 18028);

				client_connect_counter++;

				client.async_send("<abcdefghijklmnopqrstovuxyz0123456789>");
			});
			client.bind_disconnect([&]()
			{
				client_disconnect_counter++;

				ASIO2_CHECK(asio2::get_last_error());
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
			});
			client.bind_recv([&]([[maybe_unused]] std::string_view data)
			{
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!data.empty());
				ASIO2_CHECK(client.is_started());
				//client.async_send(data);
			});

			bool client_start_ret = client.async_start("127.0.0.1", 18028);

			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
		}

		while (server.get_session_count() < std::size_t(test_client_count))
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		auto session_count = server.get_session_count();
		ASIO2_CHECK_VALUE(session_count, session_count == std::size_t(test_client_count));

		auto session_ptr1 = server.find_session(session_key);
		ASIO2_CHECK(session_ptr1 != nullptr);
		std::size_t sent_bytes = session_ptr1->send("0123456789");
		ASIO2_CHECK(!asio2::get_last_error());
		ASIO2_CHECK(sent_bytes == std::strlen("0123456789"));

		auto session_ptr2 = server.find_session_if([session_key](std::shared_ptr<asio2::tcp_session>& session_ptr)
		{
			if (session_ptr->hash_key() == session_key)
				return true;
			return false;
		});
		ASIO2_CHECK(session_ptr1.get() == session_ptr2.get());

		bool find_session_key = false;
		server.foreach_session([session_ptr1, session_key, &find_session_key]
		(std::shared_ptr<asio2::tcp_session>& session_ptr) mutable
		{
			if (session_ptr.get() == session_ptr1.get())
			{
				ASIO2_CHECK(session_key == session_ptr->hash_key());
				find_session_key = true;
			}

			std::size_t sent_bytes = session_ptr->send("0123456789");
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(sent_bytes == std::strlen("0123456789"));
		});

		server.async_send(std::string("abcdefxxx0123456"));
		server.async_send("abcdefxxx0123456");
		server.async_send("abcdefxxx0123456", 10);

		ASIO2_CHECK(find_session_key);

		session_ptr1.reset();
		session_ptr2.reset();

		while (client_connect_counter < test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == test_client_count);
		ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == test_client_count);

		ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == test_client_count);
		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->stop();
			ASIO2_CHECK(clients[i]->is_stopped());
		}

		ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == test_client_count);

		// use this to ensure the ASIO2_CHECK(session_ptr->is_started());
		while (server_disconnect_counter != test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		server.stop();
		ASIO2_CHECK(server.is_stopped());

		ASIO2_CHECK_VALUE(server_disconnect_counter.load(), server_disconnect_counter == test_client_count);
		ASIO2_CHECK_VALUE(server_stop_counter      .load(), server_stop_counter       == 1);

		//-----------------------------------------------------------------------------------------

		ASIO2_CHECK_VALUE(server.get_session_count(), server.get_session_count() == std::size_t(0));

		server_init_counter = 0;
		server_start_counter = 0;
		server_disconnect_counter = 0;
		server_stop_counter = 0;
		server_accept_counter = 0;
		server_connect_counter = 0;

		server_start_ret = server.start("127.0.0.1", 18028);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		client_init_counter = 0;
		client_connect_counter = 0;
		client_disconnect_counter = 0;

		for (int i = 0; i < test_client_count; i++)
		{
			bool client_start_ret = clients[i]->async_start("127.0.0.1", 18028);
			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
		}

		while (server.get_session_count() < std::size_t(test_client_count))
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(server.get_session_count(), server.get_session_count() == std::size_t(test_client_count));

		server.async_send(std::string("abcdefxxx0123456"));
		server.async_send("abcdefxxx0123456");
		server.async_send("abcdefxxx0123456", 10);

		while (client_connect_counter < test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == test_client_count);
		ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == test_client_count);

		ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == test_client_count);
		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->stop();
			ASIO2_CHECK(clients[i]->is_stopped());
		}

		ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == test_client_count);

		// use this to ensure the ASIO2_CHECK(session_ptr->is_started());
		while (server_disconnect_counter != test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		server.stop();
		ASIO2_CHECK(server.is_stopped());

		ASIO2_CHECK_VALUE(server_disconnect_counter.load(), server_disconnect_counter == test_client_count);
		ASIO2_CHECK_VALUE(server_stop_counter      .load(), server_stop_counter       == 1);
	}

	{
		asio2::tcp_server server;
		std::size_t session_key = std::size_t(std::rand() % test_client_count);
		std::atomic<int> server_recv_counter = 0;
		server.bind_recv([&](std::shared_ptr<asio2::tcp_session> & session_ptr, std::string_view data)
		{
			server_recv_counter++;
			if (server.iopool().size() > 1)
			{
				ASIO2_CHECK(std::addressof(session_ptr->io()) != std::addressof(server.io()));
			}
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(session_ptr->is_started());
			session_ptr->async_send(data);

			ASIO2_CHECK(session_ptr->io().running_in_this_thread());
		});
		std::atomic<int> server_accept_counter = 0;
		server.bind_accept([&](auto & session_ptr)
		{
			if (!asio2::get_last_error())
			{
				session_ptr->no_delay(true);

				server_accept_counter++;

				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
				ASIO2_CHECK(server.get_listen_port() == 18028);
				ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_port() == 18028);
				ASIO2_CHECK(server.io().running_in_this_thread());
				ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());

				//// You can close the connection directly here.
				//if (session_ptr->remote_address() == "192.168.0.254")
				//	session_ptr->stop();
			}
		});
		std::atomic<int> server_connect_counter = 0;
		server.bind_connect([&](auto & session_ptr)
		{
			if (session_key == std::size_t(server_connect_counter.load()))
				session_key = session_ptr->hash_key();

			server_connect_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(!server.find_session(session_ptr->hash_key()));
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
			ASIO2_CHECK(session_ptr->is_keep_alive());
			ASIO2_CHECK(session_ptr->is_no_delay());
		});
		std::atomic<int> server_disconnect_counter = 0;
		server.bind_disconnect([&](auto & session_ptr)
		{
			server_disconnect_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(!server.find_session(session_ptr->hash_key()));
			ASIO2_CHECK(session_ptr->socket().is_open());
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_init_counter = 0;
		server.bind_init([&]()
		{
			server_init_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());

			asio::socket_base::reuse_address option;
			server.acceptor().get_option(option);
			ASIO2_CHECK(option.value());
		});
		std::atomic<int> server_start_counter = 0;
		server.bind_start([&]()
		{
			server_start_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});

		bool server_start_ret = server.start("127.0.0.1", 18028);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		std::vector<std::shared_ptr<asio2::tcp_client>> clients;
		std::atomic<int> client_init_counter = 0;
		std::atomic<int> client_connect_counter = 0;
		std::atomic<int> client_disconnect_counter = 0;
		std::atomic<int> client_recv_counter = 0;
		for (int i = 0; i < test_client_count; i++)
		{
			auto iter = clients.emplace_back(std::make_shared<asio2::tcp_client>());

			asio2::tcp_client& client = *iter;

			// disable auto reconnect, default reconnect option is "enable"
			//client.set_auto_reconnect(false);

			// enable auto reconnect and use custom delay, default delay is 1 seconds
			client.set_auto_reconnect(true, std::chrono::milliseconds(2000));

			client.bind_init([&]()
			{
				client_init_counter++;

				client.set_no_delay(true);

				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.is_keep_alive());
				ASIO2_CHECK(client.is_reuse_address());
				ASIO2_CHECK(client.is_no_delay());
			});
			client.bind_connect([&]()
			{
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.get_local_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_port() == 18028);

				client_connect_counter++;

			});
			client.bind_disconnect([&]()
			{
				client_disconnect_counter++;

				ASIO2_CHECK(asio2::get_last_error());
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
			});
			client.bind_recv([&]([[maybe_unused]] std::string_view data)
			{
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!data.empty());
				ASIO2_CHECK(client.is_started());
				client_recv_counter++;
			});

			bool client_start_ret = client.async_start("127.0.0.1", 18028);

			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
		}

		while (server.get_session_count() < std::size_t(test_client_count))
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		auto session_count = server.get_session_count();
		ASIO2_CHECK_VALUE(session_count, session_count == std::size_t(test_client_count));

		server.async_send(std::string("abcdefxxx0123456"));

		while (client_recv_counter < test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(client_recv_counter.load(), client_recv_counter >= test_client_count);

		client_recv_counter = 0;

		server.async_send("abcdefxxx0123456");

		while (client_recv_counter < test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(client_recv_counter.load(), client_recv_counter >= test_client_count);

		client_recv_counter = 0;

		server.async_send("abcdefxxx0123456", 10);

		while (client_recv_counter < test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(client_recv_counter.load(), client_recv_counter >= test_client_count);

		ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == test_client_count);
		ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == test_client_count);

		ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == test_client_count);
		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->stop();
			ASIO2_CHECK(clients[i]->is_stopped());
		}

		ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == test_client_count);

		// use this to ensure the ASIO2_CHECK(session_ptr->is_started());
		while (server_disconnect_counter != test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		server.stop();
		ASIO2_CHECK(server.is_stopped());

		ASIO2_CHECK_VALUE(server_disconnect_counter.load(), server_disconnect_counter == test_client_count);
		ASIO2_CHECK_VALUE(server_stop_counter      .load(), server_stop_counter       == 1);

		//-----------------------------------------------------------------------------------------

		ASIO2_CHECK_VALUE(server.get_session_count(), server.get_session_count() == std::size_t(0));

		server_init_counter = 0;
		server_start_counter = 0;
		server_disconnect_counter = 0;
		server_stop_counter = 0;
		server_accept_counter = 0;
		server_connect_counter = 0;

		server_start_ret = server.start("127.0.0.1", 18028);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		client_init_counter = 0;
		client_connect_counter = 0;
		client_disconnect_counter = 0;

		for (int i = 0; i < test_client_count; i++)
		{
			bool client_start_ret = clients[i]->async_start("127.0.0.1", 18028);
			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
		}

		while (server.get_session_count() < std::size_t(test_client_count))
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(server.get_session_count(), server.get_session_count() == std::size_t(test_client_count));

		while (client_connect_counter < test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == test_client_count);
		ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == test_client_count);

		ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == test_client_count);
		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->stop();
			ASIO2_CHECK(clients[i]->is_stopped());
		}

		ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == test_client_count);

		// use this to ensure the ASIO2_CHECK(session_ptr->is_started());
		while (server_disconnect_counter != test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		server.stop();
		ASIO2_CHECK(server.is_stopped());

		ASIO2_CHECK_VALUE(server_disconnect_counter.load(), server_disconnect_counter == test_client_count);
		ASIO2_CHECK_VALUE(server_stop_counter      .load(), server_stop_counter       == 1);
	}

	{
		struct ext_data
		{
			int num = 1;
			std::string buf;
		};

		asio2::tcp_server server;
		std::size_t session_key = std::size_t(std::rand() % test_client_count);
		std::atomic<int> server_recv_counter = 0;
		server.bind_recv([&](std::shared_ptr<asio2::tcp_session> & session_ptr, std::string_view data)
		{
			server_recv_counter++;
			if (server.iopool().size() > 1)
			{
				ASIO2_CHECK(std::addressof(session_ptr->io()) != std::addressof(server.io()));
			}
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(session_ptr->is_started());

			ext_data& ex = session_ptr->get_user_data<ext_data&>();

			ex.buf += data;

			while (true)
			{
				auto pos = ex.buf.find('\n');
				if (pos == std::string::npos)
					break;

				ASIO2_CHECK(ex.buf[5] == ',');
				ASIO2_CHECK(std::stoi(ex.buf.substr(0, 5)) == ex.num);

				std::string_view frag{ &ex.buf[6], size_t(&ex.buf[pos] - &ex.buf[6]) };

				for (int i = 0; i < ex.num; i++)
				{
					ASIO2_CHECK(frag.size() >= chars.size());
					ASIO2_CHECK(std::memcmp(frag.data(), chars.data(), chars.size()) == 0);
					frag = frag.substr(chars.size());
				}

				ex.num++;

				ex.buf.erase(0, pos + 1);
			}

			session_ptr->async_send(data);

			ASIO2_CHECK(session_ptr->io().running_in_this_thread());
		});
		std::atomic<int> server_accept_counter = 0;
		server.bind_accept([&](auto & session_ptr)
		{
			if (!asio2::get_last_error())
			{
				session_ptr->no_delay(true);

				server_accept_counter++;

				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
				ASIO2_CHECK(server.get_listen_port() == 18028);
				ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_port() == 18028);
				ASIO2_CHECK(server.io().running_in_this_thread());
				ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());

				//// You can close the connection directly here.
				//if (session_ptr->remote_address() == "192.168.0.254")
				//	session_ptr->stop();
			}
		});
		std::atomic<int> server_connect_counter = 0;
		server.bind_connect([&](auto & session_ptr)
		{
			if (session_key == std::size_t(server_connect_counter.load()))
				session_key = session_ptr->hash_key();

			server_connect_counter++;

			ext_data ex;
			ex.num = 1;

			session_ptr->set_user_data(ex);

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(!server.find_session(session_ptr->hash_key()));
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
			ASIO2_CHECK(session_ptr->is_keep_alive());
			ASIO2_CHECK(session_ptr->is_no_delay());
		});
		std::atomic<int> server_disconnect_counter = 0;
		server.bind_disconnect([&](auto & session_ptr)
		{
			server_disconnect_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(!server.find_session(session_ptr->hash_key()));
			ASIO2_CHECK(session_ptr->socket().is_open());
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_init_counter = 0;
		server.bind_init([&]()
		{
			server_init_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());

			asio::socket_base::reuse_address option;
			server.acceptor().get_option(option);
			ASIO2_CHECK(option.value());
		});
		std::atomic<int> server_start_counter = 0;
		server.bind_start([&]()
		{
			server_start_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});

		bool server_start_ret = server.start("127.0.0.1", 18028);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		std::vector<std::shared_ptr<asio2::tcp_client>> clients;
		std::atomic<int> client_init_counter = 0;
		std::atomic<int> client_connect_counter = 0;
		std::atomic<int> client_disconnect_counter = 0;
		std::atomic<int> client_finish_counter = 0;
		for (int i = 0; i < test_client_count; i++)
		{
			auto iter = clients.emplace_back(std::make_shared<asio2::tcp_client>());

			asio2::tcp_client& client = *iter;

			// disable auto reconnect, default reconnect option is "enable"
			//client.set_auto_reconnect(false);

			// enable auto reconnect and use custom delay, default delay is 1 seconds
			client.set_auto_reconnect(true, std::chrono::milliseconds(2000));

			client.bind_init([&]()
			{
				client_init_counter++;

				client.set_no_delay(true);

				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.is_keep_alive());
				ASIO2_CHECK(client.is_reuse_address());
				ASIO2_CHECK(client.is_no_delay());
			});
			client.bind_connect([&]()
			{
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.get_local_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_port() == 18028);

				client_connect_counter++;

				ext_data ex;
				ex.num = 1;

				client.set_user_data(ex);

				std::string msg = fmt::format("{:05d},", ex.num);

				for (int i = 0; i < ex.num; i++)
				{
					msg += chars;
				}

				msg += "\n";

				client.async_send(std::move(msg));
			});
			client.bind_disconnect([&]()
			{
				client_disconnect_counter++;

				ASIO2_CHECK(asio2::get_last_error());
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
			});
			client.bind_recv([&]([[maybe_unused]] std::string_view data)
			{
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!data.empty());
				ASIO2_CHECK(client.is_started());

				ext_data* ex = client.get_user_data<ext_data*>();

				ex->buf += data;

				while (true)
				{
					auto pos = ex->buf.find('\n');
					if (pos == std::string::npos)
						break;

					ASIO2_CHECK(ex->buf[5] == ',');
					ASIO2_CHECK(std::stoi(ex->buf.substr(0, 5)) == ex->num);

					std::string_view frag{ &ex->buf[6], size_t(&ex->buf[pos] - &ex->buf[6]) };

					for (int i = 0; i < ex->num; i++)
					{
						ASIO2_CHECK(frag.size() >= chars.size());
						ASIO2_CHECK(std::memcmp(frag.data(), chars.data(), chars.size()) == 0);
						frag = frag.substr(chars.size());
					}

					if (pos > size_t(6400))
					{
						client_finish_counter++;
						return;
					}

					ex->num++;

					std::string msg = fmt::format("{:05d},", ex->num);

					for (int i = 0; i < ex->num; i++)
					{
						msg += chars;
					}

					msg += "\n";

					client.async_send(std::move(msg));

					ex->buf.erase(0, pos + 1);
				}
			});

			bool client_start_ret = client.async_start("127.0.0.1", 18028);

			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
		}

		while (client_finish_counter < test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}
		ASIO2_CHECK_VALUE(client_finish_counter.load(), client_finish_counter == test_client_count);

		auto session_count = server.get_session_count();
		ASIO2_CHECK_VALUE(session_count, session_count == std::size_t(test_client_count));

		ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == test_client_count);
		ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == test_client_count);

		ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == test_client_count);
		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->stop();
			ASIO2_CHECK(clients[i]->is_stopped());
		}

		ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == test_client_count);

		// use this to ensure the ASIO2_CHECK(session_ptr->is_started());
		while (server_disconnect_counter != test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		server.stop();
		ASIO2_CHECK(server.is_stopped());

		ASIO2_CHECK_VALUE(server_disconnect_counter.load(), server_disconnect_counter == test_client_count);
		ASIO2_CHECK_VALUE(server_stop_counter      .load(), server_stop_counter       == 1);

		//-----------------------------------------------------------------------------------------

		ASIO2_CHECK_VALUE(server.get_session_count(), server.get_session_count() == std::size_t(0));

		server_init_counter = 0;
		server_start_counter = 0;
		server_disconnect_counter = 0;
		server_stop_counter = 0;
		server_accept_counter = 0;
		server_connect_counter = 0;

		server_start_ret = server.start("127.0.0.1", 18028);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		client_init_counter = 0;
		client_connect_counter = 0;
		client_disconnect_counter = 0;

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->user_data_any().reset();
			bool client_start_ret = clients[i]->async_start("127.0.0.1", 18028);
			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
		}

		while (server.get_session_count() < std::size_t(test_client_count))
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(server.get_session_count(), server.get_session_count() == std::size_t(test_client_count));

		while (client_connect_counter < test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == test_client_count);
		ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == test_client_count);

		ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == test_client_count);
		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->stop();
			ASIO2_CHECK(clients[i]->is_stopped());
		}

		ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == test_client_count);

		// use this to ensure the ASIO2_CHECK(session_ptr->is_started());
		while (server_disconnect_counter != test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		server.stop();
		ASIO2_CHECK(server.is_stopped());

		ASIO2_CHECK_VALUE(server_disconnect_counter.load(), server_disconnect_counter == test_client_count);
		ASIO2_CHECK_VALUE(server_stop_counter      .load(), server_stop_counter       == 1);
	}

	{
		asio2::tcp_server server;
		std::size_t session_key = std::size_t(std::rand() % test_client_count);
		std::atomic<int> server_recv_counter = 0;
		server.bind_recv([&](std::shared_ptr<asio2::tcp_session> & session_ptr, std::string_view data)
		{
			server_recv_counter++;
			if (server.iopool().size() > 1)
			{
				ASIO2_CHECK(std::addressof(session_ptr->io()) != std::addressof(server.io()));
			}
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(!data.empty());
			ASIO2_CHECK(session_ptr->is_started());
			session_ptr->async_send("abc", [session_ptr](std::size_t sent_bytes)
			{
				// when the client stopped, the async_send maybe failed.
				if (!asio2::get_last_error())
				{
					// the session_ptr->is_started() maybe false,
					//ASIO2_CHECK(session_ptr->is_started());
					ASIO2_CHECK(sent_bytes == std::size_t(3));
				}
			});

			std::size_t bytes = session_ptr->send("defg");
			ASIO2_CHECK(bytes == 0);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);

			ASIO2_CHECK(session_ptr->io().running_in_this_thread());
		});
		std::atomic<int> server_accept_counter = 0;
		server.bind_accept([&](auto & session_ptr)
		{
			if (!asio2::get_last_error())
			{
				session_ptr->no_delay(true);

				server_accept_counter++;

				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
				ASIO2_CHECK(server.get_listen_port() == 18028);
				ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_port() == 18028);
				ASIO2_CHECK(server.io().running_in_this_thread());
				ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());

				//// You can close the connection directly here.
				//if (session_ptr->remote_address() == "192.168.0.254")
				//	session_ptr->stop();
			}
		});
		std::atomic<int> server_connect_counter = 0;
		server.bind_connect([&](auto & session_ptr)
		{
			if (session_key == std::size_t(server_connect_counter.load()))
				session_key = session_ptr->hash_key();

			server_connect_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(!server.find_session(session_ptr->hash_key()));
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
			ASIO2_CHECK(session_ptr->is_keep_alive());
			ASIO2_CHECK(session_ptr->is_no_delay());
		});
		std::atomic<int> server_disconnect_counter = 0;
		server.bind_disconnect([&](auto & session_ptr)
		{
			server_disconnect_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(!server.find_session(session_ptr->hash_key()));
			ASIO2_CHECK(session_ptr->socket().is_open());
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_init_counter = 0;
		server.bind_init([&]()
		{
			server_init_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());

			asio::socket_base::reuse_address option;
			server.acceptor().get_option(option);
			ASIO2_CHECK(option.value());
		});
		std::atomic<int> server_start_counter = 0;
		server.bind_start([&]()
		{
			server_start_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});

		bool server_start_ret = server.start("127.0.0.1", 18028);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		std::vector<std::shared_ptr<asio2::tcp_client>> clients;
		std::atomic<int> client_init_counter = 0;
		std::atomic<int> client_connect_counter = 0;
		std::atomic<int> client_disconnect_counter = 0;
		for (int i = 0; i < test_client_count; i++)
		{
			auto iter = clients.emplace_back(std::make_shared<asio2::tcp_client>());

			asio2::tcp_client& client = *iter;

			// enable auto reconnect and use custom delay, default delay is 1 seconds
			client.set_auto_reconnect(true, std::chrono::milliseconds(2000));

			client.bind_init([&]()
			{
				client_init_counter++;

				client.set_no_delay(true);

				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.is_keep_alive());
				ASIO2_CHECK(client.is_reuse_address());
				ASIO2_CHECK(client.is_no_delay());
			});
			client.bind_connect([&]()
			{
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.get_local_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_port() == 18028);

				client_connect_counter++;

				client.async_send("abc", [](std::size_t sent_bytes)
				{
					ASIO2_CHECK(sent_bytes == std::size_t(3));
					ASIO2_CHECK(!asio2::get_last_error());
				});

				std::size_t bytes = client.send("defg");
				ASIO2_CHECK(bytes == 0);
				ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
			});
			client.bind_disconnect([&]()
			{
				client_disconnect_counter++;

				ASIO2_CHECK(asio2::get_last_error());
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
			});
			client.bind_recv([&]([[maybe_unused]] std::string_view data)
			{
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!data.empty());
				ASIO2_CHECK(client.is_started());
			});

			bool client_start_ret = client.start("127.0.0.1", 18028);

			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(!asio2::get_last_error());

			client.async_send("abc", [](std::size_t sent_bytes)
			{
				ASIO2_CHECK(sent_bytes == std::size_t(3));
				ASIO2_CHECK(!asio2::get_last_error());
			});

			// this sync send will block util all "send" operation is completed, include async_send above,
			// and the "send" in the bind_connect function.
			std::size_t bytes = client.send("defg");
			ASIO2_CHECK(bytes == 4);
			ASIO2_CHECK(!asio2::get_last_error());
		}

		while (server.get_session_count() < std::size_t(test_client_count))
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		auto session_count = server.get_session_count();
		ASIO2_CHECK_VALUE(session_count, session_count == std::size_t(test_client_count));

		ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == test_client_count);
		ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == test_client_count);

		ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == test_client_count);
		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->stop();
			ASIO2_CHECK(clients[i]->is_stopped());
		}

		ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == test_client_count);

		// use this to ensure the ASIO2_CHECK(session_ptr->is_started());
		while (server_disconnect_counter != test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		server.stop();
		ASIO2_CHECK(server.is_stopped());

		ASIO2_CHECK_VALUE(server_disconnect_counter.load(), server_disconnect_counter == test_client_count);
		ASIO2_CHECK_VALUE(server_stop_counter      .load(), server_stop_counter       == 1);

		//-----------------------------------------------------------------------------------------

		ASIO2_CHECK_VALUE(server.get_session_count(), server.get_session_count() == std::size_t(0));

		server_init_counter = 0;
		server_start_counter = 0;
		server_disconnect_counter = 0;
		server_stop_counter = 0;
		server_accept_counter = 0;
		server_connect_counter = 0;

		server_start_ret = server.start("127.0.0.1", 18028);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		client_init_counter = 0;
		client_connect_counter = 0;
		client_disconnect_counter = 0;

		for (int i = 0; i < test_client_count; i++)
		{
			bool client_start_ret = clients[i]->async_start("127.0.0.1", 18028);
			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
		}

		while (server.get_session_count() < std::size_t(test_client_count))
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(server.get_session_count(), server.get_session_count() == std::size_t(test_client_count));

		while (client_connect_counter < test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == test_client_count);
		ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == test_client_count);

		ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == test_client_count);
		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->stop();
			ASIO2_CHECK(clients[i]->is_stopped());
		}

		ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == test_client_count);

		// use this to ensure the ASIO2_CHECK(session_ptr->is_started());
		while (server_disconnect_counter != test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		server.stop();
		ASIO2_CHECK(server.is_stopped());

		ASIO2_CHECK_VALUE(server_disconnect_counter.load(), server_disconnect_counter == test_client_count);
		ASIO2_CHECK_VALUE(server_stop_counter      .load(), server_stop_counter       == 1);
	}

	{
		asio2::tcp_client client;

		client.set_auto_reconnect(false);
		ASIO2_CHECK(!client.is_auto_reconnect());

		std::atomic<int> client_init_counter = 0;
		client.bind_init([&]()
		{
			client_init_counter++;

			client.set_no_delay(true);

			ASIO2_CHECK(client.io().running_in_this_thread());
			ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(client.is_keep_alive());
			ASIO2_CHECK(client.is_reuse_address());
			ASIO2_CHECK(client.is_no_delay());
		});
		std::atomic<int> client_connect_counter = 0;
		client.bind_connect([&]()
		{
			ASIO2_CHECK(client.io().running_in_this_thread());
			ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
			ASIO2_CHECK(asio2::get_last_error());

			client_connect_counter++;

			client.async_send(chars, [](std::size_t sent_bytes)
			{
				ASIO2_CHECK(sent_bytes == 0);
				ASIO2_CHECK(asio2::get_last_error() == asio::error::not_connected);
			});

			std::size_t sent_bytes = client.send("3abcdefghijklmnopqrxtovwxyz");
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress && sent_bytes == std::size_t(0));
		});
		std::atomic<int> client_disconnect_counter = 0;
		client.bind_disconnect([&]()
		{
			client_disconnect_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(client.io().running_in_this_thread());
			ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
		});
		client.bind_recv([&](std::string_view data)
		{
			ASIO2_CHECK(false);
			ASIO2_CHECK(!data.empty());
			ASIO2_CHECK(client.is_started());
		});

		bool client_start_ret = client.start("127.0.0.1", 18028);

		client.async_send(chars, [](std::size_t sent_bytes)
		{
			ASIO2_CHECK(sent_bytes == 0);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::not_connected);
		});

		std::size_t sent_bytes = client.send("3abcdefghijklmnopqrxtovwxyz");
		ASIO2_CHECK(asio2::get_last_error() == asio::error::not_connected && sent_bytes == std::size_t(0));

		ASIO2_CHECK(!client_start_ret);
		ASIO2_CHECK(!client.is_started());

		ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == 1);
		ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == 1);

		//-----------------------------------------------------------------------------------------

		client.stop();
		ASIO2_CHECK(client.is_stopped());

		ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == 0);
	}

	// test auto reconnect
	{
		struct ext_data
		{
			int client_init_counter = 0;
			int client_connect_counter = 0;
			int client_disconnect_counter = 0;
			std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();
		};

		std::vector<std::shared_ptr<asio2::tcp_client>> clients;
		for (int i = 0; i < test_client_count; i++)
		{
			auto iter = clients.emplace_back(std::make_shared<asio2::tcp_client>());

			asio2::tcp_client& client = *iter;

			client.set_connect_timeout(std::chrono::milliseconds(100));
			client.set_auto_reconnect(true, std::chrono::milliseconds(100));

			ASIO2_CHECK(client.is_auto_reconnect());
			ASIO2_CHECK(client.get_auto_reconnect_delay() == std::chrono::milliseconds(100));
			ASIO2_CHECK(client.get_connect_timeout() == std::chrono::milliseconds(100));

			client.bind_init([&]()
			{
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.client_init_counter++;

				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.is_keep_alive());
				ASIO2_CHECK(client.is_reuse_address());
			});
			client.bind_connect([&]()
			{
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(asio2::get_last_error());

				ext_data& ex = client.get_user_data<ext_data&>();

				ex.client_connect_counter++;

				if (ex.client_connect_counter == 1)
				{
					auto elapse1 = std::abs(std::chrono::duration_cast<std::chrono::milliseconds>(
						std::chrono::high_resolution_clock::now() - ex.start_time).count() - 100);
					ASIO2_CHECK_VALUE(elapse1, elapse1 <= test_timer_deviation);
				}
				else
				{
					auto elapse1 = std::abs(std::chrono::duration_cast<std::chrono::milliseconds>(
						std::chrono::high_resolution_clock::now() - ex.start_time).count() - 200);
					ASIO2_CHECK_VALUE(elapse1, elapse1 <= test_timer_deviation);
				}

				ex.start_time = std::chrono::high_resolution_clock::now();

				if (ex.client_connect_counter == 3)
				{
					client.set_auto_reconnect(false);
					ASIO2_CHECK(!client.is_auto_reconnect());
				}

				std::size_t bytes = client.send("defg");
				ASIO2_CHECK(bytes == 0);
				ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
			});
			client.bind_disconnect([&]()
			{
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.client_disconnect_counter++;

				ASIO2_CHECK(asio2::get_last_error());
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
			});
			client.bind_recv([&]([[maybe_unused]] std::string_view data)
			{
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!data.empty());
				ASIO2_CHECK(client.is_started());
			});

			ext_data ex;

			client.set_user_data(std::move(ex));

			bool client_start_ret = client.async_start("127.0.0.1", 18028);

			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);

			std::size_t bytes = client.send("defg");
			ASIO2_CHECK(bytes == 0);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::not_connected);
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			while (ex.client_connect_counter < 3)
			{
				ASIO2_TEST_WAIT_CHECK();
			}
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			ASIO2_CHECK_VALUE(ex.client_init_counter   , ex.client_init_counter    == 3);
			ASIO2_CHECK_VALUE(ex.client_connect_counter, ex.client_connect_counter == 3);
		}

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->stop();
			ASIO2_CHECK(clients[i]->is_stopped());
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			ASIO2_CHECK_VALUE(ex.client_disconnect_counter, ex.client_disconnect_counter == 0);
		}

		//-----------------------------------------------------------------------------------------
		
		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->set_auto_reconnect(true);

			ASIO2_CHECK(clients[i]->is_auto_reconnect());
			ASIO2_CHECK(clients[i]->get_auto_reconnect_delay() == std::chrono::milliseconds(100));
			ASIO2_CHECK(clients[i]->get_connect_timeout() == std::chrono::milliseconds(100));

			ext_data ex;

			clients[i]->set_user_data(std::move(ex));

			bool client_start_ret = clients[i]->start("127.0.0.1", 18028);

			ASIO2_CHECK(!client_start_ret);
			ASIO2_CHECK_VALUE(asio2::last_error_msg().data(),
				asio2::get_last_error() == asio::error::timed_out ||
				asio2::get_last_error() == asio::error::connection_refused);

			std::size_t bytes = clients[i]->send("defg");
			ASIO2_CHECK(bytes == 0);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::not_connected);
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			while (ex.client_connect_counter < 3)
			{
				ASIO2_TEST_WAIT_CHECK();
			}
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			ASIO2_CHECK_VALUE(ex.client_init_counter   , ex.client_init_counter    == 3);
			ASIO2_CHECK_VALUE(ex.client_connect_counter, ex.client_connect_counter == 3);
		}

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->stop();
			ASIO2_CHECK(clients[i]->is_stopped());
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			ASIO2_CHECK_VALUE(ex.client_disconnect_counter, ex.client_disconnect_counter == 0);
		}

		//-----------------------------------------------------------------------------------------
		
		asio2::tcp_server server;
		std::size_t session_key = std::size_t(std::rand() % test_client_count);
		std::atomic<int> server_recv_counter = 0;
		server.bind_recv([&](std::shared_ptr<asio2::tcp_session> & session_ptr, std::string_view data)
		{
			server_recv_counter++;
			if (server.iopool().size() > 1)
			{
				ASIO2_CHECK(std::addressof(session_ptr->io()) != std::addressof(server.io()));
			}
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(!data.empty());
			ASIO2_CHECK(session_ptr->is_started());
			session_ptr->async_send("abc", [session_ptr](std::size_t sent_bytes)
			{
				// when the client stopped, the async_send maybe failed.
				if (!asio2::get_last_error())
				{
					// the session_ptr->is_started() maybe false,
					//ASIO2_CHECK(session_ptr->is_started());
					ASIO2_CHECK(sent_bytes == std::size_t(3));
				}
			});

			std::size_t bytes = session_ptr->send("defg");
			ASIO2_CHECK(bytes == 0);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);

			ASIO2_CHECK(session_ptr->io().running_in_this_thread());
		});
		std::atomic<int> server_accept_counter = 0;
		server.bind_accept([&](auto & session_ptr)
		{
			if (!asio2::get_last_error())
			{
				session_ptr->no_delay(true);

				server_accept_counter++;

				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
				ASIO2_CHECK(server.get_listen_port() == 18028);
				ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_port() == 18028);
				ASIO2_CHECK(server.io().running_in_this_thread());
				ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());

				//// You can close the connection directly here.
				//if (session_ptr->remote_address() == "192.168.0.254")
				//	session_ptr->stop();
			}
		});
		std::atomic<int> server_connect_counter = 0;
		server.bind_connect([&](auto & session_ptr)
		{
			if (session_key == std::size_t(server_connect_counter.load()))
				session_key = session_ptr->hash_key();

			server_connect_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(!server.find_session(session_ptr->hash_key()));
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
			ASIO2_CHECK(session_ptr->is_keep_alive());
			ASIO2_CHECK(session_ptr->is_no_delay());
		});
		std::atomic<int> server_disconnect_counter = 0;
		server.bind_disconnect([&](auto & session_ptr)
		{
			server_disconnect_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(!server.find_session(session_ptr->hash_key()));
			ASIO2_CHECK(session_ptr->socket().is_open());
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_init_counter = 0;
		server.bind_init([&]()
		{
			server_init_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());

			asio::socket_base::reuse_address option;
			server.acceptor().get_option(option);
			ASIO2_CHECK(option.value());
		});
		std::atomic<int> server_start_counter = 0;
		server.bind_start([&]()
		{
			server_start_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});

		bool server_start_ret = server.start("127.0.0.1", 18028);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->set_auto_reconnect(true);

			clients[i]->set_connect_timeout(std::chrono::milliseconds(5000));

			ASIO2_CHECK(clients[i]->is_auto_reconnect());
			ASIO2_CHECK(clients[i]->get_auto_reconnect_delay() == std::chrono::milliseconds(100));
			ASIO2_CHECK(clients[i]->get_connect_timeout() == std::chrono::milliseconds(5000));

			asio2::tcp_client& client = *clients[i];

			client.bind_init([&]()
			{
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.client_init_counter++;

				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.is_keep_alive());
				ASIO2_CHECK(client.is_reuse_address());
			});
			client.bind_connect([&]()
			{
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());

				ext_data& ex = client.get_user_data<ext_data&>();
				ex.client_connect_counter++;

				std::size_t bytes = client.send("defg");
				ASIO2_CHECK(bytes == 0);
				ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
			});
			client.bind_disconnect([&]()
			{
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.client_disconnect_counter++;

				ASIO2_CHECK(asio2::get_last_error());
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
			});
			client.bind_recv([&]([[maybe_unused]] std::string_view data)
			{
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!data.empty());
				ASIO2_CHECK(client.is_started());
			});

			ext_data ex;

			clients[i]->set_user_data(std::move(ex));

			bool client_start_ret = clients[i]->async_start("127.0.0.1", 18028);

			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
		}

		while (server.get_session_count() < std::size_t(test_client_count))
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		auto session_count = server.get_session_count();
		ASIO2_CHECK_VALUE(session_count, session_count == std::size_t(test_client_count));

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			while (ex.client_connect_counter < 1)
			{
				ASIO2_TEST_WAIT_CHECK();
			}
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			ASIO2_CHECK_VALUE(ex.client_init_counter   , ex.client_init_counter    == 1);
			ASIO2_CHECK_VALUE(ex.client_connect_counter, ex.client_connect_counter == 1);
		}

		ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == test_client_count);
		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->stop();
			ASIO2_CHECK(clients[i]->is_stopped());
			ASIO2_CHECK(!clients[i]->user_data_any().has_value());
		}

		// use this to ensure the ASIO2_CHECK(session_ptr->is_started());
		while (server_disconnect_counter != test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		server.stop();
		ASIO2_CHECK(server.is_stopped());

		ASIO2_CHECK_VALUE(server_disconnect_counter.load(), server_disconnect_counter == test_client_count);
		ASIO2_CHECK_VALUE(server_stop_counter      .load(), server_stop_counter       == 1);
	}

	// test close session
	{
		asio2::tcp_server server;
		std::size_t session_key = std::size_t(std::rand() % test_client_count);
		std::atomic<int> server_recv_counter = 0;
		server.bind_recv([&](std::shared_ptr<asio2::tcp_session> & session_ptr, std::string_view data)
		{
			server_recv_counter++;
			if (server.iopool().size() > 1)
			{
				ASIO2_CHECK(std::addressof(session_ptr->io()) != std::addressof(server.io()));
			}
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(!data.empty());
			ASIO2_CHECK(session_ptr->is_started());
			session_ptr->async_send("abc", [session_ptr](std::size_t sent_bytes)
			{
				// when the client stopped, the async_send maybe failed.
				if (!asio2::get_last_error())
				{
					// the session_ptr->is_started() maybe false,
					//ASIO2_CHECK(session_ptr->is_started());
					ASIO2_CHECK(sent_bytes == std::size_t(3));
				}
			});

			std::size_t bytes = session_ptr->send("defg");
			ASIO2_CHECK(bytes == 0);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);

			ASIO2_CHECK(session_ptr->io().running_in_this_thread());
		});
		std::atomic<int> server_accept_counter = 0;
		server.bind_accept([&](auto & session_ptr)
		{
			if (!asio2::get_last_error())
			{
				session_ptr->no_delay(true);

				server_accept_counter++;

				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
				ASIO2_CHECK(server.get_listen_port() == 18028);
				ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_port() == 18028);
				ASIO2_CHECK(server.io().running_in_this_thread());
				ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
			}
		});
		std::atomic<int> server_connect_counter = 0;
		server.bind_connect([&](auto & session_ptr)
		{
			if (session_key == std::size_t(server_connect_counter.load()))
			{
				session_key = session_ptr->hash_key();
				session_ptr->set_user_data(session_key);
				session_ptr->stop();
			}

			server_connect_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(!server.find_session(session_ptr->hash_key()));
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
			ASIO2_CHECK(session_ptr->is_keep_alive());
			ASIO2_CHECK(session_ptr->is_no_delay());
		});
		std::atomic<int> server_disconnect_counter = 0;
		server.bind_disconnect([&](auto & session_ptr)
		{
			server_disconnect_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(!server.find_session(session_ptr->hash_key()));
			ASIO2_CHECK(session_ptr->socket().is_open());
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_init_counter = 0;
		server.bind_init([&]()
		{
			server_init_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());

			asio::socket_base::reuse_address option;
			server.acceptor().get_option(option);
			ASIO2_CHECK(option.value());
		});
		std::atomic<int> server_start_counter = 0;
		server.bind_start([&]()
		{
			server_start_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});

		bool server_start_ret = server.start("127.0.0.1", 18028);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		std::vector<std::shared_ptr<asio2::tcp_client>> clients;
		std::atomic<int> client_init_counter = 0;
		std::atomic<int> client_connect_counter = 0;
		std::atomic<int> client_disconnect_counter = 0;
		for (int i = 0; i < test_client_count; i++)
		{
			auto iter = clients.emplace_back(std::make_shared<asio2::tcp_client>());

			asio2::tcp_client& client = *iter;

			// enable auto reconnect and use custom delay, default delay is 1 seconds
			client.set_auto_reconnect(false);

			client.bind_init([&]()
			{
				client_init_counter++;

				client.set_no_delay(true);

				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.is_keep_alive());
				ASIO2_CHECK(client.is_reuse_address());
				ASIO2_CHECK(client.is_no_delay());
			});
			client.bind_connect([&]()
			{
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.get_local_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_port() == 18028);

				client_connect_counter++;

				std::size_t bytes = client.send("defg");
				ASIO2_CHECK(bytes == 0);
				ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
			});
			client.bind_disconnect([&]()
			{
				client_disconnect_counter++;

				ASIO2_CHECK(asio2::get_last_error());
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
			});
			client.bind_recv([&]([[maybe_unused]] std::string_view data)
			{
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!data.empty());
				ASIO2_CHECK(client.is_started());
			});

			bool client_start_ret = client.start("127.0.0.1", 18028);

			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(!asio2::get_last_error());
		}

		while (server.get_session_count() < std::size_t(test_client_count - 1))
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		while (client_disconnect_counter < 1)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		while (client_connect_counter < test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		auto session_count = server.get_session_count();
		ASIO2_CHECK_VALUE(session_count, session_count == std::size_t(test_client_count - 1));

		ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == test_client_count);
		ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == test_client_count);

		ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == test_client_count);
		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		// find session maybe true, beacuse the next session maybe used the stopped session "this" address
		//ASIO2_CHECK(!server.find_session(session_key));
		server.foreach_session([](std::shared_ptr<asio2::tcp_session>& session_ptr) mutable
		{
			ASIO2_CHECK(!session_ptr->user_data_any().has_value());
		});

		ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == 1);

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->stop();
			ASIO2_CHECK(clients[i]->is_stopped());
		}

		ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == test_client_count);

		// use this to ensure the ASIO2_CHECK(session_ptr->is_started());
		while (server_disconnect_counter != test_client_count - 1)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		server.stop();
		ASIO2_CHECK(server.is_stopped());

		ASIO2_CHECK_VALUE(server_disconnect_counter.load(), server_disconnect_counter == test_client_count - 1);
		ASIO2_CHECK_VALUE(server_stop_counter      .load(), server_stop_counter       == 1);

		//-----------------------------------------------------------------------------------------

		ASIO2_CHECK_VALUE(server.get_session_count(), server.get_session_count() == std::size_t(0));

		server_init_counter = 0;
		server_start_counter = 0;
		server_disconnect_counter = 0;
		server_stop_counter = 0;
		server_accept_counter = 0;
		server_connect_counter = 0;

		session_key = std::size_t(std::rand() % test_client_count);

		server_start_ret = server.start("127.0.0.1", 18028);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		client_init_counter = 0;
		client_connect_counter = 0;
		client_disconnect_counter = 0;

		for (int i = 0; i < test_client_count; i++)
		{
			bool client_start_ret = clients[i]->async_start("127.0.0.1", 18028);
			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
		}

		while (server.get_session_count() < std::size_t(test_client_count - 1))
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		while (client_disconnect_counter < 1)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == 1);

		ASIO2_CHECK_VALUE(server.get_session_count(), server.get_session_count() == std::size_t(test_client_count - 1));

		while (client_connect_counter < test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		server.foreach_session([](std::shared_ptr<asio2::tcp_session>& session_ptr) mutable
		{
			ASIO2_CHECK(!session_ptr->user_data_any().has_value());
		});

		ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == test_client_count);
		ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == test_client_count);

		ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == test_client_count);
		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->stop();
			ASIO2_CHECK(clients[i]->is_stopped());
		}

		ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == test_client_count);

		// use this to ensure the ASIO2_CHECK(session_ptr->is_started());
		while (server_disconnect_counter != test_client_count - 1)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		server.stop();
		ASIO2_CHECK(server.is_stopped());

		ASIO2_CHECK_VALUE(server_disconnect_counter.load(), server_disconnect_counter == test_client_count - 1);
		ASIO2_CHECK_VALUE(server_stop_counter      .load(), server_stop_counter       == 1);
	}

	// test hook_buffer
	{
		struct ext_data
		{
			int num = 1;
		};

		asio2::tcp_server server;
		std::atomic<int> server_recv_counter = 0;
		server.bind_recv([&](std::shared_ptr<asio2::tcp_session> & session_ptr, std::string_view data)
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(session_ptr->is_started());
			if (server.iopool().size() > 1)
			{
				ASIO2_CHECK(std::addressof(session_ptr->io()) != std::addressof(server.io()));
			}
			ext_data& ex = session_ptr->get_user_data<ext_data&>();

			while (true)
			{
				auto pos = data.find('\n');

				if (pos != std::string::npos)
				{
					server_recv_counter++;

					std::string_view message = data.substr(0, pos + 1);

					std::string msg = fmt::format("{:05d},", ex.num);

					for (int i = 0; i < ex.num; i++)
					{
						msg += chars;
					}
					msg += '\n';

					ASIO2_CHECK(msg == message);

					ex.num++;

					session_ptr->async_send(msg.substr(0, msg.size() / 2));
					session_ptr->async_send(msg.substr(msg.size() / 2));

					session_ptr->buffer().consume(pos + 1);

					data = session_ptr->buffer().data_view();
				}
				else
				{
					break;
				}
			}

			ASIO2_CHECK(session_ptr->io().running_in_this_thread());
		});
		std::atomic<int> server_accept_counter = 0;
		server.bind_accept([&](auto & session_ptr)
		{
			if (!asio2::get_last_error())
			{
				session_ptr->no_delay(true);

				server_accept_counter++;

				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
				ASIO2_CHECK(server.get_listen_port() == 18028);
				ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_port() == 18028);
				ASIO2_CHECK(server.io().running_in_this_thread());
				ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());

				//// You can close the connection directly here.
				//if (session_ptr->remote_address() == "192.168.0.254")
				//	session_ptr->stop();
			}
		});
		std::atomic<int> server_connect_counter = 0;
		server.bind_connect([&](auto & session_ptr)
		{
			server_connect_counter++;

			ext_data ex;
			ex.num = 1;

			session_ptr->set_user_data(ex);

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(!server.find_session(session_ptr->hash_key()));
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
			ASIO2_CHECK(session_ptr->is_keep_alive());
			ASIO2_CHECK(session_ptr->is_no_delay());
		});
		std::atomic<int> server_disconnect_counter = 0;
		server.bind_disconnect([&](auto & session_ptr)
		{
			server_disconnect_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(!server.find_session(session_ptr->hash_key()));
			ASIO2_CHECK(session_ptr->socket().is_open());
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_init_counter = 0;
		server.bind_init([&]()
		{
			server_init_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());

			asio::socket_base::reuse_address option;
			server.acceptor().get_option(option);
			ASIO2_CHECK(option.value());
		});
		std::atomic<int> server_start_counter = 0;
		server.bind_start([&]()
		{
			server_start_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});

		bool server_start_ret = server.start("127.0.0.1", 18028, asio2::hook_buffer);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		std::vector<std::shared_ptr<asio2::tcp_client>> clients;
		std::atomic<int> client_init_counter = 0;
		std::atomic<int> client_connect_counter = 0;
		std::atomic<int> client_disconnect_counter = 0;
		for (int i = 0; i < test_client_count; i++)
		{
			auto iter = clients.emplace_back(std::make_shared<asio2::tcp_client>());

			asio2::tcp_client& client = *iter;

			// disable auto reconnect, default reconnect option is "enable"
			client.set_auto_reconnect(false);

			client.bind_init([&]()
			{
				client_init_counter++;

				client.set_no_delay(true);

				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.is_keep_alive());
				ASIO2_CHECK(client.is_reuse_address());
				ASIO2_CHECK(client.is_no_delay());
			});
			client.bind_connect([&]()
			{
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.get_local_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_port() == 18028);

				client_connect_counter++;

				ext_data ex;
				ex.num = 1;

				client.set_user_data(ex);
			});
			client.bind_disconnect([&]()
			{
				client_disconnect_counter++;

				ASIO2_CHECK(asio2::get_last_error());
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
			});
			client.bind_recv([&]([[maybe_unused]] std::string_view data)
			{
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!data.empty());
				ASIO2_CHECK(client.is_started());
				//client.async_send(data);

				ext_data* ex = client.get_user_data<ext_data*>();

				if (ex->num > 20)
				{
					client.stop();
					return;
				}

				while (true)
				{
					auto pos = data.find('\n');

					if (pos != std::string::npos)
					{
						std::string_view message = data.substr(0, pos + 1);

						std::string msg = fmt::format("{:05d},", ex->num);

						for (int i = 0; i < ex->num; i++)
						{
							msg += chars;
						}
						msg += '\n';

						ASIO2_CHECK(msg == message);

						ex->num++;

						msg = fmt::format("{:05d},", ex->num);

						for (int i = 0; i < ex->num; i++)
						{
							msg += chars;
						}
						msg += '\n';

						client.async_send(msg.substr(0, msg.size() / 2));
						client.async_send(msg.substr(msg.size() / 2));

						client.buffer().consume(pos + 1);

						data = client.buffer().data_view();
					}
					else
					{
						break;
					}
				}
			});

			bool client_start_ret = client.async_start("127.0.0.1", 18028, asio2::hook_buffer);

			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
		}

		while (server.get_session_count() < std::size_t(test_client_count))
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		auto session_count = server.get_session_count();
		ASIO2_CHECK_VALUE(session_count, session_count == std::size_t(test_client_count));

		while (client_connect_counter < test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == test_client_count);
		ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == test_client_count);

		ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == test_client_count);
		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			std::string msg = fmt::format("{:05d},", 1);

			msg += chars;
			msg += '\n';

			clients[i]->async_send(msg.substr(0, msg.size() / 2));
			clients[i]->async_send(msg.substr(msg.size() / 2));
		}

		while (client_disconnect_counter != test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		// use this to ensure the ASIO2_CHECK(session_ptr->is_started());
		while (server_disconnect_counter != test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(server_disconnect_counter.load(), server_disconnect_counter == test_client_count);
		ASIO2_CHECK_VALUE(server_recv_counter.load(), server_recv_counter == 21 * test_client_count);
		ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->stop();
			ASIO2_CHECK(clients[i]->is_stopped());
		}

		server.stop();
		ASIO2_CHECK(server.is_stopped());

		ASIO2_CHECK_VALUE(server_stop_counter      .load(), server_stop_counter       == 1);

		//-----------------------------------------------------------------------------------------

		ASIO2_CHECK_VALUE(server.get_session_count(), server.get_session_count() == std::size_t(0));

		server_init_counter = 0;
		server_start_counter = 0;
		server_disconnect_counter = 0;
		server_stop_counter = 0;
		server_accept_counter = 0;
		server_connect_counter = 0;
		server_recv_counter = 0;

		server_start_ret = server.start("127.0.0.1", 18028, asio2::hook_buffer);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		client_init_counter = 0;
		client_connect_counter = 0;
		client_disconnect_counter = 0;

		for (int i = 0; i < test_client_count; i++)
		{
			bool client_start_ret = clients[i]->async_start("127.0.0.1", 18028, asio2::hook_buffer);
			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
		}

		while (server.get_session_count() < std::size_t(test_client_count))
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(server.get_session_count(), server.get_session_count() == std::size_t(test_client_count));

		while (client_connect_counter < test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == test_client_count);
		ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == test_client_count);

		ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == test_client_count);
		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			std::string msg = fmt::format("{:05d},", 1);

			msg += chars;
			msg += '\n';

			clients[i]->async_send(msg.substr(0, msg.size() / 2));
			clients[i]->async_send(msg.substr(msg.size() / 2));
		}

		while (client_disconnect_counter != test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		// use this to ensure the ASIO2_CHECK(session_ptr->is_started());
		while (server_disconnect_counter != test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(server_disconnect_counter.load(), server_disconnect_counter == test_client_count);
		ASIO2_CHECK_VALUE(server_recv_counter.load(), server_recv_counter == 21 * test_client_count);
		ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->stop();
			ASIO2_CHECK(clients[i]->is_stopped());
		}

		server.stop();
		ASIO2_CHECK(server.is_stopped());

		ASIO2_CHECK_VALUE(server_stop_counter      .load(), server_stop_counter       == 1);
	}

	// test close session in no io_context thread.
	{
		asio2::tcp_server server;
		std::atomic<int> server_accept_counter = 0;
		server.bind_accept([&](auto & session_ptr)
		{
			if (!asio2::get_last_error())
			{
				session_ptr->no_delay(true);

				server_accept_counter++;

				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
				ASIO2_CHECK(server.get_listen_port() == 18028);
				ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_port() == 18028);
				ASIO2_CHECK(server.io().running_in_this_thread());
				ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
			}
		});
		std::atomic<int> server_connect_counter = 0;
		server.bind_connect([&](auto & session_ptr)
		{
			session_ptr->set_user_data(server_connect_counter.load());
			server_connect_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(!server.find_session(session_ptr->hash_key()));
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
			ASIO2_CHECK(session_ptr->is_keep_alive());
			ASIO2_CHECK(session_ptr->is_no_delay());
		});
		std::atomic<int> server_disconnect_counter = 0;
		server.bind_disconnect([&](auto & session_ptr)
		{
			server_disconnect_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(!server.find_session(session_ptr->hash_key()));
			ASIO2_CHECK(session_ptr->socket().is_open());
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_init_counter = 0;
		server.bind_init([&]()
		{
			server_init_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());

			asio::socket_base::reuse_address option;
			server.acceptor().get_option(option);
			ASIO2_CHECK(option.value());
		});
		std::atomic<int> server_start_counter = 0;
		server.bind_start([&]()
		{
			server_start_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18028);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});

		bool server_start_ret = server.start("127.0.0.1", 18028);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		std::vector<std::shared_ptr<asio2::tcp_client>> clients;
		std::atomic<int> client_init_counter = 0;
		std::atomic<int> client_connect_counter = 0;
		std::atomic<int> client_disconnect_counter = 0;
		for (int i = 0; i < test_client_count; i++)
		{
			auto iter = clients.emplace_back(std::make_shared<asio2::tcp_client>());

			asio2::tcp_client& client = *iter;

			// enable auto reconnect and use custom delay, default delay is 1 seconds
			client.set_auto_reconnect(false);

			client.bind_init([&]()
			{
				client_init_counter++;

				client.set_no_delay(true);

				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.is_keep_alive());
				ASIO2_CHECK(client.is_reuse_address());
				ASIO2_CHECK(client.is_no_delay());
			});
			client.bind_connect([&]()
			{
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.get_local_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_port() == 18028);

				client_connect_counter++;

				std::size_t bytes = client.send("defg");
				ASIO2_CHECK(bytes == 0);
				ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
			});
			client.bind_disconnect([&]()
			{
				client_disconnect_counter++;

				ASIO2_CHECK(asio2::get_last_error());
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
			});
			client.bind_recv([&]([[maybe_unused]] std::string_view data)
			{
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!data.empty());
				ASIO2_CHECK(client.is_started());
			});

			bool client_start_ret = client.start("127.0.0.1", 18028);

			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(!asio2::get_last_error());
		}

		while (server.get_session_count() < std::size_t(test_client_count))
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		while (client_connect_counter < test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		auto session_count = server.get_session_count();
		ASIO2_CHECK_VALUE(session_count, session_count == std::size_t(test_client_count));

		ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == test_client_count);
		ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == test_client_count);

		ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == test_client_count);
		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		std::vector<std::shared_ptr<asio2::tcp_session>> sessions;

		// find session maybe true, beacuse the next session maybe used the stopped session "this" address
		//ASIO2_CHECK(!server.find_session(session_key));
		server.foreach_session([&sessions](std::shared_ptr<asio2::tcp_session>& session_ptr) mutable
		{
			ASIO2_CHECK(session_ptr->user_data_any().has_value());

			sessions.emplace_back(session_ptr);
		});

		ASIO2_CHECK_VALUE(sessions.size(), sessions.size() == std::size_t(test_client_count));

		for (std::size_t i = 0; i < sessions.size(); i++)
		{
			std::shared_ptr<asio2::tcp_session>& session = sessions[i];

			if (i % 2 == 0)
			{
				session->stop();
				ASIO2_CHECK(session->is_stopped());
			}
			else
			{
				session->post([session]()
				{
					session->stop();
				});
				ASIO2_CHECK(!session->is_stopped());
			}
		}

		sessions.clear();

		// use this to ensure the ASIO2_CHECK(session_ptr->is_started());
		while (server_disconnect_counter != test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		// must call stop, otherwise will cause memory leaks.
		for (auto& c : clients)
		{
			c->stop();
			ASIO2_CHECK(c->is_stopped());
		}
				
		ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == test_client_count);

		server.stop();
		ASIO2_CHECK(server.is_stopped());

		ASIO2_CHECK_VALUE(server_disconnect_counter.load(), server_disconnect_counter == test_client_count);
		ASIO2_CHECK_VALUE(server_stop_counter      .load(), server_stop_counter       == 1);
	}

	ASIO2_TEST_END_LOOP;
}


ASIO2_TEST_SUITE
(
	"tcp_general",
	ASIO2_TEST_CASE(tcp_general_test)
)
