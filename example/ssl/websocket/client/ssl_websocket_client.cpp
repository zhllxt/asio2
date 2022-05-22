#ifndef ASIO2_USE_SSL
#define ASIO2_USE_SSL
#endif

#include <asio2/http/wss_client.hpp>

int main()
{
	std::string_view host = "127.0.0.1";
	std::string_view port = "8007";

	asio2::wss_client client;

	client.set_connect_timeout(std::chrono::seconds(10));

	client.set_verify_mode(asio::ssl::verify_peer);
	client.set_cert_file(
		"../../cert/ca.crt",
		"../../cert/client.crt",
		"../../cert/client.key",
		"123456");

	client.bind_init([&]()
	{
		// how to set custom websocket request data : 
		client.ws_stream().set_option(websocket::stream_base::decorator(
			[](websocket::request_type& req)
		{
			req.set(http::field::authorization, " ssl-websocket-client-coro");
		}));
	}).bind_recv([&](std::string_view data)
	{
		printf("recv : %zu %.*s\n", data.size(), (int)data.size(), data.data());

		client.async_send(data);
	}).bind_connect([&]()
	{
		if (asio2::get_last_error())
			printf("connect failure : %d %s\n",
				asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
			printf("connect success : %s %u\n",
				client.local_address().c_str(), client.local_port());

		// a new thread.....
		std::thread([&]()
		{
			// inner this new thread, we post a task, the task must be executed
			// in the client's io_context thread, not in this new thread.
			client.post([&]()
			{
				ASIO2_ASSERT(client.io().strand().running_in_this_thread());

				std::string str;
				str += '<';
				int len = 128 + std::rand() % (300);
				for (int i = 0; i < len; i++)
				{
					str += (char)((std::rand() % 26) + 'a');
				}
				str += '>';

				client.async_send(std::move(str));
			});
		}).join();
	}).bind_upgrade([&]()
	{
		if (asio2::get_last_error())
			printf("upgrade failure : %d %s\n",
				asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
			printf("upgrade success : %s %u\n",
				client.local_address().c_str(), client.local_port());
	}).bind_disconnect([]()
	{
		printf("disconnect : %d %s\n",
			asio2::last_error_val(), asio2::last_error_msg().c_str());
	});

	// the /ws is the websocket upgraged target
	client.async_start(host, port, "/ws");

	while (std::getchar() != '\n');

	return 0;
}

