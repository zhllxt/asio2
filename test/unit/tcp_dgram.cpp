#include "unit_test.hpp"
#include <fmt/format.h>
#include <asio2/tcp/tcp_server.hpp>
#include <asio2/tcp/tcp_client.hpp>

static std::string_view chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

void tcp_dgram_test()
{
	ASIO2_TEST_BEGIN_LOOP(test_loop_times);

	// illegal data 254
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

				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
				ASIO2_CHECK(server.get_listen_port() == 18027);
				ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_port() == 18027);
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
			ASIO2_CHECK(!server.find_session(session_ptr->hash_key()));
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18027);
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
			ASIO2_CHECK(session_ptr->local_port() == 18027);
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
			ASIO2_CHECK(server.get_listen_port() == 18027);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18027);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});

		bool server_start_ret = server.start("127.0.0.1", 18027, asio2::use_dgram);

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
				ASIO2_CHECK(client.get_remote_port() == 18027);

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
				//client.async_send(data);
			});

			bool client_start_ret = client.async_start("127.0.0.1", 18027, asio2::use_dgram);

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
			auto& clt = clients[i];
			clients[i]->post([&clt]()
			{
				std::vector<std::uint8_t> msg;

				int len = 200 + (std::rand() % 200);

				msg.resize(len);

				for (int i = 0; i < len; i++)
				{
					msg[i] = std::uint8_t(std::rand() % 0xff);
				}

				msg[0] = std::uint8_t(254);
				if (asio2::detail::is_little_endian())
				{
					msg[1] = std::uint8_t(253);
					msg[2] = std::uint8_t(0);
				}
				else
				{
					msg[1] = std::uint8_t(0);
					msg[2] = std::uint8_t(253);
				}
				msg[3] = std::uint8_t(0);
				msg[4] = std::uint8_t(0);
				msg[5] = std::uint8_t(0);
				msg[6] = std::uint8_t(0);
				msg[7] = std::uint8_t(0);
				msg[8] = std::uint8_t(0);

				asio::write(clt->socket(), asio::buffer(msg));
			});
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
		ASIO2_CHECK_VALUE(server_recv_counter.load(), server_recv_counter == 0);
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

		server_start_ret = server.start("127.0.0.1", 18027, asio2::use_dgram);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		client_init_counter = 0;
		client_connect_counter = 0;
		client_disconnect_counter = 0;

		for (int i = 0; i < test_client_count; i++)
		{
			bool client_start_ret = clients[i]->async_start("127.0.0.1", 18027, asio2::use_dgram);
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
			auto& clt = clients[i];
			clients[i]->post([&clt]()
			{
				std::vector<std::uint8_t> msg;

				int len = 200 + (std::rand() % 200);

				msg.resize(len);

				for (int i = 0; i < len; i++)
				{
					msg[i] = std::uint8_t(std::rand() % 0xff);
				}

				msg[0] = std::uint8_t(254);
				if (asio2::detail::is_little_endian())
				{
					msg[1] = std::uint8_t(253);
					msg[2] = std::uint8_t(0);
				}
				else
				{
					msg[1] = std::uint8_t(0);
					msg[2] = std::uint8_t(253);
				}
				msg[3] = std::uint8_t(0);
				msg[4] = std::uint8_t(0);
				msg[5] = std::uint8_t(0);
				msg[6] = std::uint8_t(0);
				msg[7] = std::uint8_t(0);
				msg[8] = std::uint8_t(0);

				asio::write(clt->socket(), asio::buffer(msg));
			});
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
		ASIO2_CHECK_VALUE(server_recv_counter.load(), server_recv_counter == 0);
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

	// illegal data 255
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

				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
				ASIO2_CHECK(server.get_listen_port() == 18027);
				ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_port() == 18027);
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
			ASIO2_CHECK(!server.find_session(session_ptr->hash_key()));
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18027);
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
			ASIO2_CHECK(session_ptr->local_port() == 18027);
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
			ASIO2_CHECK(server.get_listen_port() == 18027);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18027);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});

		bool server_start_ret = server.start("127.0.0.1", 18027, asio2::use_dgram);

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
				ASIO2_CHECK(client.get_remote_port() == 18027);

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
				//client.async_send(data);
			});

			bool client_start_ret = client.async_start("127.0.0.1", 18027, asio2::use_dgram);

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
			auto& clt = clients[i];
			clients[i]->post([&clt]()
			{
				std::vector<std::uint8_t> msg;

				int len = 200 + (std::rand() % 200);

				msg.resize(len);

				for (int i = 0; i < len; i++)
				{
					msg[i] = std::uint8_t(std::rand() % 0xff);
				}

				msg[0] = std::uint8_t(255);
				if (asio2::detail::is_little_endian())
				{
					msg[1] = std::uint8_t(0xff);
					msg[2] = std::uint8_t(0xff);
					msg[3] = std::uint8_t(0);
					msg[4] = std::uint8_t(0);
					msg[5] = std::uint8_t(0);
					msg[6] = std::uint8_t(0);
					msg[7] = std::uint8_t(0);
					msg[8] = std::uint8_t(0);
				}
				else
				{
					msg[1] = std::uint8_t(0);
					msg[2] = std::uint8_t(0);
					msg[3] = std::uint8_t(0);
					msg[4] = std::uint8_t(0);
					msg[5] = std::uint8_t(0);
					msg[6] = std::uint8_t(0);
					msg[7] = std::uint8_t(0xff);
					msg[8] = std::uint8_t(0xff);
				}

				asio::write(clt->socket(), asio::buffer(msg));
			});
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
		ASIO2_CHECK_VALUE(server_recv_counter.load(), server_recv_counter == 0);
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

		server_start_ret = server.start("127.0.0.1", 18027, asio2::use_dgram);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		client_init_counter = 0;
		client_connect_counter = 0;
		client_disconnect_counter = 0;

		for (int i = 0; i < test_client_count; i++)
		{
			bool client_start_ret = clients[i]->async_start("127.0.0.1", 18027, asio2::use_dgram);
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
			auto& clt = clients[i];
			clients[i]->post([&clt]()
			{
				std::vector<std::uint8_t> msg;

				int len = 200 + (std::rand() % 200);

				msg.resize(len);

				for (int i = 0; i < len; i++)
				{
					msg[i] = std::uint8_t(std::rand() % 0xff);
				}

				msg[0] = std::uint8_t(255);
				if (asio2::detail::is_little_endian())
				{
					msg[1] = std::uint8_t(0xff);
					msg[2] = std::uint8_t(0xff);
					msg[3] = std::uint8_t(0);
					msg[4] = std::uint8_t(0);
					msg[5] = std::uint8_t(0);
					msg[6] = std::uint8_t(0);
					msg[7] = std::uint8_t(0);
					msg[8] = std::uint8_t(0);
				}
				else
				{
					msg[1] = std::uint8_t(0);
					msg[2] = std::uint8_t(0);
					msg[3] = std::uint8_t(0);
					msg[4] = std::uint8_t(0);
					msg[5] = std::uint8_t(0);
					msg[6] = std::uint8_t(0);
					msg[7] = std::uint8_t(0xff);
					msg[8] = std::uint8_t(0xff);
				}

				asio::write(clt->socket(), asio::buffer(msg));
			});
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
		ASIO2_CHECK_VALUE(server_recv_counter.load(), server_recv_counter == 0);
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

	// test datagram send recv
	{
		struct ext_data
		{
			int num = 1;
		};

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

			ext_data& ex = session_ptr->get_user_data<ext_data&>();

			std::string msg = fmt::format("{:05d},", ex.num);

			for (int i = 0; i < ex.num; i++)
			{
				msg += chars;
			}

			ASIO2_CHECK(msg == data);

			ex.num++;

			session_ptr->async_send(std::move(msg));

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
				ASIO2_CHECK(server.get_listen_port() == 18027);
				ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_port() == 18027);
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
			ASIO2_CHECK(session_ptr->local_port() == 18027);
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
			ASIO2_CHECK(session_ptr->local_port() == 18027);
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
			ASIO2_CHECK(server.get_listen_port() == 18027);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18027);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});

		bool server_start_ret = server.start("127.0.0.1", 18027, asio2::use_dgram);

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
				ASIO2_CHECK(client.get_remote_port() == 18027);

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

				std::string msg = fmt::format("{:05d},", ex->num);

				for (int i = 0; i < ex->num; i++)
				{
					msg += chars;
				}

				ASIO2_CHECK(msg == data);

				ex->num++;

				msg = fmt::format("{:05d},", ex->num);

				for (int i = 0; i < ex->num; i++)
				{
					msg += chars;
				}

				client.async_send(std::move(msg));
			});

			bool client_start_ret = client.async_start("127.0.0.1", 18027, asio2::use_dgram);

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

			clients[i]->async_send(std::move(msg));
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

		server_start_ret = server.start("127.0.0.1", 18027, asio2::use_dgram);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		client_init_counter = 0;
		client_connect_counter = 0;
		client_disconnect_counter = 0;

		for (int i = 0; i < test_client_count; i++)
		{
			bool client_start_ret = clients[i]->async_start("127.0.0.1", 18027, asio2::use_dgram);
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

			clients[i]->async_send(std::move(msg));
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

	// test auto reconnect by async_start in bind_connect
	{
		struct ext_data
		{
			int num = 1;
			int connect_times = 1;
		};

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

			ext_data& ex = session_ptr->get_user_data<ext_data&>();

			std::string msg = fmt::format("{:05d},", ex.num);

			for (int i = 0; i < ex.num; i++)
			{
				msg += chars;
			}

			ASIO2_CHECK(msg == data);

			ex.num++;

			session_ptr->async_send(std::move(msg));

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
				ASIO2_CHECK(server.get_listen_port() == 18027);
				ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_port() == 18027);
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
			ASIO2_CHECK(session_ptr->local_port() == 18027);
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
			ASIO2_CHECK(session_ptr->local_port() == 18027);
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
			ASIO2_CHECK(server.get_listen_port() == 18027);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18027);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});

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
				if (!asio2::get_last_error())
				{
					ASIO2_CHECK(client.get_local_address() == "127.0.0.1");
					ASIO2_CHECK(client.get_remote_address() == "127.0.0.1");
					ASIO2_CHECK(client.get_remote_port() == 18027);
				}

				if (asio2::get_last_error())
				{
					ext_data* ex = client.get_user_data<ext_data*>();
					ex->connect_times++;

					client.async_start("127.0.0.1", 18027, asio2::use_dgram);
				}
				else
				{
					client_connect_counter++;
				}
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

				//fmt::print("recv : {}\n", data.substr(0, 5));

				ext_data* ex = client.get_user_data<ext_data*>();

				if (ex->num > 20)
				{
					client.stop();
					return;
				}

				std::string msg = fmt::format("{:05d},", ex->num);

				for (int i = 0; i < ex->num; i++)
				{
					msg += chars;
				}

				ASIO2_CHECK(msg == data);

				ex->num++;

				msg = fmt::format("{:05d},", ex->num);

				for (int i = 0; i < ex->num; i++)
				{
					msg += chars;
				}

				client.async_send(std::move(msg));
			});

			ext_data ex;
			ex.num = 1;
			ex.connect_times = 1;

			client.set_user_data(ex);

			bool client_start_ret = client.async_start("127.0.0.1", 18027, asio2::use_dgram);

			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			while (ex.connect_times < 3)
			{
				ASIO2_TEST_WAIT_CHECK();
			}
		}

		bool server_start_ret = server.start("127.0.0.1", 18027, asio2::use_dgram);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

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

		int total_connect_times = 0;
		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			total_connect_times += ex.connect_times;
		}

		ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == total_connect_times);
		ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == test_client_count);

		ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == test_client_count);
		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			std::string msg = fmt::format("{:05d},", 1);

			msg += chars;

			clients[i]->async_send(std::move(msg));
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

		// stop the client or not is ok both.
		if (loop % 3 == 0)
		{
			for (int i = 0; i < test_client_count; i++)
			{
				clients[i]->stop();
				ASIO2_CHECK(clients[i]->is_stopped());
			}
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

		client_init_counter = 0;
		client_connect_counter = 0;
		client_disconnect_counter = 0;

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data ex;
			ex.num = 1;
			ex.connect_times = 1;

			clients[i]->set_user_data(ex);

			bool client_start_ret = clients[i]->async_start("127.0.0.1", 18027, asio2::use_dgram);
			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			while (ex.connect_times < 3)
			{
				ASIO2_TEST_WAIT_CHECK();
			}
		}

		server_start_ret = server.start("127.0.0.1", 18027, asio2::use_dgram);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		while (server.get_session_count() < std::size_t(test_client_count))
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(server.get_session_count(), server.get_session_count() == std::size_t(test_client_count));

		while (client_connect_counter < test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		total_connect_times = 0;
		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			total_connect_times += ex.connect_times;
		}

		ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == total_connect_times);
		ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == test_client_count);

		ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == test_client_count);
		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			std::string msg = fmt::format("{:05d},", 1);

			msg += chars;

			clients[i]->async_send(std::move(msg));
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

		// stop the client or not is ok both.
		if (loop % 2 == 0)
		{
			for (int i = 0; i < test_client_count; i++)
			{
				clients[i]->stop();
				ASIO2_CHECK(clients[i]->is_stopped());
			}
		}

		server.stop();
		ASIO2_CHECK(server.is_stopped());

		ASIO2_CHECK_VALUE(server_stop_counter      .load(), server_stop_counter       == 1);
	}

	// test auto reconnect by async_start in bind_disconnect
	{
		struct ext_data
		{
			int num = 1;
			int connect_times = 1;
			bool prepare_exit = false;
		};

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

			ext_data& ex = session_ptr->get_user_data<ext_data&>();

			if (ex.num > 20)
			{
				session_ptr->stop();
				return;
			}

			std::string msg = fmt::format("{:05d},", ex.num);

			for (int i = 0; i < ex.num; i++)
			{
				msg += chars;
			}

			ASIO2_CHECK(msg == data);

			ex.num++;

			session_ptr->async_send(std::move(msg));

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
				ASIO2_CHECK(server.get_listen_port() == 18027);
				ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_port() == 18027);
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
			ASIO2_CHECK(session_ptr->local_port() == 18027);
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
			ASIO2_CHECK(session_ptr->local_port() == 18027);
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
			ASIO2_CHECK(server.get_listen_port() == 18027);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18027);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});

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
				if (!asio2::get_last_error())
				{
					ASIO2_CHECK(client.get_local_address() == "127.0.0.1");
					ASIO2_CHECK(client.get_remote_address() == "127.0.0.1");
					ASIO2_CHECK(client.get_remote_port() == 18027);
				}

				if (asio2::get_last_error())
				{
					ext_data* ex = client.get_user_data<ext_data*>();
					ex->connect_times++;

					if (ex->prepare_exit == false)
						client.async_start("127.0.0.1", 18027, asio2::use_dgram);
				}
				else
				{
					client_connect_counter++;
				}
			});
			client.bind_disconnect([&]()
			{
				client_disconnect_counter++;

				ASIO2_CHECK(asio2::get_last_error());
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());

				ext_data* ex = client.get_user_data<ext_data*>();
				if (ex->prepare_exit == false)
					client.async_start("127.0.0.1", 18027, asio2::use_dgram);
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

				std::string msg = fmt::format("{:05d},", ex->num);

				for (int i = 0; i < ex->num; i++)
				{
					msg += chars;
				}

				ASIO2_CHECK(msg == data);

				ex->num++;

				msg = fmt::format("{:05d},", ex->num);

				for (int i = 0; i < ex->num; i++)
				{
					msg += chars;
				}

				client.async_send(std::move(msg));
			});

			ext_data ex;
			ex.num = 1;
			ex.connect_times = 1;

			client.set_user_data(ex);

			bool client_start_ret = client.async_start("127.0.0.1", 18027, asio2::use_dgram);

			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			while (ex.connect_times < 3)
			{
				ASIO2_TEST_WAIT_CHECK();
			}
		}

		bool server_start_ret = server.start("127.0.0.1", 18027, asio2::use_dgram);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter.load(), server_init_counter == 1);
		ASIO2_CHECK_VALUE(server_start_counter.load(), server_start_counter == 1);

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

		int total_connect_times = 0;
		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			total_connect_times += ex.connect_times;
		}

		ASIO2_CHECK_VALUE(client_init_counter.load(), client_init_counter == total_connect_times);
		ASIO2_CHECK_VALUE(client_connect_counter.load(), client_connect_counter == test_client_count);

		ASIO2_CHECK_VALUE(server_accept_counter.load(), server_accept_counter == test_client_count);
		ASIO2_CHECK_VALUE(server_connect_counter.load(), server_connect_counter == test_client_count);

		client_init_counter = 0;
		client_connect_counter = 0;
		server_accept_counter = 0;
		server_connect_counter = 0;

		for (int i = 0; i < test_client_count; i++)
		{
			std::string msg = fmt::format("{:05d},", 1);

			msg += chars;

			clients[i]->async_send(std::move(msg));
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

		while (server.get_session_count() < std::size_t(test_client_count))
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		session_count = server.get_session_count();
		ASIO2_CHECK_VALUE(session_count, session_count == std::size_t(test_client_count));

		while (client_connect_counter < test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(server_disconnect_counter.load(), server_disconnect_counter == test_client_count);
		ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data* ex = clients[i]->get_user_data<ext_data*>();
			ex->prepare_exit = true;
			clients[i]->stop();
			ASIO2_CHECK(clients[i]->is_stopped());
		}

		server.stop();
		ASIO2_CHECK(server.is_stopped());

		ASIO2_CHECK_VALUE(server_stop_counter.load(), server_stop_counter == 1);
	}

	// test stop start in the io_context thread.
	for (int x = 0; x < test_loop_times / 50; x++)
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

				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
				ASIO2_CHECK(server.get_listen_port() == 18027);
				ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_port() == 18027);
				ASIO2_CHECK(server.io().running_in_this_thread());
				ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
			}
		});
		std::atomic<int> server_connect_counter = 0;
		server.bind_connect([&](auto & session_ptr)
		{
			server_connect_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(!server.find_session(session_ptr->hash_key()));
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18027);
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
			ASIO2_CHECK(session_ptr->local_port() == 18027);
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
			ASIO2_CHECK(server.get_listen_port() == 18027);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18027);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});

		bool server_start_ret = server.start("127.0.0.1", 18027, asio2::use_dgram);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

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
				if (!asio2::get_last_error())
				{
					ASIO2_CHECK(client.get_local_address() == "127.0.0.1");
					ASIO2_CHECK(client.get_remote_address() == "127.0.0.1");
					ASIO2_CHECK(client.get_remote_port() == 18027);
				}

				client_connect_counter++;

				std::string str;
				str += '#';
				int len = 10 + (std::rand() % 100);
				for (int i = 0; i < len; i++)
				{
					str += (char)((std::rand() % 26) + 'a');
				}

				client.async_send(std::move(str));
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

				std::string str;
				str += '#';
				int len = 10 + (std::rand() % 100);
				for (int i = 0; i < len; i++)
				{
					str += (char)((std::rand() % 26) + 'a');
				}

				client.async_send(std::move(str));
			});

			bool client_start_ret = client.async_start("127.0.0.1", 18027, asio2::use_dgram);

			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
		}

		ASIO2_CHECK_VALUE(server_init_counter.load(), server_init_counter == 1);
		ASIO2_CHECK_VALUE(server_start_counter.load(), server_start_counter == 1);

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

		ASIO2_CHECK_VALUE(client_init_counter.load(), client_init_counter == test_client_count);
		ASIO2_CHECK_VALUE(client_connect_counter.load(), client_connect_counter == test_client_count);

		ASIO2_CHECK_VALUE(server_accept_counter.load(), server_accept_counter == test_client_count);
		ASIO2_CHECK_VALUE(server_connect_counter.load(), server_connect_counter == test_client_count);

		std::this_thread::sleep_for(std::chrono::milliseconds(25 + std::rand() % 25));

		for (int i = 0; i < test_client_count; i++)
		{
			auto& client = *clients[i];
			client.post([&]()
			{
				client.stop();

				if (loop % 2 == 0)
					client.start("127.0.0.1", 18027, asio2::use_dgram);
				else
					client.async_start("127.0.0.1", 18027, asio2::use_dgram);
			});
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(25 + std::rand() % 25));

		std::vector<int> exits;
		exits.resize(test_client_count, 0);

		for (int i = 0; i < test_client_count; i++)
		{
			std::thread([&,i]()
			{
				clients[i]->stop();
				ASIO2_CHECK(clients[i]->is_stopped());
				exits[i] = 1;
			}).detach();
		}

		for (int i = 0; i < test_client_count; i++)
		{
			while (exits[i] != 1)
			{
				ASIO2_TEST_WAIT_CHECK(clients[i]->get_pending_event_count());
			}
		}

		server.stop();
		ASIO2_CHECK(server.is_stopped());

		ASIO2_CHECK_VALUE(server_stop_counter.load(), server_stop_counter == 1);
	}

	ASIO2_TEST_END_LOOP;
}


ASIO2_TEST_SUITE
(
	"tcp_dgram",
	ASIO2_TEST_CASE(tcp_dgram_test)
)
