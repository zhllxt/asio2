#include <asio2/websocket/ws_client.hpp>
#include <iostream>

int main()
{
	std::string_view host = "127.0.0.1";
	std::string_view port = "8039";

	asio2::ws_client client;

	client.set_connect_timeout(std::chrono::seconds(5));

	client.bind_init([&]()
	{
		// Set the binary message write option.
		client.ws_stream().binary(true);

		// Set the text message write option. The sent text must be utf8 format.
		//client.ws_stream().text(true);

		// how to set custom websocket request data : 
		client.ws_stream().set_option(
			websocket::stream_base::decorator([](websocket::request_type& req)
		{
			req.set(http::field::authorization, "websocket-client-authorization");
		}));

	}).bind_connect([&]()
	{
		if (asio2::get_last_error())
			printf("connect failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
			printf("connect success : %s %u\n", client.local_address().c_str(), client.local_port());

		std::string s;
		s += '<';
		int len = 128 + std::rand() % 512;
		for (int i = 0; i < len; i++)
		{
			s += (char)((std::rand() % 26) + 'a');
		}
		s += '>';

		client.async_send(std::move(s), [](std::size_t bytes_sent) { std::ignore = bytes_sent; });

	}).bind_upgrade([&]()
	{
		if (asio2::get_last_error())
			std::cout << "upgrade failure : " << asio2::last_error_val() << " " << asio2::last_error_msg() << std::endl;
		else
		{
			const websocket::response_type& rep = client.get_upgrade_response();
			auto it = rep.find(http::field::authentication_results);
			if (it != rep.end())
			{
				beast::string_view auth = it->value();
				std::cout << auth << std::endl;
				ASIO2_ASSERT(auth == "200 OK");
			}

			std::cout << "upgrade success : " << rep << std::endl;
		}
	}).bind_recv([&](std::string_view data)
	{
		printf("recv : %zu %.*s\n", data.size(), (int)data.size(), data.data());

		std::string s;
		s += '<';
		int len = 128 + std::rand() % 512;
		for (int i = 0; i < len; i++)
		{
			s += (char)((std::rand() % 26) + 'a');
		}
		s += '>';

		client.async_send(std::move(s));
	});

	// the /ws is the websocket upgraged target
	if (!client.start(host, port, "/ws"))
	{
		printf("connect websocket server failure : %d %s\n",
			asio2::last_error_val(), asio2::last_error_msg().c_str());
	}

	// blocked forever util some signal delivered.
	// Normally, pressing Ctrl + C will emit the SIGINT signal.
	client.wait_signal(SIGINT);

	return 0;
}
