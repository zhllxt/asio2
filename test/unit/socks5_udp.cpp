#include "unit_test.hpp"
#include <unordered_set>
#include <asio2/external/fmt.hpp>
#include <asio2/udp/udp_server.hpp>
#include <asio2/udp/udp_client.hpp>
#include <asio2/udp/udp_cast.hpp>
#include <asio2/proxy/socks5_server.hpp>

static std::string_view chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

void socks5_udp_test()
{
	ASIO2_TEST_BEGIN_LOOP(test_loop_times);

	// test udp socks5 with password
	{
		asio2::socks5_server s5server;

		s5server.bind_accept([&](std::shared_ptr<asio2::socks5_session>& session_ptr)
		{
			socks5::options opts;
			//opts.set_methods(socks5::method::anonymous);
			opts.set_methods(socks5::method::password);
			opts.set_username("admin");
			opts.set_password("123456");
			// if the default username and password is wrong, then this auth callback will be called.
			opts.set_auth_callback([](const std::string& username, const std::string& password)
			{
				return username == "admin" && password == "88888888";
			});
			session_ptr->set_socks5_options(std::move(opts));
		}).bind_connect([&](auto & session_ptr)
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(s5server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(s5server.get_listen_port() == 20808);
			ASIO2_CHECK(s5server.io().running_in_this_thread());
			ASIO2_CHECK(s5server.iopool().get(0)->running_in_this_thread());
			asio2::ignore_unused(session_ptr);

		}).bind_disconnect([&](auto & session_ptr)
		{
			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(s5server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(s5server.get_listen_port() == 20808);
			ASIO2_CHECK(s5server.io().running_in_this_thread());
			ASIO2_CHECK(s5server.iopool().get(0)->running_in_this_thread());
			asio2::ignore_unused(session_ptr);

		}).bind_socks5_handshake([&](auto & session_ptr)
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(s5server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(s5server.get_listen_port() == 20808);
			ASIO2_CHECK(s5server.io().running_in_this_thread());
			ASIO2_CHECK(s5server.iopool().get(0)->running_in_this_thread());
			asio2::ignore_unused(session_ptr);
		}).bind_start([&]()
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(s5server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(s5server.get_listen_port() == 20808);
			ASIO2_CHECK(s5server.io().running_in_this_thread());
			ASIO2_CHECK(s5server.iopool().get(0)->running_in_this_thread());
		}).bind_stop([&]()
		{
			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(s5server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(s5server.get_listen_port() == 20808);
			ASIO2_CHECK(s5server.io().running_in_this_thread());
			ASIO2_CHECK(s5server.iopool().get(0)->running_in_this_thread());
		});

		s5server.start("127.0.0.1", 20808);

		socks5::option<socks5::method::password> sock5_opt{"127.0.0.1", "20808", "admin", "123456"};

		struct ext_data
		{
			int num = 1;
		};

		asio2::udp_server server;
		std::atomic<int> server_recv_counter = 0;
		server.bind_recv([&](std::shared_ptr<asio2::udp_session> & session_ptr, std::string_view data)
		{
			server_recv_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(session_ptr->is_started());

			ext_data& ex = session_ptr->get_user_data<ext_data&>();

			std::string msg = fmt::format("{:05d},", ex.num);

			for (int i = 0; i < ex.num; i++)
			{
				msg += chars[i];
			}

			ASIO2_CHECK(msg == data);

			ex.num++;

			session_ptr->async_send(std::move(msg));

			ASIO2_CHECK(session_ptr->io().running_in_this_thread());
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
			ASIO2_CHECK(session_ptr->local_port() == 18040);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_disconnect_counter = 0;
		server.bind_disconnect([&](auto & session_ptr)
		{
			server_disconnect_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(!server.find_session(session_ptr->hash_key()));
			ASIO2_CHECK(session_ptr->socket().is_open());
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18040);
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
			ASIO2_CHECK(server.get_listen_port() == 18040);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18040);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});

		bool server_start_ret = server.start("127.0.0.1", 18040);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		std::vector<std::shared_ptr<asio2::udp_client>> clients;
		std::atomic<int> client_init_counter = 0;
		std::atomic<int> client_connect_counter = 0;
		std::atomic<int> client_disconnect_counter = 0;
		for (int i = 0; i < test_client_count; i++)
		{
			auto iter = clients.emplace_back(std::make_shared<asio2::udp_client>());

			asio2::udp_client& client = *iter;

			// disable auto reconnect, default reconnect option is "enable"
			client.set_auto_reconnect(false);

			client.bind_init([&]()
			{
				client_init_counter++;

				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.is_reuse_address());

				client.set_no_delay(true);
			});
			client.bind_connect([&]()
			{
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.get_local_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_address() == "127.0.0.1");

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
					msg += chars[i];
				}

				ASIO2_CHECK(msg == data);

				ex->num++;

				msg = fmt::format("{:05d},", ex->num);

				for (int i = 0; i < ex->num; i++)
				{
					msg += chars[i];
				}

				client.async_send(std::move(msg));
			});

			bool client_start_ret = client.async_start("127.0.0.1", 18040, sock5_opt);

			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
		}

		while (client_connect_counter < test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == test_client_count);
		ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			std::string msg = fmt::format("{:05d},", 1);

			msg += chars[0];

			clients[i]->async_send(std::move(msg));
		}

		while (server.get_session_count() < std::size_t(test_client_count))
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		auto session_count = server.get_session_count();
		ASIO2_CHECK_VALUE(session_count, session_count == std::size_t(test_client_count));
		ASIO2_CHECK_VALUE(server_connect_counter.load(), server_connect_counter == test_client_count);

		while (client_disconnect_counter != test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

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

	// test udp socks5 with password and socks5::options
	{
		asio2::socks5_server s5server;

		s5server.bind_accept([&](std::shared_ptr<asio2::socks5_session>& session_ptr)
		{
			socks5::options opts;
			//opts.set_methods(socks5::method::anonymous);
			opts.set_methods(socks5::method::password);
			opts.set_username("admin");
			opts.set_password("123456");
			// if the default username and password is wrong, then this auth callback will be called.
			opts.set_auth_callback([](const std::string& username, const std::string& password)
			{
				return username == "admin" && password == "88888888";
			});
			session_ptr->set_socks5_options(std::move(opts));
		}).bind_connect([&](auto & session_ptr)
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(s5server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(s5server.get_listen_port() == 20808);
			ASIO2_CHECK(s5server.io().running_in_this_thread());
			ASIO2_CHECK(s5server.iopool().get(0)->running_in_this_thread());
			asio2::ignore_unused(session_ptr);

		}).bind_disconnect([&](auto & session_ptr)
		{
			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(s5server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(s5server.get_listen_port() == 20808);
			ASIO2_CHECK(s5server.io().running_in_this_thread());
			ASIO2_CHECK(s5server.iopool().get(0)->running_in_this_thread());
			asio2::ignore_unused(session_ptr);

		}).bind_socks5_handshake([&](auto & session_ptr)
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(s5server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(s5server.get_listen_port() == 20808);
			ASIO2_CHECK(s5server.io().running_in_this_thread());
			ASIO2_CHECK(s5server.iopool().get(0)->running_in_this_thread());
			asio2::ignore_unused(session_ptr);
		}).bind_start([&]()
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(s5server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(s5server.get_listen_port() == 20808);
			ASIO2_CHECK(s5server.io().running_in_this_thread());
			ASIO2_CHECK(s5server.iopool().get(0)->running_in_this_thread());
		}).bind_stop([&]()
		{
			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(s5server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(s5server.get_listen_port() == 20808);
			ASIO2_CHECK(s5server.io().running_in_this_thread());
			ASIO2_CHECK(s5server.iopool().get(0)->running_in_this_thread());
		});

		s5server.start("127.0.0.1", 20808);

		socks5::options sock5_opt{"127.0.0.1", "20808", "admin", "123456"};

		struct ext_data
		{
			int num = 1;
		};

		asio2::udp_server server;
		std::atomic<int> server_recv_counter = 0;
		server.bind_recv([&](std::shared_ptr<asio2::udp_session> & session_ptr, std::string_view data)
		{
			server_recv_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(session_ptr->is_started());

			ext_data& ex = session_ptr->get_user_data<ext_data&>();

			std::string msg = fmt::format("{:05d},", ex.num);

			for (int i = 0; i < ex.num; i++)
			{
				msg += chars[i];
			}

			ASIO2_CHECK(msg == data);

			ex.num++;

			session_ptr->async_send(std::move(msg));

			ASIO2_CHECK(session_ptr->io().running_in_this_thread());
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
			ASIO2_CHECK(session_ptr->local_port() == 18040);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_disconnect_counter = 0;
		server.bind_disconnect([&](auto & session_ptr)
		{
			server_disconnect_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(!server.find_session(session_ptr->hash_key()));
			ASIO2_CHECK(session_ptr->socket().is_open());
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18040);
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
			ASIO2_CHECK(server.get_listen_port() == 18040);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18040);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});

		bool server_start_ret = server.start("127.0.0.1", 18040);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		std::vector<std::shared_ptr<asio2::udp_client>> clients;
		std::atomic<int> client_init_counter = 0;
		std::atomic<int> client_connect_counter = 0;
		std::atomic<int> client_disconnect_counter = 0;
		for (int i = 0; i < test_client_count; i++)
		{
			auto iter = clients.emplace_back(std::make_shared<asio2::udp_client>());

			asio2::udp_client& client = *iter;

			// disable auto reconnect, default reconnect option is "enable"
			client.set_auto_reconnect(false);

			client.bind_init([&]()
			{
				client_init_counter++;

				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.is_reuse_address());

				client.set_no_delay(true);
			});
			client.bind_connect([&]()
			{
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.get_local_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_address() == "127.0.0.1");

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
					msg += chars[i];
				}

				ASIO2_CHECK(msg == data);

				ex->num++;

				msg = fmt::format("{:05d},", ex->num);

				for (int i = 0; i < ex->num; i++)
				{
					msg += chars[i];
				}

				client.async_send(std::move(msg));
			});

			bool client_start_ret = client.async_start("127.0.0.1", 18040, sock5_opt);

			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
		}

		while (client_connect_counter < test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == test_client_count);
		ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			std::string msg = fmt::format("{:05d},", 1);

			msg += chars[0];

			clients[i]->async_send(std::move(msg));
		}

		while (server.get_session_count() < std::size_t(test_client_count))
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		auto session_count = server.get_session_count();
		ASIO2_CHECK_VALUE(session_count, session_count == std::size_t(test_client_count));
		ASIO2_CHECK_VALUE(server_connect_counter.load(), server_connect_counter == test_client_count);

		while (client_disconnect_counter != test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

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

	// test udp socks5 with password auth
	{
		asio2::socks5_server s5server;

		s5server.bind_accept([&](std::shared_ptr<asio2::socks5_session>& session_ptr)
		{
			socks5::options opts;
			//opts.set_methods(socks5::method::anonymous);
			opts.set_methods(socks5::method::password);
			opts.set_username("admin");
			opts.set_password("123456");
			// if the default username and password is wrong, then this auth callback will be called.
			opts.set_auth_callback([](const std::string& username, const std::string& password)
			{
				return username == "admin" && password == "88888888";
			});
			session_ptr->set_socks5_options(std::move(opts));
		}).bind_connect([&](auto & session_ptr)
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(s5server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(s5server.get_listen_port() == 20808);
			ASIO2_CHECK(s5server.io().running_in_this_thread());
			ASIO2_CHECK(s5server.iopool().get(0)->running_in_this_thread());
			asio2::ignore_unused(session_ptr);

		}).bind_disconnect([&](auto & session_ptr)
		{
			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(s5server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(s5server.get_listen_port() == 20808);
			ASIO2_CHECK(s5server.io().running_in_this_thread());
			ASIO2_CHECK(s5server.iopool().get(0)->running_in_this_thread());
			asio2::ignore_unused(session_ptr);

		}).bind_socks5_handshake([&](auto & session_ptr)
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(s5server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(s5server.get_listen_port() == 20808);
			ASIO2_CHECK(s5server.io().running_in_this_thread());
			ASIO2_CHECK(s5server.iopool().get(0)->running_in_this_thread());
			asio2::ignore_unused(session_ptr);
		}).bind_start([&]()
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(s5server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(s5server.get_listen_port() == 20808);
			ASIO2_CHECK(s5server.io().running_in_this_thread());
			ASIO2_CHECK(s5server.iopool().get(0)->running_in_this_thread());
		}).bind_stop([&]()
		{
			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(s5server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(s5server.get_listen_port() == 20808);
			ASIO2_CHECK(s5server.io().running_in_this_thread());
			ASIO2_CHECK(s5server.iopool().get(0)->running_in_this_thread());
		});

		s5server.start("127.0.0.1", 20808);

		socks5::option<socks5::method::password> sock5_opt{"127.0.0.1", "20808", "admin", "88888888"};

		struct ext_data
		{
			int num = 1;
		};

		asio2::udp_server server;
		std::atomic<int> server_recv_counter = 0;
		server.bind_recv([&](std::shared_ptr<asio2::udp_session> & session_ptr, std::string_view data)
		{
			server_recv_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(session_ptr->is_started());

			ext_data& ex = session_ptr->get_user_data<ext_data&>();

			std::string msg = fmt::format("{:05d},", ex.num);

			for (int i = 0; i < ex.num; i++)
			{
				msg += chars[i];
			}

			ASIO2_CHECK(msg == data);

			ex.num++;

			session_ptr->async_send(std::move(msg));

			ASIO2_CHECK(session_ptr->io().running_in_this_thread());
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
			ASIO2_CHECK(session_ptr->local_port() == 18040);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_disconnect_counter = 0;
		server.bind_disconnect([&](auto & session_ptr)
		{
			server_disconnect_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(!server.find_session(session_ptr->hash_key()));
			ASIO2_CHECK(session_ptr->socket().is_open());
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18040);
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
			ASIO2_CHECK(server.get_listen_port() == 18040);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18040);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});

		bool server_start_ret = server.start("127.0.0.1", 18040);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		std::vector<std::shared_ptr<asio2::udp_client>> clients;
		std::atomic<int> client_init_counter = 0;
		std::atomic<int> client_connect_counter = 0;
		std::atomic<int> client_disconnect_counter = 0;
		for (int i = 0; i < test_client_count; i++)
		{
			auto iter = clients.emplace_back(std::make_shared<asio2::udp_client>());

			asio2::udp_client& client = *iter;

			// disable auto reconnect, default reconnect option is "enable"
			client.set_auto_reconnect(false);

			client.bind_init([&]()
			{
				client_init_counter++;

				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.is_reuse_address());

				client.set_no_delay(true);
			});
			client.bind_connect([&]()
			{
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.get_local_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_address() == "127.0.0.1");

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
					msg += chars[i];
				}

				ASIO2_CHECK(msg == data);

				ex->num++;

				msg = fmt::format("{:05d},", ex->num);

				for (int i = 0; i < ex->num; i++)
				{
					msg += chars[i];
				}

				client.async_send(std::move(msg));
			});

			bool client_start_ret = client.async_start("127.0.0.1", 18040, sock5_opt);

			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
		}

		while (client_connect_counter < test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == test_client_count);
		ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			std::string msg = fmt::format("{:05d},", 1);

			msg += chars[0];

			clients[i]->async_send(std::move(msg));
		}

		while (server.get_session_count() < std::size_t(test_client_count))
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		auto session_count = server.get_session_count();
		ASIO2_CHECK_VALUE(session_count, session_count == std::size_t(test_client_count));
		ASIO2_CHECK_VALUE(server_connect_counter.load(), server_connect_counter == test_client_count);

		while (client_disconnect_counter != test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

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

	// test udp socks5 with wrong password
	{
		asio2::socks5_server s5server;

		std::atomic<int> s5_handshake_counter = 0;
		s5server.bind_accept([&](std::shared_ptr<asio2::socks5_session>& session_ptr)
		{
			socks5::options opts;
			//opts.set_methods(socks5::method::anonymous);
			opts.set_methods(socks5::method::password);
			opts.set_username("admin");
			opts.set_password("123456");
			// if the default username and password is wrong, then this auth callback will be called.
			opts.set_auth_callback([](const std::string& username, const std::string& password)
			{
				return username == "admin" && password == "88888888";
			});
			session_ptr->set_socks5_options(std::move(opts));
		}).bind_connect([&](auto & session_ptr)
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(s5server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(s5server.get_listen_port() == 20808);
			ASIO2_CHECK(s5server.io().running_in_this_thread());
			ASIO2_CHECK(s5server.iopool().get(0)->running_in_this_thread());
			asio2::ignore_unused(session_ptr);

		}).bind_disconnect([&](auto & session_ptr)
		{
			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(s5server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(s5server.get_listen_port() == 20808);
			ASIO2_CHECK(s5server.io().running_in_this_thread());
			ASIO2_CHECK(s5server.iopool().get(0)->running_in_this_thread());
			asio2::ignore_unused(session_ptr);

		}).bind_socks5_handshake([&](auto & session_ptr)
		{
			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(s5server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(s5server.get_listen_port() == 20808);
			ASIO2_CHECK(s5server.io().running_in_this_thread());
			ASIO2_CHECK(s5server.iopool().get(0)->running_in_this_thread());
			s5_handshake_counter++;
			asio2::ignore_unused(session_ptr);
		}).bind_start([&]()
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(s5server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(s5server.get_listen_port() == 20808);
			ASIO2_CHECK(s5server.io().running_in_this_thread());
			ASIO2_CHECK(s5server.iopool().get(0)->running_in_this_thread());
		}).bind_stop([&]()
		{
			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(s5server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(s5server.get_listen_port() == 20808);
			ASIO2_CHECK(s5server.io().running_in_this_thread());
			ASIO2_CHECK(s5server.iopool().get(0)->running_in_this_thread());
		});

		s5server.start("127.0.0.1", 20808);

		socks5::option<socks5::method::password> sock5_opt{"127.0.0.1", "20808", "admin", "12345"};

		struct ext_data
		{
			int num = 1;
		};

		asio2::udp_server server;
		std::atomic<int> server_recv_counter = 0;
		server.bind_recv([&](std::shared_ptr<asio2::udp_session> & session_ptr, std::string_view data)
		{
			server_recv_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(session_ptr->is_started());

			ext_data& ex = session_ptr->get_user_data<ext_data&>();

			std::string msg = fmt::format("{:05d},", ex.num);

			for (int i = 0; i < ex.num; i++)
			{
				msg += chars[i];
			}

			ASIO2_CHECK(msg == data);

			ex.num++;

			session_ptr->async_send(std::move(msg));

			ASIO2_CHECK(session_ptr->io().running_in_this_thread());
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
			ASIO2_CHECK(session_ptr->local_port() == 18040);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_disconnect_counter = 0;
		server.bind_disconnect([&](auto & session_ptr)
		{
			server_disconnect_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(!server.find_session(session_ptr->hash_key()));
			ASIO2_CHECK(session_ptr->socket().is_open());
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18040);
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
			ASIO2_CHECK(server.get_listen_port() == 18040);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18040);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});

		bool server_start_ret = server.start("127.0.0.1", 18040);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		std::vector<std::shared_ptr<asio2::udp_client>> clients;
		std::atomic<int> client_init_counter = 0;
		std::atomic<int> client_connect_counter = 0;
		std::atomic<int> client_disconnect_counter = 0;
		for (int i = 0; i < test_client_count; i++)
		{
			auto iter = clients.emplace_back(std::make_shared<asio2::udp_client>());

			asio2::udp_client& client = *iter;

			// disable auto reconnect, default reconnect option is "enable"
			client.set_auto_reconnect(false);

			client.bind_init([&]()
			{
				client_init_counter++;

				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.is_reuse_address());

				client.set_no_delay(true);
			});
			client.bind_connect([&]()
			{
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(asio2::get_last_error());
				ASIO2_CHECK(client.get_local_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_address() == "127.0.0.1");

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
					msg += chars[i];
				}

				ASIO2_CHECK(msg == data);

				ex->num++;

				msg = fmt::format("{:05d},", ex->num);

				for (int i = 0; i < ex->num; i++)
				{
					msg += chars[i];
				}

				client.async_send(std::move(msg));
			});

			bool client_start_ret = client.async_start("127.0.0.1", 18040, sock5_opt);

			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
		}

		while (client_connect_counter < test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == test_client_count);
		ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == test_client_count);
		ASIO2_CHECK_VALUE(s5_handshake_counter     .load(), s5_handshake_counter   == test_client_count);

		auto session_count = server.get_session_count();
		ASIO2_CHECK_VALUE(session_count, session_count == std::size_t(0));
		ASIO2_CHECK_VALUE(server_connect_counter.load(), server_connect_counter == 0);

		ASIO2_CHECK_VALUE(server_recv_counter.load(), server_recv_counter == 0);
		ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == 0);

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->stop();
			ASIO2_CHECK(clients[i]->is_stopped());
		}

		server.stop();
		ASIO2_CHECK(server.is_stopped());

		ASIO2_CHECK_VALUE(server_stop_counter      .load(), server_stop_counter       == 1);
	}

	// test udp socks5 with anonymous
	{
		asio2::socks5_server s5server;

		s5server.bind_accept([&](std::shared_ptr<asio2::socks5_session>& session_ptr)
		{
			socks5::options opts;
			opts.set_methods(socks5::method::anonymous);
			//opts.set_methods(socks5::method::password);
			//opts.set_username("admin");
			//opts.set_password("123456");
			// if the default username and password is wrong, then this auth callback will be called.
			//opts.set_auth_callback([](const std::string& username, const std::string& password)
			//{
			//	return username == "admin" && password == "88888888";
			//});
			session_ptr->set_socks5_options(std::move(opts));
		}).bind_connect([&](auto & session_ptr)
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(s5server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(s5server.get_listen_port() == 20808);
			ASIO2_CHECK(s5server.io().running_in_this_thread());
			ASIO2_CHECK(s5server.iopool().get(0)->running_in_this_thread());
			asio2::ignore_unused(session_ptr);

		}).bind_disconnect([&](auto & session_ptr)
		{
			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(s5server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(s5server.get_listen_port() == 20808);
			ASIO2_CHECK(s5server.io().running_in_this_thread());
			ASIO2_CHECK(s5server.iopool().get(0)->running_in_this_thread());
			asio2::ignore_unused(session_ptr);

		}).bind_socks5_handshake([&](auto & session_ptr)
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(s5server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(s5server.get_listen_port() == 20808);
			ASIO2_CHECK(s5server.io().running_in_this_thread());
			ASIO2_CHECK(s5server.iopool().get(0)->running_in_this_thread());
			asio2::ignore_unused(session_ptr);
		}).bind_start([&]()
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(s5server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(s5server.get_listen_port() == 20808);
			ASIO2_CHECK(s5server.io().running_in_this_thread());
			ASIO2_CHECK(s5server.iopool().get(0)->running_in_this_thread());
		}).bind_stop([&]()
		{
			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(s5server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(s5server.get_listen_port() == 20808);
			ASIO2_CHECK(s5server.io().running_in_this_thread());
			ASIO2_CHECK(s5server.iopool().get(0)->running_in_this_thread());
		});

		s5server.start("127.0.0.1", 20808);

		socks5::option<socks5::method::anonymous> sock5_opt{"127.0.0.1", "20808"};

		struct ext_data
		{
			int num = 1;
		};

		asio2::udp_server server;
		std::atomic<int> server_recv_counter = 0;
		server.bind_recv([&](std::shared_ptr<asio2::udp_session> & session_ptr, std::string_view data)
		{
			server_recv_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(session_ptr->is_started());

			ext_data& ex = session_ptr->get_user_data<ext_data&>();

			std::string msg = fmt::format("{:05d},", ex.num);

			for (int i = 0; i < ex.num; i++)
			{
				msg += chars[i];
			}

			ASIO2_CHECK(msg == data);

			ex.num++;

			session_ptr->async_send(std::move(msg));

			ASIO2_CHECK(session_ptr->io().running_in_this_thread());
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
			ASIO2_CHECK(session_ptr->local_port() == 18040);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_disconnect_counter = 0;
		server.bind_disconnect([&](auto & session_ptr)
		{
			server_disconnect_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(!server.find_session(session_ptr->hash_key()));
			ASIO2_CHECK(session_ptr->socket().is_open());
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18040);
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
			ASIO2_CHECK(server.get_listen_port() == 18040);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18040);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});

		bool server_start_ret = server.start("127.0.0.1", 18040);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		std::vector<std::shared_ptr<asio2::udp_client>> clients;
		std::atomic<int> client_init_counter = 0;
		std::atomic<int> client_connect_counter = 0;
		std::atomic<int> client_disconnect_counter = 0;
		for (int i = 0; i < test_client_count; i++)
		{
			auto iter = clients.emplace_back(std::make_shared<asio2::udp_client>());

			asio2::udp_client& client = *iter;

			// disable auto reconnect, default reconnect option is "enable"
			client.set_auto_reconnect(false);

			client.bind_init([&]()
			{
				client_init_counter++;

				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.is_reuse_address());

				client.set_no_delay(true);
			});
			client.bind_connect([&]()
			{
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.get_local_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_address() == "127.0.0.1");

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
					msg += chars[i];
				}

				ASIO2_CHECK(msg == data);

				ex->num++;

				msg = fmt::format("{:05d},", ex->num);

				for (int i = 0; i < ex->num; i++)
				{
					msg += chars[i];
				}

				client.async_send(std::move(msg));
			});

			bool client_start_ret = client.async_start("127.0.0.1", 18040, sock5_opt);

			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
		}

		while (client_connect_counter < test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == test_client_count);
		ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			std::string msg = fmt::format("{:05d},", 1);

			msg += chars[0];

			clients[i]->async_send(std::move(msg));
		}

		while (server.get_session_count() < std::size_t(test_client_count))
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		auto session_count = server.get_session_count();
		ASIO2_CHECK_VALUE(session_count, session_count == std::size_t(test_client_count));
		ASIO2_CHECK_VALUE(server_connect_counter.load(), server_connect_counter == test_client_count);

		while (client_disconnect_counter != test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

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

	// test udp socks5 with tcp channel
	{
		asio2::socks5_server s5server;

		s5server.bind_accept([&](std::shared_ptr<asio2::socks5_session>& session_ptr)
		{
			socks5::options opts;
			//opts.set_methods(socks5::method::anonymous);
			opts.set_methods(socks5::method::password);
			opts.set_username("admin");
			opts.set_password("123456");
			// if the default username and password is wrong, then this auth callback will be called.
			opts.set_auth_callback([](const std::string& username, const std::string& password)
			{
				return username == "admin" && password == "88888888";
			});
			session_ptr->set_socks5_options(std::move(opts));
		}).bind_connect([&](auto & session_ptr)
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(s5server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(s5server.get_listen_port() == 20808);
			ASIO2_CHECK(s5server.io().running_in_this_thread());
			ASIO2_CHECK(s5server.iopool().get(0)->running_in_this_thread());
			asio2::ignore_unused(session_ptr);

		}).bind_disconnect([&](auto & session_ptr)
		{
			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(s5server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(s5server.get_listen_port() == 20808);
			ASIO2_CHECK(s5server.io().running_in_this_thread());
			ASIO2_CHECK(s5server.iopool().get(0)->running_in_this_thread());
			asio2::ignore_unused(session_ptr);

		}).bind_socks5_handshake([&](auto & session_ptr)
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(s5server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(s5server.get_listen_port() == 20808);
			ASIO2_CHECK(s5server.io().running_in_this_thread());
			ASIO2_CHECK(s5server.iopool().get(0)->running_in_this_thread());
			asio2::ignore_unused(session_ptr);
		}).bind_start([&]()
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(s5server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(s5server.get_listen_port() == 20808);
			ASIO2_CHECK(s5server.io().running_in_this_thread());
			ASIO2_CHECK(s5server.iopool().get(0)->running_in_this_thread());
		}).bind_stop([&]()
		{
			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(s5server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(s5server.get_listen_port() == 20808);
			ASIO2_CHECK(s5server.io().running_in_this_thread());
			ASIO2_CHECK(s5server.iopool().get(0)->running_in_this_thread());
		});

		s5server.start("127.0.0.1", 20808);

		socks5::option<socks5::method::password> sock5_opt{"127.0.0.1", "20808", "admin", "123456"};

		struct ext_data
		{
			int num = 1;
		};

		asio2::udp_server server;
		std::atomic<int> server_recv_counter = 0;
		server.bind_recv([&](std::shared_ptr<asio2::udp_session> & session_ptr, std::string_view data)
		{
			server_recv_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(session_ptr->is_started());

			ext_data& ex = session_ptr->get_user_data<ext_data&>();

			std::string msg = fmt::format("{:05d},", ex.num);

			for (int i = 0; i < ex.num; i++)
			{
				msg += chars[i];
			}

			ASIO2_CHECK(msg == data);

			ex.num++;

			session_ptr->async_send(std::move(msg));

			ASIO2_CHECK(session_ptr->io().running_in_this_thread());
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
			ASIO2_CHECK(session_ptr->local_port() == 18040);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_disconnect_counter = 0;
		server.bind_disconnect([&](auto & session_ptr)
		{
			server_disconnect_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(!server.find_session(session_ptr->hash_key()));
			ASIO2_CHECK(session_ptr->socket().is_open());
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18040);
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
			ASIO2_CHECK(server.get_listen_port() == 18040);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18040);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});

		bool server_start_ret = server.start("127.0.0.1", 18040);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		std::vector<std::shared_ptr<asio2::udp_client>> clients;
		std::atomic<int> client_init_counter = 0;
		std::atomic<int> client_connect_counter = 0;
		std::atomic<int> client_disconnect_counter = 0;
		for (int i = 0; i < test_client_count; i++)
		{
			auto iter = clients.emplace_back(std::make_shared<asio2::udp_client>());

			asio2::udp_client& client = *iter;

			// disable auto reconnect, default reconnect option is "enable"
			client.set_auto_reconnect(false);

			client.bind_init([&]()
			{
				client_init_counter++;

				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.is_reuse_address());

				client.set_no_delay(true);
			});
			client.bind_connect([&]()
			{
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.get_local_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_address() == "127.0.0.1");

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

				ext_data* ex = client.get_user_data<ext_data*>();

				if (ex->num > 20)
				{
					client.stop();
					return;
				}

				std::string msg = fmt::format("{:05d},", ex->num);

				for (int i = 0; i < ex->num; i++)
				{
					msg += chars[i];
				}

				ASIO2_CHECK(msg == data);

				ex->num++;

				msg = fmt::format("{:05d},", ex->num);

				for (int i = 0; i < ex->num; i++)
				{
					msg += chars[i];
				}

				client.get_socks5_connection()->async_send(std::move(msg));
			});

			bool client_start_ret = client.async_start("127.0.0.1", 18040, sock5_opt);

			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
		}

		while (client_connect_counter < test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == test_client_count);
		ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			std::string msg = fmt::format("{:05d},", 1);

			msg += chars[0];

			clients[i]->get_socks5_connection()->async_send(std::move(msg));
		}

		while (server.get_session_count() < std::size_t(test_client_count))
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		auto session_count = server.get_session_count();
		ASIO2_CHECK_VALUE(session_count, session_count == std::size_t(test_client_count));
		ASIO2_CHECK_VALUE(server_connect_counter.load(), server_connect_counter == test_client_count);

		while (client_disconnect_counter != test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

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

	ASIO2_TEST_END_LOOP;
}

ASIO2_TEST_SUITE
(
	"socks5_udp",
	ASIO2_TEST_CASE(socks5_udp_test)
)
