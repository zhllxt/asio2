#pragma once

#include <asio2/asio2.hpp>

#include <filesystem>

void run_http_server(std::string_view host, std::string_view port)
{
#ifndef ASIO_STANDALONE
	//GET / HTTP/1.1
	//Host: 127.0.0.1:8443
	//Connection: keep-alive
	//Cache-Control: max-age=0
	//User-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/61.0.3163.100 Safari/537.36
	//Upgrade-Insecure-Requests: 1
	//Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8
	//Accept-Encoding: gzip, deflate, br
	//Accept-Language: zh-CN,zh;q=0.8

	//HTTP/1.1 302 Found
	//Server: openresty
	//Date: Fri, 10 Nov 2017 03:11:50 GMT
	//Content-Type: text/html; charset=utf-8
	//Transfer-Encoding: chunked
	//Connection: keep-alive
	//Keep-Alive: timeout=20
	//Location: http://bbs.csdn.net/home
	//Cache-Control: no-cache
	//
	//5a
	//<html><body>You are being <a href="http://bbs.csdn.net/home">redirected</a>.</body></html>
	//0

	//const char * url = "http://localhost:8080/engine/api/user/adduser?json=%7b%22id%22:4990560701320869680,%22name%22:%22admin%22%7d";
	//const char * url = "http://localhost:8080/engine/api/user/adduser?json=[\"id\":4990560701320869680,\"name\":\"admin\"]";
	//const char * url = "http://localhost:8080/engine/api/user/adduser";

	asio2::http_server server;

	bool flag = true;
	server.bind_recv([&](std::shared_ptr<asio2::http_session> & session_ptr, http::request<http::string_body>& req)
	{
		//if (0) // test send file
		{
			// Request path must be absolute and not contain "..".
			if (req.target().empty() ||
				req.target()[0] != '/' ||
				req.target().find("..") != beast::string_view::npos)
			{
				session_ptr->send(http::make_response(http::status::bad_request, "Illegal request-target"));
				session_ptr->stop();
				return;
			}

			// Build the path to the requested file
			std::string path(req.target().data(), req.target().size());
			path.insert(0, std::filesystem::current_path().string());
			if (req.target().back() == '/')
				path.append("index.html");
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
			path = "D:/nlohmann_json.hpp";
#else 
			path = "/usr/local/lib/libsqlite3.so";
#endif

			// Attempt to open the file
			beast::error_code ec;
			http::file_body::value_type body;
			body.open(path.c_str(), beast::file_mode::scan, ec);

			// Handle the case where the file doesn't exist
			if (ec == beast::errc::no_such_file_or_directory)
			{
				session_ptr->send(http::make_response(http::status::not_found,
					std::string_view{ req.target().data(), req.target().size() }));
				return;
			}

			// Handle an unknown error
			if (ec)
			{
				session_ptr->send(http::make_response(http::status::internal_server_error, ec.message()));
				return;
			}

			// Cache the size since we need it after the move
			auto const size = body.size();

			// Respond to HEAD request
			if (req.method() == http::verb::head)
			{
				http::response<http::empty_body> res{ http::status::ok, req.version() };
				res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
				res.set(http::field::content_type, http::extension_to_mimetype(path));
				res.content_length(size);
				res.keep_alive(req.keep_alive());
				session_ptr->send(std::move(res));
				return;
			}

			// Respond to GET request
			http::response<http::file_body> res{
				std::piecewise_construct,
				std::make_tuple(std::move(body)),
				std::make_tuple(http::status::ok, req.version()) };
			res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
			//res.set(http::field::content_type, http::extension_to_mimetype(path));
			res.content_length(size);
			res.keep_alive(req.keep_alive());
			//res.set(http::field::transfer_encoding, "chunked");
			//res.chunked(true);
			// Specify a callback function when sending
			session_ptr->send(std::move(res), [](std::size_t bytes_sent)
			{
				auto err = asio2::get_last_error();
				if (err) printf("%s\n", err.message().c_str());
			});
			//session_ptr->send(std::move(res));
			//session_ptr->send(std::move(res), asio::use_future);
			return;
		}

		std::cout << req << std::endl;
		if (flag)
		{
			// test send string_body
			auto rep = http::make_response(200, "suceess");
			session_ptr->send(rep, []()
			{
				auto err = asio2::get_last_error(); std::ignore = err;
			});
		}
		else
		{
			std::string_view rep =
				"HTTP/1.1 404 Not Found\r\n"\
				"Server: Boost.Beast/181\r\n"\
				"Content-Length: 7\r\n"\
				"\r\n"\
				"failure";
			// test send string sequence, the string will automatically parsed into a standard http request
			session_ptr->send(rep, [](std::size_t bytes_sent)
			{
				auto err = asio2::get_last_error(); std::ignore = err;
			});
		}
		flag = !flag;
	}).bind_connect([](auto & session_ptr)
	{
		printf("client enter : %s %u %s %u\n", session_ptr->remote_address().c_str(), session_ptr->remote_port(),
			session_ptr->local_address().c_str(), session_ptr->local_port());
	}).bind_disconnect([](auto & session_ptr)
	{
		printf("client leave : %s %u %s\n", session_ptr->remote_address().c_str(),
			session_ptr->remote_port(), asio2::last_error_msg().c_str());
	}).bind_start([&](asio::error_code ec)
	{
		printf("start http server : %s %u %d %s\n", server.listen_address().c_str(), server.listen_port(),
			ec.value(), ec.message().c_str());
	}).bind_stop([&](asio::error_code ec)
	{
		printf("stop : %d %s\n", ec.value(), ec.message().c_str());
	});

	server.start(host, port);

	while (std::getchar() != '\n'); // press enter to exit this program
#endif
}

void run_https_server(std::string_view host, std::string_view port)
{
#if !defined(ASIO_STANDALONE) && defined(ASIO2_USE_SSL)
	asio2::https_server server;
	server.set_cert_file("test", "server.crt", "server.key", "dh512.pem");
	server.bind_recv([](std::shared_ptr<asio2::https_session> & session_ptr, http::request<http::string_body>& req)
	{
		std::cout << req << std::endl;
		session_ptr->send(http::make_response(200, "suceess"), []() {});
	}).bind_connect([](auto & session_ptr)
	{
		printf("client enter : %s %u %s %u\n", session_ptr->remote_address().c_str(), session_ptr->remote_port(),
			session_ptr->local_address().c_str(), session_ptr->local_port());
	}).bind_disconnect([](auto & session_ptr)
	{
		printf("client leave : %s %u %s\n", session_ptr->remote_address().c_str(),
			session_ptr->remote_port(), asio2::last_error_msg().c_str());
	}).bind_handshake([](auto & session_ptr, asio::error_code ec)
	{
		printf("client handshake : %s %u %d %s\n", session_ptr->remote_address().c_str(),
			session_ptr->remote_port(), ec.value(), ec.message().c_str());
	}).bind_start([&](asio::error_code ec)
	{
		printf("start https server : %s %u %d %s\n", server.listen_address().c_str(),
			server.listen_port(), ec.value(), ec.message().c_str());
		//server.stop();
	}).bind_stop([&](asio::error_code ec)
	{
		printf("stop : %d %s\n", ec.value(), ec.message().c_str());
	});
	server.start(host, port);
	while (std::getchar() != '\n');
#endif // ASIO2_USE_SSL
}
