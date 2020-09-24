// When compiling with vs under linux, you need to copy the "asio,beast,ceral,fmt" folders to
// the /usr/local/include directory first, and copy the "libcrypto.a,libssl.a" files to 
// /usr/local/lib directory first. "libcrypto.a,libssl.a" is in "asio2/lib/x64".

#include <asio2/asio2.hpp>

class user
{
public:
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

int main()
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	std::string_view host = "127.0.0.1";
	std::string_view port = "8010";

	asio2::rpc_client client;

	// set default rpc call timeout
	client.default_timeout(std::chrono::seconds(3));

	client.bind_connect([&](asio::error_code ec)
	{
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


		//------------------------------------------------------------------
		// important : synchronous call can't be in the commucation thread.
		// this thread is a commucation thread. like bind_recv,bind_connect,
		// bind_disconnect..... is commucation thread.
		//------------------------------------------------------------------

		//ASIO2_ASSERT(false && client.call<double>("mul", 6.5, 6.5));

	});

	// bind a rpc function in client, the server will call this client's rpc function
	client.bind("sub", [](int a, int b) { return a - b; });

	client.start(host, port);

	asio::error_code ec;

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

	user u;
	u = client.call<user>(ec, "get_user");
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
	client.async_call("del_user", "hanmeimei").response([](asio::error_code ec, user)
	{
		ASIO2_ASSERT(bool(ec));
		std::cout << "del_user hanmeimei failed : " << ec.message() << std::endl;
	});

	// this call will be failed, beacuse the param is incorrect.
	client.async_call("del_user", 10, std::string("lilei"), 1).response([](asio::error_code ec, user)
	{
		ASIO2_ASSERT(bool(ec));
		std::cout << "del_user lilei failed : " << ec.message() << std::endl;
	});

	try
	{
		// Chain calls : 
		int sum = client.timeout(std::chrono::seconds(13)).call<int>("add", 11, 12);
		printf("sum5 : %d err : %d %s\n", sum, ec.value(), ec.message().c_str());
		if (!ec)
		{
			ASIO2_ASSERT(sum == 11 + 12);
		}
	}
	catch (std::exception& e)
	{
		printf("error : %s\n", e.what());
	}

	// Chain calls : 
	int sum = client.timeout(std::chrono::seconds(13)).errcode(ec).call<int>("add", 11, 32);
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

	while (std::getchar() != '\n');

	return 0;
}
