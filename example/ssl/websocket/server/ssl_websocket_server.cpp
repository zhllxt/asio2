#ifndef ASIO2_USE_SSL
#define ASIO2_USE_SSL
#endif

#include <asio2/http/wss_server.hpp>

int main()
{
	std::string_view host = "0.0.0.0";
	std::string_view port = "8007";

	asio2::wss_server server;

	// use verify_fail_if_no_peer_cert, the client must specified a cert
	server.set_verify_mode(asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert);

	server.set_cert_file(
		"../../cert/ca.crt",
		"../../cert/server.crt",
		"../../cert/server.key",
		"123456");
	server.set_dh_file("../../cert/dh1024.pem");

	server.bind_accept([](std::shared_ptr<asio2::wss_session>& session_ptr)
	{
		// how to set custom websocket response data : 
		session_ptr->ws_stream().set_option(websocket::stream_base::decorator(
			[](websocket::response_type& rep)
		{
			rep.set(http::field::authorization, " ssl-websocket-server-coro");
		}));

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
