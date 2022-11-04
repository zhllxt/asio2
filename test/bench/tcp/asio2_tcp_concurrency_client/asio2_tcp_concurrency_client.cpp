#include <asio2/tcp/tcp_client.hpp>

int main(int argc, char* argv[])
{
	asio2::iopool iopool;

	iopool.start();

	int client_count = 10000;
	if (argc > 1)
		client_count = std::stoi(argv[1]);

	for (int i = 0; i < client_count; i++)
	{
		std::shared_ptr<asio2::tcp_client> client = std::make_shared<asio2::tcp_client>(iopool.get(i));

		client->bind_connect([pclt = client.get()]()
		{
			std::string strmsg(1024, 'A');

			if (!asio2::get_last_error())
				pclt->async_send(std::move(strmsg));

		}).bind_recv([pclt = client.get()](std::string_view data)
		{
			pclt->async_send(asio::buffer(data)); // no allocate memory
			//pclt->async_send(data); // allocate memory
		});

		client->async_start("127.0.0.1", "18081");
	}

	while (std::getchar() != '\n');

	iopool.stop();

	return 0;
}
