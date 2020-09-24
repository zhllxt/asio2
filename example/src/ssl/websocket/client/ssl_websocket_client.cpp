// When compiling with vs under linux, you need to copy the "asio,beast,ceral,fmt" folders to
// the /usr/local/include directory first, and copy the "libcrypto.a,libssl.a" files to 
// /usr/local/lib directory first. "libcrypto.a,libssl.a" is in "asio2/lib/x64".

#ifndef ASIO2_USE_SSL
#define ASIO2_USE_SSL
#endif

#include <asio2/asio2.hpp>

int main()
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	std::string_view host = "127.0.0.1";
	std::string_view port = "8007";

	//bool loop = false;
	bool loop = true;
	while (loop) // use infinite loop and sleep 2 seconds to test start and stop
	{
		asio2::wss_client client;

		client.connect_timeout(std::chrono::seconds(10));

		//client.upgrade_target("/ws");

		client.set_verify_mode(asio::ssl::verify_peer);
		client.set_cert_file("../../../cert/ca.crt", "../../../cert/client.crt", "../../../cert/client.key", "client");

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
			printf("recv : %u %.*s\n", (unsigned)data.size(), (int)data.size(), data.data());

			client.send(data);

		}).bind_connect([&](asio::error_code ec)
		{
			if (asio2::get_last_error())
				printf("connect failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
			else
				printf("connect success : %s %u\n", client.local_address().c_str(), client.local_port());

			// a new thread.....
			std::thread([&]()
			{
				// inner this new thread, we post a task, the task must be executed
				// in the client's io_context thread, not in this new thread.
				client.post([&]()
				{
					std::string s;
					s += '<';
					int len = 128 + std::rand() % (300);
					for (int i = 0; i < len; i++)
					{
						s += (char)((std::rand() % 26) + 'a');
					}
					s += '>';

					client.send(std::move(s));
				});
			}).join();

		}).bind_upgrade([&](asio::error_code ec)
		{
			if (ec)
				std::cout << "upgrade failure : " << ec.value() << " " << ec.message() << std::endl;
			else
				std::cout << "upgrade success : " << client.upgrade_response() << std::endl;
		});

		client.async_start(host, port);

		if (!loop)
			while (std::getchar() != '\n');
		else
			std::this_thread::sleep_for(std::chrono::seconds(2));
	}

	return 0;
}

