#include "unit_test.hpp"
#include <asio2/asio2.hpp>
#include <asio2/external/fmt.hpp>

static std::string_view chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

void rdc_test()
{
	ASIO2_TEST_BEGIN_LOOP(test_loop_times);

	// tcp dgram rdc
	{
		struct ext_data
		{
			int num = 1;
		};

		asio2::rdc::option server_rdc_option
		{
			[](std::string_view data)
			{
				std::string num { data.substr(0,data.find(','))};
				return std::stoi(num);
			},
			[](std::string_view data)
			{
				std::string num { data.substr(0,data.find(','))};
				return std::stoi(num) - 1;
			}
		};

		asio2::rdc::option client_rdc_option
		{
			[](std::string_view data)
			{
				std::string num { data.substr(0,data.find(','))};
				return std::stoi(num);
			},
			[](std::string_view data)
			{
				std::string num { data.substr(0,data.find(','))};
				return std::stoi(num);
			}
		};

		asio2::tcp_server server;
		std::atomic<int> server_recv_counter = 0;
		server.bind_recv([&](std::shared_ptr<asio2::tcp_session> & session_ptr, std::string_view data)
		{
			server_recv_counter++;

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

			session_ptr->async_call(msg, [msg](std::string_view data) mutable
			{
				asio::error_code ec = asio2::get_last_error();
				if (!ec)
				{
					std::string s1 = msg.substr(0, msg.find(','));
					std::string s2{ data.substr(0, data.find(',')) };
					ASIO2_CHECK(std::stoi(s1) + 1 == std::stoi(s2));
				}
				else
				{
					ASIO2_CHECK(ec == asio::error::operation_aborted || ec == asio::error::timed_out);
				}
			});

			ASIO2_CHECK(session_ptr->io().running_in_this_thread());
		});
		std::atomic<int> server_accept_counter = 0;
		server.bind_accept([&](auto & session_ptr)
		{
			session_ptr->no_delay(true);

			server_accept_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18037);
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18037);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).running_in_this_thread());

			//// You can close the connection directly here.
			//if (session_ptr->remote_address() == "192.168.0.254")
			//	session_ptr->stop();

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
			ASIO2_CHECK(session_ptr->local_port() == 18037);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).running_in_this_thread());
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
			//ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18037);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).running_in_this_thread());
		});
		std::atomic<int> server_init_counter = 0;
		server.bind_init([&]()
		{
			server_init_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).running_in_this_thread());

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
			ASIO2_CHECK(server.get_listen_port() == 18037);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18037);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).running_in_this_thread());
		});

		bool server_start_ret = server.start("127.0.0.1", 18037, asio2::use_dgram, server_rdc_option);

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
				ASIO2_CHECK(client.iopool().get(0).running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.is_keep_alive());
				ASIO2_CHECK(client.is_reuse_address());
				ASIO2_CHECK(client.is_no_delay());
			});
			client.bind_connect([&]()
			{
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.get_local_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_port() == 18037);

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
				ASIO2_CHECK(client.iopool().get(0).running_in_this_thread());
			});
			client.bind_recv([&]([[maybe_unused]] std::string_view data)
			{
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).running_in_this_thread());
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
					msg += chars;
				}

				ASIO2_CHECK(msg == data);

				ex->num++;

				msg = fmt::format("{:05d},", ex->num);

				for (int i = 0; i < ex->num; i++)
				{
					msg += chars;
				}

				client.async_call(msg, [msg](std::string_view data) mutable
				{
					ASIO2_CHECK(msg == data);
				});
			});

			bool client_start_ret = client.async_start("127.0.0.1", 18037, asio2::use_dgram, client_rdc_option);

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

			clients[i]->async_call(msg, [msg](std::string_view data) mutable
			{
				ASIO2_CHECK(msg == data);
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

		server_start_ret = server.start("127.0.0.1", 18037, asio2::use_dgram, server_rdc_option);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		client_init_counter = 0;
		client_connect_counter = 0;
		client_disconnect_counter = 0;

		for (int i = 0; i < test_client_count; i++)
		{
			bool client_start_ret = clients[i]->async_start("127.0.0.1", 18037, asio2::use_dgram, client_rdc_option);
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

			std::string rep = clients[i]->call<std::string>(msg);
			ASIO2_CHECK(msg == rep);
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

	{
		asio2::tcp_client client;

		std::shared_ptr<asio2::socks5::option<asio2::socks5::method::anonymous>>
			sock5_option = std::make_shared<asio2::socks5::option<asio2::socks5::method::anonymous>>("127.0.0.1", 10808);
		
		asio2::rdc::option client_rdc_option
		{
			[](std::string_view data)
			{
				std::string num { data.substr(0,data.find(','))};
				return std::stoi(num);
			},
			[](std::string_view data)
			{
				std::string num { data.substr(0,data.find(','))};
				return std::stoi(num);
			}
		};

		// here, can't use the port 18037 again, otherwise the server_accept_counter and server_disconnect_counter
		// maybe not equal the client_connect_counter of the 18037 server, this is because this client
		// uses a socks5 proxy, even if this client was destroyed already, the proxy software will
		// still connect to the 18037 server.
		client.start("127.0.0.1", 18038, asio2::use_dgram, sock5_option, client_rdc_option);
	}

	{
		//std::shared_ptr<asio2::socks5::option<asio2::socks5::method::anonymous>>
		//	sock5_option = std::make_shared<asio2::socks5::option<asio2::socks5::method::anonymous>>("127.0.0.1", 10808);
		//asio2::socks5::option<asio2::socks5::method::anonymous, asio2::socks5::method::password>
		//	sock5_option{ "s5.doudouip.cn",1088,"zjww-1","aaa123" };

		//asio2::rdc::option rdc_option{ [](http::web_request&) { return 0; },[](http::web_response&) { return 0; } };

		//std::shared_ptr<asio2::rdc::option<int, http::web_request&, http::web_response&>> rdc_option =
		//	std::make_shared<asio2::rdc::option<int, http::web_request&, http::web_response&>>(
		//		[](http::web_request&) { return 0; }, [](http::web_response&) { return 0; });

		//client.start(host, port, sock5_option, rdc_option);
	}

	ASIO2_TEST_END_LOOP;
}


ASIO2_TEST_SUITE
(
	"rdc",
	ASIO2_TEST_CASE(rdc_test)
)
