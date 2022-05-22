#include <asio2/tcp/tcp_client.hpp>

class clt_listener
{
public:
	void on_connect(asio2::tcp_client& client)
	{
		if (asio2::get_last_error())
			printf("connect failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
			printf("connect success : %s %u\n", client.local_address().c_str(), client.local_port());

		std::string str;
		int len = 128 + std::rand() % (300);
		for (int i = 0; i < len; i++)
		{
			str += (char)((std::rand() % 26) + 'a');
		}
		str += "\r\n";

		client.async_send(std::move(str));
	}

	void on_disconnect()
	{
		printf("disconnect : %d %s\n",
			asio2::last_error_val(), asio2::last_error_msg().c_str());
	}

	void on_recv(asio2::tcp_client& client, std::string_view data)
	{
		printf("recv : %zu %.*s\n", data.size(), (int)data.size(), data.data());

		std::string str;
		int len = 128 + std::rand() % (300);
		for (int i = 0; i < len; i++)
		{
			str += (char)((std::rand() % 26) + 'a');
		}
		str += "\r\n";

		// this is just a demo to show :
		// even if we force one packet data to be sent twice,
		// but the server must recvd whole packet once
		client.async_send(str.substr(0, str.size() / 2));
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		client.async_send(str.substr(str.size() / 2));

		// of course you can sent the whole data once
		//client.async_send(std::move(str));
	}
};

int main()
{
	std::string_view host = "127.0.0.1";
	std::string_view port = "8025";

	asio2::tcp_client client;

	clt_listener listener;

	// bind member function
	client
		.bind_disconnect(&clt_listener::on_disconnect, listener)
		.bind_connect   (&clt_listener::on_connect   , listener, std::ref(client))  // not use std::bind
		.bind_recv      (&clt_listener::on_recv      , listener, std::ref(client)); // not use std::bind

	// Split data with a single character
	//client.start(host, port, '\n');

	// Split data with string
	client.start(host, port, "\r\n");

	while (std::getchar() != '\n');

	return 0;
}
