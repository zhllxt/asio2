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

void https1_test()
{
	ASIO2_TEST_BEGIN_LOOP(test_loop_times);

	asio2::http_client http_client_bd;
	asio2::http_client http_client_qq;

	bool has_internet = http_client_bd.start("www.baidu.com", 80) || http_client_qq.start("qq.com", 80);

	if (has_internet)
	{
		asio::error_code ec;

		auto rep = asio2::https_client::execute("https://www.baidu.com");
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
		std::shared_ptr<asio2::socks5::option<asio2::socks5::method::anonymous>>
			sock5_option = std::make_shared<asio2::socks5::option<asio2::socks5::method::anonymous>>(
				"127.0.0.1", 10808);

		asio::error_code ec;

		auto rep = asio2::https_client::execute("https://www.baidu.com", sock5_option);
		ec = asio2::get_last_error();
		if (ec)
		{
			ASIO2_CHECK_VALUE(asio2::last_error_val(), asio2::get_last_error() == asio::error::connection_refused ||
				ec == asio::error::timed_out || ec == http::error::end_of_stream || ec == asio::ssl::error::stream_truncated);
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
		asio2::socks5::option<asio2::socks5::method::anonymous>
			sock5_option{ "127.0.0.1", 10808 };

		asio::error_code ec;

		auto rep = asio2::https_client::execute("https://www.baidu.com", sock5_option);
		ec = asio2::get_last_error();
		if (ec)
		{
			ASIO2_CHECK_VALUE(asio2::last_error_val(), asio2::get_last_error() == asio::error::connection_refused ||
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
	if (has_internet)
	{
		asio2::socks5::option<asio2::socks5::method::anonymous>
			sock5_option{ "127.0.0.1",10808 };
		std::string url = "https://www.baidu.com/img/flexible/logo/pc/result.png";
		std::string pth = "result.png";
		asio2::https_client::download(url, [](auto&) {}, [](std::string_view) { return true; });
		asio2::https_client::download(asio::ssl::context{ ASIO2_DEFAULT_SSL_METHOD }, url, [](auto&) {}, [](std::string_view) {return true; });
		asio2::https_client::download(url, [](std::string_view) {return true; });
		asio2::https_client::download(asio::ssl::context{ ASIO2_DEFAULT_SSL_METHOD }, url, [](std::string_view) {return true; });
		asio2::https_client::download(url, pth);
		asio2::https_client::download(asio::ssl::context{ ASIO2_DEFAULT_SSL_METHOD }, url, pth);
		auto req = http::make_request(url);
		asio2::https_client::download(asio::ssl::context{ ASIO2_DEFAULT_SSL_METHOD }, req.host(), req.port(), req, [](auto&) {}, [](std::string_view) {return true; }, nullptr);
		asio2::https_client::download(asio::ssl::context{ ASIO2_DEFAULT_SSL_METHOD }, req.host(), req.port(), req, [](auto&) {}, [](std::string_view) {return true; }, sock5_option);
	}

	ASIO2_TEST_END_LOOP;
}


ASIO2_TEST_SUITE
(
	"https1",
	ASIO2_TEST_CASE(https1_test)
)
