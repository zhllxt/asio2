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

	//std::string_view host = "127.0.0.1";
	//std::string_view port = "8080";
	std::string_view host = "www.baidu.com";
	std::string_view port = "80";

	asio2::error_code ec;

	auto speed_rep = asio2::http_client::execute("http://speedtest-sgp1.digitalocean.com/10mb.test", std::chrono::seconds(2), ec);

	auto req1 = http::make_request("http://www.baidu.com/get_user?name=abc");

	std::cout << asio2::http_client::execute("www.baidu.com", "80",req1);
	system("clear");
	std::cout << asio2::http_client::execute("www.baidu.com", "80",req1, ec);

	system("clear");
	std::cout << asio2::http_client::execute("www.baidu.com", 80,req1);
	system("clear");
	std::cout << asio2::http_client::execute("www.baidu.com", 80,req1, ec);

	std::string shost = "www.baidu.com";
	std::string sport = "80";

	system("clear");
	std::cout << asio2::http_client::execute(shost, sport,req1);
	system("clear");
	std::cout << asio2::http_client::execute(shost, sport,req1, ec);

	system("clear");
	std::cout << asio2::http_client::execute(host, sport,req1);
	system("clear");
	std::cout << asio2::http_client::execute(host, sport,req1, ec);


	system("clear");
	std::cout << asio2::http_client::execute(shost, 80,req1);
	system("clear");
	std::cout << asio2::http_client::execute(shost, 80,req1, ec);

	system("clear");
	std::cout << asio2::http_client::execute(host, 80,req1);
	system("clear");
	std::cout << asio2::http_client::execute(host, 80,req1, ec);

	system("clear");
	std::cout << asio2::http_client::execute(shost, "80",req1);
	system("clear");
	std::cout << asio2::http_client::execute(shost, "80",req1, ec);

	system("clear");
	std::cout << asio2::http_client::execute(host, "80",req1);
	system("clear");
	std::cout << asio2::http_client::execute(host, "80",req1, ec);

	int nport = 80;

	system("clear");
	std::cout << asio2::http_client::execute(shost, nport,req1);
	system("clear");
	std::cout << asio2::http_client::execute(shost, nport,req1, ec);

	system("clear");
	std::cout << asio2::http_client::execute(host, nport,req1);
	system("clear");
	std::cout << asio2::http_client::execute(host, nport,req1, ec);

	auto rep1 = asio2::http_client::execute("http://www.baidu.com/get_user?name=abc", ec);
	if (ec)
		std::cout << ec.message() << std::endl;
	else
		std::cout << rep1 << std::endl;

	// GET
	auto req2 = http::make_request("GET / HTTP/1.1\r\nHost: 192.168.0.1\r\n\r\n");
	auto rep2 = asio2::http_client::execute("www.baidu.com", "80", req2, std::chrono::seconds(3), ec);
	if (ec)
		std::cout << ec.message() << std::endl;
	else
		std::cout << rep2 << std::endl;

	// POST
	auto req4 = http::make_request("POST / HTTP/1.1\r\nHost: 192.168.0.1\r\n\r\n");
	auto rep4 = asio2::http_client::execute("www.baidu.com", "80", req4, std::chrono::seconds(3), ec);
	if (ec)
		std::cout << ec.message() << std::endl;
	else
		std::cout << rep4 << std::endl;

	// POST
	http::request_t<http::string_body> req5(http::verb::post, "/", 11);
	auto rep5 = asio2::http_client::execute("www.baidu.com", "80", req5, ec);
	if (ec)
		std::cout << ec.message() << std::endl;
	else
		std::cout << rep5 << std::endl;

	// POST
	http::request_t<http::string_body> req6;
	req6.method(http::verb::post);
	req6.target("/");
	auto rep6 = asio2::http_client::execute("www.baidu.com", "80", req6, ec);
	if (ec)
		std::cout << ec.message() << std::endl;
	else
		std::cout << rep6 << std::endl;

	// POST
	http::request_t<http::string_body> req7;
	req7.method(http::verb::post);
	req7.target("/");
	req7.set(http::field::user_agent, "Chrome");
	req7.set(http::field::content_type, "text/html");
	req7.body() = "Hello World.";
	req7.prepare_payload();
	auto rep7 = asio2::http_client::execute("www.baidu.com", "80", req7, ec);
	if (ec)
		std::cout << ec.message() << std::endl;
	else
		std::cout << rep7 << std::endl;

	// convert the response body to string
	std::stringstream ss1;
	ss1 << rep7.body();
	std::cout << ss1.str() << std::endl;

	// convert the whole response to string
	std::stringstream ss2;
	ss2 << rep7;
	std::cout << ss2.str() << std::endl;


	auto path = asio2::http::url_to_path("/get_user?name=abc");
	std::cout << path << std::endl;

	auto query = asio2::http::url_to_query("/get_user?name=abc");
	std::cout << query << std::endl;

	std::cout << std::endl;

	system("clear");
	std::cout << asio2::http_client::execute("http://www.baidu.com/get_user?name=abc");
	system("clear");
	std::cout << asio2::http_client::execute("http://www.baidu.com/get_user?name=abc", ec);
	system("clear");
	std::cout << asio2::http_client::execute("www.baidu.com", "80", "/api/get_user?name=abc");
	system("clear");
	std::cout << asio2::http_client::execute(shost, sport, "/api/get_user?name=abc");
	system("clear");
	std::cout << asio2::http_client::execute(host, port, "/api/get_user?name=abc");
	system("clear");
	std::cout << asio2::http_client::execute(shost, nport, "/api/get_user?name=abc");
	system("clear");
	std::cout << asio2::http_client::execute(host, nport, "/api/get_user?name=abc");
	system("clear");
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
