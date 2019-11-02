#pragma once

#include <asio2/asio2.hpp>

void run_serial_port(const std::string& device, unsigned int baud_rate)
{
	//while (1) // use infinite loop and sleep 2 seconds to test start and stop
	{
		asio2::scp sp;
		sp.start_timer(1, std::chrono::seconds(1), []() {});
		sp.bind_init([&]()
		{
			// Set other serial port parameters at here
			sp.socket().set_option(asio::serial_port::flow_control(asio::serial_port::flow_control::type::none));
			sp.socket().set_option(asio::serial_port::parity(asio::serial_port::parity::type::none));
			sp.socket().set_option(asio::serial_port::stop_bits(asio::serial_port::stop_bits::type::one));
			sp.socket().set_option(asio::serial_port::character_size(8));

		}).bind_recv([&](std::string_view sv)
		{
			printf("recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());

			std::string s;
			uint8_t len = uint8_t(10 + (std::rand() % 20));
			s += '<';
			for (uint8_t i = 0; i < len; i++)
			{
				s += (char)((std::rand() % 26) + 'a');
			}
			s += '>';
			sp.send(s, []() {});

		}).bind_start([&](asio::error_code ec)
		{
			printf("start : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		}).bind_stop([&](asio::error_code ec)
		{
			printf("stop : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		});
		//sp.start(device, baud_rate);
		sp.start(device, baud_rate, '>');
		//sp.start(device, baud_rate, "\r\n");
		//sp.start(device, baud_rate, match_role);
		//sp.start(device, baud_rate, asio::transfer_at_least(1));
		//sp.start(device, baud_rate, asio::transfer_exactly(10));
		sp.send("abc", [](std::size_t bytes_sent) {
			printf("%u %d %s\n", (unsigned)bytes_sent, asio2::last_error_val(), asio2::last_error_msg().c_str()); });

		while (std::getchar() != '\n');
		//std::this_thread::sleep_for(std::chrono::seconds(2));

	}
}
