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

void https2_test()
{
	ASIO2_TEST_BEGIN_LOOP(test_loop_times);

	{
		asio2::https_server server;

		[[maybe_unused]] asio2::https_server server1(ASIO2_DEFAULT_SSL_METHOD);
		[[maybe_unused]] asio2::https_server server2(ASIO2_DEFAULT_SSL_METHOD, 512);
		[[maybe_unused]] asio2::https_server server3(512);
		[[maybe_unused]] asio2::https_server server4(server.io_ptr());

		server.support_websocket(true);

		// set the root directory, here is:  /asio2/example/wwwroot
		std::filesystem::path root = std::filesystem::current_path()
			.parent_path().parent_path()
			.append("example").append("wwwroot");
		server.set_root_directory(std::move(root));

		server.set_cert_file(
			"../../example/cert/ca.crt",
			"../../example/cert/server.crt",
			"../../example/cert/server.key",
			"123456");
		server.set_dh_file("../../example/cert/dh1024.pem");

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

			rep.fill_file("/index.html");
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

		asio::ssl::context ctx{ ASIO2_DEFAULT_SSL_METHOD };

		http::request_t<http::string_body> req1;
		asio2::https_client::execute(ctx, "127.0.0.1", 8443, req1, std::chrono::seconds(5));
		asio2::https_client::execute(ctx, "127.0.0.1", 8443, req1);

		asio2::https_client::execute("127.0.0.1", 8443, req1, std::chrono::seconds(5));
		asio2::https_client::execute("127.0.0.1", 8443, req1);

		asio2::https_client https_client;

		[[maybe_unused]] asio2::https_client https_client1(ASIO2_DEFAULT_SSL_METHOD);
		[[maybe_unused]] asio2::https_client https_client2(ASIO2_DEFAULT_SSL_METHOD, 512);
		[[maybe_unused]] asio2::https_client https_client3(512);
		[[maybe_unused]] asio2::https_client https_client4(https_client.io_ptr());

		//https_client.set_verify_mode(asio::ssl::verify_peer);
		https_client.set_cert_file(
			"../../example/cert/ca.crt",
			"../../example/cert/client.crt",
			"../../example/cert/client.key",
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
		ASIO2_CHECK_VALUE(asio2::last_error_val(), https_client_ret);

		asio2::wss_client wss_client;

		wss_client.set_connect_timeout(std::chrono::seconds(10));

		wss_client.set_verify_mode(asio::ssl::verify_peer);
		wss_client.set_cert_file(
			"../../example/cert/ca.crt",
			"../../example/cert/client.crt",
			"../../example/cert/client.key",
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

	ASIO2_TEST_END_LOOP;
}


ASIO2_TEST_SUITE
(
	"https2",
	ASIO2_TEST_CASE(https2_test)
)
