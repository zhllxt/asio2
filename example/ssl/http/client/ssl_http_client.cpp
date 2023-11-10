#ifndef ASIO2_ENABLE_SSL
#define ASIO2_ENABLE_SSL
#endif

#include <iostream>
#include <asio2/http/https_client.hpp>

int main()
{
	std::string_view host = "127.0.0.1";
	std::string_view port = "8443";

	asio2::socks5::option<asio2::socks5::method::anonymous>
		sock5_option{ "127.0.0.1",10808 };

	// 1. download and save the file directly. 
	// The file is in this directory: /asio2/example/bin/x64/100mb.test.ssl
	asio2::https_client::download(
		"https://cachefly.cachefly.net/100mb.test",
		"100mb.test.ssl");

	// 2. you can save the file content by youself. 
	// I find that, we must specify the SSL version to tlsv13, otherwise it will fail.
	std::fstream hugefile("CentOS-7-x86_64-DVD-2009.iso", std::ios::out | std::ios::binary | std::ios::trunc);
	asio2::https_client::download(asio::ssl::context{ asio::ssl::context::tlsv13 },
		"https://mirrors.tuna.tsinghua.edu.cn/centos/7.9.2009/isos/x86_64/CentOS-7-x86_64-DVD-2009.iso",
		// http header callback. this param is optional. the body callback is required.
		[](http::response<http::string_body>& header)
		{
			std::cout << header << std::endl;
			if (header.has_content_length())
			{
				std::cout << header.result_int() << " " << header.at(http::field::content_length) << std::endl;
			}
		},
		// http body callback.
		[&hugefile](std::string_view chunk)
		{
			hugefile.write(chunk.data(), chunk.size());
			return true; // false will break the downloading.
		}
	);
	hugefile.close();

	// download the first part: 0-1024
	{
		auto req = http::make_request("https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ls-lR.gz");
		req.set(http::field::range, "bytes=0-1024");

		std::fstream file("ls-lR.gz", std::ios::out | std::ios::binary | std::ios::trunc);
		asio2::https_client::download(asio::ssl::context{ asio::ssl::context::tlsv13 }, req,
			[&file](std::string_view chunk) // http body callback.
			{
				file.write(chunk.data(), chunk.size());
				return true; // false will break the downloading.
			}
		);
		file.close();
	}

	// download the second part: 1025-end
	{
		auto req = http::make_request("https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ls-lR.gz");
		req.set(http::field::range, "bytes=1025-");

		std::fstream file("ls-lR.gz", std::ios::out | std::ios::binary | std::ios::app);
		asio2::https_client::download(asio::ssl::context{ asio::ssl::context::tlsv13 }, req,
			[&file](std::string_view chunk) // http body callback.
			{
				file.write(chunk.data(), chunk.size());
				return true; // false will break the downloading.
			}
		);
		file.close();
	}

	std::string_view url = "https://www.baidu.com/query?key=x!#$&'()*+,/:;=?@[ ]-_.~%^{}\"|<>`\\y";

	asio::ssl::context ctx{ asio::ssl::context::sslv23 };

	http::web_request req1;
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

	auto reprss = asio2::https_client::execute("https://github.com/freefq/free", std::chrono::seconds(60), sock5_option);
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
		"../../example/cert/ca.crt",
		"../../example/cert/client.crt",
		"../../example/cert/client.key",
		"123456");
	if (asio2::get_last_error())
		std::cout << "load cert files failed: " << asio2::last_error_msg() << std::endl;

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
