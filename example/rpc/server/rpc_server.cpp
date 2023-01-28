#include <asio2/rpc/rpc_server.hpp>
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

// -- serialize the third party object
namespace nlohmann
{
	template<class Archive>
	void save(Archive& ar, json const& j)
	{
		ar << j.dump();
	}

	template<class Archive>
	void load(Archive& ar, json& j)
	{
		std::string v;
		ar >> v;
		j = json::parse(v);
	}
}

int add(int a, int b)
{
	return a + b;
}

// Asynchronous rpc function 
rpc::future<int> async_add(int a, int b)
{
	// If you want to know which client called this function, method 1:
	std::shared_ptr<asio2::rpc_session> sptr = asio2::get_current_caller<std::shared_ptr<asio2::rpc_session>>();

	rpc::promise<int> promise;
	rpc::future<int> f = promise.get_future();

	std::thread([a, b, promise = std::move(promise), sptr = std::move(sptr)]() mutable
	{
		ASIO2_ASSERT(sptr);

		std::this_thread::sleep_for(std::chrono::seconds(1));

		promise.set_value(a + b);
	}).detach();

	return f;
}

// Asynchronous rpc function, and the return value is void
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

// rpc function with return third party object
nlohmann::json test_json(nlohmann::json j)
{
	std::string s = j.dump();

	return nlohmann::json::parse(s);
}

class user_crud
{
public:
	double mul(double a, double b)
	{
		return a * b;
	}

	// If you want to know which client called this function, method 2:
	// set the first parameter to std::shared_ptr<asio2::rpc_session>& session_ptr
	userinfo get_user(std::shared_ptr<asio2::rpc_session>& session_ptr)
	{
		// use get_current_caller to get the client again, this is just for test.
		std::shared_ptr<asio2::rpc_session> sptr = asio2::get_current_caller<std::shared_ptr<asio2::rpc_session>>();
		ASIO2_ASSERT(session_ptr.get() == sptr.get());
		asio2::ignore_unused(session_ptr, sptr);

		userinfo u;
		u.name = "lilei";
		u.age = ((int)time(nullptr)) % 100;
		u.purview = { {1,"read"},{2,"write"} };
		return u;
	}

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
	std::string_view host = "0.0.0.0";
	std::string_view port = "8010";

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

		// when a client is connected, we call the client's rpc function.
		session_ptr->async_call([](int v)
		{
			if (!asio2::get_last_error())
			{
				ASIO2_ASSERT(v == 15 - 6);
			}
			printf("sub : %d err : %d %s\n", v,
				asio2::last_error_val(), asio2::last_error_msg().c_str());
		}, std::chrono::seconds(10), "sub", 15, 6);

		// just call the client's rpc function, and don't care the client's response
		session_ptr->async_call("test", "i love you");

	}).bind_start([&]()
	{
		printf("start rpc server : %s %u %d %s\n",
			server.listen_address().c_str(), server.listen_port(),
			asio2::last_error_val(), asio2::last_error_msg().c_str());
	});

	user_crud usercrud;

	// bind member rpc function
	server
		.bind("add", add)
		.bind("mul", &user_crud::mul, usercrud)
		.bind("get_user", &user_crud::get_user, usercrud)
		.bind("del_user", &user_crud::del_user, &usercrud);

	// bind global rpc function
	server.bind("async_add", async_add);
	server.bind("async_test", async_test);

	server.bind("test_json", test_json);
	server.bind("heartbeat", heartbeat);

	// bind lambda rpc function
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
				printf("async_add : %d err : %d %s\n", v,
					asio2::last_error_val(), asio2::last_error_msg().c_str());
			}, "async_add", 15, 18);

			if (!asio2::get_last_error())
			{
				ASIO2_ASSERT(v == 15 - 8);
			}
			printf("sub : %d err : %d %s\n", v,
				asio2::last_error_val(), asio2::last_error_msg().c_str());
		}, "sub", 15, 8);

		return a + b;
	});

	server.start(host, port);

	while (std::getchar() != '\n');

	return 0;
}
