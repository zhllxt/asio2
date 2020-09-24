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

	std::string_view host = "127.0.0.1";
	std::string_view port = "8039";

	asio2::ws_client client;
	
	client.upgrade_target("/ws");

	client.connect_timeout(std::chrono::seconds(10));

	client.bind_init([&]()
	{
		client.ws_stream().set_option(websocket::stream_base::decorator(
			[](websocket::request_type& req)
		{
			req.set(http::field::authorization," websocket-client-authorization");
		}));

	}).bind_connect([&](asio::error_code ec)
	{
		if (asio2::get_last_error())
			printf("connect failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
			printf("connect success : %s %u\n", client.local_address().c_str(), client.local_port());

		std::string s;
		s += '<';
		int len = 128 + std::rand() % 512;
		for (int i = 0; i < len; i++)
		{
			s += (char)((std::rand() % 26) + 'a');
		}
		s += '>';

		client.send(std::move(s), [](std::size_t bytes_sent) {});

	}).bind_upgrade([&](asio::error_code ec)
	{
		if (ec)
			std::cout << "upgrade failure : " << ec.value() << " " << ec.message() << std::endl;
		else
			std::cout << "upgrade success : " << client.upgrade_response() << std::endl;

		// this send will be failed, because connection is not fully completed
		if (!client.send("abc"))
		{
			std::cout << "send failed : " << asio2::last_error_msg() << std::endl;
		}

	}).bind_recv([&](std::string_view sv)
	{
		printf("recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());

		client.send(sv, []() {});
	});

	if (!client.start(host, port))
	{
		printf("start failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	}

	while (std::getchar() != '\n');

	client.stop();

	return 0;
}
