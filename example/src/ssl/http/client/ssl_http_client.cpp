// When compiling with vs under linux, you need to copy the "asio,beast,ceral,fmt" folders to
// the /usr/local/include directory first, and copy the "libcrypto.a,libssl.a" files to 
// /usr/local/lib directory first. "libcrypto.a,libssl.a" is in "asio2/lib/x64".

#ifndef ASIO2_USE_SSL
#define ASIO2_USE_SSL
#endif

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
	std::string_view port = "443";

	asio2::error_code ec;

	auto reprss = asio2::https_client::execute("https://github.com/freefq/free", std::chrono::seconds(60), ec);
	std::cout << reprss << std::endl;

	{
		std::fstream file;
		file.open("abcrss.txt", std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
		file.write(reprss.body().data(), reprss.body().size());
	}
	{
		auto req1 = http::make_request("http://www.baidu.com/get_user?name=abc");

		std::cout << asio2::https_client::execute("www.baidu.com", "443",req1);
		system("clear");
		std::cout << asio2::https_client::execute("www.baidu.com", "443",req1, ec);

		system("clear");
		std::cout << asio2::https_client::execute("www.baidu.com", 443,req1);
		system("clear");
		std::cout << asio2::https_client::execute("www.baidu.com", 443,req1, ec);

		std::string shost = "www.baidu.com";
		std::string sport = "443";

		system("clear");
		std::cout << asio2::https_client::execute(shost, sport,req1);
		system("clear");
		std::cout << asio2::https_client::execute(shost, sport,req1, ec);

		system("clear");
		std::cout << asio2::https_client::execute(host, sport,req1);
		system("clear");
		std::cout << asio2::https_client::execute(host, sport,req1, ec);


		system("clear");
		std::cout << asio2::https_client::execute(shost, 443,req1);
		system("clear");
		std::cout << asio2::https_client::execute(shost, 443,req1, ec);

		system("clear");
		std::cout << asio2::https_client::execute(host, 443,req1);
		system("clear");
		std::cout << asio2::https_client::execute(host, 443,req1, ec);

		system("clear");
		std::cout << asio2::https_client::execute(shost, "443",req1);
		system("clear");
		std::cout << asio2::https_client::execute(shost, "443",req1, ec);

		system("clear");
		std::cout << asio2::https_client::execute(host, "443",req1);
		system("clear");
		std::cout << asio2::https_client::execute(host, "443",req1, ec);

		int nport = 443;

		system("clear");
		std::cout << asio2::https_client::execute(shost, nport,req1);
		system("clear");
		std::cout << asio2::https_client::execute(shost, nport,req1, ec);

		system("clear");
		std::cout << asio2::https_client::execute(host, nport,req1);
		system("clear");
		std::cout << asio2::https_client::execute(host, nport,req1, ec);

		auto rep1 = asio2::https_client::execute("http://www.baidu.com/get_user?name=abc", ec);
		if (ec)
			std::cout << ec.message() << std::endl;
		else
			std::cout << rep1 << std::endl;


		auto req2 = http::make_request("GET / HTTP/1.1\r\nHost: 192.168.0.1\r\n\r\n");
		auto rep2 = asio2::https_client::execute(asio::ssl::context{ asio::ssl::context::sslv23 },"www.baidu.com", "443", req2, std::chrono::seconds(3), ec);
		if (ec)
			std::cout << ec.message() << std::endl;
		else
			std::cout << rep2 << std::endl;


		auto path = asio2::http::url_to_path("/get_user?name=abc");
		std::cout << path << std::endl;

		auto query = asio2::http::url_to_query("/get_user?name=abc");
		std::cout << query << std::endl;

		std::cout << std::endl;

		system("clear");
		std::cout << asio2::https_client::execute("http://www.baidu.com/get_user?name=abc", ec);
		system("clear");
		std::cout << asio2::https_client::execute("http://www.baidu.com/get_user?name=abc", ec);
		system("clear");
		std::cout << asio2::https_client::execute("www.baidu.com", "443", "/api/get_user?name=abc");
		system("clear");
		std::cout << asio2::https_client::execute(shost, sport, "/api/get_user?name=abc");
		system("clear");
		std::cout << asio2::https_client::execute(host, port, "/api/get_user?name=abc");
		system("clear");
		std::cout << asio2::https_client::execute(shost, nport, "/api/get_user?name=abc");
		system("clear");
		std::cout << asio2::https_client::execute(host, nport, "/api/get_user?name=abc");
		system("clear");
		auto rep3 = asio2::https_client::execute("www.baidu.com", "443", "/api/get_user?name=abc", ec);
		if (ec)
			std::cout << ec.message() << std::endl;
		else
			std::cout << rep3 << std::endl;
	}

	auto rep1 = asio2::https_client::execute("https://www.baidu.com", ec);
	if (ec)
		std::cout << ec.message() << std::endl;
	else
		std::cout << rep1 << std::endl;


	std::string url = R"(https://www.baidu.com/get_user?json={"cmd":"mode = 10"})";
	url = asio2::http::url_encode(url);
	auto rep2 = asio2::https_client::execute(url, ec);
	if (ec)
		std::cout << ec.message() << std::endl;
	else
		std::cout << rep2 << std::endl;


	http::request req1;
	req1.version(11);
	req1.target("/");
	req1.method(http::verb::post);
	req1.set(http::field::accept, "*/*");
	req1.set(http::field::accept_encoding, "gzip, deflate");
	req1.set(http::field::accept_language, "zh-CN,zh;q=0.8,zh-TW;q=0.7,zh-HK;q=0.5,en-US;q=0.3,en;q=0.2");
	req1.set(http::field::content_type, "application/x-www-form-urlencoded");
	req1.set(http::field::user_agent, "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:75.0) Gecko/20100101 Firefox/75.0");
	req1.body() = "scope=all&id=gateway&secret=gateway";
	req1.prepare_payload();
	auto rep3 = asio2::https_client::execute("www.baidu.com", "443", req1, ec);
	if (ec)
		std::cout << ec.message() << std::endl;
	else
		std::cout << rep3 << std::endl;


	asio2::https_client client;
	//client.set_verify_mode(asio::ssl::verify_peer);
	//client.set_cert_buffer(ca_crt, client_crt, client_key, "client");
	client.set_cert_file("../../../cert/ca.crt", "../../../cert/client.crt", "../../../cert/client.key", "client");
	client.connect_timeout(std::chrono::seconds(10));
	client.bind_recv([&](http::request& req, http::response& rep)
	{
		std::cout << "----------------------------------------" << std::endl;
		std::cout << rep << std::endl;

	}).bind_connect([&](asio::error_code ec)
	{
		if (asio2::get_last_error())
			printf("connect failure : %d %s\n",
				asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
			printf("connect success : %s %u\n",
				client.local_address().c_str(), client.local_port());

		// send a request
		client.send("GET / HTTP/1.1\r\n\r\n");

	}).bind_disconnect([](asio::error_code ec)
	{
		printf("disconnect : %d %s\n",
			asio2::last_error_val(), asio2::last_error_msg().c_str());
	}).bind_handshake([&](asio::error_code ec)
	{
		printf("handshake : %d %s\n",
			asio2::last_error_val(), asio2::last_error_msg().c_str());
	});

	client.start(host, port);

	while (std::getchar() != '\n');

	client.stop();

	return 0;
}
