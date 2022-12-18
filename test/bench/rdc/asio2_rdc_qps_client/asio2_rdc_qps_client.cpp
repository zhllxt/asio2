#include <asio2/tcp/tcp_client.hpp>

std::string strmsg(127, 'A');

std::function<void()> sender;

int main()
{
	asio2::tcp_client client;

	strmsg += '\n';

	sender = [&]()
	{
		client.async_call(strmsg, [](std::string_view)
		{
			if (!asio2::get_last_error())
				sender();
		});
	};

	client.bind_connect([&]()
	{
		if (!asio2::get_last_error())
			sender();
	});

	asio2::rdc::option rdc_option
	{
		[](std::string_view) { return 0; }
	};

	client.start("127.0.0.1", "8110", '\n', rdc_option);

	while (std::getchar() != '\n');

	return 0;
}
