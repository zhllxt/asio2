#include "unit_test.hpp"

#define ASIO2_ENABLE_RPC_INVOKER_THREAD_SAFE

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

rpc::future<void> async_life_test()
{
	rpc::promise<void> promise;
	rpc::future<void> f = promise.get_future();

	std::thread([promise = std::move(promise)]() mutable
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(500));

		promise.set_value();
	}).detach();

	return f;
}

rpc::future<void> async_test(std::shared_ptr<asio2::rpc_session>& session_ptr, std::string a, std::string b)
{
	ASIO2_CHECK(session_ptr->io().running_in_this_thread());

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
		ASIO2_CHECK(session_ptr->io().running_in_this_thread());

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
		ASIO2_CHECK(session_ptr->io().running_in_this_thread());
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
	ASIO2_CHECK(session_ptr->io().running_in_this_thread());
}

// set the first parameter to client reference to know which client was called
void client_fn_test(asio2::rpc_client& client, std::string str)
{
	ASIO2_CHECK(client.io().running_in_this_thread());
	ASIO2_CHECK(str == "i love you");
}

class my_rpc_session_ws : public asio2::rpc_session_t<my_rpc_session_ws, asio2::net_protocol::ws>
{
public:
	using asio2::rpc_session_t<my_rpc_session_ws, asio2::net_protocol::ws>::rpc_session_t;

	using super::send;
	using super::async_send;
};

class my_rpc_client_tcp : public asio2::rpc_client_use<asio2::net_protocol::tcp>
{
public:
	using rpc_client_use<asio2::net_protocol::tcp>::rpc_client_use;

	using super::send;
	using super::async_send;
};

void rpc1_test()
{
	ASIO2_TEST_BEGIN_LOOP(test_loop_times);

	// test max buffer size
	{
		asio2::rpc_server server(512, 1024, 4);

		std::atomic<int> server_accept_counter = 0;
		server.bind_accept([&](auto & session_ptr)
		{
			if (!asio2::get_last_error())
			{
				session_ptr->no_delay(true);

				server_accept_counter++;

				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
				ASIO2_CHECK(server.get_listen_port() == 18010);
				ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_port() == 18010);
				ASIO2_CHECK(server.io().running_in_this_thread());
				ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
			}
		});
		std::atomic<int> server_connect_counter = 0;
		server.bind_connect([&](auto & session_ptr)
		{
			server_connect_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18010);
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
			ASIO2_CHECK(session_ptr->socket().is_open());
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
			ASIO2_CHECK(server.get_listen_port() == 18010);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18010);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});

		server.bind("echo", echo);
		
		server.unbind("test_guard_by");
		ASIO2_CHECK(server.find("echo") != nullptr);

		bool server_start_ret = server.start("127.0.0.1", 18010);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		std::vector<std::shared_ptr<asio2::rpc_client>> clients;
		std::atomic<int> client_init_counter = 0;
		std::atomic<int> client_connect_counter = 0;
		std::atomic<int> client_disconnect_counter = 0;
		std::atomic<int> client_start_failed_counter = 0;
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
				ASIO2_CHECK(client.get_remote_port() == 18010);

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
			});

			bool client_start_ret = client.start("127.0.0.1", 18010);
			if (!client_start_ret)
				client_start_failed_counter++;
			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(!asio2::get_last_error());
		}

		while (server.get_session_count() < std::size_t(test_client_count - client_start_failed_counter))
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		auto session_count = server.get_session_count();
		ASIO2_CHECK_VALUE(session_count, session_count == std::size_t(test_client_count));

		ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == test_client_count);
		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		while (client_connect_counter < test_client_count - client_start_failed_counter)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == test_client_count);
		ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == test_client_count);

		std::string msg;
		msg.resize(1500);
		for (int i = 0; i < test_client_count; i++)
		{
			auto& clt = clients[i];
			clients[i]->async_call("echo", msg).response([&clt](std::string s)
			{
				ASIO2_CHECK(s.empty());
				// when send data failed, the error is not rpc::error::operation_aborted,
				ASIO2_CHECK(asio2::get_last_error() == rpc::error::operation_aborted);
				if (asio2::get_last_error() != rpc::error::operation_aborted)
				{
					// close the socket, this will trigger the auto reconnect
					clt->socket().close();
				}
			});
		}

		while (client_disconnect_counter < test_client_count - client_start_failed_counter)
		{
			ASIO2_TEST_WAIT_CHECK();
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
		while (server_disconnect_counter != test_client_count- client_start_failed_counter)
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

		server_start_ret = server.start("127.0.0.1", 18010);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		client_init_counter = 0;
		client_connect_counter = 0;
		client_disconnect_counter = 0;
		client_start_failed_counter = 0;

		for (int i = 0; i < test_client_count; i++)
		{
			bool client_start_ret = clients[i]->async_start("127.0.0.1", 18010);
			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
		}

		while (server.get_session_count() < std::size_t(test_client_count - client_start_failed_counter))
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(server.get_session_count(), server.get_session_count() == std::size_t(test_client_count));

		ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == test_client_count);
		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		while (client_connect_counter < test_client_count - client_start_failed_counter)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == test_client_count);
		ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			auto& clt = clients[i];
			clients[i]->async_call("echo", msg).response([&clt](std::string s)
			{
				ASIO2_CHECK(s.empty());
				// when send data failed, the error is not rpc::error::operation_aborted,
				ASIO2_CHECK(asio2::get_last_error() == rpc::error::operation_aborted);
				if (asio2::get_last_error() != rpc::error::operation_aborted)
				{
					// close the socket, this will trigger the auto reconnect
					clt->socket().close();
				}
			});
		}

		while (client_disconnect_counter < test_client_count - client_start_failed_counter)
		{
			ASIO2_TEST_WAIT_CHECK();
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
		while (server_disconnect_counter != test_client_count - client_start_failed_counter)
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
	"rpc1",
	ASIO2_TEST_CASE(rpc1_test)
)
