#include <asio2/rpc/rpc_server.hpp>
#include <asio2/3rd/json.hpp>

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
		std::this_thread::sleep_for(std::chrono::seconds(1));

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

class A
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
		asio2::detail::ignore_unused(session_ptr);

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

int main()
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	std::string_view host = "0.0.0.0";
	std::string_view port = "8010";

	std::srand((unsigned int)time(nullptr));

	for(;;)
	{
	// Specify the "max recv buffer size" to avoid malicious packets, if some client
	// sent data packets size is too long to the "max recv buffer size", then the
	// client will be disconnect automatic .
	asio2::rpc_server server(
		512,  // the initialize recv buffer size : 
		1024, // the max recv buffer size :
		4     // the thread count : 
	);

	server.bind_connect([&](auto & session_ptr)
	{
		printf("client enter : %s %u %s %u\n",
			session_ptr->remote_address().c_str(), session_ptr->remote_port(),
			session_ptr->local_address().c_str(), session_ptr->local_port());
		session_ptr->post([]() {}, std::chrono::seconds(3));
		session_ptr->async_call([](int v)
		{
			if (!asio2::get_last_error())
			{
				ASIO2_ASSERT(v == 15 - 6);
			}
			printf("sub : %d err : %d %s\n", v, asio2::last_error_val(), asio2::last_error_msg().c_str());
		}, std::chrono::seconds(10), "sub", 15, 6);

	}).bind_disconnect([&](auto & session_ptr)
	{
		printf("client leave : %s %u\n", session_ptr->remote_address().c_str(), session_ptr->remote_port());
	}).bind_start([&]()
	{
		printf("start rpc server : %s %u %d %s\n",
			server.listen_address().c_str(), server.listen_port(),
			asio2::last_error_val(), asio2::last_error_msg().c_str());
	}).bind_stop([&]()
	{
		printf("stop : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	});

	A a;

	server
		.bind("add", add)
		.bind("mul", &A::mul, a)
		.bind("get_user", &A::get_user, a)
		.bind("del_user", &A::del_user, &a);

	server.bind("async_add", async_add);
	server.bind("async_test", async_test);

	server.bind("test_json", test_json);
	server.bind("heartbeat", heartbeat);

	server.bind("cat", [&](std::shared_ptr<asio2::rpc_session>& session_ptr,
		const std::string& a, const std::string& b)
	{
		// Nested call rpc function in business function is ok.
		session_ptr->async_call([session_ptr](int v) mutable
		{
			// Nested call rpc function in business function is ok.
			session_ptr->async_call([](int v)
			{
				if (!asio2::get_last_error())
				{
					ASIO2_ASSERT(v == 15 + 18);
				}
				printf("async_add : %d err : %d %s\n", v, asio2::last_error_val(), asio2::last_error_msg().c_str());
			}, "async_add", 15, 18);

			if (!asio2::get_last_error())
			{
				ASIO2_ASSERT(v == 15 - 8);
			}
			printf("sub : %d err : %d %s\n", v, asio2::last_error_val(), asio2::last_error_msg().c_str());
		}, "sub", 15, 8);

		return a + b;
	});

	server.start(host, port);

	auto sec = 1 + std::rand() % 2;

	std::this_thread::sleep_for(std::chrono::seconds(sec));

	server.start(host, port);

	server.stop();

	printf("<-------------------------------------------->\n");
	}

	while (std::getchar() != '\n');


	return 0;
}
