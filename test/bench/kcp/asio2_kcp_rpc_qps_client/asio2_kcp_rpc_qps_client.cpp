#include <asio2/rpc/rpc_client.hpp>

std::string strmsg(128, 'A');

std::function<void()> sender;

int main()
{
	asio2::rpc_kcp_client client;

	sender = [&]()
	{
		client.async_call([](std::string)
		{
			if (!asio2::get_last_error())
				sender();
		}, "echo", strmsg);
	};

	client.bind_init([&]() {
		client.socket().set_option(
			asio::ip::multicast::enable_loopback()
		);
		client.socket().set_option(
			asio::ip::multicast::join_group(asio::ip::make_address("239.255.0.1"))
		);
	})
	.bind_connect([&]()
	{
		if (!asio2::get_last_error())
			sender();
	});

	client.start("127.0.0.1", "8080");

	// -- sync qps test
	//while (true)
	//{
	//	client.call<std::string>("echo", strmsg);
	//}

	while (std::getchar() != '\n');

	return 0;
}
