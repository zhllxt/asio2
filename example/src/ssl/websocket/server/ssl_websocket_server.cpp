// When compiling with vs under linux, you need to copy the "asio,beast,ceral,fmt" folders to
// the /usr/local/include directory first, and copy the "libcrypto.a,libssl.a" files to 
// /usr/local/lib directory first. "libcrypto.a,libssl.a" is in "asio2/lib/x64".

#ifndef ASIO2_USE_SSL
#define ASIO2_USE_SSL
#endif

#include <asio2/asio2.hpp>

int main()
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	std::string_view host = "0.0.0.0";
	std::string_view port = "8007";

	//bool loop = false;
	bool loop = true;
	while (loop) // use infinite loop and sleep 2 seconds to test start and stop
	{
		asio2::wss_server server;

		// use verify_fail_if_no_peer_cert, the client must specified a cert
		server.set_verify_mode(asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert);

		//server.set_cert_buffer(ca_crt, server_crt, server_key, "server"); // use memory string for cert
		//server.set_dh_buffer(dh);

		server.set_cert_file("../../../cert/ca.crt", "../../../cert/server.crt", "../../../cert/server.key", "server");
		server.set_dh_file("../../../cert/dh1024.pem");

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
			printf("recv : %u %.*s\n", (unsigned)data.size(), (int)data.size(), data.data());

			session_ptr->send(data, []() {});

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

		}).bind_start([&](asio::error_code ec)
		{
			if (asio2::get_last_error())
				printf("start websocket ssl server failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
			else
				printf("start websocket ssl server success : %s %u\n", server.listen_address().c_str(), server.listen_port());
		}).bind_stop([&](asio::error_code ec)
		{
			printf("stop : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		});

		server.start(host, port);

		if (!loop)
			while (std::getchar() != '\n');
		else
			std::this_thread::sleep_for(std::chrono::seconds(2));
	}

	return 0;
}
