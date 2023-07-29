#ifndef ASIO2_ENABLE_SSL
#define ASIO2_ENABLE_SSL
#endif

#include <asio2/websocket/wss_server.hpp>
#include <iostream>

int main()
{
	std::string_view host = "0.0.0.0";
	std::string_view port = "8007";

	asio2::wss_server server;

	// use verify_fail_if_no_peer_cert, the client must specified a cert
	server.set_verify_mode(asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert);

	server.set_cert_file(
		"../../example/cert/ca.crt",
		"../../example/cert/server.crt",
		"../../example/cert/server.key",
		"123456");

	if (asio2::get_last_error())
		std::cout << "load cert files failed: " << asio2::last_error_msg() << std::endl;

	server.set_dh_file("../../example/cert/dh1024.pem");

	if (asio2::get_last_error())
		std::cout << "load dh files failed: " << asio2::last_error_msg() << std::endl;

	server.bind_accept([](std::shared_ptr<asio2::wss_session>& session_ptr)
	{
		// accept callback maybe has error like "Too many open files", etc...
		if (!asio2::get_last_error())
		{
			// how to set custom websocket response data : 
			// the decorator is just a callback function, when the upgrade response is send,
			// this callback will be called.
			session_ptr->ws_stream().set_option(
				websocket::stream_base::decorator([session_ptr](websocket::response_type& rep)
			{
				// @see /asio2/example/ssl/websocket/client/ssl_websocket_client.cpp
				const websocket::request_type& req = session_ptr->get_upgrade_request();
				auto it = req.find(http::field::authorization);
				if (it != req.end())
					rep.set(http::field::authentication_results, "200 OK");
				else
					rep.set(http::field::authentication_results, "401 unauthorized");
			}));
		}
		else
		{
			printf("error occurred when calling the accept function : %d %s\n",
				asio2::get_last_error_val(), asio2::get_last_error_msg().data());
		}
	}).bind_recv([](std::shared_ptr<asio2::wss_session> & session_ptr, std::string_view data)
	{
		printf("recv : %zu %.*s\n", data.size(), (int)data.size(), data.data());

		session_ptr->async_send(data);

	}).bind_connect([](auto & session_ptr)
	{
		printf("client enter : %s %u %s %u\n",
			session_ptr->remote_address().c_str(), session_ptr->remote_port(),
			session_ptr->local_address().c_str(), session_ptr->local_port());

	}).bind_disconnect([](auto & session_ptr)
	{
		asio2::ignore_unused(session_ptr);
		printf("client leave : %s\n", asio2::last_error_msg().c_str());

	}).bind_handshake([](auto & session_ptr)
	{
		printf("client handshake : %s %u %d %s\n",
			session_ptr->remote_address().c_str(), session_ptr->remote_port(),
			asio2::last_error_val(), asio2::last_error_msg().c_str());

	}).bind_upgrade([](auto & session_ptr)
	{
		printf("client upgrade : %s %u %d %s\n",
			session_ptr->remote_address().c_str(), session_ptr->remote_port(),
			asio2::last_error_val(), asio2::last_error_msg().c_str());

		// how to get the upgrade request request data : 
		// @see /asio2/example/ssl/websocket/client/ssl_websocket_client.cpp
		const websocket::request_type& req = session_ptr->get_upgrade_request();
		beast::string_view auth = req.at(http::field::authorization);
		std::cout << auth << std::endl;
		ASIO2_ASSERT(auth == "ssl-websocket-client-coro");
	}).bind_start([&]()
	{
		if (asio2::get_last_error())
			printf("start websocket ssl server failure : %d %s\n",
				asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
			printf("start websocket ssl server success : %s %u\n",
				server.listen_address().c_str(), server.listen_port());
	}).bind_stop([&]()
	{
		printf("stop websocket ssl server : %d %s\n",
			asio2::last_error_val(), asio2::last_error_msg().c_str());
	});

	server.start(host, port);

	while (std::getchar() != '\n');

	return 0;
}
