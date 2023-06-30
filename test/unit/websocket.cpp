#include "unit_test.hpp"
#include <asio2/websocket/ws_server.hpp>
#include <asio2/websocket/ws_client.hpp>
#include <fmt/format.h>

void websocket_test()
{
	ASIO2_TEST_BEGIN_LOOP(test_loop_times);

	{
		asio2::ws_server server;

		std::atomic<int> server_recv_counter = 0;
		std::atomic<std::size_t> server_recv_size = 0;
		server.bind_recv([&](std::shared_ptr<asio2::ws_session> & session_ptr, std::string_view data)
		{
			server_recv_counter++;

			server_recv_size += data.size();

			ASIO2_CHECK(data.front() == '<' && data.back() == '>');
			int len = std::stoi(std::string(data.substr(1, 5)));
			ASIO2_CHECK(len == int(data.size() - 7));

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

				session_ptr->ws_stream().set_option(websocket::stream_base::decorator(
				[](websocket::response_type& rep)
				{
					rep.set(http::field::authorization, " websocket-server-coro");
				}));

				server_accept_counter++;

				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
				ASIO2_CHECK(server.get_listen_port() == 18039);
				ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_port() == 18039);
				ASIO2_CHECK(server.io().running_in_this_thread());
				ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());

				//// You can close the connection directly here.
				//if (session_ptr->remote_address() == "192.168.0.254")
				//	session_ptr->stop();
			}
		});
		std::atomic<int> server_upgrade_counter = 0;
		server.bind_upgrade([&](auto & session_ptr)
		{
			server_upgrade_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18039);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
			ASIO2_CHECK(session_ptr->is_keep_alive());
			ASIO2_CHECK(session_ptr->is_no_delay());
		});
		std::atomic<int> server_connect_counter = 0;
		server.bind_connect([&](auto & session_ptr)
		{
			server_connect_counter++;

			ASIO2_CHECK(session_ptr->ws_stream().text());
			session_ptr->ws_stream().binary(loop % 2 == 0);
			if (loop % 2 == 0)
			{
				ASIO2_CHECK(session_ptr->ws_stream().binary());
			}
			else
			{
				ASIO2_CHECK(session_ptr->ws_stream().text());
			}

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18039);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
			ASIO2_CHECK(session_ptr->is_keep_alive());
			ASIO2_CHECK(session_ptr->is_no_delay());
		});
		std::atomic<int> server_disconnect_counter = 0;
		server.bind_disconnect([&](auto & session_ptr)
		{
			server_disconnect_counter++;
			asio2::ignore_unused(session_ptr);
			ASIO2_CHECK(asio2::get_last_error());
			// after test, on linux, when disconnect is called, and there has some data 
			// is transmiting(by async_send), the remote_address maybe empty.
			// and under websocket, the socket maybe closed already in the websocket close frame.
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			//ASIO2_CHECK(session_ptr->local_port() == 18039);
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
			ASIO2_CHECK(server.get_listen_port() == 18039);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18039);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});

		bool server_start_ret = server.start("127.0.0.1", 18039);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		asio2::ws_client client;

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

			client.ws_stream().set_option(websocket::stream_base::decorator(
			[](websocket::request_type& req)
			{
				req.set(http::field::authorization, " websocket-client-authorization");
			}));

			ASIO2_CHECK(client.io().running_in_this_thread());
			ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(client.is_keep_alive());
			ASIO2_CHECK(client.is_reuse_address());
			ASIO2_CHECK(client.is_no_delay());
		});
		std::atomic<int> client_upgrade_counter = 0;
		client.bind_upgrade([&]()
		{
			ASIO2_CHECK(client.io().running_in_this_thread());
			ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(client.get_local_address() == "127.0.0.1");
			ASIO2_CHECK(client.get_remote_address() == "127.0.0.1");
			ASIO2_CHECK(client.get_remote_port() == 18039);

			client_upgrade_counter++;
		});
		std::atomic<int> client_connect_counter = 0;
		client.bind_connect([&]()
		{
			ASIO2_CHECK(client.io().running_in_this_thread());
			ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(client.get_local_address() == "127.0.0.1");
			ASIO2_CHECK(client.get_remote_address() == "127.0.0.1");
			ASIO2_CHECK(client.get_remote_port() == 18039);

			ASIO2_CHECK(client.ws_stream().text());
			client.ws_stream().binary(loop % 2 == 0);
			if (loop % 2 == 0)
			{
				ASIO2_CHECK(client.ws_stream().binary());
			}
			else
			{
				ASIO2_CHECK(client.ws_stream().text());
			}

			client_connect_counter++;

			bool binary = (loop % 2 == 0);

			std::string str;
			str += '<';
			int len = 128 + std::rand() % (300);
			str += fmt::format("{:05d}", len);
			for (int i = 0; i < len; i++)
			{
				if (binary)
				{
					str += (char)(std::rand() % 255);
				}
				else
				{
					// under text mode, the content must be chars
					str += (char)((std::rand() % 26) + 'a');
				}
			}
			str += '>';

			client_send_size += str.size();
			client.async_send(str);
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
			ASIO2_CHECK(data.front() == '<' && data.back() == '>');
			int len = std::stoi(std::string(data.substr(1, 5)));
			ASIO2_CHECK(len == int(data.size() - 7));
		});

		bool client_start_ret = client.start("127.0.0.1", 18039);

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
		asio2::ws_server server;
		std::size_t session_key = std::size_t(std::rand() % test_client_count);
		std::atomic<int> server_recv_counter = 0;
		server.bind_recv([&](std::shared_ptr<asio2::ws_session> & session_ptr, std::string_view data)
		{
			server_recv_counter++;
			if (server.iopool().size() > 1)
			{
				ASIO2_CHECK(std::addressof(session_ptr->io()) != std::addressof(server.io()));
			}
			ASIO2_CHECK(data.front() == '<' && data.back() == '>');
			int len = std::stoi(std::string(data.substr(1, 5)));
			ASIO2_CHECK(len == int(data.size() - 7));

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
				ASIO2_CHECK(server.get_listen_port() == 18039);
				ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_port() == 18039);
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

			ASIO2_CHECK(session_ptr->ws_stream().text());
			session_ptr->ws_stream().binary(loop % 2 == 0);
			if (loop % 2 == 0)
			{
				ASIO2_CHECK(session_ptr->ws_stream().binary());
			}
			else
			{
				ASIO2_CHECK(session_ptr->ws_stream().text());
			}

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(!server.find_session(session_ptr->hash_key()));
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18039);
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
			ASIO2_CHECK(server.get_listen_port() == 18039);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18039);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});

		bool server_start_ret = server.start("127.0.0.1", 18039);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		std::vector<std::shared_ptr<asio2::ws_client>> clients;
		std::atomic<int> client_init_counter = 0;
		std::atomic<int> client_connect_counter = 0;
		std::atomic<int> client_disconnect_counter = 0;
		for (int i = 0; i < test_client_count; i++)
		{
			auto iter = clients.emplace_back(std::make_shared<asio2::ws_client>());

			asio2::ws_client& client = *iter;

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
				ASIO2_CHECK(client.get_remote_port() == 18039);

				ASIO2_CHECK(client.ws_stream().text());
				client.ws_stream().binary(loop % 2 == 0);
				if (loop % 2 == 0)
				{
					ASIO2_CHECK(client.ws_stream().binary());
				}
				else
				{
					ASIO2_CHECK(client.ws_stream().text());
				}

				client_connect_counter++;

				bool binary = (loop % 2 == 0);

				std::string str;
				str += '<';
				int len = 128 + std::rand() % (300);
				str += fmt::format("{:05d}", len);
				for (int i = 0; i < len; i++)
				{
					if (binary)
					{
						str += (char)(std::rand() % 255);
					}
					else
					{
						// under text mode, the content must be chars
						str += (char)((std::rand() % 26) + 'a');
					}
				}
				str += '>';

				client.async_send(str);
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
				ASIO2_CHECK(data.front() == '<' && data.back() == '>');
				int len = std::stoi(std::string(data.substr(1, 5)));
				ASIO2_CHECK(len == int(data.size() - 7));
			});

			bool client_start_ret = client.async_start("127.0.0.1", 18039, "/admin");

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
		std::size_t sent_bytes = session_ptr1->send("<000100123456789>");
		ASIO2_CHECK(!asio2::get_last_error());
		ASIO2_CHECK(sent_bytes == std::strlen("<000100123456789>"));

		auto session_ptr2 = server.find_session_if([session_key](std::shared_ptr<asio2::ws_session>& session_ptr)
		{
			if (session_ptr->hash_key() == session_key)
				return true;
			return false;
		});
		ASIO2_CHECK(session_ptr1.get() == session_ptr2.get());

		bool find_session_key = false;
		server.foreach_session([session_ptr1, session_key, &find_session_key]
		(std::shared_ptr<asio2::ws_session>& session_ptr) mutable
		{
			if (session_ptr.get() == session_ptr1.get())
			{
				ASIO2_CHECK(session_key == session_ptr->hash_key());
				find_session_key = true;
			}

			std::size_t sent_bytes = session_ptr->send("<000100123456789>");
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(sent_bytes == std::strlen("<000100123456789>"));
		});

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

		server_start_ret = server.start("127.0.0.1", 18039);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		client_init_counter = 0;
		client_connect_counter = 0;
		client_disconnect_counter = 0;

		for (int i = 0; i < test_client_count; i++)
		{
			bool client_start_ret = clients[i]->async_start("127.0.0.1", 18039, "/admin");
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

	// test auto reconnect
	{
		struct ext_data
		{
			int client_init_counter = 0;
			int client_connect_counter = 0;
			int client_disconnect_counter = 0;
			std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();
		};

		std::vector<std::shared_ptr<asio2::ws_client>> clients;
		for (int i = 0; i < test_client_count; i++)
		{
			auto iter = clients.emplace_back(std::make_shared<asio2::ws_client>());

			asio2::ws_client& client = *iter;

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

			bool client_start_ret = client.async_start("127.0.0.1", 18039);

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

			bool client_start_ret = clients[i]->start("127.0.0.1", 18039, "/admin");

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
		
		asio2::ws_server server;
		std::size_t session_key = std::size_t(std::rand() % test_client_count);
		std::atomic<int> server_recv_counter = 0;
		server.bind_recv([&](std::shared_ptr<asio2::ws_session> & session_ptr, std::string_view data)
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

			http::request<http::string_body> req;
			req.body() = "req";
			req.prepare_payload();

			session_ptr->async_send(req);
			session_ptr->send(std::move(req));

			http::response<http::buffer_body> res;
			res.body().data = (void*)"res";
			res.body().size = 3;
			res.body().more = false;

			session_ptr->async_send(res);
			session_ptr->send(std::move(res));

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
				ASIO2_CHECK(server.get_listen_port() == 18039);
				ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_port() == 18039);
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
			ASIO2_CHECK(session_ptr->local_port() == 18039);
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
			ASIO2_CHECK(server.get_listen_port() == 18039);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18039);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});

		bool server_start_ret = server.start("127.0.0.1", 18039);

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

			asio2::ws_client& client = *clients[i];

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

				http::request<http::string_body> req;
				req.body() = "req";
				req.prepare_payload();

				client.async_send(req);
				client.send(std::move(req));

				http::response<http::buffer_body> res;
				res.body().data = (void*)"res";
				res.body().size = 3;
				res.body().more = false;

				client.async_send(res);
				client.send(std::move(res));
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

			bool client_start_ret = clients[i]->async_start("127.0.0.1", 18039);

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

	ASIO2_TEST_END_LOOP;
}


ASIO2_TEST_SUITE
(
	"websocket",
	ASIO2_TEST_CASE(websocket_test)
)
