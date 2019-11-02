#pragma once

#include <asio2/asio2.hpp>

void run_ws_server(std::string_view host, std::string_view port)
{
#ifndef ASIO_STANDALONE

	asio2::ws_server server;

	server.bind_recv([](auto & session_ptr, std::string_view s)
	{
		printf("recv : %u %.*s\n", (unsigned)s.size(), (int)s.size(), s.data());
		session_ptr->send(std::string(s), [](std::size_t bytes_sent) {});
	}).bind_connect([](auto & session_ptr)
	{
		printf("client enter : %s %u %s %u\n", session_ptr->remote_address().c_str(), session_ptr->remote_port(),
			session_ptr->local_address().c_str(), session_ptr->local_port());
	}).bind_disconnect([](auto & session_ptr)
	{
		printf("client leave : %s %u %s\n", session_ptr->remote_address().c_str(),
			session_ptr->remote_port(), asio2::last_error_msg().c_str());
	}).bind_upgrade([](auto & session_ptr, asio::error_code ec)
	{
		printf(">> upgrade %d %s\n", ec.value(), ec.message().c_str());
	}).bind_start([&](asio::error_code ec)
	{
		printf("start websocket server : %s %u %d %s\n", server.listen_address().c_str(), server.listen_port(),
			ec.value(), ec.message().c_str());
	}).bind_stop([&](asio::error_code ec)
	{
		printf("stop : %d %s\n", ec.value(), ec.message().c_str());
	});

	server.start(host, port);
	while (std::getchar() != '\n');
#endif
}


void run_wss_server(std::string_view host, std::string_view port)
{
#if !defined(ASIO_STANDALONE) && defined(ASIO2_USE_SSL)
	//while (1)
	{
		asio2::wss_server server;
		server.set_cert_file("test", "server.crt", "server.key", "dh512.pem");
		server.bind_recv([](std::shared_ptr<asio2::wss_session> & session_ptr, std::string_view s)
		{
			printf("recv : %u %.*s\n", (unsigned)s.size(), (int)s.size(), s.data());
			session_ptr->send(s, []() {});
		}).bind_connect([](auto & session_ptr)
		{
			printf("client enter : %s %u %s %u\n", session_ptr->remote_address().c_str(), session_ptr->remote_port(),
				session_ptr->local_address().c_str(), session_ptr->local_port());
		}).bind_disconnect([](auto & session_ptr)
		{
			printf("client leave : %s %u %s\n", session_ptr->remote_address().c_str(),
				session_ptr->remote_port(), asio2::last_error_msg().c_str());
		}).bind_handshake([](auto & session_ptr, asio::error_code ec)
		{
			printf("client handshake : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		}).bind_upgrade([](auto & session_ptr, asio::error_code ec)
		{
			printf("client upgrade : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		}).bind_start([&server](asio::error_code ec)
		{
			if (asio2::get_last_error())
				printf("start websocket ssl server failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
			else
				printf("start websocket ssl server success : %s %u\n", server.listen_address().c_str(), server.listen_port());
		}).bind_stop([&server](asio::error_code ec)
		{
			printf("stop : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		});
		server.start(host, port);
		while (std::getchar() != '\n');
		//std::this_thread::sleep_for(std::chrono::seconds(2));
	}
#endif // ASIO2_USE_SSL
}
