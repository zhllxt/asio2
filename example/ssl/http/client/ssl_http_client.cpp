#ifndef ASIO2_USE_SSL
#define ASIO2_USE_SSL
#endif

#include <iostream>
#include <asio2/http/https_client.hpp>

int main()
{
	std::string_view host = "127.0.0.1";
	std::string_view port = "8443";

	asio2::socks5::option<asio2::socks5::method::anonymous>
		sock5_option{ "127.0.0.1",10808 };

	std::string_view url = "https://www.baidu.com/query?key=x!#$&'()*+,/:;=?@[ ]-_.~%^{}\"|<>`\\y";

	asio::ssl::context ctx{ asio::ssl::context::sslv23 };

	http::request_t<http::string_body> req1;
	asio2::https_client::execute(ctx, "www.baidu.com", "443", req1, std::chrono::seconds(5));
	asio2::https_client::execute(ctx, "www.baidu.com", "443", req1);

	asio2::https_client::execute("www.baidu.com", "443", req1, std::chrono::seconds(5));
	asio2::https_client::execute("www.baidu.com", "443", req1);

	http::web_request req2 = http::make_request(url);
	asio2::https_client::execute(ctx, req2, std::chrono::seconds(5));
	asio2::https_client::execute(ctx, req2);

	asio2::https_client::execute(req2, std::chrono::seconds(5));
	asio2::https_client::execute(req2);

	asio2::https_client::execute(ctx, url, std::chrono::seconds(5));
	asio2::https_client::execute(ctx, url);

	asio2::https_client::execute(url, std::chrono::seconds(5));
	asio2::https_client::execute(url, sock5_option);

	auto reprss = asio2::https_client::execute("https://github.com/freefq/free", std::chrono::seconds(60));
	std::fstream file;
	file.open("freefq.html", std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
	file.write(reprss.body().data(), reprss.body().size());
	file.close();

	http::web_request req10;
	req10.version(11);
	req10.target("/");
	req10.method(http::verb::post);
	req10.set(http::field::accept, "*/*");
	req10.set(http::field::accept_encoding, "gzip, deflate");
	req10.set(http::field::accept_language, "zh-CN,zh;q=0.8,zh-TW;q=0.7,zh-HK;q=0.5,en-US;q=0.3,en;q=0.2");
	req10.set(http::field::content_type, "application/x-www-form-urlencoded");
	req10.set(http::field::user_agent, "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:75.0) Gecko/20100101 Firefox/75.0");
	req10.body() = "scope=all&id=gateway&secret=gateway";
	req10.prepare_payload();
	asio2::https_client::execute("www.baidu.com", "443", req10);


	asio2::https_client client;
	//client.set_verify_mode(asio::ssl::verify_peer);
	client.set_cert_file(
		"../../cert/ca.crt",
		"../../cert/client.crt",
		"../../cert/client.key",
		"123456");

	client.set_connect_timeout(std::chrono::seconds(10));

	client.bind_recv([&](http::web_request& req, http::web_response& rep)
	{
		asio2::ignore_unused(req);

		std::cout << "----------------------------------------" << std::endl;
		std::cout << rep << std::endl;

	}).bind_connect([&]()
	{
		if (asio2::get_last_error())
			printf("connect failure : %d %s\n",
				asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
			printf("connect success : %s %u\n",
				client.local_address().c_str(), client.local_port());

		// send a request
		client.async_send("GET / HTTP/1.1\r\n\r\n");

	}).bind_disconnect([]()
	{
		printf("disconnect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	}).bind_handshake([&]()
	{
		printf("handshake : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	});

	client.start(host, port);

	while (std::getchar() != '\n');

	return 0;
}
