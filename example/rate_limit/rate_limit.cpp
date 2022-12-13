#ifndef ASIO2_INCLUDE_RATE_LIMIT
#define ASIO2_INCLUDE_RATE_LIMIT
#endif

#include <asio2/tcp/tcp_server.hpp>
#include <asio2/tcp/tcp_client.hpp>
#include <iostream>

int main()
{
	asio2::tcp_rate_server tcp_server;

	tcp_server.bind_connect([](std::shared_ptr<asio2::tcp_rate_session>& session_ptr)
	{
		// set the send rate
		session_ptr->socket().rate_policy().write_limit(256);

		// set the recv rate
		session_ptr->socket().rate_policy().read_limit(256);

	}).bind_recv([](auto& session_ptr, std::string_view data)
	{
		printf("recv : %zu %.*s\n", data.size(), (int)data.size(), data.data());

		session_ptr->async_send(data);
	});

	tcp_server.start("0.0.0.0", 8102);


	asio2::tcp_rate_client tcp_client;

	tcp_client.bind_connect([&tcp_client]()
	{
		// set the send rate
		tcp_client.socket().rate_policy().write_limit(512);

		if (!asio2::get_last_error())
		{
			std::string str;

			for (int i = 0; i < 1024; i++)
			{
				str += '0' + std::rand() % 64;
			}

			tcp_client.async_send(str);
		}
	}).bind_recv([&tcp_client](std::string_view data)
	{
		tcp_client.async_send(data);
	});

	tcp_client.start("127.0.0.1", 8102);

	std::thread([&tcp_client]()
	{
		// note: if you set the rate in the other thread which is not the io_context
		// thread, you should use post function to make the operation is executed in
		// the io_context thread, otherwise it is not thread safety.
		tcp_client.post([&tcp_client]()
		{
			// set the recv rate
			tcp_client.socket().rate_policy().read_limit(512);
		});
	}).detach();

	while (std::getchar() != '\n');

	return 0;
}
