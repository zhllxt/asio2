#ifndef ASIO2_USE_SSL
#define ASIO2_USE_SSL
#endif

#include "unit_test.hpp"
#include <iostream>
#include <asio2/asio2.hpp>

struct aop_log
{
	bool before(http::web_request& req, http::web_response& rep)
	{
		asio2::ignore_unused(req, rep);
		return true;
	}
	bool after(std::shared_ptr<asio2::https_session>& session_ptr, http::web_request& req, http::web_response& rep)
	{
		asio2::ignore_unused(session_ptr, req, rep);
		return true;
	}
};

struct aop_check
{
	bool before(std::shared_ptr<asio2::https_session>& session_ptr, http::web_request& req, http::web_response& rep)
	{
		asio2::ignore_unused(session_ptr, req, rep);
		return true;
	}
	bool after(http::web_request& req, http::web_response& rep)
	{
		asio2::ignore_unused(req, rep);
		return true;
	}
};

void https_test()
{
	ASIO2_TEST_BEGIN_LOOP(test_loop_times);

	{
		asio::error_code ec;

		auto rep = asio2::https_client::execute("https://www.baidu.com");
		ec = asio2::get_last_error();
		if (ec)
		{
			ASIO2_CHECK_VALUE(asio2::last_error_msg(), asio2::get_last_error() == asio::error::connection_refused ||
				ec == asio::error::timed_out || ec == http::error::end_of_stream);
		}
		else
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(rep.version() == 11);
			ASIO2_CHECK(rep.result() == http::status::ok);
		}
	}

	{
		std::shared_ptr<asio2::socks5::option<asio2::socks5::method::anonymous>>
			sock5_option = std::make_shared<asio2::socks5::option<asio2::socks5::method::anonymous>>(
				"127.0.0.1", 10808);

		asio::error_code ec;

		auto rep = asio2::https_client::execute("https://www.baidu.com", sock5_option);
		ec = asio2::get_last_error();
		if (ec)
		{
			ASIO2_CHECK_VALUE(asio2::last_error_msg(), asio2::get_last_error() == asio::error::connection_refused ||
				ec == asio::error::timed_out || ec == http::error::end_of_stream || ec == asio::ssl::error::stream_truncated);
		}
		else
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(rep.version() == 11);
			ASIO2_CHECK(rep.result() == http::status::ok);
		}
	}

	{
		asio2::socks5::option<asio2::socks5::method::anonymous>
			sock5_option{ "127.0.0.1", 10808 };

		asio::error_code ec;

		auto rep = asio2::https_client::execute("https://www.baidu.com", sock5_option);
		ec = asio2::get_last_error();
		if (ec)
		{
			ASIO2_CHECK_VALUE(asio2::last_error_msg(), asio2::get_last_error() == asio::error::connection_refused ||
				ec == asio::error::timed_out || ec == http::error::end_of_stream || ec == asio::ssl::error::stream_truncated);
		}
		else
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(rep.version() == 11);
			ASIO2_CHECK(rep.result() == http::status::ok);
		}
	}

	// test https download
	{
		asio2::socks5::option<asio2::socks5::method::anonymous>
			sock5_option{ "127.0.0.1",10808 };
		std::string url = "https://www.baidu.com/img/flexible/logo/pc/result.png";
		std::string pth = "result.png";
		asio2::https_client::download(url, [](auto&) {}, [](std::string_view) {});
		asio2::https_client::download(asio::ssl::context{ asio::ssl::context::sslv23 }, url, [](auto&) {}, [](std::string_view) {});
		asio2::https_client::download(url, [](std::string_view) {});
		asio2::https_client::download(asio::ssl::context{ asio::ssl::context::sslv23 }, url, [](std::string_view) {});
		asio2::https_client::download(url, pth);
		asio2::https_client::download(asio::ssl::context{ asio::ssl::context::sslv23 }, url, pth);
		auto req = http::make_request(url);
		asio2::https_client::download(asio::ssl::context{ asio::ssl::context::sslv23 }, req.host(), req.port(), req, [](auto&) {}, [](std::string_view) {}, nullptr);
		asio2::https_client::download(asio::ssl::context{ asio::ssl::context::sslv23 }, req.host(), req.port(), req, [](auto&) {}, [](std::string_view) {}, sock5_option);
	}

	{
		asio2::https_server server;

		server.support_websocket(true);

		// set the root directory, here is:  /asio2/example/wwwroot
		std::filesystem::path root = std::filesystem::current_path()
			.parent_path().parent_path().parent_path()
			.append("example").append("wwwroot");
		server.set_root_directory(std::move(root));

		server.set_cert_file(
			"../../../example/cert/ca.crt",
			"../../../example/cert/server.crt",
			"../../../example/cert/server.key",
			"123456");
		server.set_dh_file("../../../example/cert/dh1024.pem");

		server.bind_accept([](std::shared_ptr<asio2::https_session> & session_ptr)
		{
			if (!asio2::get_last_error())
			{
				if (session_ptr->remote_address().find("192.168.10.") != std::string::npos)
					session_ptr->stop();
			}
		}).bind_start([&]()
		{
		}).bind_stop([&]()
		{
		});

		server.bind<http::verb::get, http::verb::post>("/", [](http::web_request& req, http::web_response& rep)
		{
			asio2::ignore_unused(req, rep);

			rep.fill_file("index.html");
		}, aop_log{});

		// If no method is specified, GET and POST are both enabled by default.
		server.bind("*", [](http::web_request& req, http::web_response& rep)
		{
			rep.fill_file(req.target());
		}, aop_check{});

		bool wss_open_flag = false, wss_close_flag = false;
		server.bind("/ws", websocket::listener<asio2::https_session>{}.
			on("message", [](std::shared_ptr<asio2::https_session>& session_ptr, std::string_view data)
		{
			session_ptr->async_send(data);
		}).on("open", [&](std::shared_ptr<asio2::https_session>& session_ptr)
		{
			wss_open_flag = true;
			asio2::ignore_unused(session_ptr);
		}).on("close", [&](std::shared_ptr<asio2::https_session>& session_ptr)
		{
			wss_close_flag = true;
			asio2::ignore_unused(session_ptr);
		}).on("ping", [](std::shared_ptr<asio2::https_session>& session_ptr, std::string_view data)
		{
			asio2::ignore_unused(session_ptr, data);
		}).on("pong", [](std::shared_ptr<asio2::https_session>& session_ptr, std::string_view data)
		{
			asio2::ignore_unused(session_ptr, data);
		}));

		server.bind_not_found([](/*std::shared_ptr<asio2::http_session>& session_ptr, */
			http::web_request& req, http::web_response& rep)
		{
			asio2::ignore_unused(req);

			rep.fill_page(http::status::not_found);
		});

		bool https_server_ret = server.start("127.0.0.1", 8443);
		ASIO2_CHECK(https_server_ret);

		asio::ssl::context ctx{ asio::ssl::context::sslv23 };

		http::request_t<http::string_body> req1;
		asio2::https_client::execute(ctx, "127.0.0.1", 8443, req1, std::chrono::seconds(5));
		asio2::https_client::execute(ctx, "127.0.0.1", 8443, req1);

		asio2::https_client::execute("127.0.0.1", 8443, req1, std::chrono::seconds(5));
		asio2::https_client::execute("127.0.0.1", 8443, req1);

		asio2::https_client https_client;
		//https_client.set_verify_mode(asio::ssl::verify_peer);
		https_client.set_cert_file(
			"../../../example/cert/ca.crt",
			"../../../example/cert/client.crt",
			"../../../example/cert/client.key",
			"123456");

		https_client.set_connect_timeout(std::chrono::seconds(10));

		https_client.bind_recv([&](http::web_request& req, http::web_response& rep)
		{
			asio2::ignore_unused(req, rep);

		}).bind_connect([&]()
		{
			// send a request
			https_client.async_send("GET / HTTP/1.1\r\n\r\n");

		}).bind_disconnect([]()
		{
		}).bind_handshake([&]()
		{
		});

		bool https_client_ret = https_client.start("127.0.0.1", 8443);
		ASIO2_CHECK_VALUE(asio2::last_error_msg(), https_client_ret);

		asio2::wss_client wss_client;

		wss_client.set_connect_timeout(std::chrono::seconds(10));

		wss_client.set_verify_mode(asio::ssl::verify_peer);
		wss_client.set_cert_file(
			"../../../example/cert/ca.crt",
			"../../../example/cert/client.crt",
			"../../../example/cert/client.key",
			"123456");

		wss_client.bind_init([&]()
		{
			// how to set custom websocket request data : 
			wss_client.ws_stream().set_option(websocket::stream_base::decorator(
				[](websocket::request_type& req)
			{
				req.set(http::field::authorization, " ssl-websocket-coro");
			}));
		}).bind_recv([&](std::string_view data)
		{
			wss_client.async_send(data);
		}).bind_connect([&]()
		{
			// a new thread.....
			std::thread([&]()
			{
				wss_client.post([&]()
				{
					ASIO2_CHECK(wss_client.io().running_in_this_thread());

					std::string str(std::size_t(100 + (std::rand() % 300)), char(std::rand() % 255));

					wss_client.async_send(std::move(str));
				});
			}).join();
		}).bind_upgrade([&]()
		{
			wss_client.async_send("abc", []()
			{
				ASIO2_CHECK(!asio2::get_last_error());
			});
		}).bind_disconnect([]()
		{
		});

		bool wss_client_ret = wss_client.start("127.0.0.1", 8443, "/ws");

		ASIO2_CHECK(wss_client_ret);

		ASIO2_CHECK(wss_open_flag);

		std::this_thread::sleep_for(std::chrono::milliseconds(50 + std::rand() % 50));

		wss_client.stop();

		server.stop();

		//ASIO2_CHECK(wss_close_flag);
	}

	// https client with socks5 option
	{
		asio2::socks5::option<asio2::socks5::method::anonymous>
			sock5_option{ "127.0.0.1", 10808 };
		//asio2::socks5::option<asio2::socks5::method::anonymous, asio2::socks5::method::password>
		//	sock5_option{ "s5.doudouip.cn",1088,"zjww-1","aaa123" };


		asio2::https_client https_client;

		int counter = 0;
		https_client.bind_recv([&](http::web_request& req, http::web_response& rep)
		{
			counter++;

			ASIO2_CHECK(rep.result() == http::status::ok);

			// Remove all fields
			req.clear();

			req.set(http::field::user_agent, "Chrome");
			req.set(http::field::content_type, "text/html");

			req.method(http::verb::get);
			req.keep_alive(true);
			req.target("/");
			req.body() = "Hello World.";
			req.prepare_payload();

			https_client.async_send(std::move(req));

		}).bind_connect([&]()
		{
			// connect success, send a request.
			if (!asio2::get_last_error())
			{
				const char * msg = "GET / HTTP/1.1\r\n\r\n";
				https_client.async_send(msg);
			}
		});

		bool http_client_ret = https_client.start("www.baidu.com", 443, std::move(sock5_option));
		ASIO2_CHECK(http_client_ret);
		while (http_client_ret && counter < 3)
		{
			ASIO2_TEST_WAIT_CHECK();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10 + std::rand() % 10));
	}

	// https client with socks5 option shared_ptr
	{
		std::shared_ptr<asio2::socks5::option<asio2::socks5::method::anonymous>>
			sock5_option = std::make_shared<asio2::socks5::option<asio2::socks5::method::anonymous>>(
				"127.0.0.1", 10808);
		//asio2::socks5::option<asio2::socks5::method::anonymous, asio2::socks5::method::password>
		//	sock5_option{ "s5.doudouip.cn",1088,"zjww-1","aaa123" };


		asio2::https_client https_client;

		int counter = 0;
		https_client.bind_recv([&](http::web_request& req, http::web_response& rep)
		{
			counter++;

			ASIO2_CHECK(rep.result() == http::status::ok);

			// Remove all fields
			req.clear();

			req.set(http::field::user_agent, "Chrome");
			req.set(http::field::content_type, "text/html");

			req.method(http::verb::get);
			req.keep_alive(true);
			req.target("/");
			req.prepare_payload();

			https_client.async_send(std::move(req));

		}).bind_connect([&, sock5_option]()
		{
			// connect success, send a request.
			if (!asio2::get_last_error())
			{
				const char * msg = "GET / HTTP/1.1\r\n\r\n";
				https_client.async_send(msg);
			}
			else
			{
				sock5_option->set_port(std::stoi(sock5_option->get_port()) + 1);
			}
		});

		bool http_client_ret = https_client.start("www.baidu.com", 443, std::move(sock5_option));
		ASIO2_CHECK(http_client_ret);
		while (http_client_ret && counter < 3)
		{
			ASIO2_TEST_WAIT_CHECK();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10 + std::rand() % 10));
	}

	ASIO2_TEST_END_LOOP;
}


ASIO2_TEST_SUITE
(
	"https",
	ASIO2_TEST_CASE(https_test)
)
