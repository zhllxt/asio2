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

void https3_test()
{
	ASIO2_TEST_BEGIN_LOOP(test_loop_times);

	asio2::http_client http_client_bd;
	asio2::http_client http_client_qq;

	bool has_internet = http_client_bd.start("www.baidu.com", 80) || http_client_qq.start("qq.com", 80);

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

		if (has_internet)
		{
			bool http_client_ret = https_client.start("www.baidu.com", 443, std::move(sock5_option));
			if (std::filesystem::exists("../../.CMakeBuild.cmd") ||
				std::filesystem::exists("../../.CMakeGenerate.cmd") ||
				std::filesystem::exists("../../.CMakeTest.cmd"))
			{
				ASIO2_CHECK(http_client_ret);
			}
			while (http_client_ret && counter < 3)
			{
				ASIO2_TEST_WAIT_CHECK();
			}
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

		if (has_internet)
		{
			bool http_client_ret = https_client.start("www.baidu.com", 443, std::move(sock5_option));
			if (std::filesystem::exists("../../.CMakeBuild.cmd") ||
				std::filesystem::exists("../../.CMakeGenerate.cmd") ||
				std::filesystem::exists("../../.CMakeTest.cmd"))
			{
				ASIO2_CHECK(http_client_ret);
			}
			while (http_client_ret && counter < 3)
			{
				ASIO2_TEST_WAIT_CHECK();
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10 + std::rand() % 10));
	}

	ASIO2_TEST_END_LOOP;
}


ASIO2_TEST_SUITE
(
	"https3",
	ASIO2_TEST_CASE(https3_test)
)
