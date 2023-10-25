#include "unit_test.hpp"

#include <asio2/config.hpp>

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

rpc::future<void> async_test(std::shared_ptr<asio2::rpc_kcp_session>& session_ptr, std::string a, std::string b)
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
	// to std::shared_ptr<asio2::rpc_kcp_session>& session_ptr
	userinfo get_user(std::shared_ptr<asio2::rpc_kcp_session> session_ptr)
	{
		ASIO2_CHECK(session_ptr->io().running_in_this_thread());

		userinfo u;
		u.name = "lilei";
		u.age = 32;
		u.purview = { {1,"read"},{2,"write"} };
		return u;
	}

	// If you want to know which client called this function, set the first parameter
	// to std::shared_ptr<asio2::rpc_kcp_session>& session_ptr
	void del_user(std::shared_ptr<asio2::rpc_kcp_session> session_ptr, const userinfo& u)
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

void heartbeat(std::shared_ptr<asio2::rpc_kcp_session>& session_ptr)
{
	ASIO2_CHECK(session_ptr->io().running_in_this_thread());
}

// set the first parameter to client reference to know which client was called
void client_fn_test(asio2::rpc_kcp_client& client, std::string str)
{
	ASIO2_CHECK(client.io().running_in_this_thread());
	ASIO2_CHECK(str == "i love you");
}

class my_rpc_kcp_client_kcp : public asio2::rpc_client_use<asio2::net_protocol::udp>
{
public:
	using rpc_client_use<asio2::net_protocol::udp>::rpc_client_use;

	using super::send;
	using super::async_send;
};

void rpc_kcp3_test()
{
	ASIO2_TEST_BEGIN_LOOP(test_loop_times);

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

		asio2::rpc_kcp_server server(512, 1024, 4);

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
			
			session_ptr->get_kcp_stream()->set_illegal_response_handler(
			[p = session_ptr.get()](std::string_view data)
			{
				std::ignore = data;
				p->stop();
			});

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

		bool server_start_ret = server.start("127.0.0.1", 18010);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		std::vector<std::shared_ptr<asio2::rpc_kcp_client>> clients;
		for (int i = 0; i < test_client_count; i++)
		{
			auto iter = clients.emplace_back(std::make_shared<asio2::rpc_kcp_client>());

			asio2::rpc_kcp_client& client = *iter;

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

				client.get_kcp_stream()->set_illegal_response_handler(
				[&client](std::string_view data)
				{
					std::ignore = data;
					client.stop();
				});

				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				
				ASIO2_CHECK(client.is_reuse_address());
				
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
						// when send data failed, the error is not rpc::error::operation_aborted,
						// and the server can't recv the illage data, and the server will can't
						// disconnect this client, this will cause client_connect_counter can't 
						// be equal to 3.
						if (asio2::get_last_error() != rpc::error::operation_aborted)
						{
							// close the socket, this will trigger the auto reconnect
							client.socket().close();
						}
					}
				});

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
				ASIO2_TEST_WAIT_CHECK();
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
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count * 3);

		for (int i = 0; i < test_client_count; i++)
		{
			ext_data& ex = clients[i]->get_user_data<ext_data&>();
			while (ex.client_disconnect_counter < 3)
			{
				ASIO2_TEST_WAIT_CHECK();
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
		//while (server_disconnect_counter != test_client_count * 3)
		while (server_disconnect_counter < test_client_count * 3)
		{
			ASIO2_TEST_WAIT_CHECK();
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
		server_connect_counter = 0;

		server_start_ret = server.start("127.0.0.1", 18010);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		for (int i = 0; i < test_client_count; i++)
		{
			asio2::rpc_kcp_client& client = *clients[i];

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
				ASIO2_TEST_WAIT_CHECK();
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
			ASIO2_TEST_WAIT_CHECK();
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
	"rpc_kcp3",
	ASIO2_TEST_CASE(rpc_kcp3_test)
)
