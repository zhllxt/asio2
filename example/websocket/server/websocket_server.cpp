#include <asio2/http/ws_server.hpp>

int main()
{
	std::string_view host = "0.0.0.0";
	std::string_view port = "8039";

	asio2::ws_server server;

	server.bind_accept([&](std::shared_ptr<asio2::ws_session>& session_ptr)
	{
		// Set the binary message write option.
		session_ptr->ws_stream().binary(true);

		// Set the text message write option.
		//session_ptr->ws_stream().text(true);

		// how to set custom websocket response data : 
		session_ptr->ws_stream().set_option(websocket::stream_base::decorator(
			[](websocket::response_type& rep)
		{
			rep.set(http::field::authorization, " websocket-server-coro");
		}));

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

	while (std::getchar() != '\n');

	return 0;
}
