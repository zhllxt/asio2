#include <asio2/websocket/ws_server.hpp>
#include <iostream>

int main()
{
	std::string_view host = "0.0.0.0";
	std::string_view port = "8039";

	asio2::ws_server server;

	server.bind_accept([&](std::shared_ptr<asio2::ws_session>& session_ptr)
	{
		// accept callback maybe has error like "Too many open files", etc...
		if (!asio2::get_last_error())
		{
			// Set the binary message write option.
			session_ptr->ws_stream().binary(true);

			// Set the text message write option. The sent text must be utf8 format.
			//session_ptr->ws_stream().text(true);

			// how to set custom websocket response data : 
			// the decorator is just a callback function, when the upgrade response is send,
			// this callback will be called.
			session_ptr->ws_stream().set_option(
				websocket::stream_base::decorator([session_ptr](websocket::response_type& rep)
			{
				// @see /asio2/example/websocket/client/websocket_client.cpp
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
	}).bind_recv([&](auto & session_ptr, std::string_view data)
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

	}).bind_upgrade([](auto & session_ptr)
	{
		printf("client upgrade : %s %u %d %s\n",
			session_ptr->remote_address().c_str(), session_ptr->remote_port(),
			asio2::last_error_val(), asio2::last_error_msg().c_str());

		// how to get the upgrade request data : 
		// @see /asio2/example/websocket/client/websocket_client.cpp
		const websocket::request_type& req = session_ptr->get_upgrade_request();
		auto it = req.find(http::field::authorization);
		if (it != req.end())
		{
			beast::string_view auth = it->value();
			std::cout << auth << std::endl;
			ASIO2_ASSERT(auth == "websocket-client-authorization");
		}
		
	}).bind_start([&]()
	{
		if (asio2::get_last_error())
			printf("start websocket server failure : %s %u %d %s\n",
				server.listen_address().c_str(), server.listen_port(),
				asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
			printf("start websocket server success : %s %u\n",
				server.listen_address().c_str(), server.listen_port());
	}).bind_stop([&]()
	{
		printf("stop websocket server : %s %u %d %s\n",
			server.listen_address().c_str(), server.listen_port(),
			asio2::last_error_val(), asio2::last_error_msg().c_str());
	});

	server.start(host, port);

	// blocked forever util some signal delivered.
	// Normally, pressing Ctrl + C will emit the SIGINT signal.
	server.wait_signal(SIGINT, SIGTERM);

	return 0;
}
