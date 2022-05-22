#include <asio2/tcp/tcp_server.hpp>

class svr_listener
{
public:
	void on_recv(std::shared_ptr<asio2::tcp_session>& session_ptr, std::string_view data)
	{
		printf("recv : %zu %.*s\n", data.size(), (int)data.size(), data.data());

		// this is just a demo to show :
		// even if we force one packet data to be sent twice,
		// but the client must recvd whole packet once
		session_ptr->async_send(data.substr(0, data.size() / 2));
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		session_ptr->async_send(data.substr(data.size() / 2));
	}

	void on_connect(std::shared_ptr<asio2::tcp_session>& session_ptr)
	{
		session_ptr->no_delay(true);

		printf("client enter : %s %u %s %u\n",
			session_ptr->remote_address().c_str(), session_ptr->remote_port(),
			session_ptr->local_address().c_str(), session_ptr->local_port());
	}

	void on_disconnect(std::shared_ptr<asio2::tcp_session>& session_ptr)
	{
		printf("client leave : %s %u %s\n",
			session_ptr->remote_address().c_str(), session_ptr->remote_port(),
			asio2::last_error_msg().c_str());
	}

	void on_start(asio2::tcp_server& server)
	{
		printf("start tcp server character : %s %u %d %s\n",
			server.listen_address().c_str(), server.listen_port(),
			asio2::last_error_val(), asio2::last_error_msg().c_str());
	}

	void on_stop(asio2::tcp_server& server)
	{
		printf("stop tcp server character : %s %u %d %s\n",
			server.listen_address().c_str(), server.listen_port(),
			asio2::last_error_val(), asio2::last_error_msg().c_str());
	}
};

int main()
{
	std::string_view host = "0.0.0.0";
	std::string_view port = "8025";

	asio2::tcp_server server;

	svr_listener listener;

	// bind member function
	server
		.bind_recv      (&svr_listener::on_recv      ,  listener) // by reference
		.bind_connect   (&svr_listener::on_connect   , &listener) // by pointer
		.bind_disconnect(&svr_listener::on_disconnect, &listener)
		.bind_start     (std::bind(&svr_listener::on_start, &listener, std::ref(server))) //     use std::bind
		.bind_stop      (          &svr_listener::on_stop ,  listener, std::ref(server)); // not use std::bind

	// Split data with a single character
	//server.start(host, port, '\n');

	// Split data with string
	server.start(host, port, "\r\n");

	while (std::getchar() != '\n');

	return 0;
}
