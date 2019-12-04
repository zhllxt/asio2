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

void run_rpc_client(std::string_view host, std::string_view port)
{
	//while (1) // use infinite loop and sleep 2 seconds to test start and stop
	{
		int count = 1;
		std::unique_ptr<asio2::rpc_client[]> clients = std::make_unique<asio2::rpc_client[]>(count);
		for (int i = 0; i < count; ++i)
		{
			auto & client = clients[i];
			client.start_timer(1, std::chrono::seconds(1), []() {});
			client.timeout(std::chrono::seconds(3));
			client.bind_connect([&](asio::error_code ec)
			{
				printf("connect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
				// the type of the callback's second parameter is auto, so you have to specify 
				// the return type in the template function like 'async_call<int>'
				client.async_call<int>([](asio::error_code ec, auto v)
				{
					printf("sum : %d err : %d %s\n", v, ec.value(), ec.message().c_str());
				}, "add", 120, 11);
			}).bind_disconnect([](asio::error_code ec)
			{
				printf("disconnect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
			}).bind_recv([&](std::string_view sv)
			{
			});

			client.bind("sub", [](int a, int b) { return a - b; });

			// Using tcp dgram mode as the underlying communication support(This is the default setting)
			// Then must use "use_dgram" parameter.
			client.start(host, port, asio2::use_dgram);

			// Using websocket as the underlying communication support.
			// Need to goto the tail of the (rpc_client.hpp rpc_server.hpp rpc_session.hpp) files,
			// and modified to use websocket.
			//client.start(host, port);

			//for (;;)
			{
				asio::error_code ec;
				int sum = client.call<int>(ec, std::chrono::seconds(3), "add", 11, 2);
				printf("sum : %d err : %d %s\n", sum, ec.value(), ec.message().c_str());

				// the type of the callback's second parameter is int, so you have't to specify 
				// the return type in the template function
				client.async_call([](asio::error_code ec, int v)
				{
					printf("sum : %d err : %d %s\n", v, ec.value(), ec.message().c_str());
				}, "add", 10, 20);

				client.async_call([](asio::error_code ec, int v)
				{
					printf("no_exists_fn : %d err : %d %s\n", v, ec.value(), ec.message().c_str());
				}, "no_exists_fn", 10, 20);

				// the type of the callback's second parameter is auto, so you have to specify 
				// the return type in the template function like 'async_call<int>'
				// of course you can set the timeout like : std::chrono::seconds(3)
				client.async_call<int>([](asio::error_code ec, auto v)
				{
					printf("sum : %d err : %d %s\n", v, ec.value(), ec.message().c_str());
				}, std::chrono::seconds(3), "add", 12, 21);

				try
				{
					double mul = client.call<double>("mul", 2.5, 2.5);
					printf("mul : %lf err : %d %s\n", mul, ec.value(), ec.message().c_str());
				}
				catch (asio2::system_error& e) { printf("mul : %d %s\n", e.code().value(), e.code().message().c_str()); }

				client.async_call([](asio::error_code ec, std::string v)
				{
					printf("cat : %s err : %d %s\n", v.c_str(), ec.value(), ec.message().c_str());
				}, "cat", std::string("abc"), std::string("123"));

				user u = client.call<user>(ec, "get_user");
				printf("%s %d ", u.name.c_str(), u.age);
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
				}, "del_user", std::move(u));

				//std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		}

		while (std::getchar() != '\n');
		//std::this_thread::sleep_for(std::chrono::seconds(2));

		for (int i = 0; i < count; ++i)
		{
			auto & client = clients[i];
			client.stop();
		}
	}
}
