// When compiling with vs under linux, you need to copy the "asio,beast,ceral,fmt" folders to
// the /usr/local/include directory first, and copy the "libcrypto.a,libssl.a" files to 
// /usr/local/lib directory first. "libcrypto.a,libssl.a" is in "asio2/lib/x64".

#include <asio2/asio2.hpp>

int main()
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	std::string_view host = "0.0.0.0";
	std::string_view port = "8028";

	asio2::tcp_server server;

	server.start_timer(1, std::chrono::seconds(1), []() {}); // test timer
	server.bind_recv([&](std::shared_ptr<asio2::tcp_session> & session_ptr, std::string_view s)
	{
		printf("recv : %u %.*s\n", (unsigned)s.size(), (int)s.size(), s.data());

		session_ptr->send(s, [](std::size_t bytes_sent) {});

		// ##Thread-safe send operation example:
		//session_ptr->post([session_ptr]()
		//{
		//	asio::write(session_ptr->stream(), asio::buffer(std::string("abcdefghijklmn")));
		//});

		// ##Use this to check whether the send operation is running in current thread.
		//if (session_ptr->io().strand().running_in_this_thread())
		//{
		//}

	}).bind_connect([&](auto & session_ptr)
	{
		session_ptr->no_delay(true);

		session_ptr->start_timer(2, std::chrono::seconds(1), []() {}); // test timer

		//session_ptr->stop(); // You can close the connection directly here.

		printf("client enter : %s %u %s %u\n",
			session_ptr->remote_address().c_str(), session_ptr->remote_port(),
			session_ptr->local_address().c_str(), session_ptr->local_port());

	}).bind_disconnect([&](auto & session_ptr)
	{
		printf("client leave : %s %u %s\n",
			session_ptr->remote_address().c_str(),
			session_ptr->remote_port(), asio2::last_error_msg().c_str());
	}).bind_start([&](asio::error_code ec)
	{
		printf("start tcp server : %s %u %d %s\n",
			server.listen_address().c_str(), server.listen_port(),
			ec.value(), ec.message().c_str());
	}).bind_stop([&](asio::error_code ec)
	{
		printf("stop : %d %s\n", ec.value(), ec.message().c_str());
	});

	server.start(host, port);
	//server.start(host, port, asio::transfer_at_least(100));
	//server.start(host, port, asio::transfer_exactly(100));

	while (std::getchar() != '\n');  // press enter to exit this program

	server.stop();

	return 0;
}
