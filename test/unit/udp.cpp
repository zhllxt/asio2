#include "unit_test.hpp"
#include <unordered_set>
#include <asio2/external/fmt.hpp>
#include <asio2/udp/udp_server.hpp>
#include <asio2/udp/udp_client.hpp>
#include <asio2/udp/udp_cast.hpp>

static std::string_view chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

struct hasher
{
	inline std::size_t operator()(asio::ip::udp::endpoint const& ep) const noexcept
	{
		std::uint32_t addr = ep.address().to_v4().to_uint();

		std::uint32_t v = asio2::detail::fnv1a_hash<std::uint32_t>((const unsigned char*)&addr, sizeof(std::uint32_t));

		std::uint16_t port = ep.port();

		return asio2::detail::fnv1a_hash<std::uint32_t>(v, (const unsigned char*)&port, sizeof(std::uint16_t));
	}
};

class my_udp_client : public asio2::udp_client_t<my_udp_client>
{
public:
	using asio2::udp_client_t<my_udp_client>::udp_client_t;

public:
	using asio2::udp_client_t<my_udp_client>::kcp_stream_;

	void set_send_fin(bool v) { kcp_stream_->send_fin_ = v; }
};

void udp_test()
{
	ASIO2_TEST_BEGIN_LOOP(test_loop_times);

	// test endpoint hash collision
	{
		{
			std::unordered_map<asio::ip::udp::endpoint, std::size_t, hasher> map;

			asio::ip::udp::endpoint s1(asio::ip::address_v4{ {0, 0, 23, 37 } }, 40889);
			asio::ip::udp::endpoint s2(asio::ip::address_v4{ {0, 0, 23, 136} }, 129  );

			auto h1 = hasher()(s1);
			auto h2 = hasher()(s2);

			ASIO2_CHECK(h1 == h2);

			map.emplace(s1, 1);

			ASIO2_CHECK(map.find(s1) != map.end());
			ASIO2_CHECK(map.find(s2) == map.end());

			map.emplace(s2, 2);

			ASIO2_CHECK(map.find(s2) != map.end());

			ASIO2_CHECK(map.find(s1)->second == 1);
			ASIO2_CHECK(map.find(s2)->second == 2);

			ASIO2_CHECK(map.size() == 2);
		}

		// after test, there are a lot of hash collisions.
		std::uint64_t count = 0;

		//std::unordered_set<std::uint32_t> sets;

		//for (std::uint8_t a = 0; a < 0xff; a++)
		//{
		//	for (std::uint8_t b = 0; b < 0xff; b++)
		//	{
		//		for (std::uint8_t c = 0; c < 0xff; c++)
		//		{
		//			for (std::uint8_t d = 0; d < 0xff; d++)
		//			{
		//				for (std::uint16_t port = 0; port < 0xffff; port++)
		//				{
		//					asio::ip::udp::endpoint s(asio::ip::address_v4{ {a,b,c,d} }, port);

		//					std::uint32_t hash = (std::uint32_t)hasher()(s);

		//					auto pair = sets.emplace(hash);

		//					if (!pair.second)
		//					{
		//						count++;
		//						printf("%d %d %d %d %d %u\n", a, b, c, d, port, hash);
		//						while (std::getchar() != '\n');
		//					}

		//					ASIO2_CHECK_VALUE(hash, pair.second);
		//				}
		//			}
		//			ASIO2_CHECK_VALUE(count, count == 0);
		//		}
		//	}
		//}

		ASIO2_CHECK_VALUE(count, count == 0);
	}

	// test udp send recv
	{
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
			ASIO2_CHECK(session_ptr->local_port() == 18036);
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
			ASIO2_CHECK(session_ptr->local_port() == 18036);
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
			ASIO2_CHECK(server.get_listen_port() == 18036);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18036);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});

		bool server_start_ret = server.start("127.0.0.1", 18036);

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
				ASIO2_CHECK(client.get_remote_port() == 18036);

				client_connect_counter++;

				ext_data ex;
				ex.num = 1;

				client.set_user_data(ex);

				std::string msg = fmt::format("{:05d},", 1);

				msg += chars[0];

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

			bool client_start_ret = client.async_start("127.0.0.1", 18036);

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

		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		while (client_disconnect_counter != test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(server_disconnect_counter.load(), server_disconnect_counter == 0);
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

	// test udp kcp
	{
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
			ASIO2_CHECK(session_ptr->local_port() == 18036);
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
			ASIO2_CHECK(session_ptr->local_port() == 18036);
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
			ASIO2_CHECK(server.get_listen_port() == 18036);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18036);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});

		bool server_start_ret = server.start("127.0.0.1", 18036, asio2::use_kcp);

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
				ASIO2_CHECK(client.get_remote_port() == 18036);

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

			bool client_start_ret = client.async_start("127.0.0.1", 18036, asio2::use_kcp);

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

		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			std::string msg = fmt::format("{:05d},", 1);

			msg += chars[0];

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
		server_connect_counter = 0;
		server_recv_counter = 0;

		server_start_ret = server.start("127.0.0.1", 18036, asio2::use_kcp);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		client_init_counter = 0;
		client_connect_counter = 0;
		client_disconnect_counter = 0;

		for (int i = 0; i < test_client_count; i++)
		{
			bool client_start_ret = clients[i]->async_start("127.0.0.1", 18036, asio2::use_kcp);
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

		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			std::string msg = fmt::format("{:05d},", 1);

			msg += chars[0];

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

	// test udp kcp with client provided kcp conv
	{
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
			ASIO2_CHECK(session_ptr->local_port() == 18036);
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
			ASIO2_CHECK(session_ptr->local_port() == 18036);
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
			ASIO2_CHECK(server.get_listen_port() == 18036);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18036);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});

		bool server_start_ret = server.start("127.0.0.1", 18036, asio2::use_kcp);

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

			client.set_kcp_conv(i + 100);

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
			client.bind_connect([&, i]()
			{
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.get_local_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_port() == 18036);

				if (!asio2::get_last_error())
				{
					ASIO2_CHECK(client.get_kcp()->conv == std::uint32_t(i + 100));
				}

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

			bool client_start_ret = client.async_start("127.0.0.1", 18036, asio2::use_kcp);

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

		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			std::string msg = fmt::format("{:05d},", 1);

			msg += chars[0];

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
		server_connect_counter = 0;
		server_recv_counter = 0;

		server_start_ret = server.start("127.0.0.1", 18036, asio2::use_kcp);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		client_init_counter = 0;
		client_connect_counter = 0;
		client_disconnect_counter = 0;

		for (int i = 0; i < test_client_count; i++)
		{
			bool client_start_ret = clients[i]->async_start("127.0.0.1", 18036, asio2::use_kcp);
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

		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			std::string msg = fmt::format("{:05d},", 1);

			msg += chars[0];

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

	// test udp kcp with reconnect by the same endpoint
	{
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
			ASIO2_CHECK(session_ptr->local_port() == 18036);
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
			ASIO2_CHECK(session_ptr->local_port() == 18036);
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
			ASIO2_CHECK(server.get_listen_port() == 18036);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18036);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});

		bool server_start_ret = server.start("127.0.0.1", 18036, asio2::use_kcp);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		std::vector<std::shared_ptr<my_udp_client>> clients;
		std::atomic<int> client_init_counter = 0;
		std::atomic<int> client_connect_counter = 0;
		std::atomic<int> client_disconnect_counter = 0;
		for (int i = 0; i < test_client_count; i++)
		{
			auto iter = clients.emplace_back(std::make_shared<my_udp_client>());

			my_udp_client& client = *iter;

			client.set_kcp_conv(i + 1000);

			// disable auto reconnect, default reconnect option is "enable"
			client.set_auto_reconnect(false);

			client.bind_init([&, i]()
			{
				client_init_counter++;

				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.is_reuse_address());

				// Specify the local port to which the socket is bind.
				asio::ip::udp::endpoint ep(asio::ip::udp::v4(), std::uint16_t(1234 + i));
				client.socket().bind(ep);

				client.set_no_delay(true);
			});
			client.bind_connect([&, i]()
			{
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.get_local_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_port() == 18036);

				if (!asio2::get_last_error())
				{
					ASIO2_CHECK(client.get_kcp()->conv == std::uint32_t(i + 1000));
				}

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

				if (ex->num > 10)
				{
					// before client.stop, the recvd kcp message should be reponsed ack, so
					// we use a post to delay call client.stop, otherwise if we call
					// client.stop() directly, the recvd kcp message will can't be reponsed,
					// this will cause the kcp server will resent the message, and when 
					// this client restart again, this client maybe recvd the prev kcp
					// message in the handshaking, it will cause the handshake failed.
					client.post([&client]() { client.stop(); }, std::chrono::seconds(1));
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

			bool client_start_ret = client.async_start("127.0.0.1", 18036, asio2::use_kcp);

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

		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->set_send_fin(false);
		}

		for (int i = 0; i < test_client_count; i++)
		{
			std::string msg = fmt::format("{:05d},", 1);

			msg += chars[0];

			clients[i]->async_send(std::move(msg));
		}

		while (client_disconnect_counter != test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(server_recv_counter.load(), server_recv_counter == 11 * test_client_count);
		ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->stop();
			ASIO2_CHECK(clients[i]->is_stopped());
		}

		//-----------------------------------------------------------------------------------------

		ASIO2_CHECK_VALUE(server.get_session_count(), server.get_session_count() == std::size_t(test_client_count));

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		client_init_counter = 0;
		client_connect_counter = 0;
		client_disconnect_counter = 0;

		server.foreach_session([](auto& session_ptr)
		{
			ext_data ex;
			ex.num = 1;

			session_ptr->set_user_data(ex);
		});

		for (int i = 0; i < test_client_count; i++)
		{
			bool client_start_ret = clients[i]->async_start("127.0.0.1", 18036, asio2::use_kcp);
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

		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->set_send_fin(true);
		}

		for (int i = 0; i < test_client_count; i++)
		{
			std::string msg = fmt::format("{:05d},", 1);

			msg += chars[0];

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
		ASIO2_CHECK_VALUE(server_recv_counter.load(), server_recv_counter == 22 * test_client_count);
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
		asio2::udp_server server;
		std::atomic<int> server_connect_counter = 0;
		server.bind_connect([&](auto & session_ptr)
		{
			session_ptr->set_user_data(server_connect_counter.load());
			server_connect_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(!server.find_session(session_ptr->hash_key()));
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18036);
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
			ASIO2_CHECK(session_ptr->local_port() == 18036);
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
			ASIO2_CHECK(server.get_listen_port() == 18036);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18036);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});

		bool server_start_ret = server.start("127.0.0.1", 18036);

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

			// enable auto reconnect and use custom delay, default delay is 1 seconds
			client.set_auto_reconnect(false);

			client.bind_init([&]()
			{
				client_init_counter++;

				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
			});
			client.bind_connect([&]()
			{
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.get_local_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_port() == 18036);

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

			bool client_start_ret = client.start("127.0.0.1", 18036);

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

		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		server.async_send("abc0123456789def");

		std::vector<std::shared_ptr<asio2::udp_session>> sessions;

		// find session maybe true, beacuse the next session maybe used the stopped session "this" address
		//ASIO2_CHECK(!server.find_session(session_key));
		server.foreach_session([&sessions](std::shared_ptr<asio2::udp_session>& session_ptr) mutable
			{
				ASIO2_CHECK(session_ptr->user_data_any().has_value());

				sessions.emplace_back(session_ptr);
			});

		ASIO2_CHECK_VALUE(sessions.size(), sessions.size() == std::size_t(test_client_count));

		for (std::size_t i = 0; i < sessions.size(); i++)
		{
			std::shared_ptr<asio2::udp_session>& session = sessions[i];

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
				//ASIO2_CHECK(!session->is_stopped()); // maybe stopped==true
			}
		}

		sessions.clear();

		// use this to ensure the ASIO2_CHECK(session_ptr->is_started());
		while (server_disconnect_counter != test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

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

	// test udp cast
	{
		std::shared_ptr<asio2::udp_cast> udp_cast_ptr = std::make_shared<asio2::udp_cast>();
		asio2::udp_cast& udp_cast = *udp_cast_ptr;

		udp_cast.bind_recv([&](asio::ip::udp::endpoint& endpoint, std::string_view data)
		{
			udp_cast.async_send(endpoint, data);
		}).bind_start([&]()
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(udp_cast.get_local_port() == 18036);
			ASIO2_CHECK(udp_cast.io().running_in_this_thread());
			ASIO2_CHECK(udp_cast.iopool().get(0)->running_in_this_thread());
		}).bind_stop([&]()
		{
			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(udp_cast.get_local_port() == 18036);
			ASIO2_CHECK(udp_cast.io().running_in_this_thread());
			ASIO2_CHECK(udp_cast.iopool().get(0)->running_in_this_thread());
		}).bind_init([&]()
		{
			ASIO2_CHECK(udp_cast.io().running_in_this_thread());
			ASIO2_CHECK(udp_cast.iopool().get(0)->running_in_this_thread());
			ASIO2_CHECK(!asio2::get_last_error());
		});

		udp_cast.start("127.0.0.1", 18036);

		std::string str("<0123456789abcdefghijklmnopqrstowvxyz>");

		udp_cast.async_send("127.0.0.1", 18036, str);
		udp_cast.async_send("127.0.0.1", "18036", str);
		udp_cast.async_send("127.0.0.1", 18036, "abc123");
		udp_cast.async_send("127.0.0.1", 18036, "abc123", 6);
		{
			auto fut1 = udp_cast.async_send("127.0.0.1", 18036, str, asio::use_future);
			auto [e1, s1] = fut1.get();
			ASIO2_CHECK(!e1);
			ASIO2_CHECK(s1 == str.size());
		}
		{
			auto fut1 = udp_cast.async_send("127.0.0.1", "18036", str, asio::use_future);
			auto [e1, s1] = fut1.get();
			ASIO2_CHECK(!e1);
			ASIO2_CHECK(s1 == str.size());
		}
		{
			auto fut1 = udp_cast.async_send("127.0.0.1", 18036, "abc123", asio::use_future);
			auto [e1, s1] = fut1.get();
			ASIO2_CHECK(!e1);
			ASIO2_CHECK(s1 == 6);
		}
		{
			auto fut1 = udp_cast.async_send("127.0.0.1", "18036", "abc123", asio::use_future);
			auto [e1, s1] = fut1.get();
			ASIO2_CHECK(!e1);
			ASIO2_CHECK(s1 == 6);
		}
		{
			auto fut1 = udp_cast.async_send("127.0.0.1", 18036, "abc123", 6, asio::use_future);
			auto [e1, s1] = fut1.get();
			ASIO2_CHECK(!e1);
			ASIO2_CHECK(s1 == 6);
		}
		{
			auto fut1 = udp_cast.async_send("127.0.0.1", "18036", "abc123", 6, asio::use_future);
			auto [e1, s1] = fut1.get();
			ASIO2_CHECK(!e1);
			ASIO2_CHECK(s1 == 6);
		}

		udp_cast.async_send("127.0.0.1", 18036, str, [str](std::size_t bytes)
		{
			ASIO2_CHECK(bytes == str.size());
			ASIO2_CHECK(!asio2::get_last_error());
		});
		udp_cast.async_send("127.0.0.1", "18036", str, [str](std::size_t bytes)
		{
			ASIO2_CHECK(bytes == str.size());
			ASIO2_CHECK(!asio2::get_last_error());
		});

		udp_cast.async_send("127.0.0.1", 18036, "abc123", [](std::size_t bytes)
		{
			ASIO2_CHECK(bytes == 6);
			ASIO2_CHECK(!asio2::get_last_error());
		});
		udp_cast.async_send("127.0.0.1", "18036", "abc123", [](std::size_t bytes)
		{
			ASIO2_CHECK(bytes == 6);
			ASIO2_CHECK(!asio2::get_last_error());
		});

		udp_cast.async_send("127.0.0.1", 18036, "abc123", 6, [](std::size_t bytes)
		{
			ASIO2_CHECK(bytes == 6);
			ASIO2_CHECK(!asio2::get_last_error());
		});
		udp_cast.async_send("127.0.0.1", "18036", "abc123", 6, [](std::size_t bytes)
		{
			ASIO2_CHECK(bytes == 6);
			ASIO2_CHECK(!asio2::get_last_error());
		});

		asio::ip::udp::endpoint ep(asio::ip::make_address("127.0.0.1"), 18020);

		udp_cast.async_send(ep, str);
		udp_cast.async_send(ep, str);
		udp_cast.async_send(ep, "abc123");
		udp_cast.async_send(ep, "abc123", 6);
		{
			auto fut1 = udp_cast.async_send(ep, str, asio::use_future);
			auto [e1, s1] = fut1.get();
			ASIO2_CHECK(!e1);
			ASIO2_CHECK(s1 == str.size());
		}
		{
			auto fut1 = udp_cast.async_send(ep, str, asio::use_future);
			auto [e1, s1] = fut1.get();
			ASIO2_CHECK(!e1);
			ASIO2_CHECK(s1 == str.size());
		}
		{
			auto fut1 = udp_cast.async_send(ep, "abc123", asio::use_future);
			auto [e1, s1] = fut1.get();
			ASIO2_CHECK(!e1);
			ASIO2_CHECK(s1 == 6);
		}
		{
			auto fut1 = udp_cast.async_send(ep, "abc123", asio::use_future);
			auto [e1, s1] = fut1.get();
			ASIO2_CHECK(!e1);
			ASIO2_CHECK(s1 == 6);
		}
		{
			auto fut1 = udp_cast.async_send(ep, "abc123", 6, asio::use_future);
			auto [e1, s1] = fut1.get();
			ASIO2_CHECK(!e1);
			ASIO2_CHECK(s1 == 6);
		}
		{
			auto fut1 = udp_cast.async_send(ep, "abc123", 6, asio::use_future);
			auto [e1, s1] = fut1.get();
			ASIO2_CHECK(!e1);
			ASIO2_CHECK(s1 == 6);
		}

		udp_cast.async_send(ep, str, [str](std::size_t bytes)
		{
			ASIO2_CHECK(bytes == str.size());
			ASIO2_CHECK(!asio2::get_last_error());
		});
		udp_cast.async_send(ep, str, [str](std::size_t bytes)
		{
			ASIO2_CHECK(bytes == str.size());
			ASIO2_CHECK(!asio2::get_last_error());
		});

		udp_cast.async_send(ep, "abc123", [](std::size_t bytes)
		{
			ASIO2_CHECK(bytes == 6);
			ASIO2_CHECK(!asio2::get_last_error());
		});
		udp_cast.async_send(ep, "abc123", [](std::size_t bytes)
		{
			ASIO2_CHECK(bytes == 6);
			ASIO2_CHECK(!asio2::get_last_error());
		});

		udp_cast.async_send(ep, "abc123", 6, [](std::size_t bytes)
		{
			ASIO2_CHECK(bytes == 6);
			ASIO2_CHECK(!asio2::get_last_error());
		});
		udp_cast.async_send(ep, "abc123", 6, [](std::size_t bytes)
		{
			ASIO2_CHECK(bytes == 6);
			ASIO2_CHECK(!asio2::get_last_error());
		});

		// this is a multicast address
		udp_cast.async_send("239.255.0.1", "8030", str);

		// the resolve function is a time-consuming operation
		asio::ip::udp::resolver resolver(udp_cast.io().context());
		asio::ip::udp::resolver::query query("127.0.0.1", "18080");
		asio::ip::udp::endpoint ep2 = *resolver.resolve(query);
		udp_cast.async_send(ep2, str);

		// stop the udp cast after 10 seconds. 
		udp_cast.start_timer(1, 3000, 1, [&]()
		{
			// note : the stop is called in the io_context thread is ok.
			// of course you can call the udp_cast.stop() function anywhere.
			udp_cast.stop();
		});

		// the udp_cast.wait_stop() will be blocked forever until the udp_cast.stop() is called.
		udp_cast.wait_stop();

		// Or you can call the wait_for function directly to block for 10 seconds
		//udp_cast.wait_for(std::chrono::seconds(10));
		
		// must call stop, beacuse this is std::shared_ptr<asio2::udp_cast>
		udp_cast.stop();
	}

	ASIO2_TEST_END_LOOP;
}

ASIO2_TEST_SUITE
(
	"udp",
	ASIO2_TEST_CASE(udp_test)
)
