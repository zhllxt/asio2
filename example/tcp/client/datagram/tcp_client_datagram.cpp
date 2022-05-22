#include <asio2/tcp/tcp_client.hpp>

void on_recv(asio2::tcp_client& client, std::string_view data)
{
	printf("recv : %.*s\n", (int)data.size(), data.data());

	// No matter what data we send, the server recved data must
	// to be exactly the same as what we sent here
	std::string str;
	str += '#';
	int len = 10 + (std::rand() % 100);
	for (int i = 0; i < len; i++)
	{
		str += (char)((std::rand() % 26) + 'a');
	}

	client.async_send(std::move(str));
}

void on_connect(asio2::tcp_client& client)
{
	if (asio2::get_last_error())
		printf("connect failure : %d %s\n",
			asio2::last_error_val(), asio2::last_error_msg().c_str());
	else
		printf("connect success : %s %u\n",
			client.local_address().c_str(), client.local_port());

	std::string str = "abc0123456789xyz";

	// Beacuse the server specify the "max recv buffer size" to 1024, so if we
	// send a too long packet, then this client will be disconnect .
	//str.resize(1500);

	client.async_send(std::move(str));
}

void on_disconnect()
{
	printf("disconnect : %d %s\n",
		asio2::last_error_val(), asio2::last_error_msg().c_str());
}

int main()
{
	std::string_view host = "127.0.0.1";
	std::string_view port = "8027";

	asio2::tcp_client client;

	// bind global function
	client
		.bind_recv   (std::bind(on_recv   , std::ref(client), std::placeholders::_1)) // use std::bind
		.bind_connect(std::bind(on_connect, std::ref(client)))
		.bind_disconnect(on_disconnect);

	client.start(host, port, asio2::use_dgram);

	while (std::getchar() != '\n');

	return 0;
}
