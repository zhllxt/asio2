#include <asio2/tcp/tcp_server.hpp>

void on_recv(std::shared_ptr<asio2::tcp_session>& session_ptr, std::string_view data)
{
	printf("recv : %zu %.*s\n", data.size(), (int)data.size(), data.data());

	session_ptr->async_send(data);
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
	printf("start tcp server dgram : %s %u %d %s\n",
		server.listen_address().c_str(), server.listen_port(),
		asio2::last_error_val(), asio2::last_error_msg().c_str());
}

void on_stop(asio2::tcp_server& server)
{
	printf("stop tcp server dgram : %s %u %d %s\n",
		server.listen_address().c_str(), server.listen_port(),
		asio2::last_error_val(), asio2::last_error_msg().c_str());
}

int main()
{
	std::string_view host = "0.0.0.0";
	std::string_view port = "8027";

	// Specify the "max recv buffer size" to avoid malicious packets, if some client
	// sent data packets size is too long to the "max recv buffer size", then the
	// client will be disconnect automatic .
	asio2::tcp_server server(
		512,  // the initialize recv buffer size : 
		1024, // the max recv buffer size :
		4     // the thread count : 
	);

	// bind global function
	server
		.bind_recv      (on_recv) // use global function
		.bind_connect   (on_connect)
		.bind_disconnect(on_disconnect)
		.bind_start     (std::bind(on_start, std::ref(server))) //     use std::bind
		.bind_stop      (          on_stop , std::ref(server)); // not use std::bind

	server.start(host, port, asio2::use_dgram); // dgram tcp

	while (std::getchar() != '\n');

	server.stop();

	return 0;
}
