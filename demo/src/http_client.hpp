#pragma once

#include <asio2/asio2.hpp>

void run_http_client(std::string_view host, std::string_view port)
{
#ifndef ASIO_STANDALONE
	//GET /DataSvr/api/anchor/AddAnchor?json=%7b%22id%22:4990560701320869680,%22name%22:%22anchor222%22,%22ip%22:%22192.168.0.101%22%7d HTTP/1.1
	//Host: localhost:8443
	//Connection: keep-alive
	//User-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/61.0.3163.100 Safari/537.36
	//Upgrade-Insecure-Requests: 1
	//Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8
	//Accept-Encoding: gzip, deflate, br
	//Accept-Language: zh-CN,zh;q=0.8

	asio2::error_code ec;
	//auto req1 = http::make_request("http://www.baidu.com/get_user?name=a");
	//auto req2 = http::make_request("http://www.baidu.com");
	//auto req3 = http::make_request("GET / HTTP/1.1\r\nHost: 127.0.0.1:8443\r\n\r\n");
	//auto req4 = http::make_request("GET / HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n");
	//auto rep1 = asio2::http_client::execute("http://www.baidu.com/get_user?name=a", ec);
	//auto path = asio2::http::url_to_path("/get_user?name=a");
	//auto rep2 = asio2::http_client::execute("127.0.0.1", "8080", req3);
	//auto rep3 = asio2::http_client::execute("http://127.0.0.1:8080/get_user?name=a", ec);
	//if (ec) std::cout << ec.message() << std::endl;
	//std::cout << rep3 << std::endl;

	auto rep = asio2::http_client::execute(host, port, "/api/get_user?name=zhl", ec);
	std::cout << rep << std::endl << "--------------------\n";

	//std::string en = http::url_encode("http://www.baidu.com/get_user?name=zhl");
	//std::cout << en << std::endl;
	//std::string de = http::url_decode(en);
	//std::cout << de << std::endl;

	asio2::http_client client;

	client.bind_recv([&](http::response<http::string_body>& rep)
	{
		std::cout << std::endl << "--------------------\n" << rep;
	}).bind_connect([&](asio::error_code ec)
	{
		if (asio2::get_last_error())
			printf("connect failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
			printf("connect success : %s %u\n", client.local_address().c_str(), client.local_port());

		auto * msg = "GET / HTTP/1.1\r\n\r\n";
		client.send(msg, [](std::size_t bytes_sent) {});

		const char * repstr = "HTTP/1.1 404 Not Found\r\n"\
			"Server: Boost.Beast/181\r\n"\
			"Content-Length: 7\r\n"\
			"\r\n"\
			"failure";

		{
			asio::const_buffer cb1 = asio::buffer(std::string_view{ repstr });
			const asio::const_buffer cb2 = asio::buffer(std::string_view{ repstr });
			client.send(cb1);
			client.send(cb2);
			client.send(std::move(cb1));
			client.send(std::move(cb2));
		}
		{
			asio::mutable_buffer cb1{ (void *)repstr ,std::string::traits_type::length(repstr) };
			const asio::mutable_buffer cb2{ (void *)repstr ,std::string::traits_type::length(repstr) };
			client.send(cb1);
			client.send(cb2);
			client.send(std::move(cb1));
			client.send(std::move(cb2));
		}

	}).bind_disconnect([](asio::error_code ec)
	{
		printf("disconnect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	});

	client.start(host, port);
	//client.start(host, port, "/api/get_user?name=zhl");
	while (std::getchar() != '\n');
#endif
}


void run_https_client(std::string_view host, std::string_view port)
{
#if !defined(ASIO_STANDALONE) && defined(ASIO2_USE_SSL)
	
	asio2::error_code ec;
	auto rep2 = asio2::https_client::execute("127.0.0.1", "8080", "/get_user?name=a", ec);
	std::cout << rep2 << std::endl;
	std::cout << "--------------------execute 2---------------------\n" << std::endl;

	std::string url = R"(http://127.0.0.1:8080/get_user?json={"cmd":"mode = 10"})";
	url = asio2::http::url_encode(url);
	auto rep3 = asio2::https_client::execute(url, ec);
	std::cout << rep3 << std::endl;
	std::cout << "--------------------execute 3---------------------\n" << std::endl;

	http::request<http::string_body> req;
	req.version(11);
	req.target("/token");
	req.method(http::verb::post);
	req.set(http::field::accept, "*/*");
	req.set(http::field::accept_encoding, "gzip, deflate");
	req.set(http::field::accept_language, "zh-CN,zh;q=0.8,zh-TW;q=0.7,zh-HK;q=0.5,en-US;q=0.3,en;q=0.2");
	req.set(http::field::connection, "keep-alive");
	req.set(http::field::content_length, "435");
	req.set(http::field::content_type, "application/x-www-form-urlencoded");
	req.set(http::field::host, host);
	req.set(http::field::user_agent, "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:75.0) Gecko/20100101 Firefox/75.0");
	req.body() = "scope=all&client_id=gateway&client_secret=gateway";
	req.prepare_payload();
	auto rep4 = asio2::https_client::execute("127.0.0.1", "8080", req, ec);
	std::cout << rep4 << std::endl;
	std::cout << "--------------------execute 4---------------------\n" << std::endl;


	asio2::https_client client;
	client.set_verify_mode(asio::ssl::verify_peer);
	//client.set_cert_buffer(ca_crt, client_crt, client_key, "client");
	client.set_cert_file("ca.crt", "client.crt", "client.key", "client");
	client.connect_timeout(std::chrono::seconds(10));
	client.bind_recv([&](http::response<http::string_body>& rep)
	{
		std::cout << std::endl << "--------------------\n" << rep << std::endl;
	}).bind_connect([&](asio::error_code ec)
	{
		if (asio2::get_last_error())
			printf("connect failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
			printf("connect success : %s %u\n", client.local_address().c_str(), client.local_port());

		auto * msg = "GET / HTTP/1.1\r\n\r\n";
		client.send(msg);
	}).bind_disconnect([](asio::error_code ec)
	{
		printf("disconnect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	}).bind_handshake([&](asio::error_code ec)
	{
		printf("handshake : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	});
	client.post([]() {});
	client.start(host, port, "/api/get_user?name=zhl");
	while (std::getchar() != '\n');
	client.stop();
#endif // ASIO2_USE_SSL
}
