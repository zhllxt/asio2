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

void rpc5_test()
{
	ASIO2_TEST_BEGIN_LOOP(test_loop_times);

	// test websocket rpc function 
	std::shared_ptr<my_rpc_session_ws> rpc_session_ptr;
	{
		struct ext_data
		{
			int async_call_counter = 0;
			int sync_call_counter = 0;
			int client_init_counter = 0;
			int client_connect_counter = 0;
			int client_disconnect_counter = 0;
		};

		asio2::rpc_server_t<my_rpc_session_ws> server;

		std::atomic<int> server_accept_counter = 0;
		std::atomic<int> server_async_call_counter = 0;
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

			if (!rpc_session_ptr)
				rpc_session_ptr = session_ptr;

			session_ptr->async_call([&, session_ptr](int v)
			{
				server_async_call_counter++;
				ASIO2_CHECK(session_ptr->io().running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(v == 15 - 6);
			}, std::chrono::seconds(10), "sub", 15, 6);

			session_ptr->async_call("test", "i love you");

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
			asio2::ignore_unused(session_ptr);
			ASIO2_CHECK(asio2::get_last_error());
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

		usercrud crud;

		server
			.bind("add", add)
			.bind("mul", &usercrud::mul, crud)
			.bind("get_user", []
			(std::shared_ptr<my_rpc_session_ws>& session_ptr)
			{
				ASIO2_CHECK(session_ptr->io().running_in_this_thread());

				userinfo u;
				u.name = "lilei";
				u.age = 32;
				u.purview = { {1,"read"},{2,"write"} };
				return u;
			})
			.bind("del_user", []
			(std::shared_ptr<my_rpc_session_ws>& session_ptr, const userinfo& u)
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
			});

		server.bind("async_add", async_add);
		server.bind("async_test", []
		(std::shared_ptr<my_rpc_session_ws>& session_ptr, std::string a, std::string b)
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
		});

		server.bind("test_json", test_json);
		server.bind("heartbeat", []
		(std::shared_ptr<my_rpc_session_ws>& session_ptr)
		{
			ASIO2_CHECK(session_ptr->io().running_in_this_thread());
		});

		server.bind("cat", [&](std::shared_ptr<my_rpc_session_ws>& session_ptr,
			const std::string& a, const std::string& b)
		{
			ASIO2_CHECK(session_ptr->io().running_in_this_thread());
			ASIO2_CHECK(!asio2::get_last_error());

			// Nested call rpc function in business function is ok.
			session_ptr->async_call([&, session_ptr](int v) mutable
			{
				server_async_call_counter++;

				ASIO2_CHECK(session_ptr->io().running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(v == 15 - 8);

				// Nested call rpc function in business function is ok.
				session_ptr->async_call([&, session_ptr](int v)
				{
					server_async_call_counter++;
					ASIO2_CHECK(session_ptr->io().running_in_this_thread());
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

				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.is_keep_alive());
				ASIO2_CHECK(client.is_reuse_address());
				ASIO2_CHECK(client.is_no_delay());
			});
			client.bind_connect([&]()
			{
				if (asio2::get_last_error())
					return;

				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
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
					ASIO2_CHECK(client.io().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ASIO2_CHECK(v == 12 + 11);
					ex.async_call_counter++;
				}, std::chrono::seconds(13), "add", 12, 11);

				client.async_call<int>([&](auto v)
				{
					ASIO2_CHECK(client.io().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ASIO2_CHECK(v == 112 + 11);
					ex.async_call_counter++;
				}, std::chrono::seconds(13), "add", 112, 11);

				client.async_call<int>([&](int v)
				{
					ASIO2_CHECK(client.io().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ASIO2_CHECK(v == 1112 + 11);
					ex.async_call_counter++;
				}, std::chrono::seconds(13), "add", 1112, 11);

				client.async_call<std::string>([&](auto v)
				{
					ASIO2_CHECK(client.io().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ex.async_call_counter++;
					ASIO2_CHECK(v == "abcdef0123456789xx");
				}, "echo", "abcdef0123456789xx");

				client.async_call<std::string>([&](std::string v)
				{
					ASIO2_CHECK(client.io().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ASIO2_CHECK(v == "xxabcdef0123456789");
					ex.async_call_counter++;
				}, "echo", "xxabcdef0123456789");

				client.async_call([&]()
				{
					ASIO2_CHECK(client.io().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ex.async_call_counter++;
				}, std::chrono::seconds(3), "heartbeat");

				nlohmann::json j = nlohmann::json::object();

				j["name"] = "lilei";
				j["age"] = 30;

				client.async_call("test_json", j).response([&](nlohmann::json js)
				{
					ASIO2_CHECK(client.io().running_in_this_thread());
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
					ASIO2_CHECK(client.io().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ASIO2_CHECK(v == 12 + 21);
					ex.async_call_counter++;
				}, "add", 12, 21);

				// param 1 is empty, the result of the rpc call is not returned
				client.async_call("mul0", 2.5, 2.5);

				// Chain calls : 
				client.set_timeout(std::chrono::seconds(5)).async_call("mul", 2.5, 2.5).response([&](double v)
				{
					ASIO2_CHECK(client.io().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ASIO2_CHECK(v == 2.5 * 2.5);
					ex.async_call_counter++;
				});

				// Chain calls : 
				client.timeout(std::chrono::seconds(13)).response([&](double v)
				{
					ASIO2_CHECK(client.io().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ASIO2_CHECK(v == 3.5 * 3.5);
					ex.async_call_counter++;
				}).async_call("mul", 3.5, 3.5);

				// Chain calls : 
				client.response([&](double v)
				{
					ASIO2_CHECK(client.io().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ASIO2_CHECK(v == 4.5 * 4.5);
					ex.async_call_counter++;
				}).timeout(std::chrono::seconds(5)).async_call("mul", 4.5, 4.5);

				// Chain calls : 
				client.async_call("mul", 5.5, 5.5).response([&](double v)
				{
					ASIO2_CHECK(client.io().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ASIO2_CHECK(v == 5.5 * 5.5);
					ex.async_call_counter++;
				}).timeout(std::chrono::seconds(10));

				client.async_call([&](int v)
				{
					ASIO2_CHECK(client.io().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ASIO2_CHECK(v == 1 + 11);
					ex.async_call_counter++;
				}, std::chrono::seconds(3), "async_add", 1, 11);

				client.async_call([&]()
				{
					ASIO2_CHECK(client.io().running_in_this_thread());
					ASIO2_CHECK(!asio2::get_last_error());
					ex.async_call_counter++;
				}, std::chrono::seconds(3), "async_test", "abc", "def");

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

			// bind a rpc function in client, the server will call this client's rpc function
			client.bind("sub", [](int a, int b) { return a - b; });

			client.bind("async_add", async_add);

			client.bind("test", [](asio2::rpc_client_use<asio2::net_protocol::ws>& client, std::string str)
			{
				ASIO2_CHECK(client.io().running_in_this_thread());
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
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.async_call_counter++;
			}, "del_user", u);
	

			// just call rpc function, don't need the rpc result
			client.async_call("del_user", u);

			// this call will be failed, beacuse the param is incorrect.
			client.async_call("del_user", "hanmeimei").response([&](userinfo)
			{
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(asio2::get_last_error() == rpc::error::invalid_argument);
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.async_call_counter++;
			});

			// this call will be failed, beacuse the param is incorrect.
			client.async_call("del_user", 10, std::string("lilei"), 1).response([&](userinfo)
			{
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(asio2::get_last_error() == rpc::error::invalid_argument);
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.async_call_counter++;
			});

			// this call will be failed, beacuse the param is incorrect.
			client.async_call("del_user", u, std::string("lilei"), 1).response([&](userinfo)
			{
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
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
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
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
				ASIO2_TEST_WAIT_CHECK();
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
			ASIO2_TEST_WAIT_CHECK();
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
				ASIO2_TEST_WAIT_CHECK();
			}
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			ASIO2_CHECK_VALUE(ex.async_call_counter, ex.async_call_counter == 19);
		}

		while (server_async_call_counter < test_client_count * 3)
		{
			ASIO2_TEST_WAIT_CHECK();
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
			ASIO2_TEST_WAIT_CHECK();
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
				ASIO2_TEST_WAIT_CHECK();
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
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.async_call_counter++;
			}, "del_user", u);
	

			// just call rpc function, don't need the rpc result
			client.async_call("del_user", u);

			// this call will be failed, beacuse the param is incorrect.
			client.async_call("del_user", "hanmeimei").response([&](userinfo)
			{
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(asio2::get_last_error() == rpc::error::invalid_argument);
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.async_call_counter++;
			});

			// this call will be failed, beacuse the param is incorrect.
			client.async_call("del_user", 10, std::string("lilei"), 1).response([&](userinfo)
			{
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(asio2::get_last_error() == rpc::error::invalid_argument);
				ext_data& ex = client.get_user_data<ext_data&>();
				ex.async_call_counter++;
			});

			// this call will be failed, beacuse the param is incorrect.
			client.async_call("del_user", u, std::string("lilei"), 1).response([&](userinfo)
			{
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
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
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
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
			ASIO2_TEST_WAIT_CHECK();
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
				ASIO2_TEST_WAIT_CHECK();
			}
		}

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			ASIO2_CHECK_VALUE(ex.async_call_counter, ex.async_call_counter == 19);
		}

		while (server_async_call_counter < test_client_count * 3)
		{
			ASIO2_TEST_WAIT_CHECK();
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
			ASIO2_TEST_WAIT_CHECK();
		}

		server.stop();
		ASIO2_CHECK(server.is_stopped());

		ASIO2_CHECK_VALUE(server_disconnect_counter.load(), server_disconnect_counter == test_client_count*1);
		ASIO2_CHECK_VALUE(server_stop_counter      .load(), server_stop_counter       == 1);

		// test memory leaks 
		// the clients is stopped already, do post again, this will cause circular reference.
		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->async_call("leak_test").response([](std::string) {});
		}

		// So, we can call destroy to clear the all circular references. then there will has no memory leaks.
		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->destroy();
		}
	}

	ASIO2_CHECK(rpc_session_ptr != nullptr);

	// server is destroyed already, and the io_context is destroyed already, test session for this.
	std::shared_ptr<asio2::condition_event> evt_ptr = rpc_session_ptr->post_condition_event([]()
	{
		std::ignore = true;
	});
	//ASIO2_CHECK(asio2::get_last_error() == asio::error::eof); asio2::clear_last_error();
	ASIO2_CHECK(evt_ptr != nullptr);

	rpc_session_ptr->notify_all_condition_events();
	//ASIO2_CHECK(asio2::get_last_error() == asio::error::eof); asio2::clear_last_error();

	rpc_session_ptr->post_queued_event([]()
	{
		std::ignore = true;
	});
	//ASIO2_CHECK(asio2::get_last_error() == asio::error::eof); asio2::clear_last_error();

	auto fut1 = rpc_session_ptr->post_queued_event([]()
	{
		std::ignore = true;
	}, asio::use_future);
	ASIO2_CHECK(fut1.wait_for(std::chrono::milliseconds(10)) == std::future_status::timeout);
	//ASIO2_CHECK(asio2::get_last_error() == asio::error::eof); asio2::clear_last_error();

	rpc_session_ptr->post([]()
	{
		std::ignore = true;
	});
	//ASIO2_CHECK(asio2::get_last_error() == asio::error::eof); asio2::clear_last_error();

	rpc_session_ptr->post([]()
	{
		std::ignore = true;
	}, std::chrono::milliseconds(50));
	//ASIO2_CHECK(asio2::get_last_error() == asio::error::eof); asio2::clear_last_error();

	auto fut2 = rpc_session_ptr->post([]()
	{
		std::ignore = true;
	}, asio::use_future);
	ASIO2_CHECK(fut2.wait_for(std::chrono::milliseconds(10)) == std::future_status::timeout);
	//ASIO2_CHECK(asio2::get_last_error() == asio::error::eof); asio2::clear_last_error();

	auto fut3 = rpc_session_ptr->post([]()
	{
		std::ignore = true;
	}, std::chrono::milliseconds(50), asio::use_future);
	ASIO2_CHECK(fut3.wait_for(std::chrono::milliseconds(10)) == std::future_status::timeout);
	//ASIO2_CHECK(asio2::get_last_error() == asio::error::eof); asio2::clear_last_error();

	rpc_session_ptr->dispatch([]()
	{
		std::ignore = true;
	});
	//ASIO2_CHECK(asio2::get_last_error() == asio::error::eof); asio2::clear_last_error();

	auto fut4 = rpc_session_ptr->dispatch([]()
	{
		std::ignore = true;
	}, asio::use_future);
	ASIO2_CHECK(fut4.wait_for(std::chrono::milliseconds(10)) == std::future_status::timeout);
	//ASIO2_CHECK(asio2::get_last_error() == asio::error::eof); asio2::clear_last_error();

	rpc_session_ptr->stop_all_timed_events();
	//ASIO2_CHECK(asio2::get_last_error() == asio::error::eof); asio2::clear_last_error();

	std::string msg = "abc";

	rpc_session_ptr->async_send(msg);
	//ASIO2_CHECK(asio2::get_last_error() == asio::error::eof); asio2::clear_last_error();

	rpc_session_ptr->async_send("abc", 3);
	//ASIO2_CHECK(asio2::get_last_error() == asio::error::eof); asio2::clear_last_error();

	auto fut5 = rpc_session_ptr->async_send(msg, asio::use_future);
	//auto [ec5, bytes5] = fut5.get();
	//ASIO2_CHECK(ec5 == asio::error::eof && bytes5 == 0);
	ASIO2_CHECK(fut5.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout);

	auto fut6 = rpc_session_ptr->async_send("abc", 3, asio::use_future);
	//auto [ec6, bytes6] = fut6.get();
	//ASIO2_CHECK(ec6 == asio::error::eof && bytes6 == 0);
	ASIO2_CHECK(fut6.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout);

	rpc_session_ptr->async_send(msg, []()
	{
		std::ignore = true;
	});
	//ASIO2_CHECK(asio2::get_last_error() == asio::error::eof); asio2::clear_last_error();

	rpc_session_ptr->async_send("abc", 3, []()
	{
		std::ignore = true;
	});
	//ASIO2_CHECK(asio2::get_last_error() == asio::error::eof); asio2::clear_last_error();

	rpc_session_ptr->start_timer("abc", 1000, []()
	{
		std::ignore = true;
	});
	//ASIO2_CHECK(asio2::get_last_error() == asio::error::eof); asio2::clear_last_error();

	rpc_session_ptr->stop_timer("abc");
	//ASIO2_CHECK(asio2::get_last_error() == asio::error::eof); asio2::clear_last_error();

	rpc_session_ptr->stop_all_timers();
	//ASIO2_CHECK(asio2::get_last_error() == asio::error::eof); asio2::clear_last_error();

	//ASIO2_CHECK(rpc_session_ptr->is_timer_exists("abc") == false);
	//ASIO2_CHECK(asio2::get_last_error() == asio::error::eof); asio2::clear_last_error();

	//ASIO2_CHECK(rpc_session_ptr->get_timer_interval("abc") == asio::steady_timer::duration{});
	//ASIO2_CHECK(asio2::get_last_error() == asio::error::eof); asio2::clear_last_error();

	rpc_session_ptr->set_timer_interval("abc", asio::steady_timer::duration{});
	//ASIO2_CHECK(asio2::get_last_error() == asio::error::eof); asio2::clear_last_error();

	ASIO2_CHECK(rpc_session_ptr->timeout(std::chrono::milliseconds(100)).call<int>("add", 1, 2) == 0);
	//ASIO2_CHECK(asio2::get_last_error() == rpc::error::eof); asio2::clear_last_error();
	ASIO2_CHECK(asio2::get_last_error() == rpc::error::not_connected); asio2::clear_last_error();

	rpc_session_ptr->async_call("add", 1, 2).response([](int sum)
	{
		std::ignore = sum;
	});
	//ASIO2_CHECK(asio2::get_last_error() == rpc::error::eof); asio2::clear_last_error();
	ASIO2_CHECK(asio2::get_last_error() == rpc::error::in_progress); asio2::clear_last_error();

	// call destroy to clear the all circular references and memory leaks.
	rpc_session_ptr->destroy();

	std::shared_ptr<asio2::rpc_session> life_rpc_session_ptr;
	{
		asio2::rpc_server server;
		server.bind_connect([&](auto & session_ptr)
		{
			if (!life_rpc_session_ptr)
				life_rpc_session_ptr = session_ptr;
		});

		server.bind("async_life_test", async_life_test);

		bool server_start_ret = server.start("127.0.0.1", 18010);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		asio2::rpc_client client;

		bool client_start_ret = client.start("127.0.0.1", 18010);
		ASIO2_CHECK(client_start_ret);
		ASIO2_CHECK(!asio2::get_last_error());

		client.async_call("async_life_test").response([]()
		{
			std::ignore = true;
		});

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	ASIO2_ASSERT(life_rpc_session_ptr != nullptr);

	// call destroy to clear the all circular references and memory leaks.
	life_rpc_session_ptr->destroy();

	ASIO2_TEST_END_LOOP;
}


ASIO2_TEST_SUITE
(
	"rpc5",
	ASIO2_TEST_CASE(rpc5_test)
)
