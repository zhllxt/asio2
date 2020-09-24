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
	std::string_view port = "8080";
	//std::string_view host = "www.baidu.com";
	//std::string_view port = "80";

	asio2::error_code ec;

	auto req1 = http::make_request("http://www.baidu.com/get_user?name=abc");
	auto rep1 = asio2::http_client::execute("http://www.baidu.com/get_user?name=abc", ec);
	if (ec)
		std::cout << ec.message() << std::endl;
	else
		std::cout << rep1 << std::endl;


	auto req2 = http::make_request("GET / HTTP/1.1\r\nHost: 192.168.0.1\r\n\r\n");
	auto rep2 = asio2::http_client::execute("www.baidu.com", "80", req2, std::chrono::seconds(3), ec);
	if (ec)
		std::cout << ec.message() << std::endl;
	else
		std::cout << rep2 << std::endl;


	auto path = asio2::http::url_to_path("/get_user?name=abc");
	std::cout << path << std::endl;

	auto query = asio2::http::url_to_query("/get_user?name=abc");
	std::cout << query << std::endl;

	std::cout << std::endl;

	auto rep3 = asio2::http_client::execute("www.baidu.com", "80", "/api/get_user?name=abc", ec);
	if (ec)
		std::cout << ec.message() << std::endl;
	else
		std::cout << rep3 << std::endl;

	std::string en = http::url_encode(R"(http://www.baidu.com/json={"qeury":"name like '%abc%'","id":1})");
	std::cout << en << std::endl;
	std::string de = http::url_decode(en);
	std::cout << de << std::endl;

	asio2::http_client client;

	int times = 0;

	client.bind_recv([&](http::request& req, http::response& rep)
	{
		std::cout << "----------------------------------------" << std::endl;
		// print the whole response
		std::cout << rep << std::endl;
		// print the response body
		std::cout << rep.body() << std::endl;

		// convert the response body to string
		std::stringstream ss;
		ss << rep.body();
		std::cout << ss.str() << std::endl;

		times++;

		if /**/ (times == 1)
		{
			const char * msg = "GET / HTTP/1.1\r\n\r\n";
			asio::const_buffer buffer = asio::buffer(std::string_view{ msg });
			// "send" use asio::buffer to avoid memory allocation, the underlying
			// buffer must be persistent, like the static pointer "msg" above
			client.send(buffer);
		}
		else if (times == 2)
		{
			http::request req;
			req.method(http::verb::get);
			req.keep_alive(true);
			req.target("/get_user?name=abc");
			client.send(std::move(req), []()
			{
				printf("sent result : %d %s\n",
					asio2::last_error_val(), asio2::last_error_msg().c_str());
			});
		}

	}).bind_connect([&](asio::error_code ec)
	{
		if (asio2::get_last_error())
			printf("connect failure : %d %s\n",
				asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
			printf("connect success : %s %u\n",
				client.local_address().c_str(), client.local_port());

		// connect success, send a request.
		if (!asio2::get_last_error())
		{
			const char * msg = "GET / HTTP/1.1\r\n\r\n";
			client.send(msg, [](std::size_t bytes_sent) {});
		}

	}).bind_disconnect([](asio::error_code ec)
	{
		printf("disconnect : %d %s\n",
			asio2::last_error_val(), asio2::last_error_msg().c_str());
	});

	client.start(host, port);

	while (std::getchar() != '\n');

	return 0;
}
