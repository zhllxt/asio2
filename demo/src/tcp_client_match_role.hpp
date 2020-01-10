#pragma once

#include <asio2/asio2.hpp>

using iterator = asio::buffers_iterator<asio::streambuf::const_buffers_type>;
std::pair<iterator, bool> match_role(iterator begin, iterator end)
{
	iterator i = begin;
	while (i != end)
	{
		if (*i != '#')
			return std::pair(begin, true); // head character is not #, return and kill the client

		i++;
		if (i == end) break;

		int length = std::uint8_t(*i); // get content length

		i++;
		if (i == end) break;

		if (end - i >= length)
			return std::pair(i + length, true);

		break;
	}
	return std::pair(begin, false);
}

void run_tcp_client_match_role(std::string_view host, std::string_view port)
{
	//while (1) // use infinite loop and sleep 2 seconds to test start and stop
	{
		//listener lis;
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
				// demo of force a packet of data to be sent twice
				client.send(s.substr(0, s.size() / 2), []() {});
				//std::this_thread::sleep_for(std::chrono::milliseconds(10));
				client.send(s.substr(s.size() / 2), [](std::size_t bytes_sent) {});
			});
			client.async_start(host, port, match_role);
		}

		while (std::getchar() != '\n');
		//std::this_thread::sleep_for(std::chrono::milliseconds(100));

		for (int i = 0; i < count; ++i)
		{
			auto & client = clients[i];
			client.stop();
		}
	}
}
