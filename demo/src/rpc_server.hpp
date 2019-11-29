#pragma once

#include <asio2/asio2.hpp>

class user
{
public:
	std::string name;
	int age;
	std::map<int, std::string> purview;

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
		printf("del_user is called by %s : %s %d ", session_ptr->remote_address().c_str(), u.name.c_str(), u.age);
		for (auto &[k, v] : u.purview)
		{
			printf("%d %s ", k, v.c_str());
		}
		printf("\n");
	}
};

void run_rpc_server(std::string_view host, std::string_view port)
{
	asio2::rpc_server server;
	//while (1) // use infinite loop and sleep 2 seconds to test start and stop
	{
		std::shared_ptr<asio2::rpc_session> client_ptr;
		printf("\n");
		server.start_timer(1, std::chrono::seconds(1), []() {});
		server.bind_recv([&server](auto & session_ptr, std::string_view s)
		{
			//printf("recv : %u %.*s\n", (unsigned)s.size(), (int)s.size(), s.data());
		}).bind_send([&](auto & session_ptr, std::string_view s)
		{
		}).bind_connect([&server,&client_ptr](auto & session_ptr)
		{
			//session_ptr->stop();
			printf("client enter : %s %u %s %u\n",
				session_ptr->remote_address().c_str(), session_ptr->remote_port(),
				session_ptr->local_address().c_str(), session_ptr->local_port());
			client_ptr = session_ptr;
			session_ptr->async_call([](asio::error_code ec, int v)
			{
				printf("sub : %d err : %d %s\n", v, ec.value(), ec.message().c_str());
			}, std::chrono::seconds(10), "sub", 15, 8);
			
		}).bind_disconnect([&server](auto & session_ptr)
		{
			printf("client leave : %s %u %s\n",
				session_ptr->remote_address().c_str(),
				session_ptr->remote_port(), asio2::last_error_msg().c_str());
		}).bind_start([&server](asio::error_code ec)
		{
			printf("start rpc server : %s %u %d %s\n", server.listen_address().c_str(), server.listen_port(),
				ec.value(), ec.message().c_str());
		}).bind_stop([&server](asio::error_code ec)
		{
			printf("stop : %d %s\n", ec.value(), ec.message().c_str());
		});

		A a;
		server.bind("add", add);
		server.bind("mul", &A::mul, a);
		server.bind("cat", [&](const std::string& a, const std::string& b)
		{
			// Nested call rpc function in business function is ok.
			client_ptr->async_call([](asio::error_code ec, int v)
			{
				printf("sub : %d err : %d %s\n", v, ec.value(), ec.message().c_str());
			}, std::chrono::seconds(10), "sub", 15, 8);
			return a + b;
		});
		server.bind("get_user", &A::get_user, a);
		server.bind("del_user", &A::del_user, &a);

		// Using tcp dgram mode as the underlying communication support(This is the default setting)
		// Then must use "use_dgram" parameter.
		server.start(host, port, asio2::use_dgram);

		// Using websocket as the underlying communication support.
		// Need to goto the tail of the (rpc_client.hpp rpc_server.hpp rpc_session.hpp) files,
		// and modified to use websocket.
		//server.start(host, port);

		while (std::getchar() != '\n');
		//std::this_thread::sleep_for(std::chrono::seconds(1));

		server.stop();
	}
}
