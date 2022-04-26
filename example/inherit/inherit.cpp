#include <asio2/asio2.hpp>
#include <asio2/tcp/tcp_server.hpp>
#include <asio2/tcp/tcp_client.hpp>
#include <iostream>

class my_tcp_session : public asio2::tcp_session_t<my_tcp_session>
{
public:
	//using asio2::tcp_session_t<my_tcp_session>::tcp_session_t;

	template<class... Args>
	my_tcp_session(Args&&... args) : asio2::tcp_session_t<my_tcp_session>(std::forward<Args>(args)...)
	{
		uuid = "custom string";
	}

	// ... user custom properties and functions
	std::string uuid;
};

using my_tcp_server1 = asio2::tcp_server_t<my_tcp_session>;

class my_tcp_server2 : public asio2::tcp_server_t<my_tcp_session>
{
public:
	using asio2::tcp_server_t<my_tcp_session>::tcp_server_t;
};

class my_tcp_client1 : public asio2::tcp_client_t<my_tcp_client1>
{
public:
	// ... user custom properties and functions
	std::string uuid;
};

class my_tcp_client2 : public asio2::tcp_client
{
public:
	// ... user custom properties and functions
	std::string uuid;
};

int main()
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	my_tcp_server1 my_server1;

	my_server1.post([]()
	{
		std::ignore = true;
	});
	my_server1.post([]()
	{
		std::ignore = true; 
	}, std::chrono::seconds(3));
	auto future1 = my_server1.post([]()
	{
		std::ignore = true; 
	}, asio::use_future);
	auto future2 = my_server1.post([]()
	{
		std::ignore = true;
	}, std::chrono::seconds(3), asio::use_future);
	my_server1.dispatch([]()
	{
		std::ignore = true;
	});
	auto future3 = my_server1.dispatch([]()
	{
		std::ignore = true;
	}, asio::use_future);

	my_server1.bind_connect([&](std::shared_ptr<my_tcp_session>& session_ptr)
	{
		session_ptr->uuid = std::to_string(session_ptr->hash_key());

	}).bind_recv([&](std::shared_ptr<my_tcp_session>& session_ptr, std::string_view sv)
	{
		asio2::ignore_unused(session_ptr, sv);

		ASIO2_ASSERT(session_ptr->uuid == std::to_string(session_ptr->hash_key()));

		printf("recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());

		session_ptr->async_send(sv);
	});

	my_server1.start("0.0.0.0", 9981);

	// --------------------------------------------------------------------------------

	my_tcp_client1 my_client1;

	my_client1.bind_connect([&]()
	{
		if (!asio2::get_last_error())
			my_client1.async_send("<0123456789abcdefghijklmnopqrstovwxyz>");

	}).bind_recv([&](std::string_view sv)
	{
		//printf("recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());

		my_client1.async_send(sv);
	});

	my_client1.async_start("127.0.0.1", 9981);

	while (std::getchar() != '\n');

	return 0;
}
