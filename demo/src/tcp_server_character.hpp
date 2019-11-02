#pragma once

#include <asio2/asio2.hpp>

void run_tcp_server_character(std::string_view host, std::string_view port)
{
	asio2::tcp_server server;
	//while (1) // use infinite loop and sleep 2 seconds to test start and stop
	{
		printf("\n");
		server.start_timer(1, std::chrono::seconds(1), []() {});
		server.bind_recv([&server](auto & session_ptr, std::string_view s)
		{
			printf("recv : %u %.*s\n", (unsigned)s.size(), (int)s.size(), s.data());

			// ##Thread-safe send operation example:
			//session_ptr->post([session_ptr]()
			//{
			//	asio::write(session_ptr->stream(), asio::buffer(std::string("abcdefghijklmn")));
			//});

			// ##Use this to check whether the send operation is running in current thread.
			//if (session_ptr->io().strand().running_in_this_thread())
			//{
			//}

			// force one packet data to be sent twice, and the client will recvd compeleted packet
			session_ptr->send(s.substr(0, s.size() / 2), []() {});
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			session_ptr->send(s.substr(s.size() / 2), [](std::size_t bytes_sent) {});

		}).bind_connect([&server](auto & session_ptr)
		{
			session_ptr->no_delay(true);
			//session_ptr->stop(); // You can close the connection directly here.
			printf("client enter : %s %u %s %u\n",
				session_ptr->remote_address().c_str(), session_ptr->remote_port(),
				session_ptr->local_address().c_str(), session_ptr->local_port());
		}).bind_disconnect([&server](auto & session_ptr)
		{
			printf("client leave : %s %u %s\n",
				session_ptr->remote_address().c_str(),
				session_ptr->remote_port(), asio2::last_error_msg().c_str());
		}).bind_start([&server](asio::error_code ec)
		{
			printf("start tcp server character : %s %u %d %s\n", server.listen_address().c_str(), server.listen_port(),
				ec.value(), ec.message().c_str());
		}).bind_stop([&server](asio::error_code ec)
		{
			printf("stop : %d %s\n", ec.value(), ec.message().c_str());
		});
		server.start(host, port, '>');
		//server.start(host, port, "\r\n");

		while (std::getchar() != '\n');
		//std::this_thread::sleep_for(std::chrono::seconds(1));

		server.stop();
	}
}
