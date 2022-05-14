#include <asio2/rpc/rpc_client.hpp>

std::string strmsg(128, 'A');

std::function<void()> sender;

int main()
{
	asio2::rpc_client client;

	sender = [&]()
	{
		client.async_call([](std::string)
		{
			if (!asio2::get_last_error())
				sender();
		}, "echo", strmsg);
	};

	client.bind_connect([&]()
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
