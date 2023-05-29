#include <asio2/rpc/rpc_client.hpp>
#include <asio2/external/json.hpp>

asio2::rpc_client* pclient = nullptr; // just for test get_current_caller

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

// -- serialize the third party object
namespace nlohmann
{
	template<class Archive>
	void save(Archive& ar, nlohmann::json const& j)
	{
		ar << j.dump();
	}

	template<class Archive>
	void load(Archive& ar, nlohmann::json& j)
	{
		std::string v;
		ar >> v;
		j = nlohmann::json::parse(v);
	}
}

// Asynchronous rpc function 
rpc::future<int> async_add(int a, int b)
{
	ASIO2_ASSERT(pclient == asio2::get_current_caller<asio2::rpc_client*>());

	rpc::promise<int> promise;
	rpc::future<int> f = promise.get_future();

	std::thread([a, b, promise = std::move(promise)]() mutable
	{
		promise.set_value(a + b);
	}).detach();

	return f;
}

// set the first parameter to client reference to know which client was called
void test(asio2::rpc_client& client, std::string str)
{
	asio2::rpc_client& rc = asio2::get_current_caller<asio2::rpc_client&>();

	asio2::ignore_unused(rc);

	ASIO2_ASSERT(&client == &rc);
	ASIO2_ASSERT(&client == pclient);

	std::cout << client.get_user_data<int>() << " - test : " << str << std::endl;
}

int main()
{
	std::string_view host = "127.0.0.1";
	std::string_view port = "8010";

	asio2::rpc_client client;

	// just for test get_current_caller
	pclient = &client;
	client.set_user_data(1);

	// set default rpc call timeout
	client.set_default_timeout(std::chrono::seconds(3));

	client.bind_connect([&]()
	{
		if (asio2::get_last_error())
			return;

		//------------------------------------------------------------------
		// this thread is a commucation thread. like bind_recv,bind_connect,
		// bind_disconnect..... is commucation thread.
		// important : synchronous call rpc function in the commucation thread,
		// then the call will degenerates into async_call and the return value is empty.
		//------------------------------------------------------------------
		client.call<double>("mul", 16.5, 26.5);
		if (client.is_started())
		{
			ASIO2_ASSERT(asio2::get_last_error() == rpc::error::in_progress);
		}
		else
		{
			ASIO2_ASSERT(asio2::get_last_error() == rpc::error::not_connected);
		}

		// param 1 : user callback function(this param can be empty)
		// param 2 : timeout (this param can be empty, if this param is empty, use the default timeout)
		// param 3 : function name
		// param 4 : function params
		client.async_call([](int v)
		{
			if (!asio2::get_last_error())
			{
				ASIO2_ASSERT(v == 12 + 11);
			}
			printf("sum1 : %d err : %d %s\n",
				v, asio2::last_error_val(), asio2::last_error_msg().c_str());
		}, std::chrono::seconds(13), "add", 12, 11);

		// call the rpc function which has no param
		client.async_call([]()
		{
			printf("heartbeat err : %d %s\n",
				asio2::last_error_val(), asio2::last_error_msg().c_str());
		}, std::chrono::seconds(3), "heartbeat");

		nlohmann::json j = nlohmann::json::object();

		j["name"] = "lilei";
		j["age"] = 30;

		// Chain calls
		client.async_call("test_json", j).response([](nlohmann::json js)
		{
			std::string s = js.dump();

			if (!asio2::get_last_error())
			{
				ASIO2_ASSERT(js["age"].get<int>() == 30);
				ASIO2_ASSERT(js["name"].get<std::string>() == "lilei");
			}

			asio2::ignore_unused(js, s);
		});

		// param 2 is empty, use the default timeout
		client.async_call([](int v)
		{
			if (!asio2::get_last_error())
			{
				ASIO2_ASSERT(v == 12 + 21);
			}
			printf("sum2 : %d err : %d %s\n", v,
				asio2::last_error_val(), asio2::last_error_msg().c_str());
		}, "add", 12, 21);

		// call asynchronous rpc function, server's async_add is a asynchronous rpc function
		client.async_call([](int v)
		{
			if (!asio2::get_last_error())
			{
				ASIO2_ASSERT(v == 1 + 11);
			}
			printf("async_add : %d err : %d %s\n", v,
				asio2::last_error_val(), asio2::last_error_msg().c_str());
		}, std::chrono::seconds(3), "async_add", 1, 11);

		// param 1 is empty, the result of the rpc call is not returned
		client.async_call("mul0", 2.5, 2.5);

		// Chain calls : 
		client.set_timeout(std::chrono::seconds(5)).async_call("mul", 2.5, 2.5).response([](double v)
		{
			if (!asio2::get_last_error())
			{
				ASIO2_ASSERT(v == 2.5 * 2.5);
			}
			std::cout << "mul1 " << v << std::endl;
		});

		client.async_call([]()
		{
			printf("async_test err : %d %s\n",
				asio2::last_error_val(), asio2::last_error_msg().c_str());
		}, std::chrono::seconds(3), "async_test", "abc", "def");
	});

	// bind a rpc function in client, the server will call this client's rpc function
	client.bind("sub", [](int a, int b) { return a - b; });

	client.bind("async_add", async_add);

	client.bind("test", test);

	client.start(host, port);

	// synchronous call
	// param 1 : timeout (this param can be empty, if this param is empty, use the default timeout)
	// param 2 : rpc function name
	// param 3 : rpc function params
	double mul = client.call<double>(std::chrono::seconds(3), "mul", 6.5, 6.5);
	printf("mul5 : %lf err : %d %s\n", mul, asio2::last_error_val(), asio2::last_error_msg().c_str());
	if (!asio2::get_last_error())
	{
		ASIO2_ASSERT(mul == 6.5 * 6.5);
	}

	nlohmann::json j = nlohmann::json::object();

	j["name"] = "hanmeimei";
	j["age"] = 20;

	nlohmann::json j2 = client.call<nlohmann::json>(std::chrono::minutes(1), "test_json", j);
	if (!asio2::get_last_error())
	{
		ASIO2_ASSERT(j2["age"].get<int>() == 20);
		ASIO2_ASSERT(j2["name"].get<std::string>() == "hanmeimei");
	}

	userinfo u;
	u = client.call<userinfo>("get_user");
	if (!asio2::get_last_error())
	{
		ASIO2_ASSERT(u.name == "lilei" && u.purview.size() == 2);
	}

	u.name = "hanmeimei";
	u.age = ((int)time(nullptr)) % 100;
	u.purview = { {10,"get"},{20,"set"} };
	client.async_call([]()
	{
		if (asio2::get_last_error())
			printf("del_user : failed : %s\n\n", asio2::last_error_msg().c_str());
		else
			printf("del_user : successed\n\n");

	}, "del_user", u);
	

	// just call rpc function, don't need the rpc result
	client.async_call("del_user", std::move(u));

	// this call will be failed, beacuse the param is incorrect.
	client.async_call("del_user", "hanmeimei").response([](userinfo)
	{
		ASIO2_ASSERT(bool(asio2::get_last_error()));
		std::cout << "del_user hanmeimei failed : " << asio2::last_error_msg() << std::endl;
	});

	// this call will be failed, beacuse the param is incorrect.
	client.async_call("del_user", 10, std::string("lilei"), 1).response([](userinfo)
	{
		ASIO2_ASSERT(bool(asio2::get_last_error()));
		std::cout << "del_user lilei failed : " << asio2::last_error_msg() << std::endl;
	});

	// Chain calls : 
	int sum = client.timeout(std::chrono::seconds(13)).call<int>("add", 11, 12);
	printf("sum5 : %d err : %d %s\n", sum, asio2::last_error_val(), asio2::last_error_msg().c_str());
	if (!asio2::get_last_error())
	{
		ASIO2_ASSERT(sum == 11 + 12);
	}

	// Chain calls : 
	std::string str = client.call<std::string>("cat", "abc", "123");
	printf("cat : %s err : %d %s\n", str.data(), asio2::last_error_val(), asio2::last_error_msg().c_str());
	if (!asio2::get_last_error())
	{
		ASIO2_ASSERT(str == "abc123");
	}

	// Call a non-existent rpc function
	client.async_call([](int v)
	{
		printf("test call no_exists_fn : %d err : %d %s\n",
			v, asio2::last_error_val(), asio2::last_error_msg().c_str());
		ASIO2_ASSERT(bool(asio2::get_last_error()));
	}, "no_exists_fn", 10);

	client.start_timer("timer_id1", std::chrono::milliseconds(500), [&]()
	{
		std::string s1;
		s1 += '<';
		for (int i = 100 + std::rand() % (100); i > 0; i--)
		{
			s1 += (char)((std::rand() % 26) + 'a');
		}

		std::string s2;
		for (int i = 100 + std::rand() % (100); i > 0; i--)
		{
			s2 += (char)((std::rand() % 26) + 'a');
		}
		s2 += '>';

		client.async_call([s1, s2](std::string v)
		{
			if (!asio2::get_last_error())
			{
				ASIO2_ASSERT(v == s1 + s2);
			}
			printf("cat : %s - %d %s\n", v.c_str(),
				asio2::last_error_val(), asio2::last_error_msg().c_str());

		}, "cat", s1, s2);
	});

	while (std::getchar() != '\n');

	return 0;
}
