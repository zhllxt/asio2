#include <asio2/rpc/rpc_client.hpp>
#include <nlohmann/json.hpp>

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

// -- method 1
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

// -- method 2
namespace nlohmann
{
	//template<class Archive>
	//void save(Archive& ar, nlohmann::json const& j)
	//{
	//	ar << j.dump();
	//}

	//template<class Archive>
	//void load(Archive& ar, nlohmann::json& j)
	//{
	//	std::string v;
	//	ar >> v;
	//	j = nlohmann::json::parse(v);
	//}
}

rpc::future<int> async_add(int a, int b)
{
	rpc::promise<int> promise;
	rpc::future<int> f = promise.get_future();

	std::thread([a, b, promise = std::move(promise)]() mutable
	{
		promise.set_value(a + b);
	}).detach();

	return f;
}

int main()
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	std::string_view host = "127.0.0.1";
	std::string_view port = "8010";

	std::srand((unsigned int)time(nullptr));

	while (!asio2::detail::has_unexpected_behavior())
	{
	asio2::rpc_client *clients = new asio2::rpc_client[10];

	for (int i = 0; i < 10; i++)
	{
		auto & client = clients[i];

	// set default rpc call timeout
	client.default_timeout(std::chrono::seconds(3));

	client.bind_connect([&](asio::error_code ec)
	{
		asio2::detail::ignore_unused(ec);

		//------------------------------------------------------------------
		// this thread is a commucation thread. like bind_recv,bind_connect,
		// bind_disconnect..... is commucation thread.
		// important : synchronous call rpc function in the commucation thread,
		// then the call will degenerates into async_call and the return value is empty.
		//------------------------------------------------------------------
		client.call<double>("mul", 16.5, 26.5);
		if (client.is_started())
			ASIO2_ASSERT(asio2::get_last_error() == asio::error::in_progress);
		else
			ASIO2_ASSERT(asio2::get_last_error() == asio::error::not_connected);

		// param 1 : user callback function(this param can be empty)
		// param 2 : timeout (this param can be empty, if this param is empty, use the default_timeout)
		// param 3 : function name
		// param 4 : function params
		client.async_call([](asio::error_code ec, int v)
		{
			if (!ec)
			{
				ASIO2_ASSERT(v == 12 + 11);
			}
			printf("sum1 : %d err : %d %s\n", v, ec.value(), ec.message().c_str());
		}, std::chrono::seconds(13), "add", 12, 11);

		nlohmann::json j = nlohmann::json::object();

		j["name"] = "lilei";
		j["age"] = 30;

		client.async_call("test_json", j).response([](asio::error_code ec, nlohmann::json js)
		{
			std::string s = js.dump();

			if (!ec)
			{
				ASIO2_ASSERT(js["age"].get<int>() == 30);
				ASIO2_ASSERT(js["name"].get<std::string>() == "lilei");
			}

			asio2::detail::ignore_unused(ec, js, s);
		});

		// param 2 is empty, use the default_timeout
		client.async_call([](asio::error_code ec, int v)
		{
			if (!ec)
			{
				ASIO2_ASSERT(v == 12 + 21);
			}
			printf("sum2 : %d err : %d %s\n", v, ec.value(), ec.message().c_str());
		}, "add", 12, 21);

		// param 1 is empty, the result of the rpc call is not returned
		client.async_call("mul0", 2.5, 2.5);


		// Chain calls : 
		client.timeout(std::chrono::seconds(5)).async_call("mul", 2.5, 2.5).response(
			[](asio::error_code ec, double v)
		{
			if (!ec)
			{
				ASIO2_ASSERT(v == 2.5 * 2.5);
			}
			std::cout << "mul1 " << v << std::endl;
		});

		// Chain calls : 
		client.timeout(std::chrono::seconds(13)).response([](asio::error_code ec, double v)
		{
			if (!ec)
			{
				ASIO2_ASSERT(v == 3.5 * 3.5);
			}
			std::cout << "mul2 " << v << std::endl;
		}).async_call("mul", 3.5, 3.5);

		// Chain calls : 
		client.response([](asio::error_code ec, double v)
		{
			if (!ec)
			{
				ASIO2_ASSERT(v == 4.5 * 4.5);
			}
			std::cout << "mul3 " << v << std::endl;
		}).timeout(std::chrono::seconds(5)).async_call("mul", 4.5, 4.5);

		// Chain calls : 
		client.async_call("mul", 5.5, 5.5).response([](asio::error_code ec, double v)
		{
			if (!ec)
			{
				ASIO2_ASSERT(v == 5.5 * 5.5);
			}
			std::cout << "mul4 " << v << std::endl;
		}).timeout(std::chrono::seconds(10));

		client.async_call([](asio::error_code ec, int v)
		{
			if (!ec)
			{
				ASIO2_ASSERT(v == 1 + 11);
			}
			printf("async_add : %d err : %d %s\n", v, ec.value(), ec.message().c_str());
		}, std::chrono::seconds(3), "async_add", 1, 11);

		client.async_call([](asio::error_code ec)
		{
			printf("async_test err : %d %s\n", ec.value(), ec.message().c_str());
		}, std::chrono::seconds(3), "async_test", "abc", "def");
	});

	// bind a rpc function in client, the server will call this client's rpc function
	client.bind("sub", [](int a, int b) { return a - b; });

	client.bind("async_add", async_add);

	client.start(host, port);

	client.start(host, port);

	client.async_start(host, port);

	asio::error_code ec;

	nlohmann::json j = nlohmann::json::object();

	j["name"] = "hanmeimei";
	j["age"] = 20;

	nlohmann::json j2 = client.call<nlohmann::json>(ec, std::chrono::minutes(1), "test_json", j);
	if (!ec)
	{
		ASIO2_ASSERT(j2["age"].get<int>() == 20);
		ASIO2_ASSERT(j2["name"].get<std::string>() == "hanmeimei");
	}

	// synchronous call
	// param 1 : error_code refrence (this param can be empty)
	// param 2 : timeout (this param can be empty, if this param is empty, use the default_timeout)
	// param 3 : rpc function name
	// param 4 : rpc function params
	double mul = client.call<double>(ec, std::chrono::seconds(3), "mul", 6.5, 6.5);
	printf("mul5 : %lf err : %d %s\n", mul, ec.value(), ec.message().c_str());
	if (!ec)
	{
		ASIO2_ASSERT(mul == 6.5 * 6.5);
	}

	userinfo u;
	u = client.call<userinfo>(ec, "get_user");
	if (!ec)
	{
		ASIO2_ASSERT(u.name == "lilei" && u.purview.size() == 2);
	}
	printf("get_user : %s %d -> ", u.name.c_str(), u.age);
	for (auto &[k, v] : u.purview)
	{
		printf("%d %s ", k, v.c_str());
	}
	printf("\n");

	u.name = "hanmeimei";
	u.age = ((int)time(nullptr)) % 100;
	u.purview = { {10,"get"},{20,"set"} };
	client.async_call([](asio::error_code ec)
	{
		if (ec)
			printf("del_user : failed : %s\n\n", ec.message().c_str());
		else
			printf("del_user : successed\n\n");

	}, "del_user", u);
	

	// just call rpc function, don't need the rpc result
	client.async_call("del_user", std::move(u));

	// this call will be failed, beacuse the param is incorrect.
	client.async_call("del_user", "hanmeimei").response([](asio::error_code ec, userinfo)
	{
		ASIO2_ASSERT(bool(ec));
		std::cout << "del_user hanmeimei failed : " << ec.message() << std::endl;
	});

	// this call will be failed, beacuse the param is incorrect.
	client.async_call("del_user", 10, std::string("lilei"), 1).response([](asio::error_code ec, userinfo)
	{
		ASIO2_ASSERT(bool(ec));
		std::cout << "del_user lilei failed : " << ec.message() << std::endl;
	});

	// Chain calls : 
	int sum = client.timeout(std::chrono::seconds(13)).call<int>("add", 11, 12);
	printf("sum5 : %d err : %d %s\n", sum, asio2::last_error_val(), asio2::last_error_msg().c_str());
	if (!asio2::get_last_error())
	{
		ASIO2_ASSERT(sum == 11 + 12);
	}

	// Chain calls : 
	sum = client.timeout(std::chrono::seconds(13)).errcode(ec).call<int>("add", 11, 32);
	printf("sum6 : %d err : %d %s\n", sum, ec.value(), ec.message().c_str());
	if (!ec)
	{
		ASIO2_ASSERT(sum == 11 + 32);
	}

	// Chain calls : 
	std::string str = client.errcode(ec).call<std::string>("cat", "abc", "123");
	printf("cat : %s err : %d %s\n", str.data(), ec.value(), ec.message().c_str());
	if (!ec)
	{
		ASIO2_ASSERT(str == "abc123");
	}

	client.async_call([](asio::error_code ec, int v)
	{
		printf("test call no_exists_fn : %d err : %d %s\n", v, ec.value(), ec.message().c_str());
		ASIO2_ASSERT(bool(ec));
	}, "no_exists_fn", 10);

	}

	std::this_thread::sleep_for(std::chrono::seconds(1 + std::rand() % 2));

	delete[]clients;

	ASIO2_LOG(spdlog::level::debug, "<-------------------------------------------->");

	}

	while (std::getchar() != '\n');

	return 0;
}
