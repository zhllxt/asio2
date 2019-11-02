#pragma once

#include <asio2/asio2.hpp>

void run_tcp_client_character(std::string_view host, std::string_view port)
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
			s += '<';
			int len = 128 + std::rand() % (300);
			for (int i = 0; i < len; i++)
			{
				s += (char)((std::rand() % 26) + 'a');
			}
			s += '>';
			//s += "\r\n";

			client.send(s);

		}).bind_disconnect([](asio::error_code ec)
		{
			printf("disconnect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		}).bind_recv([&](std::string_view sv)
		{
			printf("recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());

			std::string s;
			s += '<';
			int len = 128 + std::rand() % (300);
			for (int i = 0; i < len; i++)
			{
				s += (char)((std::rand() % 26) + 'a');
			}
			s += '>';
			//s += "\r\n";

			// demo of force a packet of data to be sent twice 
			client.send(s.substr(0, s.size() / 2), []() {});
			//std::this_thread::sleep_for(std::chrono::milliseconds(10));
			client.send(s.substr(s.size() / 2), [](std::size_t bytes_sent) {});
		});
		client.start(host, port, '>');
		//client.async_start(host, port, "\r\n");
	}

	while (std::getchar() != '\n');
	//std::this_thread::sleep_for(std::chrono::seconds(2));

	for (int i = 0; i < count; ++i)
	{
		auto & client = clients[i];
		client.stop();
	}
}
