#include <asio2/tcp/tcp_server.hpp>

int main()
{
	std::string_view host = "0.0.0.0";
	std::string_view port = "8028";

	asio2::tcp_server server;

	server.bind_recv([&](std::shared_ptr<asio2::tcp_session>& session_ptr, std::string_view data)
	{
		printf("recv : %zu %.*s\n", data.size(), (int)data.size(), data.data());

		session_ptr->async_send(data);

	}).bind_accept([](std::shared_ptr<asio2::tcp_session>& session_ptr)
	{
		// accept callback maybe has error like "Too many open files", etc...
		if (!asio2::get_last_error())
		{
			// eg: If the remote ip is in the blacklist, we close the connection with RST.
			if (session_ptr->remote_address() == "192.168.0.250")
				session_ptr->stop();
			else
				session_ptr->set_silence_timeout(std::chrono::seconds(60));
		}
		else
		{
			printf("error occurred when calling the accept function : %d %s\n",
				asio2::get_last_error_val(), asio2::get_last_error_msg().data());
		}
	}).bind_connect([&](std::shared_ptr<asio2::tcp_session>& session_ptr)
	{
		session_ptr->no_delay(true);

		// Or you can close the connection here.
		// Of course, you can also close the connection anywhere.
		if (session_ptr->remote_address() == "192.168.0.254")
			session_ptr->stop();

		printf("client enter : %s %u %s %u\n",
			session_ptr->remote_address().c_str(), session_ptr->remote_port(),
			session_ptr->local_address().c_str(), session_ptr->local_port());

	}).bind_disconnect([&](std::shared_ptr<asio2::tcp_session>& session_ptr)
	{
		printf("client leave : %s %u %s\n",
			session_ptr->remote_address().c_str(), session_ptr->remote_port(),
			asio2::last_error_msg().c_str());
	}).bind_start([&]()
	{
		printf("start tcp server : %s %u %d %s\n",
			server.listen_address().c_str(), server.listen_port(),
			asio2::last_error_val(), asio2::last_error_msg().c_str());
	}).bind_stop([&]()
	{
		printf("stop tcp server : %d %s\n",
			asio2::last_error_val(), asio2::last_error_msg().c_str());
	});

	server.start(host, port);

	while (std::getchar() != '\n');  // press enter to exit this program

	server.stop();

	return 0;
}
