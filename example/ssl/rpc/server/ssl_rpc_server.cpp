#ifndef ASIO2_USE_SSL
#define ASIO2_USE_SSL
#endif

#include <asio2/rpc/rpc_server.hpp>

class userinfo
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

	userinfo get_user()
	{
		userinfo u;
		u.name = "lilei";
		u.age = ((int)time(nullptr)) % 100;
		u.purview = { {1,"read"},{2,"write"} };
		return u;
	}

	// If you want to know which client called this function, set the first parameter
	// to std::shared_ptr<asio2::rpcs_session>& session_ptr
	void del_user(std::shared_ptr<asio2::rpcs_session>& session_ptr, const userinfo& u)
	{
		printf("del_user is called by %s %u : %s %d \n",
			session_ptr->remote_address().c_str(), session_ptr->remote_port(),
			u.name.c_str(), u.age);
	}
};

int main()
{
	std::string_view host = "0.0.0.0";
	std::string_view port = "8011";

	asio2::rpcs_server server;

	// use file for cert
	server.set_cert_file("../../cert/ca.crt", "../../cert/server.crt", "../../cert/server.key", "123456");

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
		printf("start rpcs server : %s %u %d %s\n", server.listen_address().c_str(), server.listen_port(),
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

	server.bind("cat", [&](std::shared_ptr<asio2::rpcs_session>& session_ptr, const std::string& a, const std::string& b)
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
