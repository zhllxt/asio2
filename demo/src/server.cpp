/*
> Additional dependency Libraries: (stdc++fs; is used for gcc std::filesystem)
boost_system;ssl;crypto;stdc++fs;
> Compile:
g++ -c -x c++ /root/projects/server/demo/src/server.cpp -I /usr/local/include -I /root/projects/server -g2 -gdwarf-2 -o "/root/projects/server/obj/x64/Debug/server/server.o" -Wall -Wswitch -W"no-deprecated-declarations" -W"empty-body" -Wconversion -W"return-type" -Wparentheses -W"no-format" -Wuninitialized -W"unreachable-code" -W"unused-function" -W"unused-value" -W"unused-variable" -O0 -fno-strict-aliasing -fno-omit-frame-pointer -fthreadsafe-statics -fexceptions -frtti -std=c++17
> Link:
g++ -o "/root/projects/server/bin/x64/Debug/server.out" -Wl,--no-undefined -Wl,-L/usr/local/lib -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -lpthread -lrt -ldl /root/projects/server/obj/x64/Debug/server/server.o -lboost_system -lssl -lcrypto
*/
#include <asio2/asio2.hpp>
#include <iostream>
#include <filesystem>

// byte 1    head   : #
// byte 2    length : the body length
// byte 3... body   : the body content
class match_role
{
public:
	explicit match_role(char c) : c_(c) {}

	// The first member of the
	// return value is an iterator marking one-past-the-end of the bytes that have
	// been consumed by the match function.This iterator is used to calculate the
	// begin parameter for any subsequent invocation of the match condition.The
	// second member of the return value is true if a match has been found, false
	// otherwise.
	template <typename Iterator>
	std::pair<Iterator, bool> operator()(Iterator begin, Iterator end) const
	{
		Iterator i = begin;
		while (i != end)
		{
			// eg : How to close illegal clients
			// If the first byte is not # indicating that the client is illegal, we return
			// the matching success here and then determine the number of bytes received
			// in the on_recv callback function, if it is 0, we close the connection in on_recv.
			if (*i != c_)
				return std::pair(begin, true);

			if (*i++ == c_ && i != end)
			{
				unsigned length = std::uint8_t(*i++);
				if (length <= end - i)
					return std::pair(i + length, true);
			}
		}
		return std::pair(begin, false);
	}

private:
	char c_;
};

#ifdef ASIO_STANDALONE
namespace asio {
#else
namespace boost::asio {
#endif
	template <> struct is_match_condition<match_role> : public std::true_type {};
} // namespace asio

void run_tcp_server(std::string_view host, std::string_view port)
{
	asio2::tcp_server server;
	//while (1) // use infinite loop and sleep 2 seconds to test start and stop
	{
		printf("\n");
		server.start_timer(1, std::chrono::seconds(1), []() {});
		server.bind_recv([&server](std::shared_ptr<asio2::tcp_session> & session_ptr, std::string_view s)
		{
			if (s.size() == 0)
			{
				printf("close illegal client : %s %u\n",
					session_ptr->remote_address().c_str(), session_ptr->remote_port());
				session_ptr->stop();
				return;
			}

			printf("recv : %u %.*s\n", (unsigned)s.size(), (int)s.size(), s.data());

			session_ptr->send(s, [](std::size_t bytes_sent) {});

			// ##Thread-safe send operation example:
			//session_ptr->post([session_ptr]()
			//{
			//	asio::write(session_ptr->stream(), asio::buffer(std::string("abcdefghijklmn")));
			//});

			// ##Use this to check whether the send operation is running in current thread.
			//if (session_ptr->io().strand().running_in_this_thread())
			//{
			//}

		}).bind_connect([&server](auto & session_ptr)
		{
			session_ptr->no_delay(true);
			//session_ptr->start_timer(1, std::chrono::seconds(1), []() {});
			//session_ptr->stop(); // You can close the connection directly here.
			printf("client enter : %s %u %s %u\n",
				session_ptr->remote_address().c_str(), session_ptr->remote_port(),
				session_ptr->local_address().c_str(), session_ptr->local_port());
		}).bind_disconnect([&server](auto & session_ptr)
		{
			printf("client leave : %s %u %s\n",
				session_ptr->remote_address().c_str(),
				session_ptr->remote_port(), asio2::last_error_msg().c_str());
		}).bind_start([&server](asio::error_code ec)
		{
			printf("start : %s %u %d %s\n", server.listen_address().c_str(), server.listen_port(),
				ec.value(), ec.message().c_str());
		}).bind_stop([&server](asio::error_code ec)
		{
			printf("stop : %d %s\n", ec.value(), ec.message().c_str());
		});
		server.start(host, port);
		//server.start(host, port, '>');
		//server.start(host, port, "\r\n");
		//server.start(host, port, match_role('#'));
		//server.start(host, port, asio::transfer_at_least(1));
		//server.start(host, port, asio::transfer_exactly(100));
		//server.start(host, port, asio2::use_dgram);

		while (std::getchar() != '\n');
		//std::this_thread::sleep_for(std::chrono::seconds(1));

		server.stop();
	}
}

void run_udp_server(std::string_view host, std::string_view port)
{
	//while (1) // use infinite loop and sleep 2 seconds to test start and stop
	{
		printf("\n");
		asio2::udp_server server;
		server.bind_recv([](std::shared_ptr<asio2::udp_session> & session_ptr, std::string_view s)
		{
			printf("recv : %u %.*s\n", (unsigned)s.size(), (int)s.size(), s.data());
			session_ptr->no_delay(true); // No effect
			session_ptr->send(s, []() {});
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
			printf("client handshake : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		}).bind_start([&server](asio::error_code ec)
		{
			if (asio2::get_last_error())
				printf("start failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
			else
				printf("start success : %s %u\n", server.listen_address().c_str(), server.listen_port());
			//server.stop();
		}).bind_stop([&](asio::error_code ec)
		{
			printf("stop : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		}).bind_init([&]()
		{
			//// Join the multicast group. you can set this option in the on_init(_fire_init) function.
			//server.acceptor().set_option(
			//	// for ipv6, the host must be a ipv6 address like 0::0
			//	asio::ip::multicast::join_group(asio::ip::make_address("ff31::8000:1234")));
			//	// for ipv4, the host must be a ipv4 address like 0.0.0.0
			//	//asio::ip::multicast::join_group(asio::ip::make_address("239.255.0.1")));
		});
		server.start(host, port);
		//server.start(host, port, asio2::use_kcp);

		while (std::getchar() != '\n');
		//std::this_thread::sleep_for(std::chrono::seconds(1));

		server.stop();
	}
}

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
		if (1) // test send file
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
			path = "D:/TDDownload/zh-hans_windows_xp_professional_with_service_pack_3_x86_cd_vl_x14-74070.iso";
			//path = "/usr/local/lib/libsqlite3.so"; 

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
			res.set(http::field::content_type, http::extension_to_mimetype(path));
			res.content_length(size);
			res.keep_alive(req.keep_alive()); 
			res.chunked(true);
			// Specify a callback function when sending
			session_ptr->send(std::move(res), [&res](std::size_t bytes_sent)
			{
				auto opened = res.body().is_open(); std::ignore = opened;
				auto err = asio2::get_last_error(); std::ignore = err;
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
		printf("start : %s %u %d %s\n", server.listen_address().c_str(), server.listen_port(),
			ec.value(), ec.message().c_str());
	}).bind_stop([&](asio::error_code ec)
	{
		printf("stop : %d %s\n", ec.value(), ec.message().c_str());
	});
	server.start(host, port);
	while (std::getchar() != '\n');
#endif
}

void run_ws_server(std::string_view host, std::string_view port)
{
#ifndef ASIO_STANDALONE
	asio2::ws_server server;
	server.bind_recv([](std::shared_ptr<asio2::ws_session> & session_ptr, std::string_view s)
	{
		printf("recv : %u %.*s\n", (unsigned)s.size(), (int)s.size(), s.data());
		session_ptr->send(std::string(s), [](std::size_t bytes_sent) {});
	}).bind_connect([](auto & session_ptr)
	{
		printf("client enter : %s %u %s %u\n", session_ptr->remote_address().c_str(), session_ptr->remote_port(),
			session_ptr->local_address().c_str(), session_ptr->local_port());
	}).bind_disconnect([](auto & session_ptr)
	{
		printf("client leave : %s %u %s\n", session_ptr->remote_address().c_str(),
			session_ptr->remote_port(), asio2::last_error_msg().c_str());
	}).bind_upgrade([](auto & session_ptr, asio::error_code ec)
	{
		printf(">> upgrade %d %s\n", ec.value(), ec.message().c_str());
	}).bind_start([&](asio::error_code ec)
	{
		printf("start : %s %u %d %s\n", server.listen_address().c_str(), server.listen_port(),
			ec.value(), ec.message().c_str());
	}).bind_stop([&](asio::error_code ec)
	{
		printf("stop : %d %s\n", ec.value(), ec.message().c_str());
	});
	server.start(host, port);
	while (std::getchar() != '\n');
#endif
}

void run_httpws_server(std::string_view host, std::string_view port)
{
#ifndef ASIO_STANDALONE
	asio2::httpws_server server;
	//asio2::httpwss_server server;
	//server.set_cert_file("test", "server.crt", "server.key", "dh512.pem");
	//server.bind_recv([](std::shared_ptr<asio2::httpwss_session> & session_ptr, http::request<http::string_body>& req, std::string_view s)
	server.bind_recv([](std::shared_ptr<asio2::httpws_session> & session_ptr, http::request<http::string_body>& req, std::string_view s)
	{
		if (session_ptr->is_http())
		{
			std::cout << req << std::endl << std::endl;
			//bool flag = session_ptr->send(http::make_response(200, "http:result:success"), [](std::size_t bytes_sent) {});
			session_ptr->send(http::make_response(200, "http:result:success"), asio::use_future);
		}
		else
		{
			printf("recv : %u %.*s\n", (unsigned)s.size(), (int)s.size(), (const char*)s.data());
			session_ptr->send(s, [](std::size_t bytes_sent) {});
			//session_ptr->send(s, asio::use_future);
		}
	}).bind_connect([](auto & session_ptr)
	{
		printf("client enter : %s %u %s %u\n", session_ptr->remote_address().c_str(), session_ptr->remote_port(),
			session_ptr->local_address().c_str(), session_ptr->local_port());
	}).bind_disconnect([](auto & session_ptr)
	{
		printf("client leave : %s %u %s\n", session_ptr->remote_address().c_str(),
			session_ptr->remote_port(), asio2::last_error_msg().c_str());
	}).bind_upgrade([](auto & session_ptr, asio::error_code ec)
	{
		printf("client upgrade : %s %u %d %s\n", session_ptr->remote_address().c_str(),
			session_ptr->remote_port(), ec.value(), ec.message().c_str());
	}).bind_start([&server](asio::error_code ec)
	{
		printf("start : %s %u %d %s\n", server.listen_address().c_str(), server.listen_port(),
			ec.value(), ec.message().c_str());
	}).bind_stop([&server](asio::error_code ec)
	{
		printf("stop : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	});
	server.start(host, port);
	while (std::getchar() != '\n');
#endif
}

void run_tcps_server(std::string_view host, std::string_view port)
{
#ifdef ASIO2_USE_SSL
	std::string_view cer =
		"-----BEGIN CERTIFICATE-----\r\n"\
		"MIICcTCCAdoCCQDYl7YrsugMEDANBgkqhkiG9w0BAQsFADB9MQswCQYDVQQGEwJD\r\n"\
		"TjEOMAwGA1UECAwFSEVOQU4xEjAQBgNVBAcMCVpIRU5HWkhPVTENMAsGA1UECgwE\r\n"\
		"SE5aWDENMAsGA1UECwwESE5aWDEMMAoGA1UEAwwDWkhMMR4wHAYJKoZIhvcNAQkB\r\n"\
		"Fg8zNzc5MjczOEBxcS5jb20wHhcNMTcxMDE1MTQzNjI2WhcNMjcxMDEzMTQzNjI2\r\n"\
		"WjB9MQswCQYDVQQGEwJDTjEOMAwGA1UECAwFSEVOQU4xEjAQBgNVBAcMCVpIRU5H\r\n"\
		"WkhPVTENMAsGA1UECgwESE5aWDENMAsGA1UECwwESE5aWDEMMAoGA1UEAwwDWkhM\r\n"\
		"MR4wHAYJKoZIhvcNAQkBFg8zNzc5MjczOEBxcS5jb20wgZ8wDQYJKoZIhvcNAQEB\r\n"\
		"BQADgY0AMIGJAoGBAMc2Svpl4UgxCVKGwoYJBxNWObXvQzw74ksY6Zoiq5tJNJzf\r\n"\
		"q9ZCJigwjx3vAFF7tELRxsgAf6l7AvReu1O6difjdpMkEic0W7acZtldislDjUbu\r\n"\
		"qitfHsWeKTucBu3+3TUawvv+fdeWgeN54jMoL+Oo3CV7d2gFRV2fD5z4tryXAgMB\r\n"\
		"AAEwDQYJKoZIhvcNAQELBQADgYEAwDIC3xYmYJ6kLI8NgmX89re0scSWCcA8VgEZ\r\n"\
		"u8roYjYauCLkp1aXNlZtJFQjwlfo+8FLzgp3dP8Y75YFwQ5zy8fFaLQSQ/0syDbx\r\n"\
		"sftKSVmxDo3S27IklEyJAIdB9eKBTeVvrT96R610j24t1eYENr59Vk6A/fKTWJgU\r\n"\
		"EstmrAs=\r\n"\
		"-----END CERTIFICATE-----\r\n";

	std::string_view key =
		"-----BEGIN RSA PRIVATE KEY-----\r\n"\
		"Proc-Type: 4,ENCRYPTED\r\n"\
		"DEK-Info: DES-EDE3-CBC,EC5314BD06CD5FB6\r\n"\
		"\r\n"\
		"tP93tjR4iOGfOLHjIBQA0aHUE5wQ7EDcUeKacFfuYrtlYbYpbRzhQS+vGtoO1wGg\r\n"\
		"h/s9DbEN1XaiV9aE+N3E54zu2LuVO1lYDtCf3L26cd1Bu6gj0cWiAMco1Vm7RV9j\r\n"\
		"vmgmeOYkqbOiAbiIa4HCmDkEaHY4nCPlW+cdYxrozkAQCAiTpFQR8taRB0lsly0i\r\n"\
		"lUQitYLz3nhEMucLffcwAXN9IOnXFoURVZnLc53CX857iizOXeP9XeNE63UwDZ4v\r\n"\
		"1wnglnGUJA6vCxnxk6KvptF9rSdCD/sz1Y+J5mAVr+2y4vPLO4YOCL6HSFY6285M\r\n"\
		"RyGNVVx3vX0u6FbWJC3qt5yj6tMdVJ4O7U4XgqOKnS5jVLk+fKcTVyNySB5yAT2b\r\n"\
		"qwWCZcRPP2M+qlsSWhgzsucyz0eVOPVJxAJ4Vp/X6saO4xyRPsFV3USbRKlOMS7+\r\n"\
		"SEJ/7ANU9mEgLIQRKEfSKXWpQtm95pCVlajWQ7/3nXNjdV7mNi42ukdItBvOtdv+\r\n"\
		"oUiN8MkP/e+4SsGmJayNT7HvBC9DjoyDQIK6sZOgtsbAu/bDBhPnjnNsZcsgxJ/O\r\n"\
		"ijnj+0HyNS/Vr6emAkxTFgryUdBTuoY7019vcNWTYPDS3ugpe3goRHE0FTOwNdUe\r\n"\
		"dk+KM4bYAa0+1z1QEZTEoNqdT7WYwMD1QzgSWukYHemsWqoAvW5f4PrdoVA21W9D\r\n"\
		"L8I1YZf8ZHBnkuGX0oHi5w/4DkVNOT5BaZRmqXinZgFPwduYGVCh04x7ohuOQ5m0\r\n"\
		"etrTAVwJd2mcI7rDTaKCPT528/QWxZxXpHzggRoDil/5T7fn35ixRg==\r\n"\
		"-----END RSA PRIVATE KEY-----\r\n";

	std::string_view dh =
		"-----BEGIN DH PARAMETERS-----\r\n"\
		"MEYCQQCdoJif7jYqTh5+vLgt3q1FZvG+7WymoAoMKWMNOtqLZ+uFhZH3e9vFhV7z\r\n"\
		"NgWnHCe/vsGJok2wHS4R/laH6MQTAgEC\r\n"\
		"-----END DH PARAMETERS-----\r\n";

	//while (1)
	{
		printf("\n");
		bool stopped = false;
		asio2::tcps_server server;
		server.set_cert("test", cer, key, dh);
		//server.set_cert_file("test", "server.crt", "server.key", "dh512.pem");
		server.start_timer(1, std::chrono::seconds(1), []() {});
		server.bind_recv([&server](auto & session_ptr, std::string_view s)
		{
			//printf("recv : %u %.*s\n", (unsigned)s.size(), (int)s.size(), s.data());
			//if (s.front() != '<' || s.back() != '>')
			//{
			//	printf("%s\n", asio2::last_error_msg().c_str());
			//	while (std::getchar() != '\n');
			//}
			session_ptr->send(s, []() {});
		}).bind_connect([&](auto & session_ptr)
		{
			printf("client enter : %s %u %s %u\n", session_ptr->remote_address().c_str(), session_ptr->remote_port(),
				session_ptr->local_address().c_str(), session_ptr->local_port());
		}).bind_disconnect([&](auto & session_ptr)
		{
			// Used to test that all sessions must be closed before entering the on_stop(bind_stop) function.
			if (stopped)
			{
				ASIO2_ASSERT(false);
			}
			printf("client leave : %s %u %s\n", session_ptr->remote_address().c_str(),
				session_ptr->remote_port(), asio2::last_error_msg().c_str());
		}).bind_handshake([&](auto & session_ptr, asio::error_code ec)
		{
			if (asio2::get_last_error())
				printf("handshake failure : %d %s\n", ec.value(), ec.message().c_str());
			else
				printf("handshake success : %s %u\n", session_ptr->remote_address().c_str(), session_ptr->remote_port());
		}).bind_start([&](asio::error_code ec)
		{
			if (asio2::get_last_error())
				printf("start failure : %d %s\n", ec.value(), ec.message().c_str());
			else
				printf("start success : %s %u\n", server.listen_address().c_str(), server.listen_port());
			//server.stop();
		}).bind_stop([&](asio::error_code ec)
		{
			stopped = true;
			printf("stop : %d %s\n", ec.value(), ec.message().c_str());
		});
		server.start(host, port);
		//server.start(host, port, '>');
		//server.start(host, port, "\r\n");
		//server.start(host, port, match_role('#'));
		//server.start(host, port, asio::transfer_at_least(1));
		//server.start(host, port, asio::transfer_exactly(100));
		//server.start(host, port, asio2::use_dgram);

		while (std::getchar() != '\n');
		//std::this_thread::sleep_for(std::chrono::seconds(2));

		server.stop();
	}
#endif // ASIO2_USE_SSL
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
		printf("start : %s %u %d %s\n", server.listen_address().c_str(),
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

void run_wss_server(std::string_view host, std::string_view port)
{
#if !defined(ASIO_STANDALONE) && defined(ASIO2_USE_SSL)
	//while (1)
	{
		asio2::wss_server server;
		server.set_cert_file("test", "server.crt", "server.key", "dh512.pem");
		server.bind_recv([](std::shared_ptr<asio2::wss_session> & session_ptr, std::string_view s)
		{
			printf("recv : %u %.*s\n", (unsigned)s.size(), (int)s.size(), s.data());
			session_ptr->send(s, []() {});
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
			printf("client handshake : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		}).bind_upgrade([](auto & session_ptr, asio::error_code ec)
		{
			printf("client upgrade : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		}).bind_start([&server](asio::error_code ec)
		{
			if (asio2::get_last_error())
				printf("start failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
			else
				printf("start success : %s %u\n", server.listen_address().c_str(), server.listen_port());
		}).bind_stop([&server](asio::error_code ec)
		{
			printf("stop : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		});
		server.start(host, port);
		while (std::getchar() != '\n');
		//std::this_thread::sleep_for(std::chrono::seconds(2));
	}
#endif // ASIO2_USE_SSL
}

void run_udp_cast(std::string_view host, std::string_view port)
{
	//while (1)
	{
		printf("\n");
		asio2::udp_cast recver;
		recver.bind_recv([&](asio::ip::udp::endpoint& endpoint, std::string_view sv)
		{
			printf("recv : %s %u %u %.*s\n", endpoint.address().to_string().c_str(),
				endpoint.port(), (unsigned)sv.size(), (int)sv.size(), sv.data());
			recver.send(endpoint, sv);
		}).bind_start([&](asio::error_code ec)
		{
			printf("start : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		}).bind_stop([&](asio::error_code ec)
		{
			printf("stop : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		}).bind_init([&]()
		{
			// Join the multicast group.
			recver.socket().set_option(
				asio::ip::multicast::join_group(asio::ip::make_address("239.255.0.1")));
		});
		recver.start(host, port);

		while (std::getchar() != '\n');
		//std::this_thread::sleep_for(std::chrono::seconds(2));

		recver.stop();
	}
}

class user
{
public:
	std::string name;
	int age;
	std::map<int, std::string> purview;

	template <class Archive>
	void serialize(Archive & ar)
	{
		ar(name);
		ar(age);
		ar(purview);
	}
};

int add(int a, int b)
{
	return a + b;
}

class A
{
public:
	double mul(double a, double b)
	{
		return a * b;
	}

	user get_user()
	{
		user u;
		u.name = "lilei";
		u.age = ((int)time(nullptr)) % 100;
		u.purview = { {1,"read"},{2,"write"} };
		return u;
	}

	void del_user(const user& u)
	{
		printf("%s %d ", u.name.c_str(), u.age);
		for (auto &[k, v] : u.purview)
		{
			printf("%d %s ", k, v.c_str());
		}
		printf("\n");
	}
};

void run_rpc_server(std::string_view host, std::string_view port)
{
	asio2::rpc_server server;
	//while (1) // use infinite loop and sleep 2 seconds to test start and stop
	{
		printf("\n");
		server.start_timer(1, std::chrono::seconds(1), []() {});
		server.bind_recv([&server](auto & session_ptr, std::string_view s)
		{
			//printf("recv : %u %.*s\n", (unsigned)s.size(), (int)s.size(), s.data());
		}).bind_send([&](auto & session_ptr, std::string_view s)
		{
		}).bind_connect([&server](auto & session_ptr)
		{
			//session_ptr->stop();
			printf("client enter : %s %u %s %u\n",
				session_ptr->remote_address().c_str(), session_ptr->remote_port(),
				session_ptr->local_address().c_str(), session_ptr->local_port());

			session_ptr->async_call([](asio::error_code ec, int v)
			{
				printf("sub : %d err : %d %s\n", v, ec.value(), ec.message().c_str());
			}, std::chrono::seconds(10), "sub", 15, 8);
		}).bind_disconnect([&server](auto & session_ptr)
		{
			printf("client leave : %s %u %s\n",
				session_ptr->remote_address().c_str(),
				session_ptr->remote_port(), asio2::last_error_msg().c_str());
		}).bind_start([&server](asio::error_code ec)
		{
			printf("start : %s %u %d %s\n", server.listen_address().c_str(), server.listen_port(),
				ec.value(), ec.message().c_str());
		}).bind_stop([&server](asio::error_code ec)
		{
			printf("stop : %d %s\n", ec.value(), ec.message().c_str());
		});

		A a;
		server.bind("add", add);
		server.bind("mul", &A::mul, a);
		server.bind("cat", [&](const std::string& a, const std::string& b) { return a + b; });
		server.bind("get_user", &A::get_user, a);
		server.bind("del_user", &A::del_user, &a);

		// Using tcp dgram mode as the underlying communication support(This is the default setting)
		// Then must use "use_dgram" parameter.
		server.start(host, port, asio2::use_dgram);

		// Using websocket as the underlying communication support.
		// Need to goto the tail of the (rpc_client.hpp rpc_server.hpp rpc_session.hpp) files,
		// and modified to use websocket.
		//server.start(host, port);

		while (std::getchar() != '\n');
		//std::this_thread::sleep_for(std::chrono::seconds(1));

		server.stop();
	}
}

int main(int argc, char *argv[])
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system,linux has't these function
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	// the number is the memory leak line num of the vs output window content.
	//_CrtSetBreakAlloc(1640); 
#endif

	//std::srand(static_cast<unsigned int>(std::time(nullptr)));

	//auto t1 = std::chrono::steady_clock::now();
	//auto t2 = std::chrono::steady_clock::now();
	//auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
	//printf("%lld\n", ms);

	asio2::timer timer;
	timer.start_timer(1, std::chrono::seconds(1), [&]()
	{
		printf("timer 1\n");
		timer.stop_timer(1);
	});

	std::string_view host = "0.0.0.0", port = "8080";
	//port = argv[1];
	run_tcp_server(host, port);
	//run_udp_server(host, port);
	//run_http_server(host, port);
	//run_ws_server(host, port);
	//run_httpws_server(host, port); 
	//run_tcps_server(host, port);
	//run_https_server(host, port);
	//run_wss_server(host, port);
	//run_udp_cast(host, port);
	//run_rpc_server(host, port);

#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	//system("pause");
#endif

	return 0;
};
