#pragma once

#include <asio2/asio2.hpp>

void run_httpws_server(std::string_view host, std::string_view port)
{
#ifndef ASIO_STANDALONE

	asio2::httpws_server server;

	server.bind_recv([](auto & session_ptr, http::request<http::string_body>& req, std::string_view s)
	{
		if (session_ptr->is_http())
		{
			std::cout << req << std::endl << std::endl;
			//bool flag = session_ptr->send(http::make_response(200, "http:result:success"), [](std::size_t bytes_sent) {});
			session_ptr->send(http::make_response(200, "http:result:success"), asio::use_future);
		}
		else
		{
			printf("recv : %u %.*s\n", (unsigned)s.size(), (int)s.size(), (const char*)s.data());
			session_ptr->send(s, [](std::size_t bytes_sent) {});
			//session_ptr->send(s, asio::use_future);
		}
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
		printf("client upgrade : %s %u %d %s\n", session_ptr->remote_address().c_str(),
			session_ptr->remote_port(), ec.value(), ec.message().c_str());
	}).bind_start([&server](asio::error_code ec)
	{
		printf("start http websocket server: %s %u %d %s\n", server.listen_address().c_str(), server.listen_port(),
			ec.value(), ec.message().c_str());
	}).bind_stop([&server](asio::error_code ec)
	{
		printf("stop : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	});
	server.start(host, port);
	while (std::getchar() != '\n');
	server.stop();
#endif
}

void run_httpwss_server(std::string_view host, std::string_view port)
{
#if !defined(ASIO_STANDALONE) && defined(ASIO2_USE_SSL)

	asio2::httpwss_server server;
	server.set_cert_file("test", "server.crt", "server.key", "dh512.pem");

	server.bind_recv([](auto & session_ptr, http::request<http::string_body>& req, std::string_view s)
	{
		if (session_ptr->is_http())
		{
			std::cout << req << std::endl << std::endl;
			//bool flag = session_ptr->send(http::make_response(200, "http:result:success"), [](std::size_t bytes_sent) {});
			session_ptr->send(http::make_response(200, "http:result:success"), asio::use_future);
		}
		else
		{
			printf("recv : %u %.*s\n", (unsigned)s.size(), (int)s.size(), (const char*)s.data());
			session_ptr->send(s, [](std::size_t bytes_sent) {});
			//session_ptr->send(s, asio::use_future);
		}
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
		printf("client upgrade : %s %u %d %s\n", session_ptr->remote_address().c_str(),
			session_ptr->remote_port(), ec.value(), ec.message().c_str());
	}).bind_start([&server](asio::error_code ec)
	{
		printf("start http websocket ssl server : %s %u %d %s\n", server.listen_address().c_str(), server.listen_port(),
			ec.value(), ec.message().c_str());
	}).bind_stop([&server](asio::error_code ec)
	{
		printf("stop : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	});
	server.start(host, port);
	while (std::getchar() != '\n');
#endif
}
