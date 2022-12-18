#include <asio2/udp/udp_client.hpp>

int main()
{
	asio2::udp_client client(1500, 1500);

	client.bind_connect([&]()
	{
		std::string strmsg(1024, 'A');

		if (!asio2::get_last_error())
			client.async_send(std::move(strmsg));

	}).bind_recv([&](std::string_view data)
	{
		client.async_send(asio::buffer(data)); // no allocate memory
		//client.async_send(data); // allocate memory
	});

	client.start("127.0.0.1", "8116");

	while (std::getchar() != '\n');

	return 0;
}
