#include <asio2/http/ws_server.hpp>

int main()
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	std::string_view host = "0.0.0.0";
	std::string_view port = "8039";

	std::srand((unsigned int)time(nullptr));

	for(;;)
	{
		asio2::ws_server server;

		server.bind_accept([&server](std::shared_ptr<asio2::ws_session>& session_ptr)
		{
			// how to set custom websocket response data : 
			session_ptr->ws_stream().set_option(websocket::stream_base::decorator(
				[](websocket::response_type& rep)
			{
				rep.set(http::field::authorization, " websocket-server-coro");
			}));

			server.post_event([]()
			{
				//printf("def\n");
			});

			session_ptr->post_event([]()
			{
				//printf("xyz\n");
			});

		}).bind_recv([&server](auto & session_ptr, std::string_view s)
		{
			asio2::detail::ignore_unused(server, session_ptr);

			//printf("recv : %u %.*s\n", (unsigned)s.size(), (int)s.size(), s.data());

			session_ptr->async_send(std::string(s), [](std::size_t) {});

		}).bind_connect([](auto & session_ptr)
		{
			printf("client enter : %s %u %s %u\n",
				session_ptr->remote_address().c_str(), session_ptr->remote_port(),
				session_ptr->local_address().c_str(), session_ptr->local_port());
		}).bind_disconnect([](auto & session_ptr)
		{
			printf("client leave : %s %u %s\n", session_ptr->remote_address().c_str(),
				session_ptr->remote_port(), asio2::last_error_msg().c_str());
		}).bind_upgrade([](auto & session_ptr, asio::error_code ec)
		{
			asio2::detail::ignore_unused(session_ptr);

			printf(">> upgrade %d %s\n", ec.value(), ec.message().c_str());
		}).bind_start([&](asio::error_code ec)
		{
			printf("start websocket server : %s %u %d %s\n",
				server.listen_address().c_str(), server.listen_port(),
				ec.value(), ec.message().c_str());
		}).bind_stop([&](asio::error_code ec)
		{
			printf("stop : %d %s\n", ec.value(), ec.message().c_str());
		});

		server.post_event([]()
		{ 
			//printf("abc\n");
		});

		server.start(host, port);

		server.post_event([]()
		{ 
			//printf("abc\n");
		});

		server.start(host, port);

		auto sec = 1 + std::rand() % 2;

		printf("enter %d\n", sec);
		std::this_thread::sleep_for(std::chrono::seconds(sec));
		printf("leave %d\n", sec);

		server.start(host, port);

		server.stop();
	}

	while (std::getchar() != '\n');

	return 0;
}
