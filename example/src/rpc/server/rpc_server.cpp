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

int add(int a, int b)
{
	return a + b;
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
	user get_user(std::shared_ptr<asio2::rpc_session>& session_ptr)
	{
		user u;
		u.name = "lilei";
		u.age = ((int)time(nullptr)) % 100;
		u.purview = { {1,"read"},{2,"write"} };
		return u;
	}

	// If you want to know which client called this function, set the first parameter
	// to std::shared_ptr<asio2::rpc_session>& session_ptr
	void del_user(std::shared_ptr<asio2::rpc_session>& session_ptr, const user& u)
	{
		printf("del_user is called by %s %u : %s %d \n",
			session_ptr->remote_address().c_str(), session_ptr->remote_port(),
			u.name.c_str(), u.age);
	}
};

int main()
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

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

		session_ptr->async_call([](asio::error_code ec, int v)
		{
			if (!ec)
			{
				ASIO2_ASSERT(v == 15 - 6);
			}
			printf("sub : %d err : %d %s\n", v, ec.value(), ec.message().c_str());
		}, std::chrono::seconds(10), "sub", 15, 6);

	}).bind_start([&](asio::error_code ec)
	{
		printf("start rpc server : %s %u %d %s\n",
			server.listen_address().c_str(), server.listen_port(),
			ec.value(), ec.message().c_str());
	}).bind_stop([&](asio::error_code ec)
	{
		printf("stop : %d %s\n", ec.value(), ec.message().c_str());
	});

	A a;

	server
		.bind("add", add)
		.bind("mul", &A::mul, a)
		.bind("get_user", &A::get_user, a)
		.bind("del_user", &A::del_user, &a);

	server.bind("cat", [&](std::shared_ptr<asio2::rpc_session>& session_ptr,
		const std::string& a, const std::string& b)
	{
		// Nested call rpc function in business function is ok.
		session_ptr->async_call([](asio::error_code ec, int v)
		{
			if (!ec)
			{
				ASIO2_ASSERT(v == 15 - 8);
			}
			printf("sub : %d err : %d %s\n", v, ec.value(), ec.message().c_str());
		}, "sub", 15, 8);

		return a + b;
	});

	server.start(host, port);

	while (std::getchar() != '\n');

	server.stop();

	return 0;
}
