#include "unit_test.hpp"

#include <asio2/config.hpp>

#undef ASIO2_USE_WEBSOCKET_RPC

#include <asio2/rpc/rpc_server.hpp>
#include <asio2/rpc/rpc_client.hpp>
#include <asio2/external/json.hpp>

struct userinfo
{
	std::string name;
	int age;
	std::map<int, std::string> purview;

	// User defined object types require serialized the members like this:
	template <class Archive>
	void serialize(Archive & ar)
	{
		ar(name);
		ar(age);
		ar(purview);
	}
};

namespace nlohmann
{
	void operator<<(asio2::rpc::oarchive& sr, const nlohmann::json& j)
	{
		sr << j.dump();
	}

	void operator>>(asio2::rpc::iarchive& dr, nlohmann::json& j)
	{
		std::string v;
		dr >> v;
		j = nlohmann::json::parse(v);
	}
}

std::string echo(std::string s)
{
	return s;
}

int add(int a, int b)
{
	return a + b;
}

rpc::future<int> async_add(int a, int b)
{
	rpc::promise<int> promise;
	rpc::future<int> f = promise.get_future();

	std::thread([a, b, promise = std::move(promise)]() mutable
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		promise.set_value(a + b);
	}).detach();

	return f;
}

rpc::future<void> async_test(std::shared_ptr<asio2::rpc_session>& session_ptr, std::string a, std::string b)
{
	ASIO2_CHECK(session_ptr->io().strand().running_in_this_thread());

	rpc::promise<void> promise;
	rpc::future<void> f = promise.get_future();

	ASIO2_CHECK(a == "abc" && b == "def");

	std::thread([a, b, promise]() mutable
	{
		asio2::ignore_unused(a, b);
		promise.set_value();
	}).detach();

	return f;
}

nlohmann::json test_json(nlohmann::json j)
{
	std::string s = j.dump();

	return nlohmann::json::parse(s);
}

class usercrud
{
public:
	double mul(double a, double b)
	{
		return a * b;
	}

	// If you want to know which client called this function, set the first parameter
	// to std::shared_ptr<asio2::rpc_session>& session_ptr
	userinfo get_user(std::shared_ptr<asio2::rpc_session>& session_ptr)
	{
		ASIO2_CHECK(session_ptr->io().strand().running_in_this_thread());

		userinfo u;
		u.name = "lilei";
		u.age = 32;
		u.purview = { {1,"read"},{2,"write"} };
		return u;
	}

	// If you want to know which client called this function, set the first parameter
	// to std::shared_ptr<asio2::rpc_session>& session_ptr
	void del_user(std::shared_ptr<asio2::rpc_session>& session_ptr, const userinfo& u)
	{
		ASIO2_CHECK(session_ptr->io().strand().running_in_this_thread());
		ASIO2_CHECK(u.name == "hanmeimei");
		ASIO2_CHECK(u.age == 33);
		ASIO2_CHECK(u.purview.size() == 2);
		for (auto&[k, v] : u.purview)
		{
			ASIO2_CHECK(k == 10 || k == 20);
			if (k == 10) ASIO2_CHECK(v == "get");
			if (k == 20) ASIO2_CHECK(v == "set");
		}
	}
};

void heartbeat(std::shared_ptr<asio2::rpc_session>& session_ptr)
{
	ASIO2_CHECK(session_ptr->io().strand().running_in_this_thread());
}

// set the first parameter to client reference to know which client was called
void client_fn_test(asio2::rpc_client& client, std::string str)
{
	ASIO2_CHECK(client.io().strand().running_in_this_thread());
	ASIO2_CHECK(str == "i love you");
}

class my_rpc_client_tcp : public asio2::rpc_client_use<asio2::net_protocol::tcp>
{
public:
	using rpc_client_use<asio2::net_protocol::tcp>::rpc_client_use;

	using super::send;
	using super::async_send;
};

void rpc_test()
{
	ASIO2_TEST_BEGIN_LOOP(test_loop_times);

	// test max buffer size
	{
		asio2::rpc_server server(512, 1024, 4);

		std::atomic<int> server_accept_counter = 0;
		server.bind_accept([&](auto & session_ptr)
		{
			session_ptr->no_delay(true);

			server_accept_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18010);
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18010);
			ASIO2_CHECK(server.io().strand().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).strand().running_in_this_thread());

		});
		std::atomic<int> server_connect_counter = 0;
		server.bind_connect([&](auto & session_ptr)
		{
			server_connect_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18010);
			ASIO2_CHECK(server.io().strand().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).strand().running_in_this_thread());
			ASIO2_CHECK(session_ptr->is_keep_alive());
			ASIO2_CHECK(session_ptr->is_no_delay());

		});
		std::atomic<int> server_disconnect_counter = 0;
		server.bind_disconnect([&](auto & session_ptr)
		{
			server_disconnect_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(session_ptr->socket().is_open());
			ASIO2_CHECK(server.io().strand().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).strand().running_in_this_thread());
		});
		std::atomic<int> server_init_counter = 0;
		server.bind_init([&]()
		{
			server_init_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(server.io().strand().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).strand().running_in_this_thread());

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
			ASIO2_CHECK(server.get_listen_port() == 18010);
			ASIO2_CHECK(server.io().strand().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).strand().running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18010);
			ASIO2_CHECK(server.io().strand().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).strand().running_in_this_thread());
		});

		server.bind("echo", echo);

		bool server_start_ret = server.start("127.0.0.1", 18010);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		std::vector<std::shared_ptr<asio2::rpc_client>> clients;
		std::atomic<int> client_init_counter = 0;
		std::atomic<int> client_connect_counter = 0;
		std::atomic<int> client_disconnect_counter = 0;
		for (int i = 0; i < test_client_count; i++)
		{
			auto iter = clients.emplace_back(std::make_shared<asio2::rpc_client>());

			asio2::rpc_client& client = *iter;

			// set default rpc call timeout
			client.set_default_timeout(std::chrono::seconds(3));
			ASIO2_CHECK(client.get_default_timeout() == std::chrono::seconds(3));
			client.set_auto_reconnect(false);
			ASIO2_CHECK(!client.is_auto_reconnect());

			client.bind_init([&]()
			{
				client_init_counter++;

				client.set_no_delay(true);

				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.is_keep_alive());
				ASIO2_CHECK(client.is_reuse_address());
				ASIO2_CHECK(client.is_no_delay());
			});
			client.bind_connect([&]()
			{
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.get_local_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_port() == 18010);

				client_connect_counter++;

			});
			client.bind_disconnect([&]()
			{
				client_disconnect_counter++;

				ASIO2_CHECK(asio2::get_last_error());
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
			});
			client.bind_recv([&]([[maybe_unused]] std::string_view data)
			{
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(!data.empty());
				ASIO2_CHECK(client.is_started());
			});

			bool client_start_ret = client.start("127.0.0.1", 18010);

			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(!asio2::get_last_error());
		}

		while (server.get_session_count() < std::size_t(test_client_count))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		auto session_count = server.get_session_count();
		ASIO2_CHECK_VALUE(session_count, session_count == std::size_t(test_client_count));

		ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == test_client_count);
		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		while (client_connect_counter < test_client_count)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == test_client_count);
		ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == test_client_count);

		std::string msg;
		msg.resize(1500);
		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->async_call("echo", msg).response([](std::string s)
			{
				ASIO2_CHECK(s.empty());
				ASIO2_CHECK(asio2::get_last_error() == rpc::error::operation_aborted);
			});
		}

		while (client_disconnect_counter < test_client_count)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			ASIO2_CHECK(!clients[i]->is_stopped());
			ASIO2_CHECK(!clients[i]->is_started());
		}

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->stop();
			ASIO2_CHECK(clients[i]->is_stopped());
			ASIO2_CHECK(!clients[i]->is_started());
		}

		ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == test_client_count);

		// use this to ensure the ASIO2_CHECK(session_ptr->is_started());
		while (server_disconnect_counter != test_client_count)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
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

		server_start_ret = server.start("127.0.0.1", 18010);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		client_init_counter = 0;
		client_connect_counter = 0;
		client_disconnect_counter = 0;

		for (int i = 0; i < test_client_count; i++)
		{
			bool client_start_ret = clients[i]->async_start("127.0.0.1", 18010);
			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
		}

		while (server.get_session_count() < std::size_t(test_client_count))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		ASIO2_CHECK_VALUE(server.get_session_count(), server.get_session_count() == std::size_t(test_client_count));

		ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == test_client_count);
		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		while (client_connect_counter < test_client_count)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == test_client_count);
		ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->async_call("echo", msg).response([](std::string s)
			{
				ASIO2_CHECK(s.empty());
				ASIO2_CHECK(asio2::get_last_error() == rpc::error::operation_aborted);
			});
		}

		while (client_disconnect_counter < test_client_count)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			ASIO2_CHECK(!clients[i]->is_stopped());
			ASIO2_CHECK(!clients[i]->is_started());
		}

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->stop();
			ASIO2_CHECK(clients[i]->is_stopped());
			ASIO2_CHECK(!clients[i]->is_started());
		}

		ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == test_client_count);

		// use this to ensure the ASIO2_CHECK(session_ptr->is_started());
		while (server_disconnect_counter != test_client_count)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		server.stop();
		ASIO2_CHECK(server.is_stopped());

		ASIO2_CHECK_VALUE(server_disconnect_counter.load(), server_disconnect_counter == test_client_count);
		ASIO2_CHECK_VALUE(server_stop_counter      .load(), server_stop_counter       == 1);
	}

	// test illegal data
	{
		asio2::rpc_server server;

		std::atomic<int> server_accept_counter = 0;
		server.bind_accept([&](auto & session_ptr)
		{
			session_ptr->no_delay(true);

			server_accept_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18010);
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18010);
			ASIO2_CHECK(server.io().strand().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).strand().running_in_this_thread());

		});
		std::atomic<int> server_connect_counter = 0;
		server.bind_connect([&](auto & session_ptr)
		{
			server_connect_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18010);
			ASIO2_CHECK(server.io().strand().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).strand().running_in_this_thread());
			ASIO2_CHECK(session_ptr->is_keep_alive());
			ASIO2_CHECK(session_ptr->is_no_delay());

		});
		std::atomic<int> server_disconnect_counter = 0;
		server.bind_disconnect([&](auto & session_ptr)
		{
			server_disconnect_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(session_ptr->socket().is_open());
			ASIO2_CHECK(server.io().strand().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).strand().running_in_this_thread());
		});
		std::atomic<int> server_init_counter = 0;
		server.bind_init([&]()
		{
			server_init_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(server.io().strand().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).strand().running_in_this_thread());

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
			ASIO2_CHECK(server.get_listen_port() == 18010);
			ASIO2_CHECK(server.io().strand().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).strand().running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18010);
			ASIO2_CHECK(server.io().strand().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).strand().running_in_this_thread());
		});

		server.bind("echo", echo);

		bool server_start_ret = server.start("127.0.0.1", 18010);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		std::vector<std::shared_ptr<my_rpc_client_tcp>> clients;
		std::atomic<int> client_init_counter = 0;
		std::atomic<int> client_connect_counter = 0;
		std::atomic<int> client_disconnect_counter = 0;
		for (int i = 0; i < test_client_count; i++)
		{
			auto iter = clients.emplace_back(std::make_shared<my_rpc_client_tcp>());

			my_rpc_client_tcp& client = *iter;

			// set default rpc call timeout
			client.set_default_timeout(std::chrono::seconds(3));
			ASIO2_CHECK(client.get_default_timeout() == std::chrono::seconds(3));
			client.set_auto_reconnect(false);
			ASIO2_CHECK(!client.is_auto_reconnect());

			client.bind_init([&]()
			{
				client_init_counter++;

				client.set_no_delay(true);

				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.is_keep_alive());
				ASIO2_CHECK(client.is_reuse_address());
				ASIO2_CHECK(client.is_no_delay());
			});
			client.bind_connect([&]()
			{
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.get_local_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_port() == 18010);

				client_connect_counter++;

			});
			client.bind_disconnect([&]()
			{
				client_disconnect_counter++;

				ASIO2_CHECK(asio2::get_last_error());
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
			});
			client.bind_recv([&]([[maybe_unused]] std::string_view data)
			{
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(!data.empty());
				ASIO2_CHECK(client.is_started());
			});

			bool client_start_ret = client.start("127.0.0.1", 18010);

			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(!asio2::get_last_error());
		}

		while (server.get_session_count() < std::size_t(test_client_count))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		auto session_count = server.get_session_count();
		ASIO2_CHECK_VALUE(session_count, session_count == std::size_t(test_client_count));

		ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == test_client_count);
		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		while (client_connect_counter < test_client_count)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == test_client_count);
		ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == test_client_count);

		std::string msg;

		while (client_disconnect_counter < test_client_count)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));

			msg.clear();
			int len = 200 + (std::rand() % 200);
			for (int i = 0; i < len; i++)
			{
				msg += (char)(std::rand() % 255);
			}
			// send random data to server for every client, if the data is invalid, the session will be closed
			for (int i = 0; i < test_client_count; i++)
			{
				clients[i]->async_send(msg);
			}
		}

		ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			ASIO2_CHECK(!clients[i]->is_stopped());
			ASIO2_CHECK(!clients[i]->is_started());
		}

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->stop();
			ASIO2_CHECK( clients[i]->is_stopped());
			ASIO2_CHECK(!clients[i]->is_started());
		}

		ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == test_client_count);

		// use this to ensure the ASIO2_CHECK(session_ptr->is_started());
		while (server_disconnect_counter != test_client_count)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
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

		server_start_ret = server.start("127.0.0.1", 18010);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		client_init_counter = 0;
		client_connect_counter = 0;
		client_disconnect_counter = 0;

		for (int i = 0; i < test_client_count; i++)
		{
			bool client_start_ret = clients[i]->async_start("127.0.0.1", 18010);
			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
		}

		while (server.get_session_count() < std::size_t(test_client_count))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		ASIO2_CHECK_VALUE(server.get_session_count(), server.get_session_count() == std::size_t(test_client_count));

		ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == test_client_count);
		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		while (client_connect_counter < test_client_count)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == test_client_count);
		ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == test_client_count);

		while (client_disconnect_counter < test_client_count)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));

			msg.clear();
			int len = 200 + (std::rand() % 200);
			for (int i = 0; i < len; i++)
			{
				msg += (char)(std::rand() % 255);
			}
			// send random data to server for every client, if the data is invalid, the session will be closed
			for (int i = 0; i < test_client_count; i++)
			{
				clients[i]->async_send(msg);
			}
		}

		ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			ASIO2_CHECK(!clients[i]->is_stopped());
			ASIO2_CHECK(!clients[i]->is_started());
		}

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->stop();
			ASIO2_CHECK( clients[i]->is_stopped());
			ASIO2_CHECK(!clients[i]->is_started());
		}

		ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == test_client_count);

		// use this to ensure the ASIO2_CHECK(session_ptr->is_started());
		while (server_disconnect_counter != test_client_count)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		server.stop();
		ASIO2_CHECK(server.is_stopped());

		ASIO2_CHECK_VALUE(server_disconnect_counter.load(), server_disconnect_counter == test_client_count);
		ASIO2_CHECK_VALUE(server_stop_counter      .load(), server_stop_counter       == 1);
	}

	// test max buffer size and auto reconnect
	{
		struct ext_data
		{
			int data_zie = 0;
			int send_counter = 0;
			int client_init_counter = 0;
			int client_connect_counter = 0;
			int client_disconnect_counter = 0;
			std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();
		};

		asio2::rpc_server server(512, 1024, 4);

		std::atomic<int> server_accept_counter = 0;
		server.bind_accept([&](auto & session_ptr)
		{
			session_ptr->no_delay(true);

			server_accept_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18010);
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18010);
			ASIO2_CHECK(server.io().strand().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).strand().running_in_this_thread());

		});
		std::atomic<int> server_connect_counter = 0;
		server.bind_connect([&](auto & session_ptr)
		{
			server_connect_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18010);
			ASIO2_CHECK(server.io().strand().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).strand().running_in_this_thread());
			ASIO2_CHECK(session_ptr->is_keep_alive());
			ASIO2_CHECK(session_ptr->is_no_delay());

		});
		std::atomic<int> server_disconnect_counter = 0;
		server.bind_disconnect([&](auto & session_ptr)
		{
			server_disconnect_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(session_ptr->socket().is_open());
			ASIO2_CHECK(server.io().strand().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).strand().running_in_this_thread());
		});
		std::atomic<int> server_init_counter = 0;
		server.bind_init([&]()
		{
			server_init_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(server.io().strand().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).strand().running_in_this_thread());

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
			ASIO2_CHECK(server.get_listen_port() == 18010);
			ASIO2_CHECK(server.io().strand().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).strand().running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18010);
			ASIO2_CHECK(server.io().strand().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).strand().running_in_this_thread());
		});

		server.bind("echo", echo);

		bool server_start_ret = server.start("127.0.0.1", 18010);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		std::vector<std::shared_ptr<asio2::rpc_client>> clients;
		for (int i = 0; i < test_client_count; i++)
		{
			auto iter = clients.emplace_back(std::make_shared<asio2::rpc_client>());

			asio2::rpc_client& client = *iter;

			// set default rpc call timeout
			client.set_default_timeout(std::chrono::seconds(3));
			ASIO2_CHECK(client.get_default_timeout() == std::chrono::seconds(3));
			client.set_connect_timeout(std::chrono::milliseconds(100));
			client.set_auto_reconnect(true, std::chrono::milliseconds(100));
			ASIO2_CHECK(client.is_auto_reconnect());
			ASIO2_CHECK(client.get_auto_reconnect_delay() == std::chrono::milliseconds(100));
			ASIO2_CHECK(client.get_connect_timeout() == std::chrono::milliseconds(100));

			client.bind_init([&]()
			{
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.client_init_counter++;

				client.set_no_delay(true);

				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.is_keep_alive());
				ASIO2_CHECK(client.is_reuse_address());
				ASIO2_CHECK(client.is_no_delay());
			});
			client.bind_connect([&]()
			{
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.get_local_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_port() == 18010);

				ext_data& ex = client.get_user_data<ext_data&>();

				std::string msg;
				msg.resize(ex.data_zie);
				client.async_call("echo", msg).response([&client](std::string s)
				{
					ext_data& ex = client.get_user_data<ext_data&>();
					ex.send_counter++;

					if (ex.data_zie <= 1024)
					{
						ASIO2_CHECK(!s.empty());
						ASIO2_CHECK_VALUE(asio2::last_error_msg(), !asio2::get_last_error());
					}
					else
					{
						ASIO2_CHECK(s.empty());
						ASIO2_CHECK(asio2::get_last_error() == rpc::error::operation_aborted);
					}
				});

				ex.client_connect_counter++;

				if (ex.client_connect_counter == 1)
				{
					auto elapse1 = std::abs(std::chrono::duration_cast<std::chrono::milliseconds>(
						std::chrono::high_resolution_clock::now() - ex.start_time).count() - 100);
					ASIO2_CHECK_VALUE(elapse1, elapse1 <= 100);
				}
				else
				{
					auto elapse1 = std::abs(std::chrono::duration_cast<std::chrono::milliseconds>(
						std::chrono::high_resolution_clock::now() - ex.start_time).count() - 200);
					ASIO2_CHECK_VALUE(elapse1, elapse1 <= 100);
				}

				ex.start_time = std::chrono::high_resolution_clock::now();

				if (ex.client_connect_counter == 3)
				{
					client.set_auto_reconnect(false);
					ASIO2_CHECK(!client.is_auto_reconnect());
				}

			});
			client.bind_disconnect([&]()
			{
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.client_disconnect_counter++;

				ASIO2_CHECK(asio2::get_last_error());
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
			});
			client.bind_recv([&]([[maybe_unused]] std::string_view data)
			{
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(!data.empty());
				ASIO2_CHECK(client.is_started());
			});

			ext_data ex;
			ex.data_zie = 1500;

			client.set_user_data(std::move(ex));

			bool client_start_ret = client.start("127.0.0.1", 18010);
			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(!asio2::get_last_error());
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			while (ex.client_connect_counter < 3)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			ASIO2_CHECK_VALUE(ex.client_init_counter, ex.client_init_counter == 3);
			ASIO2_CHECK_VALUE(ex.client_connect_counter, ex.client_connect_counter == 3);
		}

		while (server_connect_counter < test_client_count * 3)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == test_client_count * 3);
		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count * 3);

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			while (ex.client_disconnect_counter < 3)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ASIO2_CHECK(!clients[i]->is_stopped());
			ASIO2_CHECK(!clients[i]->is_started());
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			ASIO2_CHECK_VALUE(ex.client_disconnect_counter, ex.client_disconnect_counter == 3);
		}

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->stop();
			ASIO2_CHECK( clients[i]->is_stopped());
			ASIO2_CHECK(!clients[i]->is_started());
			ASIO2_CHECK(!clients[i]->user_data_any().has_value());
		}

		// use this to ensure the ASIO2_CHECK(session_ptr->is_started());
		while (server_disconnect_counter != test_client_count * 3)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		server.stop();
		ASIO2_CHECK(server.is_stopped());

		ASIO2_CHECK_VALUE(server_disconnect_counter.load(), server_disconnect_counter == test_client_count*3);
		ASIO2_CHECK_VALUE(server_stop_counter      .load(), server_stop_counter       == 1);

		//-----------------------------------------------------------------------------------------

		ASIO2_CHECK_VALUE(server.get_session_count(), server.get_session_count() == std::size_t(0));

		server_init_counter = 0;
		server_start_counter = 0;
		server_disconnect_counter = 0;
		server_stop_counter = 0;
		server_accept_counter = 0;
		server_connect_counter = 0;

		server_start_ret = server.start("127.0.0.1", 18010);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		for (int i = 0; i < test_client_count; i++)
		{
			asio2::rpc_client& client = *clients[i];

			ASIO2_CHECK(client.get_default_timeout() == std::chrono::seconds(3));
			client.set_auto_reconnect(true, std::chrono::milliseconds(100));
			ASIO2_CHECK(client.is_auto_reconnect());
			ASIO2_CHECK(client.get_auto_reconnect_delay() == std::chrono::milliseconds(100));
			ASIO2_CHECK(client.get_connect_timeout() == std::chrono::milliseconds(100));

			ext_data ex;
			ex.data_zie = 500;

			client.set_user_data(std::move(ex));

			bool client_start_ret = client.start("127.0.0.1", 18010);
			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(!asio2::get_last_error());
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			while (ex.client_connect_counter < 1)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			ASIO2_CHECK_VALUE(ex.client_init_counter, ex.client_init_counter == 1);
			ASIO2_CHECK_VALUE(ex.client_connect_counter, ex.client_connect_counter == 1);
		}

		while (server_connect_counter < test_client_count * 1)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == test_client_count * 1);
		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count * 1);

		for (int i = 0; i < test_client_count; i++)
		{
			ASIO2_CHECK(!clients[i]->is_stopped());
			ASIO2_CHECK( clients[i]->is_started());
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			while (ex.send_counter < 1)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->stop();
			ASIO2_CHECK( clients[i]->is_stopped());
			ASIO2_CHECK(!clients[i]->is_started());
			ASIO2_CHECK(!clients[i]->user_data_any().has_value());
		}

		// use this to ensure the ASIO2_CHECK(session_ptr->is_started());
		while (server_disconnect_counter != test_client_count * 1)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		server.stop();
		ASIO2_CHECK(server.is_stopped());

		ASIO2_CHECK_VALUE(server_disconnect_counter.load(), server_disconnect_counter == test_client_count*1);
		ASIO2_CHECK_VALUE(server_stop_counter      .load(), server_stop_counter       == 1);
	}

	// test rpc function
	{
		struct ext_data
		{
			int async_call_counter = 0;
			int sync_call_counter = 0;
			int client_init_counter = 0;
			int client_connect_counter = 0;
			int client_disconnect_counter = 0;
		};

		asio2::rpc_server server;

		std::atomic<int> server_accept_counter = 0;
		std::atomic<int> server_async_call_counter = 0;
		server.bind_accept([&](auto & session_ptr)
		{
			session_ptr->no_delay(true);

			server_accept_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18010);
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18010);
			ASIO2_CHECK(server.io().strand().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).strand().running_in_this_thread());

		});
		std::atomic<int> server_connect_counter = 0;
		server.bind_connect([&](auto & session_ptr)
		{
			server_connect_counter++;

			session_ptr->async_call([&, session_ptr](int v)
			{
				server_async_call_counter++;
				ASIO2_CHECK(session_ptr->io().strand().running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(v == 15 - 6);
			}, std::chrono::seconds(10), "sub", 15, 6);

			session_ptr->async_call("test", "i love you");

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18010);
			ASIO2_CHECK(server.io().strand().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).strand().running_in_this_thread());
			ASIO2_CHECK(session_ptr->is_keep_alive());
			ASIO2_CHECK(session_ptr->is_no_delay());

		});
		std::atomic<int> server_disconnect_counter = 0;
		server.bind_disconnect([&](auto & session_ptr)
		{
			server_disconnect_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(session_ptr->socket().is_open());
			ASIO2_CHECK(server.io().strand().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).strand().running_in_this_thread());
		});
		std::atomic<int> server_init_counter = 0;
		server.bind_init([&]()
		{
			server_init_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(server.io().strand().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).strand().running_in_this_thread());

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
			ASIO2_CHECK(server.get_listen_port() == 18010);
			ASIO2_CHECK(server.io().strand().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).strand().running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18010);
			ASIO2_CHECK(server.io().strand().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).strand().running_in_this_thread());
		});

		server.bind("echo", echo);

		usercrud crud;

		server
			.bind("add", add)
			.bind("mul", &usercrud::mul, crud)
			.bind("get_user", &usercrud::get_user, crud)
			.bind("del_user", &usercrud::del_user, &crud);

		server.bind("async_add", async_add);
		server.bind("async_test", async_test);

		server.bind("test_json", test_json);
		server.bind("heartbeat", heartbeat);

		server.bind("cat", [&](std::shared_ptr<asio2::rpc_session>& session_ptr,
			const std::string& a, const std::string& b)
		{
			ASIO2_CHECK(session_ptr->io().strand().running_in_this_thread());
			ASIO2_CHECK(!asio2::get_last_error());

			// Nested call rpc function in business function is ok.
			session_ptr->async_call([&, session_ptr](int v) mutable
			{
				server_async_call_counter++;

				ASIO2_CHECK(session_ptr->io().strand().running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(v == 15 - 8);

				// Nested call rpc function in business function is ok.
				session_ptr->async_call([&, session_ptr](int v)
				{
					server_async_call_counter++;
					ASIO2_CHECK(session_ptr->io().strand().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ASIO2_CHECK(v == 15 + 18);
				}, "async_add", 15, 18);

			}, "sub", 15, 8);

			return a + b;
		});

		bool server_start_ret = server.start("127.0.0.1", 18010);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		std::vector<std::shared_ptr<asio2::rpc_client>> clients;
		for (int i = 0; i < test_client_count; i++)
		{
			auto iter = clients.emplace_back(std::make_shared<asio2::rpc_client>());

			asio2::rpc_client& client = *iter;

			// set default rpc call timeout
			client.set_default_timeout(std::chrono::seconds(3));
			ASIO2_CHECK(client.get_default_timeout() == std::chrono::seconds(3));
			client.set_auto_reconnect(false);
			ASIO2_CHECK(!client.is_auto_reconnect());

			client.bind_init([&]()
			{
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.client_init_counter++;

				client.set_no_delay(true);

				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.is_keep_alive());
				ASIO2_CHECK(client.is_reuse_address());
				ASIO2_CHECK(client.is_no_delay());
			});
			client.bind_connect([&]()
			{
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.get_local_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_port() == 18010);

				ext_data& ex = client.get_user_data<ext_data&>();
				ex.client_connect_counter++;

				//------------------------------------------------------------------
				client.call<double>("mul", 16.5, 26.5);
				ASIO2_CHECK(asio2::get_last_error() == rpc::error::in_progress);

				client.async_call([&](int v)
				{
					ASIO2_CHECK(client.io().strand().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ASIO2_CHECK(v == 12 + 11);
					ex.async_call_counter++;
				}, std::chrono::seconds(13), "add", 12, 11);

				client.async_call([&]()
				{
					ASIO2_CHECK(client.io().strand().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ex.async_call_counter++;
				}, std::chrono::seconds(3), "heartbeat");

				nlohmann::json j = nlohmann::json::object();

				j["name"] = "lilei";
				j["age"] = 30;

				client.async_call("test_json", j).response([&](nlohmann::json js)
				{
					ASIO2_CHECK(client.io().strand().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ex.async_call_counter++;

					std::string s = js.dump();

					ASIO2_CHECK(js["age"].get<int>() == 30);
					ASIO2_CHECK(js["name"].get<std::string>() == "lilei");

					asio2::ignore_unused(js, s);
				});

				// param 2 is empty, use the default_timeout
				client.async_call([&](int v)
				{
					ASIO2_CHECK(client.io().strand().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ASIO2_CHECK(v == 12 + 21);
					ex.async_call_counter++;
				}, "add", 12, 21);

				// param 1 is empty, the result of the rpc call is not returned
				client.async_call("mul0", 2.5, 2.5);

				// Chain calls : 
				client.set_timeout(std::chrono::seconds(5)).async_call("mul", 2.5, 2.5).response([&](double v)
				{
					ASIO2_CHECK(client.io().strand().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ASIO2_CHECK(v == 2.5 * 2.5);
					ex.async_call_counter++;
				});

				// Chain calls : 
				client.timeout(std::chrono::seconds(13)).response([&](double v)
				{
					ASIO2_CHECK(client.io().strand().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ASIO2_CHECK(v == 3.5 * 3.5);
					ex.async_call_counter++;
				}).async_call("mul", 3.5, 3.5);

				// Chain calls : 
				client.response([&](double v)
				{
					ASIO2_CHECK(client.io().strand().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ASIO2_CHECK(v == 4.5 * 4.5);
					ex.async_call_counter++;
				}).timeout(std::chrono::seconds(5)).async_call("mul", 4.5, 4.5);

				// Chain calls : 
				client.async_call("mul", 5.5, 5.5).response([&](double v)
				{
					ASIO2_CHECK(client.io().strand().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ASIO2_CHECK(v == 5.5 * 5.5);
					ex.async_call_counter++;
				}).timeout(std::chrono::seconds(10));

				client.async_call([&](int v)
				{
					ASIO2_CHECK(client.io().strand().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ASIO2_CHECK(v == 1 + 11);
					ex.async_call_counter++;
				}, std::chrono::seconds(3), "async_add", 1, 11);

				client.async_call([&]()
				{
					ASIO2_CHECK(client.io().strand().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ex.async_call_counter++;
				}, std::chrono::seconds(3), "async_test", "abc", "def");

			});
			client.bind_disconnect([&]()
			{
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.client_disconnect_counter++;

				ASIO2_CHECK(asio2::get_last_error());
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
			});
			client.bind_recv([&]([[maybe_unused]] std::string_view data)
			{
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(!data.empty());
				ASIO2_CHECK(client.is_started());
			});

			// bind a rpc function in client, the server will call this client's rpc function
			client.bind("sub", [](int a, int b) { return a - b; });

			client.bind("async_add", async_add);

			client.bind("test", client_fn_test);

			ext_data ex;

			client.set_user_data(std::move(ex));

			bool client_start_ret = client.start("127.0.0.1", 18010);

			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(!asio2::get_last_error());

			nlohmann::json j = nlohmann::json::object();

			j["name"] = "hanmeimei";
			j["age"] = 20;

			nlohmann::json j2 = client.call<nlohmann::json>(std::chrono::minutes(1), "test_json", j);
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(j2["age"].get<int>() == 20);
			ASIO2_CHECK(j2["name"].get<std::string>() == "hanmeimei");

			double mul = client.call<double>(std::chrono::seconds(3), "mul", 6.5, 6.5);
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(mul == 6.5 * 6.5);

			userinfo u;
			u = client.call<userinfo>("get_user");
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(u.name == "lilei" && u.age == 32 && u.purview.size() == 2);
			for (auto &[k, v] : u.purview)
			{
				ASIO2_CHECK(k == 1 || k == 2);
				if (k == 1) ASIO2_CHECK(v == "read");
				if (k == 2) ASIO2_CHECK(v == "write");
			}

			u.name = "hanmeimei";
			u.age = 33;
			u.purview = { {10,"get"},{20,"set"} };
			client.async_call([&]()
			{
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.async_call_counter++;
			}, "del_user", u);
	

			// just call rpc function, don't need the rpc result
			client.async_call("del_user", u);

			// this call will be failed, beacuse the param is incorrect.
			client.async_call("del_user", "hanmeimei").response([&](userinfo)
			{
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(asio2::get_last_error() == rpc::error::invalid_argument);
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.async_call_counter++;
			});

			// this call will be failed, beacuse the param is incorrect.
			client.async_call("del_user", 10, std::string("lilei"), 1).response([&](userinfo)
			{
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(asio2::get_last_error() == rpc::error::invalid_argument);
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.async_call_counter++;
			});

			// this call will be failed, beacuse the param is incorrect.
			client.async_call("del_user", u, std::string("lilei"), 1).response([&](userinfo)
			{
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(asio2::get_last_error() == rpc::error::invalid_argument);
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.async_call_counter++;
			});

			// Chain calls : 
			int sum = client.timeout(std::chrono::seconds(13)).call<int>("add", 11, 12);
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(sum == 11 + 12);

			// Chain calls : 
			sum = client.timeout(std::chrono::seconds(13)).call<int>("add", 11, 32);
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(sum == 11 + 32);

			sum = client.timeout(std::chrono::seconds(13)).call<int>("add", 11);
			ASIO2_CHECK(asio2::get_last_error() == rpc::error::invalid_argument);
			ASIO2_CHECK(sum == 0);

			sum = client.timeout(std::chrono::seconds(13)).call<int>("add", 11, 12, 13);
			ASIO2_CHECK(asio2::get_last_error() == rpc::error::invalid_argument);
			ASIO2_CHECK(sum == 0);

			sum = client.timeout(std::chrono::seconds(13)).call<int>("add", "11", 12, 13);
			ASIO2_CHECK(asio2::get_last_error() == rpc::error::invalid_argument);
			ASIO2_CHECK(sum == 0);

			// Chain calls : 
			std::string str = client.call<std::string>("cat", "abc", "123");
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(str == "abc123");

			client.async_call([&](int)
			{
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(asio2::get_last_error() == rpc::error::not_found);
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.async_call_counter++;
			}, "no_exists_fn", 10);

			sum = client.timeout(std::chrono::seconds(13)).call<int>("no_exists_fn", 12, 13);
			ASIO2_CHECK(asio2::get_last_error() == rpc::error::not_found);
			ASIO2_CHECK(sum == 0);
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			while (ex.client_connect_counter < 1)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			ASIO2_CHECK_VALUE(ex.client_init_counter, ex.client_init_counter == 1);
			ASIO2_CHECK_VALUE(ex.client_connect_counter, ex.client_connect_counter == 1);
		}

		while (server_connect_counter < test_client_count * 1)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == test_client_count * 1);
		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count * 1);

		for (int i = 0; i < test_client_count; i++)
		{
			ASIO2_CHECK(!clients[i]->is_stopped());
			ASIO2_CHECK( clients[i]->is_started());
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			while (ex.async_call_counter < 15)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			ASIO2_CHECK_VALUE(ex.async_call_counter, ex.async_call_counter == 15);
		}

		while (server_async_call_counter < test_client_count * 3)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		ASIO2_CHECK_VALUE(server_async_call_counter.load(), server_async_call_counter == test_client_count * 3);

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->stop();
			ASIO2_CHECK( clients[i]->is_stopped());
			ASIO2_CHECK(!clients[i]->is_started());
			ASIO2_CHECK(!clients[i]->user_data_any().has_value());
		}

		// use this to ensure the ASIO2_CHECK(session_ptr->is_started());
		while (server_disconnect_counter != test_client_count * 1)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		server.stop();
		ASIO2_CHECK(server.is_stopped());

		ASIO2_CHECK_VALUE(server_disconnect_counter.load(), server_disconnect_counter == test_client_count*1);
		ASIO2_CHECK_VALUE(server_stop_counter      .load(), server_stop_counter       == 1);

		//-----------------------------------------------------------------------------------------

		ASIO2_CHECK_VALUE(server.get_session_count(), server.get_session_count() == std::size_t(0));

		server_init_counter = 0;
		server_start_counter = 0;
		server_disconnect_counter = 0;
		server_stop_counter = 0;
		server_accept_counter = 0;
		server_connect_counter = 0;
		server_async_call_counter = 0;

		server_start_ret = server.start("127.0.0.1", 18010);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		for (int i = 0; i < test_client_count; i++)
		{
			asio2::rpc_client& client = *clients[i];

			ASIO2_CHECK(client.get_default_timeout() == std::chrono::seconds(3));
			client.set_auto_reconnect(true, std::chrono::milliseconds(100));
			ASIO2_CHECK(client.is_auto_reconnect());
			ASIO2_CHECK(client.get_auto_reconnect_delay() == std::chrono::milliseconds(100));

			ext_data ex;

			client.set_user_data(std::move(ex));

			bool client_start_ret = client.async_start("127.0.0.1", 18010);
			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			while (ex.client_connect_counter < 1)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			ASIO2_CHECK_VALUE(ex.client_init_counter, ex.client_init_counter == 1);
			ASIO2_CHECK_VALUE(ex.client_connect_counter, ex.client_connect_counter == 1);
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ASIO2_CHECK(!clients[i]->is_stopped());
			ASIO2_CHECK( clients[i]->is_started());
		}

		for (int i = 0; i < test_client_count; i++)
		{
			asio2::rpc_client& client = *clients[i];

			nlohmann::json j = nlohmann::json::object();

			j["name"] = "hanmeimei";
			j["age"] = 20;

			nlohmann::json j2 = client.call<nlohmann::json>(std::chrono::minutes(1), "test_json", j);
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(j2["age"].get<int>() == 20);
			ASIO2_CHECK(j2["name"].get<std::string>() == "hanmeimei");

			double mul = client.call<double>(std::chrono::seconds(3), "mul", 6.5, 6.5);
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(mul == 6.5 * 6.5);

			userinfo u;
			u = client.call<userinfo>("get_user");
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(u.name == "lilei" && u.age == 32 && u.purview.size() == 2);
			for (auto &[k, v] : u.purview)
			{
				ASIO2_CHECK(k == 1 || k == 2);
				if (k == 1) ASIO2_CHECK(v == "read");
				if (k == 2) ASIO2_CHECK(v == "write");
			}

			u.name = "hanmeimei";
			u.age = 33;
			u.purview = { {10,"get"},{20,"set"} };
			client.async_call([&]()
			{
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.async_call_counter++;
			}, "del_user", u);
	

			// just call rpc function, don't need the rpc result
			client.async_call("del_user", u);

			// this call will be failed, beacuse the param is incorrect.
			client.async_call("del_user", "hanmeimei").response([&](userinfo)
			{
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(asio2::get_last_error() == rpc::error::invalid_argument);
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.async_call_counter++;
			});

			// this call will be failed, beacuse the param is incorrect.
			client.async_call("del_user", 10, std::string("lilei"), 1).response([&](userinfo)
			{
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(asio2::get_last_error() == rpc::error::invalid_argument);
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.async_call_counter++;
			});

			// this call will be failed, beacuse the param is incorrect.
			client.async_call("del_user", u, std::string("lilei"), 1).response([&](userinfo)
			{
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(asio2::get_last_error() == rpc::error::invalid_argument);
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.async_call_counter++;
			});

			// Chain calls : 
			int sum = client.timeout(std::chrono::seconds(13)).call<int>("add", 11, 12);
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(sum == 11 + 12);

			// Chain calls : 
			sum = client.timeout(std::chrono::seconds(13)).call<int>("add", 11, 32);
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(sum == 11 + 32);

			sum = client.timeout(std::chrono::seconds(13)).call<int>("add", 11);
			ASIO2_CHECK(asio2::get_last_error() == rpc::error::invalid_argument);
			ASIO2_CHECK(sum == 0);

			sum = client.timeout(std::chrono::seconds(13)).call<int>("add", 11, 12, 13);
			ASIO2_CHECK(asio2::get_last_error() == rpc::error::invalid_argument);
			ASIO2_CHECK(sum == 0);

			sum = client.timeout(std::chrono::seconds(13)).call<int>("add", "11", 12, 13);
			ASIO2_CHECK(asio2::get_last_error() == rpc::error::invalid_argument);
			ASIO2_CHECK(sum == 0);

			// Chain calls : 
			std::string str = client.call<std::string>("cat", "abc", "123");
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(str == "abc123");

			client.async_call([&](int)
			{
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(asio2::get_last_error() == rpc::error::not_found);
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.async_call_counter++;
			}, "no_exists_fn", 10);

			sum = client.timeout(std::chrono::seconds(13)).call<int>("no_exists_fn", 12, 13);
			ASIO2_CHECK(asio2::get_last_error() == rpc::error::not_found);
			ASIO2_CHECK(sum == 0);
		}

		while (server_connect_counter < test_client_count * 1)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == test_client_count * 1);
		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count * 1);

		for (int i = 0; i < test_client_count; i++)
		{
			ASIO2_CHECK(!clients[i]->is_stopped());
			ASIO2_CHECK( clients[i]->is_started());
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			while (ex.async_call_counter < 15)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			ASIO2_CHECK_VALUE(ex.async_call_counter, ex.async_call_counter == 15);
		}

		while (server_async_call_counter < test_client_count * 3)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		ASIO2_CHECK_VALUE(server_async_call_counter.load(), server_async_call_counter == test_client_count * 3);

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->stop();
			ASIO2_CHECK( clients[i]->is_stopped());
			ASIO2_CHECK(!clients[i]->is_started());
			ASIO2_CHECK(!clients[i]->user_data_any().has_value());
		}

		// use this to ensure the ASIO2_CHECK(session_ptr->is_started());
		while (server_disconnect_counter != test_client_count * 1)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		server.stop();
		ASIO2_CHECK(server.is_stopped());

		ASIO2_CHECK_VALUE(server_disconnect_counter.load(), server_disconnect_counter == test_client_count*1);
		ASIO2_CHECK_VALUE(server_stop_counter      .load(), server_stop_counter       == 1);
	}

	// test websocket rpc function 
	{
		struct ext_data
		{
			int async_call_counter = 0;
			int sync_call_counter = 0;
			int client_init_counter = 0;
			int client_connect_counter = 0;
			int client_disconnect_counter = 0;
		};

		asio2::rpc_server_use<asio2::net_protocol::ws> server;

		std::atomic<int> server_accept_counter = 0;
		std::atomic<int> server_async_call_counter = 0;
		server.bind_accept([&](auto & session_ptr)
		{
			session_ptr->no_delay(true);

			server_accept_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18010);
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18010);
			ASIO2_CHECK(server.io().strand().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).strand().running_in_this_thread());

		});
		std::atomic<int> server_connect_counter = 0;
		server.bind_connect([&](auto & session_ptr)
		{
			server_connect_counter++;

			session_ptr->async_call([&, session_ptr](int v)
			{
				server_async_call_counter++;
				ASIO2_CHECK(session_ptr->io().strand().running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(v == 15 - 6);
			}, std::chrono::seconds(10), "sub", 15, 6);

			session_ptr->async_call("test", "i love you");

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18010);
			ASIO2_CHECK(server.io().strand().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).strand().running_in_this_thread());
			ASIO2_CHECK(session_ptr->is_keep_alive());
			ASIO2_CHECK(session_ptr->is_no_delay());

		});
		std::atomic<int> server_disconnect_counter = 0;
		server.bind_disconnect([&](auto & session_ptr)
		{
			server_disconnect_counter++;
			asio2::ignore_unused(session_ptr);
			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.io().strand().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).strand().running_in_this_thread());
		});
		std::atomic<int> server_init_counter = 0;
		server.bind_init([&]()
		{
			server_init_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(server.io().strand().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).strand().running_in_this_thread());

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
			ASIO2_CHECK(server.get_listen_port() == 18010);
			ASIO2_CHECK(server.io().strand().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).strand().running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18010);
			ASIO2_CHECK(server.io().strand().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0).strand().running_in_this_thread());
		});

		server.bind("echo", echo);

		usercrud crud;

		server
			.bind("add", add)
			.bind("mul", &usercrud::mul, crud)
			.bind("get_user", []
			(std::shared_ptr<asio2::rpc_session_use<asio2::net_protocol::ws>>& session_ptr)
			{
				ASIO2_CHECK(session_ptr->io().strand().running_in_this_thread());

				userinfo u;
				u.name = "lilei";
				u.age = 32;
				u.purview = { {1,"read"},{2,"write"} };
				return u;
			})
			.bind("del_user", []
			(std::shared_ptr<asio2::rpc_session_use<asio2::net_protocol::ws>>& session_ptr, const userinfo& u)
			{
				ASIO2_CHECK(session_ptr->io().strand().running_in_this_thread());
				ASIO2_CHECK(u.name == "hanmeimei");
				ASIO2_CHECK(u.age == 33);
				ASIO2_CHECK(u.purview.size() == 2);
				for (auto&[k, v] : u.purview)
				{
					ASIO2_CHECK(k == 10 || k == 20);
					if (k == 10) ASIO2_CHECK(v == "get");
					if (k == 20) ASIO2_CHECK(v == "set");
				}
			});

		server.bind("async_add", async_add);
		server.bind("async_test", []
		(std::shared_ptr<asio2::rpc_session_use<asio2::net_protocol::ws>>& session_ptr, std::string a, std::string b)
		{
			ASIO2_CHECK(session_ptr->io().strand().running_in_this_thread());

			rpc::promise<void> promise;
			rpc::future<void> f = promise.get_future();

			ASIO2_CHECK(a == "abc" && b == "def");

			std::thread([a, b, promise]() mutable
			{
				asio2::ignore_unused(a, b);
				promise.set_value();
			}).detach();

			return f;
		});

		server.bind("test_json", test_json);
		server.bind("heartbeat", []
		(std::shared_ptr<asio2::rpc_session_use<asio2::net_protocol::ws>>& session_ptr)
		{
			ASIO2_CHECK(session_ptr->io().strand().running_in_this_thread());
		});

		server.bind("cat", [&](std::shared_ptr<asio2::rpc_session_use<asio2::net_protocol::ws>>& session_ptr,
			const std::string& a, const std::string& b)
		{
			ASIO2_CHECK(session_ptr->io().strand().running_in_this_thread());
			ASIO2_CHECK(!asio2::get_last_error());

			// Nested call rpc function in business function is ok.
			session_ptr->async_call([&, session_ptr](int v) mutable
			{
				server_async_call_counter++;

				ASIO2_CHECK(session_ptr->io().strand().running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(v == 15 - 8);

				// Nested call rpc function in business function is ok.
				session_ptr->async_call([&, session_ptr](int v)
				{
					server_async_call_counter++;
					ASIO2_CHECK(session_ptr->io().strand().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ASIO2_CHECK(v == 15 + 18);
				}, "async_add", 15, 18);

			}, "sub", 15, 8);

			return a + b;
		});

		bool server_start_ret = server.start("127.0.0.1", 18010);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		std::vector<std::shared_ptr<asio2::rpc_client_use<asio2::net_protocol::ws>>> clients;
		for (int i = 0; i < test_client_count; i++)
		{
			auto iter = clients.emplace_back(std::make_shared<asio2::rpc_client_use<asio2::net_protocol::ws>>());

			asio2::rpc_client_use<asio2::net_protocol::ws>& client = *iter;

			// set default rpc call timeout
			client.set_default_timeout(std::chrono::seconds(3));
			ASIO2_CHECK(client.get_default_timeout() == std::chrono::seconds(3));
			client.set_auto_reconnect(false);
			ASIO2_CHECK(!client.is_auto_reconnect());

			client.bind_init([&]()
			{
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.client_init_counter++;

				client.set_no_delay(true);

				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.is_keep_alive());
				ASIO2_CHECK(client.is_reuse_address());
				ASIO2_CHECK(client.is_no_delay());
			});
			client.bind_connect([&]()
			{
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.get_local_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_port() == 18010);

				ext_data& ex = client.get_user_data<ext_data&>();
				ex.client_connect_counter++;

				//------------------------------------------------------------------
				client.call<double>("mul", 16.5, 26.5);
				ASIO2_CHECK(asio2::get_last_error() == rpc::error::in_progress);

				client.async_call([&](int v)
				{
					ASIO2_CHECK(client.io().strand().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ASIO2_CHECK(v == 12 + 11);
					ex.async_call_counter++;
				}, std::chrono::seconds(13), "add", 12, 11);

				client.async_call<int>([&](auto v)
				{
					ASIO2_CHECK(client.io().strand().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ASIO2_CHECK(v == 112 + 11);
					ex.async_call_counter++;
				}, std::chrono::seconds(13), "add", 112, 11);

				client.async_call<int>([&](int v)
				{
					ASIO2_CHECK(client.io().strand().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ASIO2_CHECK(v == 1112 + 11);
					ex.async_call_counter++;
				}, std::chrono::seconds(13), "add", 1112, 11);

				client.async_call<std::string>([&](auto v)
				{
					ASIO2_CHECK(client.io().strand().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ex.async_call_counter++;
					ASIO2_CHECK(v == "abcdef0123456789xx");
				}, "echo", "abcdef0123456789xx");

				client.async_call<std::string>([&](std::string v)
				{
					ASIO2_CHECK(client.io().strand().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ASIO2_CHECK(v == "xxabcdef0123456789");
					ex.async_call_counter++;
				}, "echo", "xxabcdef0123456789");

				client.async_call([&]()
				{
					ASIO2_CHECK(client.io().strand().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ex.async_call_counter++;
				}, std::chrono::seconds(3), "heartbeat");

				nlohmann::json j = nlohmann::json::object();

				j["name"] = "lilei";
				j["age"] = 30;

				client.async_call("test_json", j).response([&](nlohmann::json js)
				{
					ASIO2_CHECK(client.io().strand().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ex.async_call_counter++;

					std::string s = js.dump();

					ASIO2_CHECK(js["age"].get<int>() == 30);
					ASIO2_CHECK(js["name"].get<std::string>() == "lilei");

					asio2::ignore_unused(js, s);
				});

				// param 2 is empty, use the default_timeout
				client.async_call([&](int v)
				{
					ASIO2_CHECK(client.io().strand().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ASIO2_CHECK(v == 12 + 21);
					ex.async_call_counter++;
				}, "add", 12, 21);

				// param 1 is empty, the result of the rpc call is not returned
				client.async_call("mul0", 2.5, 2.5);

				// Chain calls : 
				client.set_timeout(std::chrono::seconds(5)).async_call("mul", 2.5, 2.5).response([&](double v)
				{
					ASIO2_CHECK(client.io().strand().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ASIO2_CHECK(v == 2.5 * 2.5);
					ex.async_call_counter++;
				});

				// Chain calls : 
				client.timeout(std::chrono::seconds(13)).response([&](double v)
				{
					ASIO2_CHECK(client.io().strand().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ASIO2_CHECK(v == 3.5 * 3.5);
					ex.async_call_counter++;
				}).async_call("mul", 3.5, 3.5);

				// Chain calls : 
				client.response([&](double v)
				{
					ASIO2_CHECK(client.io().strand().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ASIO2_CHECK(v == 4.5 * 4.5);
					ex.async_call_counter++;
				}).timeout(std::chrono::seconds(5)).async_call("mul", 4.5, 4.5);

				// Chain calls : 
				client.async_call("mul", 5.5, 5.5).response([&](double v)
				{
					ASIO2_CHECK(client.io().strand().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ASIO2_CHECK(v == 5.5 * 5.5);
					ex.async_call_counter++;
				}).timeout(std::chrono::seconds(10));

				client.async_call([&](int v)
				{
					ASIO2_CHECK(client.io().strand().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ASIO2_CHECK(v == 1 + 11);
					ex.async_call_counter++;
				}, std::chrono::seconds(3), "async_add", 1, 11);

				client.async_call([&]()
				{
					ASIO2_CHECK(client.io().strand().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ex.async_call_counter++;
				}, std::chrono::seconds(3), "async_test", "abc", "def");

			});
			client.bind_disconnect([&]()
			{
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.client_disconnect_counter++;

				ASIO2_CHECK(asio2::get_last_error());
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
			});
			client.bind_recv([&]([[maybe_unused]] std::string_view data)
			{
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(!data.empty());
				ASIO2_CHECK(client.is_started());
			});

			// bind a rpc function in client, the server will call this client's rpc function
			client.bind("sub", [](int a, int b) { return a - b; });

			client.bind("async_add", async_add);

			client.bind("test", [](asio2::rpc_client_use<asio2::net_protocol::ws>& client, std::string str)
			{
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(str == "i love you");
			});

			ext_data ex;

			client.set_user_data(std::move(ex));

			bool client_start_ret = client.start("127.0.0.1", 18010);

			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(!asio2::get_last_error());

			nlohmann::json j = nlohmann::json::object();

			j["name"] = "hanmeimei";
			j["age"] = 20;

			nlohmann::json j2 = client.call<nlohmann::json>(std::chrono::minutes(1), "test_json", j);
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(j2["age"].get<int>() == 20);
			ASIO2_CHECK(j2["name"].get<std::string>() == "hanmeimei");

			double mul = client.call<double>(std::chrono::seconds(3), "mul", 6.5, 6.5);
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(mul == 6.5 * 6.5);

			userinfo u;
			u = client.call<userinfo>("get_user");
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(u.name == "lilei" && u.age == 32 && u.purview.size() == 2);
			for (auto &[k, v] : u.purview)
			{
				ASIO2_CHECK(k == 1 || k == 2);
				if (k == 1) ASIO2_CHECK(v == "read");
				if (k == 2) ASIO2_CHECK(v == "write");
			}

			u.name = "hanmeimei";
			u.age = 33;
			u.purview = { {10,"get"},{20,"set"} };
			client.async_call([&]()
			{
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.async_call_counter++;
			}, "del_user", u);
	

			// just call rpc function, don't need the rpc result
			client.async_call("del_user", u);

			// this call will be failed, beacuse the param is incorrect.
			client.async_call("del_user", "hanmeimei").response([&](userinfo)
			{
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(asio2::get_last_error() == rpc::error::invalid_argument);
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.async_call_counter++;
			});

			// this call will be failed, beacuse the param is incorrect.
			client.async_call("del_user", 10, std::string("lilei"), 1).response([&](userinfo)
			{
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(asio2::get_last_error() == rpc::error::invalid_argument);
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.async_call_counter++;
			});

			// this call will be failed, beacuse the param is incorrect.
			client.async_call("del_user", u, std::string("lilei"), 1).response([&](userinfo)
			{
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(asio2::get_last_error() == rpc::error::invalid_argument);
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.async_call_counter++;
			});

			// Chain calls : 
			int sum = client.timeout(std::chrono::seconds(13)).call<int>("add", 11, 12);
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(sum == 11 + 12);

			// Chain calls : 
			sum = client.timeout(std::chrono::seconds(13)).call<int>("add", 11, 32);
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(sum == 11 + 32);

			sum = client.timeout(std::chrono::seconds(13)).call<int>("add", 11);
			ASIO2_CHECK(asio2::get_last_error() == rpc::error::invalid_argument);
			ASIO2_CHECK(sum == 0);

			sum = client.timeout(std::chrono::seconds(13)).call<int>("add", 11, 12, 13);
			ASIO2_CHECK(asio2::get_last_error() == rpc::error::invalid_argument);
			ASIO2_CHECK(sum == 0);

			sum = client.timeout(std::chrono::seconds(13)).call<int>("add", "11", 12, 13);
			ASIO2_CHECK(asio2::get_last_error() == rpc::error::invalid_argument);
			ASIO2_CHECK(sum == 0);

			// Chain calls : 
			std::string str = client.call<std::string>("cat", "abc", "123");
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(str == "abc123");

			client.async_call([&](int)
			{
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(asio2::get_last_error() == rpc::error::not_found);
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.async_call_counter++;
			}, "no_exists_fn", 10);

			sum = client.timeout(std::chrono::seconds(13)).call<int>("no_exists_fn", 12, 13);
			ASIO2_CHECK(asio2::get_last_error() == rpc::error::not_found);
			ASIO2_CHECK(sum == 0);
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			while (ex.client_connect_counter < 1)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			ASIO2_CHECK_VALUE(ex.client_init_counter, ex.client_init_counter == 1);
			ASIO2_CHECK_VALUE(ex.client_connect_counter, ex.client_connect_counter == 1);
		}

		while (server_connect_counter < test_client_count * 1)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == test_client_count * 1);
		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count * 1);

		for (int i = 0; i < test_client_count; i++)
		{
			ASIO2_CHECK(!clients[i]->is_stopped());
			ASIO2_CHECK( clients[i]->is_started());
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			while (ex.async_call_counter < 19)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			ASIO2_CHECK_VALUE(ex.async_call_counter, ex.async_call_counter == 19);
		}

		while (server_async_call_counter < test_client_count * 3)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		ASIO2_CHECK_VALUE(server_async_call_counter.load(), server_async_call_counter == test_client_count * 3);

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->stop();
			ASIO2_CHECK( clients[i]->is_stopped());
			ASIO2_CHECK(!clients[i]->is_started());
			ASIO2_CHECK(!clients[i]->user_data_any().has_value());
		}

		// use this to ensure the ASIO2_CHECK(session_ptr->is_started());
		while (server_disconnect_counter != test_client_count * 1)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		server.stop();
		ASIO2_CHECK(server.is_stopped());

		ASIO2_CHECK_VALUE(server_disconnect_counter.load(), server_disconnect_counter == test_client_count*1);
		ASIO2_CHECK_VALUE(server_stop_counter      .load(), server_stop_counter       == 1);

		//-----------------------------------------------------------------------------------------

		ASIO2_CHECK_VALUE(server.get_session_count(), server.get_session_count() == std::size_t(0));

		server_init_counter = 0;
		server_start_counter = 0;
		server_disconnect_counter = 0;
		server_stop_counter = 0;
		server_accept_counter = 0;
		server_connect_counter = 0;
		server_async_call_counter = 0;

		server_start_ret = server.start("127.0.0.1", 18010);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		for (int i = 0; i < test_client_count; i++)
		{
			asio2::rpc_client_use<asio2::net_protocol::ws>& client = *clients[i];

			ASIO2_CHECK(client.get_default_timeout() == std::chrono::seconds(3));
			client.set_auto_reconnect(true, std::chrono::milliseconds(100));
			ASIO2_CHECK(client.is_auto_reconnect());
			ASIO2_CHECK(client.get_auto_reconnect_delay() == std::chrono::milliseconds(100));

			ext_data ex;

			client.set_user_data(std::move(ex));

			bool client_start_ret = client.async_start("127.0.0.1", 18010);
			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			while (ex.client_connect_counter < 1)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			ASIO2_CHECK_VALUE(ex.client_init_counter, ex.client_init_counter == 1);
			ASIO2_CHECK_VALUE(ex.client_connect_counter, ex.client_connect_counter == 1);
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ASIO2_CHECK(!clients[i]->is_stopped());
			ASIO2_CHECK( clients[i]->is_started());
		}

		for (int i = 0; i < test_client_count; i++)
		{
			asio2::rpc_client_use<asio2::net_protocol::ws>& client = *clients[i];

			nlohmann::json j = nlohmann::json::object();

			j["name"] = "hanmeimei";
			j["age"] = 20;

			nlohmann::json j2 = client.call<nlohmann::json>(std::chrono::minutes(1), "test_json", j);
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(j2["age"].get<int>() == 20);
			ASIO2_CHECK(j2["name"].get<std::string>() == "hanmeimei");

			double mul = client.call<double>(std::chrono::seconds(3), "mul", 6.5, 6.5);
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(mul == 6.5 * 6.5);

			userinfo u;
			u = client.call<userinfo>("get_user");
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(u.name == "lilei" && u.age == 32 && u.purview.size() == 2);
			for (auto &[k, v] : u.purview)
			{
				ASIO2_CHECK(k == 1 || k == 2);
				if (k == 1) ASIO2_CHECK(v == "read");
				if (k == 2) ASIO2_CHECK(v == "write");
			}

			u.name = "hanmeimei";
			u.age = 33;
			u.purview = { {10,"get"},{20,"set"} };
			client.async_call([&]()
			{
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.async_call_counter++;
			}, "del_user", u);
	

			// just call rpc function, don't need the rpc result
			client.async_call("del_user", u);

			// this call will be failed, beacuse the param is incorrect.
			client.async_call("del_user", "hanmeimei").response([&](userinfo)
			{
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(asio2::get_last_error() == rpc::error::invalid_argument);
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.async_call_counter++;
			});

			// this call will be failed, beacuse the param is incorrect.
			client.async_call("del_user", 10, std::string("lilei"), 1).response([&](userinfo)
			{
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(asio2::get_last_error() == rpc::error::invalid_argument);
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.async_call_counter++;
			});

			// this call will be failed, beacuse the param is incorrect.
			client.async_call("del_user", u, std::string("lilei"), 1).response([&](userinfo)
			{
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(asio2::get_last_error() == rpc::error::invalid_argument);
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.async_call_counter++;
			});

			// Chain calls : 
			int sum = client.timeout(std::chrono::seconds(13)).call<int>("add", 11, 12);
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(sum == 11 + 12);

			// Chain calls : 
			sum = client.timeout(std::chrono::seconds(13)).call<int>("add", 11, 32);
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(sum == 11 + 32);

			sum = client.timeout(std::chrono::seconds(13)).call<int>("add", 11);
			ASIO2_CHECK(asio2::get_last_error() == rpc::error::invalid_argument);
			ASIO2_CHECK(sum == 0);

			sum = client.timeout(std::chrono::seconds(13)).call<int>("add", 11, 12, 13);
			ASIO2_CHECK(asio2::get_last_error() == rpc::error::invalid_argument);
			ASIO2_CHECK(sum == 0);

			sum = client.timeout(std::chrono::seconds(13)).call<int>("add", "11", 12, 13);
			ASIO2_CHECK(asio2::get_last_error() == rpc::error::invalid_argument);
			ASIO2_CHECK(sum == 0);

			// Chain calls : 
			std::string str = client.call<std::string>("cat", "abc", "123");
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(str == "abc123");

			client.async_call([&](int)
			{
				ASIO2_CHECK(client.io().strand().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0).strand().running_in_this_thread());
				ASIO2_CHECK(asio2::get_last_error() == rpc::error::not_found);
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.async_call_counter++;
			}, "no_exists_fn", 10);

			sum = client.timeout(std::chrono::seconds(13)).call<int>("no_exists_fn", 12, 13);
			ASIO2_CHECK(asio2::get_last_error() == rpc::error::not_found);
			ASIO2_CHECK(sum == 0);
		}

		while (server_connect_counter < test_client_count * 1)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == test_client_count * 1);
		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count * 1);

		for (int i = 0; i < test_client_count; i++)
		{
			ASIO2_CHECK(!clients[i]->is_stopped());
			ASIO2_CHECK( clients[i]->is_started());
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			while (ex.async_call_counter < 19)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			ASIO2_CHECK_VALUE(ex.async_call_counter, ex.async_call_counter == 19);
		}

		while (server_async_call_counter < test_client_count * 3)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		ASIO2_CHECK_VALUE(server_async_call_counter.load(), server_async_call_counter == test_client_count * 3);

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->stop();
			ASIO2_CHECK( clients[i]->is_stopped());
			ASIO2_CHECK(!clients[i]->is_started());
			ASIO2_CHECK(!clients[i]->user_data_any().has_value());
		}

		// use this to ensure the ASIO2_CHECK(session_ptr->is_started());
		while (server_disconnect_counter != test_client_count * 1)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		server.stop();
		ASIO2_CHECK(server.is_stopped());

		ASIO2_CHECK_VALUE(server_disconnect_counter.load(), server_disconnect_counter == test_client_count*1);
		ASIO2_CHECK_VALUE(server_stop_counter      .load(), server_stop_counter       == 1);
	}

	ASIO2_TEST_END_LOOP;
}


ASIO2_TEST_SUITE
(
	"rpc",
	ASIO2_TEST_CASE(rpc_test)
)
