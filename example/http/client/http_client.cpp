#include <asio2/http/http_client.hpp>
#include <iostream>

int main()
{
	std::string_view host = "127.0.0.1";
	std::string_view port = "8080";

	asio2::socks5::option<asio2::socks5::method::anonymous>
		sock5_option{ "127.0.0.1",10808 };
	//asio2::socks5::option<asio2::socks5::method::anonymous, asio2::socks5::method::password>
	//	sock5_option{ "s5.doudouip.cn",1088,"zjww-1","aaa123" };

	// download and save the file directly. // see ssl_http_client.cpp
	// The file is in this directory: /asio2/example/bin/x64/100mb.test
	asio2::http_client::download("http://cachefly.cachefly.net/100mb.test", "100mb.test");

	std::string_view url = R"(http://www.baidu.com/cond?json={"qeury":"name like '%abc%'","id":1})";

	std::string en = http::url_encode(url);
	std::cout << en << std::endl;
	std::string de = http::url_decode(en);
	std::cout << de << std::endl;

	auto path = asio2::http::url_to_path(url);
	std::cout << path << std::endl;

	auto query = asio2::http::url_to_query(url);
	std::cout << query << std::endl;


	auto rep21 = asio2::http_client::execute(url);
	std::cout << rep21 << std::endl;

	http::web_request req24 = http::make_request(url);
	req24.set(http::field::user_agent, "Chrome");
	auto rep26 = asio2::http_client::execute(req24);
	std::cout << rep26 << std::endl;


	asio2::http_client::execute("www.baidu.com", "80", "/", std::chrono::seconds(5));
	asio2::http_client::execute("www.baidu.com", "80", "/", sock5_option);


	// GET
	auto req2 = http::make_request("GET / HTTP/1.1\r\nHost: 192.168.0.1\r\n\r\n");
	auto rep2 = asio2::http_client::execute("www.baidu.com", "80", req2, std::chrono::seconds(3));
	if (asio2::get_last_error())
		std::cout << asio2::last_error_msg() << std::endl;
	else
		std::cout << rep2 << std::endl;

	// POST
	auto req4 = http::make_request("POST / HTTP/1.1\r\nHost: 192.168.0.1\r\n\r\n");
	auto rep4 = asio2::http_client::execute("www.baidu.com", "80", req4);
	if (asio2::get_last_error())
		std::cout << asio2::last_error_msg() << std::endl;
	else
		std::cout << rep4 << std::endl;

	// GET
	http::web_request req6(http::verb::get, "/", 11);
	req6.set(http::field::user_agent, "Chrome");
	auto rep6 = asio2::http_client::execute("www.baidu.com", "80", req6, sock5_option);
	if (asio2::get_last_error())
		std::cout << asio2::last_error_msg() << std::endl;
	else
		std::cout << rep6 << std::endl;

	// POST
	http::web_request req7;
	req7.method(http::verb::post);
	req7.target("/");
	req7.set(http::field::user_agent, "Chrome");
	req7.set(http::field::content_type, "text/html");
	req7.body() = "Hello World.";
	req7.prepare_payload();
	auto rep7 = asio2::http_client::execute("www.baidu.com", "80", req7);
	if (asio2::get_last_error())
		std::cout << asio2::last_error_msg() << std::endl;
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


	asio2::http_client client;

	client.bind_recv([&](http::web_request& req, http::web_response& rep)
	{
		// print the whole response
		std::cout << rep << std::endl;

		// print the response body
		std::cout << rep.body() << std::endl;

		// convert the response body to string
		std::stringstream ss;
		ss << rep.body();
		std::cout << ss.str() << std::endl;

		// Remove all fields
		req.clear();

		req.set(http::field::user_agent, "Chrome");
		req.set(http::field::content_type, "text/html");

		req.method(http::verb::get);
		req.keep_alive(true);
		req.target("/get_user?name=abc");
		req.body() = "Hello World.";
		req.prepare_payload();

		client.async_send(std::move(req));

	}).bind_connect([&]()
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
			client.async_send(msg);
		}
	});

	client.start(host, port/*, sock5_option*/);

	while (std::getchar() != '\n');

	return 0;
}
