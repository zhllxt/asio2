#define ASIO2_ENABLE_HTTP_REQUEST_USER_DATA
#define ASIO2_ENABLE_HTTP_RESPONSE_USER_DATA

#include "unit_test.hpp"
#include <iostream>
#include <asio2/base/detail/filesystem.hpp>
#include <asio2/asio2.hpp>

struct aop_log
{
	bool before(http::web_request& req, http::web_response& rep)
	{
		asio2::ignore_unused(req, rep);
		return true;
	}
	bool after(std::shared_ptr<asio2::http_session>& session_ptr, http::web_request& req, http::web_response& rep)
	{
		asio2::ignore_unused(session_ptr, req, rep);
		return true;
	}
};

struct aop_check
{
	bool before(std::shared_ptr<asio2::http_session>& session_ptr, http::web_request& req, http::web_response& rep)
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

void http2_test()
{
	ASIO2_TEST_BEGIN_LOOP(test_loop_times);

	asio2::http_client http_client_bd;
	asio2::http_client http_client_qq;

	bool has_internet = http_client_bd.start("www.baidu.com", 80) || http_client_qq.start("qq.com", 80);

	// test http_client::execute
	if (has_internet)
	{
		std::string_view url = "http://www.baidu.com/query?x@y";

		asio2::socks5::option<asio2::socks5::method::anonymous>
			sock5_option{ "127.0.0.1",10808 };
		//asio2::socks5::option<asio2::socks5::method::anonymous, asio2::socks5::method::password>
		//	sock5_option{ "s5.doudouip.cn",1088,"zjww-1","aaa123" };

		asio::error_code ec;
		http::request_t<http::string_body> req1;

		auto rep = asio2::http_client::execute("www.baidu.com", "80", req1, std::chrono::seconds(5), sock5_option);
		ec = asio2::get_last_error();
		if (ec)
		{
			ASIO2_CHECK_VALUE(asio2::last_error_val(), asio2::get_last_error() == asio::error::connection_refused ||
				ec == asio::error::timed_out || ec == http::error::end_of_stream);
		}
		else
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(rep.version() == 11);
			ASIO2_CHECK(rep.result() == http::status::bad_request);
		}

		asio2::http_client::execute("www.baidu.com", "80", req1, std::chrono::seconds(5), nullptr);
		asio2::http_client::execute("www.baidu.com", "80", req1, std::chrono::seconds(5), std::in_place);
		asio2::http_client::execute("www.baidu.com", "80", req1, std::chrono::seconds(5), std::nullopt);

		rep = asio2::http_client::execute("www.baidu.com", "80", req1, std::chrono::seconds(5));
		ec = asio2::get_last_error();
		if (!ec)
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(rep.version() == 11);
			ASIO2_CHECK(rep.result() == http::status::bad_request);
		}

		rep = asio2::http_client::execute("www.baidu.com", "80", req1);
		ec = asio2::get_last_error();
		if (!ec)
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(rep.version() == 11);
			ASIO2_CHECK(rep.result() == http::status::bad_request);
		}

		http::web_request req2 = http::make_request(url);
		ec = asio2::get_last_error();
		ASIO2_CHECK(!asio2::get_last_error());

		rep = asio2::http_client::execute(req2, std::chrono::seconds(5));
		ec = asio2::get_last_error();
		if (!ec)
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(rep.version() == 11);
			ASIO2_CHECK(rep.result() == http::status::not_found);
		}

		rep = asio2::http_client::execute(req2);
		ec = asio2::get_last_error();
		if (!ec)
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(rep.version() == 11);
			ASIO2_CHECK(rep.result() == http::status::not_found);
		}

		url = "http://www.baidu.com";

		rep = asio2::http_client::execute(url, std::chrono::seconds(5));
		ec = asio2::get_last_error();
		if (!ec)
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(rep.version() == 11);
			ASIO2_CHECK(rep.result() == http::status::ok);
		}

		rep = asio2::http_client::execute(url);
		ec = asio2::get_last_error();
		if (!ec)
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(rep.version() == 11);
			ASIO2_CHECK(rep.result() == http::status::ok);
		}

		rep = asio2::http_client::execute("www.baidu.com", "80", "/", std::chrono::seconds(5));
		ec = asio2::get_last_error();
		if (!ec)
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(rep.version() == 11);
			ASIO2_CHECK(rep.result() == http::status::ok);
		}

		rep = asio2::http_client::execute("www.baidu.com", "80", "/");
		ec = asio2::get_last_error();
		if (!ec)
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(rep.version() == 11);
			ASIO2_CHECK(rep.result() == http::status::ok);
		}

		req1.target("/");
		req1.method(http::verb::get);
		rep = asio2::http_client::execute("www.baidu.com", "80", req1, sock5_option);
		ec = asio2::get_last_error();
		if (ec)
		{
			ASIO2_CHECK_VALUE(asio2::last_error_val(), asio2::get_last_error() == asio::error::connection_refused ||
				ec == asio::error::timed_out || ec == http::error::end_of_stream);
		}
		else
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(rep.version() == 11);
			ASIO2_CHECK_VALUE(rep.result(), rep.result() == http::status::ok || rep.result() == http::status::internal_server_error);
		}

		rep = asio2::http_client::execute(req2, std::chrono::seconds(5), sock5_option);
		ec = asio2::get_last_error();
		if (ec)
		{
			ASIO2_CHECK_VALUE(asio2::last_error_val(), asio2::get_last_error() == asio::error::connection_refused ||
				ec == asio::error::timed_out || ec == http::error::end_of_stream);
		}
		else
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(rep.version() == 11);
			ASIO2_CHECK(rep.result() == http::status::not_found);
		}
	
		rep = asio2::http_client::execute(req2, sock5_option);
		ec = asio2::get_last_error();
		if (ec)
		{
			ASIO2_CHECK_VALUE(asio2::last_error_val(), asio2::get_last_error() == asio::error::connection_refused ||
				ec == asio::error::timed_out || ec == http::error::end_of_stream);
		}
		else
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(rep.version() == 11);
			ASIO2_CHECK(rep.result() == http::status::not_found);
		}
		
		rep = asio2::http_client::execute(url, std::chrono::seconds(5), sock5_option);
		ec = asio2::get_last_error();
		if (ec)
		{
			ASIO2_CHECK_VALUE(asio2::last_error_val(), asio2::get_last_error() == asio::error::connection_refused ||
				ec == asio::error::timed_out || ec == http::error::end_of_stream);
		}
		else
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(rep.version() == 11);
			ASIO2_CHECK(rep.result() == http::status::ok);
		}
		
		rep = asio2::http_client::execute(url, sock5_option);
		ec = asio2::get_last_error();
		if (ec)
		{
			ASIO2_CHECK_VALUE(asio2::last_error_val(), asio2::get_last_error() == asio::error::connection_refused ||
				ec == asio::error::timed_out || ec == http::error::end_of_stream);
		}
		else
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(rep.version() == 11);
			ASIO2_CHECK(rep.result() == http::status::ok);
		}
		
		rep = asio2::http_client::execute("www.baidu.com", "80", "/", std::chrono::seconds(5), sock5_option);
		ec = asio2::get_last_error();
		if (ec)
		{
			ASIO2_CHECK_VALUE(asio2::last_error_val(), asio2::get_last_error() == asio::error::connection_refused ||
				ec == asio::error::timed_out || ec == http::error::end_of_stream);
		}
		else
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(rep.version() == 11);
			ASIO2_CHECK(rep.result() == http::status::ok);
		}
		
		rep = asio2::http_client::execute("www.baidu.com", "80", "/", sock5_option);
		ec = asio2::get_last_error();
		if (ec)
		{
			ASIO2_CHECK_VALUE(asio2::last_error_val(), asio2::get_last_error() == asio::error::connection_refused ||
				ec == asio::error::timed_out || ec == http::error::end_of_stream);
		}
		else
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(rep.version() == 11);
			ASIO2_CHECK(rep.result() == http::status::ok);
		}
	}

	// test http_client::execute
	if (has_internet)
	{
		std::shared_ptr<asio2::socks5::option<asio2::socks5::method::anonymous>>
			sock5_option = std::make_shared<asio2::socks5::option<asio2::socks5::method::anonymous>>(
				"127.0.0.1", 10808);
		//asio2::socks5::option<asio2::socks5::method::anonymous, asio2::socks5::method::password>
		//	sock5_option{ "s5.doudouip.cn",1088,"zjww-1","aaa123" };

		asio::error_code ec;
		http::request_t<http::string_body> req1;

		auto rep = asio2::http_client::execute("www.baidu.com", 80, req1, std::chrono::seconds(5), sock5_option);
		ec = asio2::get_last_error();
		if (ec)
		{
			ASIO2_CHECK_VALUE(asio2::last_error_val(), asio2::get_last_error() == asio::error::connection_refused ||
				ec == asio::error::timed_out || ec == http::error::end_of_stream);
		}
		else
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(rep.version() == 11);
			ASIO2_CHECK(rep.result() == http::status::bad_request);
		}

		rep = asio2::http_client::execute("www.baidu.com", 80, req1, std::chrono::seconds(5));
		ec = asio2::get_last_error();
		if (!ec)
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(rep.version() == 11);
			ASIO2_CHECK(rep.result() == http::status::bad_request);
		}

		rep = asio2::http_client::execute("www.baidu.com", 80, req1);
		ec = asio2::get_last_error();
		if (!ec)
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(rep.version() == 11);
			ASIO2_CHECK(rep.result() == http::status::bad_request);
		}

		rep = asio2::http_client::execute("www.baidu.com", 80, "/", std::chrono::seconds(5));
		ec = asio2::get_last_error();
		if (!ec)
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(rep.version() == 11);
			ASIO2_CHECK(rep.result() == http::status::ok);
		}

		rep = asio2::http_client::execute("www.baidu.com", 80, "/");
		ec = asio2::get_last_error();
		if (!ec)
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(rep.version() == 11);
			ASIO2_CHECK(rep.result() == http::status::ok);
		}

		req1.target("/");
		req1.method(http::verb::get);
		rep = asio2::http_client::execute("www.baidu.com", 80, req1, sock5_option);
		ec = asio2::get_last_error();
		if (ec)
		{
			ASIO2_CHECK_VALUE(asio2::last_error_val(), asio2::get_last_error() == asio::error::connection_refused ||
				ec == asio::error::timed_out || ec == http::error::end_of_stream);
		}
		else
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(rep.version() == 11);
			ASIO2_CHECK_VALUE(rep.result(), rep.result() == http::status::ok || rep.result() == http::status::internal_server_error);
		}

		rep = asio2::http_client::execute("www.baidu.com", 80, "/", std::chrono::seconds(5), sock5_option);
		ec = asio2::get_last_error();
		if (ec)
		{
			ASIO2_CHECK_VALUE(asio2::last_error_val(), asio2::get_last_error() == asio::error::connection_refused ||
				ec == asio::error::timed_out || ec == http::error::end_of_stream);
		}
		else
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(rep.version() == 11);
			ASIO2_CHECK(rep.result() == http::status::ok);
		}
		
		rep = asio2::http_client::execute("www.baidu.com", 80, "/", sock5_option);
		ec = asio2::get_last_error();
		if (ec)
		{
			ASIO2_CHECK_VALUE(asio2::last_error_val(), asio2::get_last_error() == asio::error::connection_refused ||
				ec == asio::error::timed_out || ec == http::error::end_of_stream);
		}
		else
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(rep.version() == 11);
			ASIO2_CHECK(rep.result() == http::status::ok);
		}
	}

	if (has_internet)
	{
		asio::error_code ec;

		// GET
		auto req2 = http::make_request("GET / HTTP/1.1\r\nHost: 192.168.0.1\r\n\r\n");
		ASIO2_CHECK(!asio2::get_last_error());
		ASIO2_CHECK(req2.method() == http::verb::get);
		ASIO2_CHECK(req2.version() == 11);
		ASIO2_CHECK(req2.at("Host") == "192.168.0.1");
		ASIO2_CHECK(req2.at(http::field::host) == "192.168.0.1");

		auto rep2 = asio2::http_client::execute("www.baidu.com", "80", req2, std::chrono::seconds(3));
		ec = asio2::get_last_error();
		if (!ec)
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(rep2.version() == 11);
			//ASIO2_CHECK_VALUE(rep2.result(), rep2.result() == http::status::forbidden);
		}

		// POST
		auto req4 = http::make_request("POST / HTTP/1.1\r\nHost: 192.168.0.1\r\n\r\n");
		ASIO2_CHECK(!asio2::get_last_error());
		ASIO2_CHECK(req4.method() == http::verb::post);
		ASIO2_CHECK(req4.version() == 11);
		ASIO2_CHECK(req4.at("Host") == "192.168.0.1");
		ASIO2_CHECK(req4.at(http::field::host) == "192.168.0.1");

		auto rep4 = asio2::http_client::execute("www.baidu.com", "80", req4, std::chrono::seconds(3));
		ec = asio2::get_last_error();
		if (!ec)
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(rep4.version() == 11);
			//ASIO2_CHECK_VALUE(rep4.result(), rep4.result() == http::status::forbidden);
		}

		// POST
		http::request_t<http::string_body> req5(http::verb::post, "/", 11);
		auto rep5 = asio2::http_client::execute("www.baidu.com", "80", req5);
		if (!ec)
		{
			ec = asio2::get_last_error();
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(rep5.version() == 11);
			//ASIO2_CHECK_VALUE(rep5.result(), rep5.result() == http::status::found);
		}

		// POST
		http::request_t<http::string_body> req6;
		req6.method(http::verb::post);
		req6.target("/");
		auto rep6 = asio2::http_client::execute("www.baidu.com", "80", req6);
		ec = asio2::get_last_error();
		if (!ec)
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(rep6.version() == 11);
			//ASIO2_CHECK_VALUE(rep6.result(), rep6.result() == http::status::found);
		}

		// POST
		http::request_t<http::string_body> req7;
		req7.method(http::verb::post);
		req7.target("/");
		req7.set(http::field::user_agent, "Chrome");
		req7.set(http::field::content_type, "text/html");
		req7.body() = "Hello World.";
		req7.prepare_payload();
		auto rep7 = asio2::http_client::execute("www.baidu.com", "80", req7);
		ec = asio2::get_last_error();
		if (!ec)
		{
			ASIO2_CHECK(std::distance(rep7.begin(), rep7.end()) != 0);
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(rep7.version() == 11);
			//ASIO2_CHECK_VALUE(rep7.result(), rep7.result() == http::status::found);

			// convert the response body to string
			std::stringstream ss1;
			ss1 << rep7.body();
			auto body = ss1.str();
			if (!body.empty()) { ASIO2_CHECK(!body.empty()); }
			if (!body.empty()) { ASIO2_CHECK(body.find("<html>") != std::string::npos); }

			// convert the whole response to string
			std::stringstream ss2;
			ss2 << rep7;
			auto text = ss2.str();
			ASIO2_CHECK(!text.empty());
			//ASIO2_CHECK(text.find("<html>") != std::string::npos);
			ASIO2_CHECK(text.find("HTTP/1.1") != std::string::npos);
		}
	}

	ASIO2_TEST_END_LOOP;
}


ASIO2_TEST_SUITE
(
	"http2",
	ASIO2_TEST_CASE(http2_test)
)
