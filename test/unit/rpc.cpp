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
	asio2::ignore_unused(session_ptr, a, b);

	rpc::promise<void> promise;
	rpc::future<void> f = promise.get_future();

	ASIO2_ASSERT(a == "abc" && b == "def");

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
		asio2::ignore_unused(session_ptr);

		userinfo u;
		u.name = "lilei";
		u.age = ((int)time(nullptr)) % 100;
		u.purview = { {1,"read"},{2,"write"} };
		return u;
	}

	// If you want to know which client called this function, set the first parameter
	// to std::shared_ptr<asio2::rpc_session>& session_ptr
	void del_user(std::shared_ptr<asio2::rpc_session>& session_ptr, const userinfo& u)
	{
		printf("del_user is called by %s %u : %s %d \n",
			session_ptr->remote_address().c_str(), session_ptr->remote_port(),
			u.name.c_str(), u.age);
	}
};

void heartbeat(std::shared_ptr<asio2::rpc_session>& session_ptr)
{
	printf("heartbeat %s\n", session_ptr->remote_address().c_str());
}

// set the first parameter to client reference to know which client was called
void test(asio2::rpc_client& client, std::string str)
{
	std::cout << client.get_user_data<int>() << " - test : " << str << std::endl;
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
	ASIO2_TEST_BEGIN_LOOP(test_loop_times * 100);

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
			// on linux, when disconnect is called, the remote_address maybe empty...
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
#endif
			ASIO2_CHECK(session_ptr->local_port() == 18010);
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
			client.default_timeout(std::chrono::seconds(3));
			ASIO2_CHECK(client.default_timeout() == std::chrono::seconds(3));
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
				ASIO2_CHECK(asio2::get_last_error() == asio::error::operation_aborted);
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
				ASIO2_CHECK(asio2::get_last_error() == asio::error::operation_aborted);
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
			// on linux, when disconnect is called, the remote_address maybe empty...
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
#endif
			ASIO2_CHECK(session_ptr->local_port() == 18010);
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
			client.default_timeout(std::chrono::seconds(3));
			ASIO2_CHECK(client.default_timeout() == std::chrono::seconds(3));
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
		int len = 200 + (std::rand() % 200);
		for (int i = 0; i < len; i++)
		{
			msg += (char)(std::rand() % 255);
		}
		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->async_send(msg, []()
			{
				ASIO2_CHECK(!asio2::get_last_error());
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
			clients[i]->async_send(msg, []()
			{
				ASIO2_CHECK(!asio2::get_last_error());
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

	//{
	//// Specify the "max recv buffer size" to avoid malicious packets, if some client
	//// sent data packets size is too long to the "max recv buffer size", then the
	//// client will be disconnect automatic .
	//asio2::rpc_server server(
	//	512,  // the initialize recv buffer size : 
	//	1024, // the max recv buffer size :
	//	4     // the thread count : 
	//);

	//server.bind_connect([&](auto & session_ptr)
	//{
	//	printf("client enter : %s %u %s %u\n",
	//		session_ptr->remote_address().c_str(), session_ptr->remote_port(),
	//		session_ptr->local_address().c_str(), session_ptr->local_port());
	//	session_ptr->post([]() {}, std::chrono::seconds(3));
	//	session_ptr->async_call([](int v)
	//	{
	//		if (!asio2::get_last_error())
	//		{
	//			ASIO2_ASSERT(v == 15 - 6);
	//		}
	//		printf("sub : %d err : %d %s\n", v, asio2::last_error_val(), asio2::last_error_msg().c_str());
	//	}, std::chrono::seconds(10), "sub", 15, 6);

	//	session_ptr->async_call("test", "i love you");

	//}).bind_disconnect([&](auto & session_ptr)
	//{
	//	printf("client leave : %s %u\n", session_ptr->remote_address().c_str(), session_ptr->remote_port());
	//}).bind_start([&]()
	//{
	//	printf("start rpc server : %s %u %d %s\n",
	//		server.listen_address().c_str(), server.listen_port(),
	//		asio2::last_error_val(), asio2::last_error_msg().c_str());
	//}).bind_stop([&]()
	//{
	//	printf("stop : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	//});

	//usercrud a;

	//server
	//	.bind("add", add)
	//	.bind("mul", &usercrud::mul, a)
	//	.bind("get_user", &usercrud::get_user, a)
	//	.bind("del_user", &usercrud::del_user, &a);

	//server.bind("async_add", async_add);
	//server.bind("async_test", async_test);

	//server.bind("test_json", test_json);
	//server.bind("heartbeat", heartbeat);

	//server.bind("cat", [&](std::shared_ptr<asio2::rpc_session>& session_ptr,
	//	const std::string& a, const std::string& b)
	//{
	//	// Nested call rpc function in business function is ok.
	//	session_ptr->async_call([session_ptr](int v) mutable
	//	{
	//		// Nested call rpc function in business function is ok.
	//		session_ptr->async_call([](int v)
	//		{
	//			if (!asio2::get_last_error())
	//			{
	//				ASIO2_ASSERT(v == 15 + 18);
	//			}
	//			printf("async_add : %d err : %d %s\n", v, asio2::last_error_val(), asio2::last_error_msg().c_str());
	//		}, "async_add", 15, 18);

	//		if (!asio2::get_last_error())
	//		{
	//			ASIO2_ASSERT(v == 15 - 8);
	//		}
	//		printf("sub : %d err : %d %s\n", v, asio2::last_error_val(), asio2::last_error_msg().c_str());
	//	}, "sub", 15, 8);

	//	return a + b;
	//});

	//server.start(host, port);

	//auto sec = 1 + std::rand() % 2;

	//std::this_thread::sleep_for(std::chrono::seconds(sec));

	//server.start(host, port);

	//server.stop();

	//asio2::rpc_client *clients = new asio2::rpc_client[10];

	//for (int i = 0; i < 10; i++)
	//{
	//	auto & client = clients[i];

	//	client.set_user_data(i + 99);

	//// set default rpc call timeout
	//client.default_timeout(std::chrono::seconds(3));

	//client.bind_connect([&]()
	//{
	//	if (asio2::get_last_error())
	//		return;

	//	client.post([]() {}, std::chrono::seconds(3));

	//	//------------------------------------------------------------------
	//	// this thread is a commucation thread. like bind_recv,bind_connect,
	//	// bind_disconnect..... is commucation thread.
	//	// important : synchronous call rpc function in the commucation thread,
	//	// then the call will degenerates into async_call and the return value is empty.
	//	//------------------------------------------------------------------
	//	client.call<double>("mul", 16.5, 26.5);
	//	if (client.is_started())
	//		ASIO2_ASSERT(asio2::get_last_error() == asio::error::in_progress);
	//	else
	//		ASIO2_ASSERT(asio2::get_last_error() == asio::error::not_connected);

	//	// param 1 : user callback function(this param can be empty)
	//	// param 2 : timeout (this param can be empty, if this param is empty, use the default_timeout)
	//	// param 3 : function name
	//	// param 4 : function params
	//	client.async_call([](int v)
	//	{
	//		if (!asio2::get_last_error())
	//		{
	//			ASIO2_ASSERT(v == 12 + 11);
	//		}
	//		printf("sum1 : %d err : %d %s\n", v, asio2::last_error_val(), asio2::last_error_msg().c_str());
	//	}, std::chrono::seconds(13), "add", 12, 11);

	//	client.async_call([]()
	//	{
	//		printf("heartbeat err : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	//	}, std::chrono::seconds(3), "heartbeat");

	//	nlohmann::json j = nlohmann::json::object();

	//	j["name"] = "lilei";
	//	j["age"] = 30;

	//	client.async_call("test_json", j).response([](nlohmann::json js)
	//	{
	//		std::string s = js.dump();

	//		if (!asio2::get_last_error())
	//		{
	//			ASIO2_ASSERT(js["age"].get<int>() == 30);
	//			ASIO2_ASSERT(js["name"].get<std::string>() == "lilei");
	//		}

	//		asio2::ignore_unused(js, s);
	//	});

	//	// param 2 is empty, use the default_timeout
	//	client.async_call([](int v)
	//	{
	//		if (!asio2::get_last_error())
	//		{
	//			ASIO2_ASSERT(v == 12 + 21);
	//		}
	//		printf("sum2 : %d err : %d %s\n", v, asio2::last_error_val(), asio2::last_error_msg().c_str());
	//	}, "add", 12, 21);

	//	// param 1 is empty, the result of the rpc call is not returned
	//	client.async_call("mul0", 2.5, 2.5);


	//	// Chain calls : 
	//	client.set_timeout(std::chrono::seconds(5)).async_call("mul", 2.5, 2.5).response([](double v)
	//	{
	//		if (!asio2::get_last_error())
	//		{
	//			ASIO2_ASSERT(v == 2.5 * 2.5);
	//		}
	//		std::cout << "mul1 " << v << std::endl;
	//	});

	//	// Chain calls : 
	//	client.timeout(std::chrono::seconds(13)).response([](double v)
	//	{
	//		if (!asio2::get_last_error())
	//		{
	//			ASIO2_ASSERT(v == 3.5 * 3.5);
	//		}
	//		std::cout << "mul2 " << v << std::endl;
	//	}).async_call("mul", 3.5, 3.5);

	//	// Chain calls : 
	//	client.response([](double v)
	//	{
	//		if (!asio2::get_last_error())
	//		{
	//			ASIO2_ASSERT(v == 4.5 * 4.5);
	//		}
	//		std::cout << "mul3 " << v << std::endl;
	//	}).timeout(std::chrono::seconds(5)).async_call("mul", 4.5, 4.5);

	//	// Chain calls : 
	//	client.async_call("mul", 5.5, 5.5).response([](double v)
	//	{
	//		if (!asio2::get_last_error())
	//		{
	//			ASIO2_ASSERT(v == 5.5 * 5.5);
	//		}
	//		std::cout << "mul4 " << v << std::endl;
	//	}).timeout(std::chrono::seconds(10));

	//	client.async_call([](int v)
	//	{
	//		if (!asio2::get_last_error())
	//		{
	//			ASIO2_ASSERT(v == 1 + 11);
	//		}
	//		printf("async_add : %d err : %d %s\n", v, asio2::last_error_val(), asio2::last_error_msg().c_str());
	//	}, std::chrono::seconds(3), "async_add", 1, 11);

	//	client.async_call([]()
	//	{
	//		printf("async_test err : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	//	}, std::chrono::seconds(3), "async_test", "abc", "def");
	//});

	//// bind a rpc function in client, the server will call this client's rpc function
	//client.bind("sub", [](int a, int b) { return a - b; });

	//client.bind("async_add", async_add);

	//client.bind("test", test);

	//client.start(host, port);

	//client.start(host, port);

	//client.async_start(host, port);

	//nlohmann::json j = nlohmann::json::object();

	//j["name"] = "hanmeimei";
	//j["age"] = 20;

	//nlohmann::json j2 = client.call<nlohmann::json>(std::chrono::minutes(1), "test_json", j);
	//if (!asio2::get_last_error())
	//{
	//	ASIO2_ASSERT(j2["age"].get<int>() == 20);
	//	ASIO2_ASSERT(j2["name"].get<std::string>() == "hanmeimei");
	//}

	//// synchronous call
	//// param 1 : error_code refrence (this param can be empty)
	//// param 2 : timeout (this param can be empty, if this param is empty, use the default_timeout)
	//// param 3 : rpc function name
	//// param 4 : rpc function params
	//double mul = client.call<double>(std::chrono::seconds(3), "mul", 6.5, 6.5);
	//printf("mul5 : %lf err : %d %s\n", mul, asio2::last_error_val(), asio2::last_error_msg().c_str());
	//if (!asio2::get_last_error())
	//{
	//	ASIO2_ASSERT(mul == 6.5 * 6.5);
	//}

	//userinfo u;
	//u = client.call<userinfo>("get_user");
	//if (!asio2::get_last_error())
	//{
	//	ASIO2_ASSERT(u.name == "lilei" && u.purview.size() == 2);
	//}
	//printf("get_user : %s %d -> ", u.name.c_str(), u.age);
	//for (auto &[k, v] : u.purview)
	//{
	//	printf("%d %s ", k, v.c_str());
	//}
	//printf("\n");

	//u.name = "hanmeimei";
	//u.age = ((int)time(nullptr)) % 100;
	//u.purview = { {10,"get"},{20,"set"} };
	//client.async_call([]()
	//{
	//	if (asio2::get_last_error())
	//		printf("del_user : failed : %s\n\n", asio2::last_error_msg().c_str());
	//	else
	//		printf("del_user : successed\n\n");

	//}, "del_user", u);
	//

	//// just call rpc function, don't need the rpc result
	//client.async_call("del_user", std::move(u));

	//// this call will be failed, beacuse the param is incorrect.
	//client.async_call("del_user", "hanmeimei").response([](userinfo)
	//{
	//	ASIO2_ASSERT(bool(asio2::get_last_error()));
	//	std::cout << "del_user hanmeimei failed : " << asio2::last_error_msg() << std::endl;
	//});

	//// this call will be failed, beacuse the param is incorrect.
	//client.async_call("del_user", 10, std::string("lilei"), 1).response([](userinfo)
	//{
	//	ASIO2_ASSERT(bool(asio2::get_last_error()));
	//	std::cout << "del_user lilei failed : " << asio2::last_error_msg() << std::endl;
	//});

	//// Chain calls : 
	//int sum = client.timeout(std::chrono::seconds(13)).call<int>("add", 11, 12);
	//printf("sum5 : %d err : %d %s\n", sum, asio2::last_error_val(), asio2::last_error_msg().c_str());
	//if (!asio2::get_last_error())
	//{
	//	ASIO2_ASSERT(sum == 11 + 12);
	//}

	//// Chain calls : 
	//sum = client.timeout(std::chrono::seconds(13)).call<int>("add", 11, 32);
	//printf("sum6 : %d err : %d %s\n", sum, asio2::last_error_val(), asio2::last_error_msg().c_str());
	//if (!asio2::get_last_error())
	//{
	//	ASIO2_ASSERT(sum == 11 + 32);
	//}

	//// Chain calls : 
	//std::string str = client.call<std::string>("cat", "abc", "123");
	//printf("cat : %s err : %d %s\n", str.data(), asio2::last_error_val(), asio2::last_error_msg().c_str());
	//if (!asio2::get_last_error())
	//{
	//	ASIO2_ASSERT(str == "abc123");
	//}

	//client.async_call([](int v)
	//{
	//	printf("test call no_exists_fn : %d err : %d %s\n",
	//		v, asio2::last_error_val(), asio2::last_error_msg().c_str());
	//	ASIO2_ASSERT(bool(asio2::get_last_error()));
	//}, "no_exists_fn", 10);

	//}

	//std::this_thread::sleep_for(std::chrono::seconds(1 + std::rand() % 2));

	//delete[]clients;

	//}

	//{
	//// Specify the "max recv buffer size" to avoid malicious packets, if some client
	//// sent data packets size is too long to the "max recv buffer size", then the
	//// client will be disconnect automatic .
	//asio2::rpc_server server(
	//	512,  // the initialize recv buffer size : 
	//	1024, // the max recv buffer size :
	//	4     // the thread count : 
	//);

	//server.bind_connect([&](auto & session_ptr)
	//{
	//	printf("client enter : %s %u %s %u\n",
	//		session_ptr->remote_address().c_str(), session_ptr->remote_port(),
	//		session_ptr->local_address().c_str(), session_ptr->local_port());
	//	session_ptr->post([]() {}, std::chrono::seconds(3));
	//	session_ptr->async_call([](int v)
	//	{
	//		if (!asio2::get_last_error())
	//		{
	//			ASIO2_ASSERT(v == 15 - 6);
	//		}
	//		printf("sub : %d err : %d %s\n", v, asio2::last_error_val(), asio2::last_error_msg().c_str());
	//	}, std::chrono::seconds(10), "sub", 15, 6);

	//	session_ptr->async_call("test", "i love you");

	//}).bind_disconnect([&](auto & session_ptr)
	//{
	//	printf("client leave : %s %u\n", session_ptr->remote_address().c_str(), session_ptr->remote_port());
	//}).bind_start([&]()
	//{
	//	printf("start rpc server : %s %u %d %s\n",
	//		server.listen_address().c_str(), server.listen_port(),
	//		asio2::last_error_val(), asio2::last_error_msg().c_str());
	//}).bind_stop([&]()
	//{
	//	printf("stop : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	//});

	//usercrud a;

	//server
	//	.bind("add", add)
	//	.bind("mul", &usercrud::mul, a)
	//	.bind("get_user", &usercrud::get_user, a)
	//	.bind("del_user", &usercrud::del_user, &a);

	//server.bind("async_add", async_add);
	//server.bind("async_test", async_test);

	//server.bind("test_json", test_json);
	//server.bind("heartbeat", heartbeat);

	//server.bind("cat", [&](std::shared_ptr<asio2::rpc_session>& session_ptr,
	//	const std::string& a, const std::string& b)
	//{
	//	// Nested call rpc function in business function is ok.
	//	session_ptr->async_call([session_ptr](int v) mutable
	//	{
	//		// Nested call rpc function in business function is ok.
	//		session_ptr->async_call([](int v)
	//		{
	//			if (!asio2::get_last_error())
	//			{
	//				ASIO2_ASSERT(v == 15 + 18);
	//			}
	//			printf("async_add : %d err : %d %s\n", v, asio2::last_error_val(), asio2::last_error_msg().c_str());
	//		}, "async_add", 15, 18);

	//		if (!asio2::get_last_error())
	//		{
	//			ASIO2_ASSERT(v == 15 - 8);
	//		}
	//		printf("sub : %d err : %d %s\n", v, asio2::last_error_val(), asio2::last_error_msg().c_str());
	//	}, "sub", 15, 8);

	//	return a + b;
	//});

	//server.start(host, port);

	//auto sec = 1 + std::rand() % 2;

	//std::this_thread::sleep_for(std::chrono::seconds(sec));

	//server.start(host, port);

	//server.stop();

	//asio2::rpc_client *clients = new asio2::rpc_client[10];

	//for (int i = 0; i < 10; i++)
	//{
	//	auto & client = clients[i];

	//	client.set_user_data(i + 99);

	//// set default rpc call timeout
	//client.default_timeout(std::chrono::seconds(3));

	//client.bind_connect([&]()
	//{
	//	if (asio2::get_last_error())
	//		return;

	//	client.post([]() {}, std::chrono::seconds(3));

	//	//------------------------------------------------------------------
	//	// this thread is a commucation thread. like bind_recv,bind_connect,
	//	// bind_disconnect..... is commucation thread.
	//	// important : synchronous call rpc function in the commucation thread,
	//	// then the call will degenerates into async_call and the return value is empty.
	//	//------------------------------------------------------------------
	//	client.call<double>("mul", 16.5, 26.5);
	//	if (client.is_started())
	//		ASIO2_ASSERT(asio2::get_last_error() == asio::error::in_progress);
	//	else
	//		ASIO2_ASSERT(asio2::get_last_error() == asio::error::not_connected);

	//	// param 1 : user callback function(this param can be empty)
	//	// param 2 : timeout (this param can be empty, if this param is empty, use the default_timeout)
	//	// param 3 : function name
	//	// param 4 : function params
	//	client.async_call([](int v)
	//	{
	//		if (!asio2::get_last_error())
	//		{
	//			ASIO2_ASSERT(v == 12 + 11);
	//		}
	//		printf("sum1 : %d err : %d %s\n", v, asio2::last_error_val(), asio2::last_error_msg().c_str());
	//	}, std::chrono::seconds(13), "add", 12, 11);

	//	client.async_call([]()
	//	{
	//		printf("heartbeat err : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	//	}, std::chrono::seconds(3), "heartbeat");

	//	nlohmann::json j = nlohmann::json::object();

	//	j["name"] = "lilei";
	//	j["age"] = 30;

	//	client.async_call("test_json", j).response([](nlohmann::json js)
	//	{
	//		std::string s = js.dump();

	//		if (!asio2::get_last_error())
	//		{
	//			ASIO2_ASSERT(js["age"].get<int>() == 30);
	//			ASIO2_ASSERT(js["name"].get<std::string>() == "lilei");
	//		}

	//		asio2::ignore_unused(js, s);
	//	});

	//	// param 2 is empty, use the default_timeout
	//	client.async_call([](int v)
	//	{
	//		if (!asio2::get_last_error())
	//		{
	//			ASIO2_ASSERT(v == 12 + 21);
	//		}
	//		printf("sum2 : %d err : %d %s\n", v, asio2::last_error_val(), asio2::last_error_msg().c_str());
	//	}, "add", 12, 21);

	//	// param 1 is empty, the result of the rpc call is not returned
	//	client.async_call("mul0", 2.5, 2.5);


	//	// Chain calls : 
	//	client.set_timeout(std::chrono::seconds(5)).async_call("mul", 2.5, 2.5).response([](double v)
	//	{
	//		if (!asio2::get_last_error())
	//		{
	//			ASIO2_ASSERT(v == 2.5 * 2.5);
	//		}
	//		std::cout << "mul1 " << v << std::endl;
	//	});

	//	// Chain calls : 
	//	client.timeout(std::chrono::seconds(13)).response([](double v)
	//	{
	//		if (!asio2::get_last_error())
	//		{
	//			ASIO2_ASSERT(v == 3.5 * 3.5);
	//		}
	//		std::cout << "mul2 " << v << std::endl;
	//	}).async_call("mul", 3.5, 3.5);

	//	// Chain calls : 
	//	client.response([](double v)
	//	{
	//		if (!asio2::get_last_error())
	//		{
	//			ASIO2_ASSERT(v == 4.5 * 4.5);
	//		}
	//		std::cout << "mul3 " << v << std::endl;
	//	}).timeout(std::chrono::seconds(5)).async_call("mul", 4.5, 4.5);

	//	// Chain calls : 
	//	client.async_call("mul", 5.5, 5.5).response([](double v)
	//	{
	//		if (!asio2::get_last_error())
	//		{
	//			ASIO2_ASSERT(v == 5.5 * 5.5);
	//		}
	//		std::cout << "mul4 " << v << std::endl;
	//	}).timeout(std::chrono::seconds(10));

	//	client.async_call([](int v)
	//	{
	//		if (!asio2::get_last_error())
	//		{
	//			ASIO2_ASSERT(v == 1 + 11);
	//		}
	//		printf("async_add : %d err : %d %s\n", v, asio2::last_error_val(), asio2::last_error_msg().c_str());
	//	}, std::chrono::seconds(3), "async_add", 1, 11);

	//	client.async_call([]()
	//	{
	//		printf("async_test err : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	//	}, std::chrono::seconds(3), "async_test", "abc", "def");
	//});

	//// bind a rpc function in client, the server will call this client's rpc function
	//client.bind("sub", [](int a, int b) { return a - b; });

	//client.bind("async_add", async_add);

	//client.bind("test", test);

	//client.start(host, port);

	//client.start(host, port);

	//client.async_start(host, port);

	//nlohmann::json j = nlohmann::json::object();

	//j["name"] = "hanmeimei";
	//j["age"] = 20;

	//nlohmann::json j2 = client.call<nlohmann::json>(std::chrono::minutes(1), "test_json", j);
	//if (!asio2::get_last_error())
	//{
	//	ASIO2_ASSERT(j2["age"].get<int>() == 20);
	//	ASIO2_ASSERT(j2["name"].get<std::string>() == "hanmeimei");
	//}

	//// synchronous call
	//// param 1 : error_code refrence (this param can be empty)
	//// param 2 : timeout (this param can be empty, if this param is empty, use the default_timeout)
	//// param 3 : rpc function name
	//// param 4 : rpc function params
	//double mul = client.call<double>(std::chrono::seconds(3), "mul", 6.5, 6.5);
	//printf("mul5 : %lf err : %d %s\n", mul, asio2::last_error_val(), asio2::last_error_msg().c_str());
	//if (!asio2::get_last_error())
	//{
	//	ASIO2_ASSERT(mul == 6.5 * 6.5);
	//}

	//userinfo u;
	//u = client.call<userinfo>("get_user");
	//if (!asio2::get_last_error())
	//{
	//	ASIO2_ASSERT(u.name == "lilei" && u.purview.size() == 2);
	//}
	//printf("get_user : %s %d -> ", u.name.c_str(), u.age);
	//for (auto &[k, v] : u.purview)
	//{
	//	printf("%d %s ", k, v.c_str());
	//}
	//printf("\n");

	//u.name = "hanmeimei";
	//u.age = ((int)time(nullptr)) % 100;
	//u.purview = { {10,"get"},{20,"set"} };
	//client.async_call([]()
	//{
	//	if (asio2::get_last_error())
	//		printf("del_user : failed : %s\n\n", asio2::last_error_msg().c_str());
	//	else
	//		printf("del_user : successed\n\n");

	//}, "del_user", u);
	//

	//// just call rpc function, don't need the rpc result
	//client.async_call("del_user", std::move(u));

	//// this call will be failed, beacuse the param is incorrect.
	//client.async_call("del_user", "hanmeimei").response([](userinfo)
	//{
	//	ASIO2_ASSERT(bool(asio2::get_last_error()));
	//	std::cout << "del_user hanmeimei failed : " << asio2::last_error_msg() << std::endl;
	//});

	//// this call will be failed, beacuse the param is incorrect.
	//client.async_call("del_user", 10, std::string("lilei"), 1).response([](userinfo)
	//{
	//	ASIO2_ASSERT(bool(asio2::get_last_error()));
	//	std::cout << "del_user lilei failed : " << asio2::last_error_msg() << std::endl;
	//});

	//// Chain calls : 
	//int sum = client.timeout(std::chrono::seconds(13)).call<int>("add", 11, 12);
	//printf("sum5 : %d err : %d %s\n", sum, asio2::last_error_val(), asio2::last_error_msg().c_str());
	//if (!asio2::get_last_error())
	//{
	//	ASIO2_ASSERT(sum == 11 + 12);
	//}

	//// Chain calls : 
	//sum = client.timeout(std::chrono::seconds(13)).call<int>("add", 11, 32);
	//printf("sum6 : %d err : %d %s\n", sum, asio2::last_error_val(), asio2::last_error_msg().c_str());
	//if (!asio2::get_last_error())
	//{
	//	ASIO2_ASSERT(sum == 11 + 32);
	//}

	//// Chain calls : 
	//std::string str = client.call<std::string>("cat", "abc", "123");
	//printf("cat : %s err : %d %s\n", str.data(), asio2::last_error_val(), asio2::last_error_msg().c_str());
	//if (!asio2::get_last_error())
	//{
	//	ASIO2_ASSERT(str == "abc123");
	//}

	//client.async_call([](int v)
	//{
	//	printf("test call no_exists_fn : %d err : %d %s\n",
	//		v, asio2::last_error_val(), asio2::last_error_msg().c_str());
	//	ASIO2_ASSERT(bool(asio2::get_last_error()));
	//}, "no_exists_fn", 10);

	//}

	//std::this_thread::sleep_for(std::chrono::seconds(1 + std::rand() % 2));

	//delete[]clients;

	//}

	ASIO2_TEST_END_LOOP;
}


ASIO2_TEST_SUITE
(
	"rpc",
	ASIO2_TEST_CASE(rpc_test)
)
