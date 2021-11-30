#include <asio2/http/ws_client.hpp>
#include <iostream>

int main()
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	std::string_view host = "127.0.0.1";
	std::string_view port = "8039";

	std::srand((unsigned int)time(nullptr));

	//for (;;)
	while(!asio2::detail::has_unexpected_behavior())
	{
		asio2::ws_client * clients = new asio2::ws_client[1];

		for (int i = 0; i < 1; i++)
		{
			auto & client = clients[i];

			client.upgrade_target("/ws");

			client.connect_timeout(std::chrono::seconds(10));

			client.bind_init([&]()
			{
				client.ws_stream().set_option(websocket::stream_base::decorator(
					[](websocket::request_type& req)
				{
					req.set(http::field::authorization, " websocket-client-authorization");
				}));

			}).bind_connect([&](asio::error_code ec)
			{
				if (ec)
					printf("connect failure : %d %s\n", ec.value(), ec.message().c_str());
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

				client.async_send(std::move(s), [](std::size_t bytes_sent) {std::ignore = bytes_sent; });

			}).bind_upgrade([&](asio::error_code ec)
			{
				std::ignore = ec;
				//if (ec)
				//	std::cout << "upgrade failure : " << ec.value() << " " << ec.message() << std::endl;
				//else
				//	std::cout << "upgrade success : " << client.upgrade_response() << std::endl;

				// this send will be failed, because connection is not fully completed
				client.async_send("abc", []()
				{
					ASIO2_ASSERT(asio2::get_last_error());
					std::cout << "send failed : " << asio2::last_error_msg() << std::endl;
				});

			}).bind_recv([&](std::string_view sv)
			{
				//printf("recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());

				client.async_send(sv, []() {});
			});

			asio2::rdc::option rdc_option{ [](std::string_view) { return 0; } };

			asio2::socks5::option<asio2::socks5::method::anonymous> sock5_option
			{
				"127.0.0.1",
				10808
			};

			if (!client.start(host, port))
			{
				printf("start failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
			}

			if (!client.start(host, port, "/user"))
			{
				printf("start failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
			}

			if (!client.start(host, port, "/user", std::move(rdc_option), std::move(sock5_option)))
			{
				printf("start failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
			}
		}

		std::this_thread::sleep_for(std::chrono::seconds(1 + std::rand() % 2));

		delete[]clients;

		ASIO2_LOG(spdlog::level::debug, "<-------------------------------------------->");
	}

	while (std::getchar() != '\n');

	//client.stop();

	return 0;
}
