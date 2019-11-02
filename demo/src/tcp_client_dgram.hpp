#pragma once

#include <asio2/asio2.hpp>

void run_tcp_client_dgram(std::string_view host, std::string_view port)
{
	//while (1) // use infinite loop and sleep 2 seconds to test start and stop
	{
		int count = 1;
		std::unique_ptr<asio2::tcp_client[]> clients = std::make_unique<asio2::tcp_client[]>(count);
		for (int i = 0; i < count; ++i)
		{
			auto & client = clients[i];
			client.start_timer(1, std::chrono::seconds(1), []() {});
			client.bind_connect([&](asio::error_code ec)
			{
				if (asio2::get_last_error())
					printf("connect failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
				else
					printf("connect success : %s %u\n", client.local_address().c_str(), client.local_port());

				std::string s;
				s += '#';
				s += char(1);
				s += 'a';

				client.send(s);

			}).bind_disconnect([](asio::error_code ec)
			{
				printf("disconnect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
			}).bind_recv([&](std::string_view sv)
			{
				printf("recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());

				std::string s;
				s += '#';
				uint8_t len = uint8_t(100 + (std::rand() % 100));
				s += char(len);
				for (uint8_t i = 0; i < len; i++)
				{
					s += (char)((std::rand() % 26) + 'a');
				}
				client.send(std::move(s));

			});
			client.start(host, port, asio2::use_dgram);
		}

		while (std::getchar() != '\n');
		//std::this_thread::sleep_for(std::chrono::seconds(2));

		for (int i = 0; i < count; ++i)
		{
			auto & client = clients[i];
			client.stop();
		}
	}
}
