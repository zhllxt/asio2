#include <asio2/http/ws_client.hpp>
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

		// Set the text message write option.
		//client.ws_stream().text(true);

		// how to set custom websocket request data : 
		client.ws_stream().set_option(websocket::stream_base::decorator(
			[](websocket::request_type& req)
		{
			req.set(http::field::authorization, " websocket-client-authorization");
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
			std::cout << "upgrade success : " << client.get_upgrade_response() << std::endl;

		// this send will be failed, because connection is not fully completed
		client.async_send("abc", []()
		{
			ASIO2_ASSERT(asio2::get_last_error());
			std::cout << "send failed : " << asio2::last_error_msg() << std::endl;
		});

	}).bind_recv([&](std::string_view data)
	{
		printf("recv : %zu %.*s\n", data.size(), (int)data.size(), data.data());

		client.async_send(data);
	});

	// the /ws is the websocket upgraged target
	if (!client.start(host, port, "/ws"))
	{
		printf("connect websocket server failure : %d %s\n",
			asio2::last_error_val(), asio2::last_error_msg().c_str());
	}

	while (std::getchar() != '\n');

	return 0;
}
