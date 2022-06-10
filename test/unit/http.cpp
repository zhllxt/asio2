#include "unit_test.hpp"
#include <filesystem>
#include <iostream>
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

void http_test()
{
	ASIO2_TEST_BEGIN_LOOP(test_loop_times);

	// test http url encode decode
	{
		std::string_view url = "http://www.baidu.com/query?key=x!#$&'()*,/:;=?@[ ]-_.~%^{}\"|<>`\\y";
		std::string_view dst = "http://www.baidu.com/query?key=x%21%23%24&%27%28%29*%2C%2F%3A%3B=?%40%5B%20%5D-_.%7E%25%5E%7B%7D%22%7C%3C%3E%60%5Cy";

		ASIO2_CHECK(http::url_encode(url) == dst);
		ASIO2_CHECK(http::url_decode(dst) == url);

		ASIO2_CHECK(http::has_unencode_char(url) == true);
		ASIO2_CHECK(http::has_undecode_char(url) == false);
		ASIO2_CHECK(http::has_unencode_char(dst) == false);
		ASIO2_CHECK(http::has_undecode_char(dst) == true);

		http::web_request req = http::make_request(url);
		ASIO2_CHECK(!asio2::get_last_error());
		ASIO2_CHECK(http::has_multipart(req) == false);
		ASIO2_CHECK(req.has_multipart() == false);
		ASIO2_CHECK(req.is_upgrade() == false);

		ASIO2_CHECK(req.schema() == "http");
		ASIO2_CHECK(req.host  () == "www.baidu.com");
		ASIO2_CHECK(req.port  () == "80");
		ASIO2_CHECK(req.path  () == "/query");
		ASIO2_CHECK(req.query () == "key=x%21%23%24&%27%28%29*%2C%2F%3A%3B=?%40%5B%20%5D-_.%7E%25%5E%7B%7D%22%7C%3C%3E%60%5Cy");
		ASIO2_CHECK(req.target() == "/query?key=x%21%23%24&%27%28%29*%2C%2F%3A%3B=?%40%5B%20%5D-_.%7E%25%5E%7B%7D%22%7C%3C%3E%60%5Cy");

		ASIO2_CHECK(req.url().schema() == req.schema());
		ASIO2_CHECK(req.url().host  () == req.host  ());
		ASIO2_CHECK(req.url().port  () == req.port  ());
		ASIO2_CHECK(req.url().path  () == req.path  ());
		ASIO2_CHECK(req.url().query () == req.query ());
		ASIO2_CHECK(req.url().target() == req.target());

		ASIO2_CHECK(http::url_to_host (url) == "www.baidu.com");
		ASIO2_CHECK(http::url_to_port (url) == "80");
		ASIO2_CHECK(http::url_to_path (url) == "/query");
		// get the query with no encoded url will return a incorrect result
		//ASIO2_CHECK(http::url_to_query(url) == "key=x!#$&'()*,/:;=?@[ ]-_.~%^{}\"|<>`\\y");

		ASIO2_CHECK(http::url_to_host (dst) == req.host  ());
		ASIO2_CHECK(http::url_to_port (dst) == req.port  ());
		ASIO2_CHECK(http::url_to_path (dst) == req.path  ());
		ASIO2_CHECK(http::url_to_query(dst) == req.query ());
	}

	{
		std::string_view url = R"(http://www.baidu.com/json={"qeury":"name like '%abc%'","id":1})";
		std::string_view dst = R"(http://www.baidu.com/json=%7B%22qeury%22%3A%22name%20like%20%27%25abc%25%27%22%2C%22id%22%3A1%7D)";

		ASIO2_CHECK(http::url_encode(url) == dst);
		ASIO2_CHECK(http::url_decode(dst) == url);

		ASIO2_CHECK(asio2::http::url_to_path("/get_user?name=abc") == "/get_user");
		ASIO2_CHECK(asio2::http::url_to_query("/get_user?name=abc") == "name=abc");
	}

	// test http url encode decode
	{
		std::string_view url = "http://127.0.0.1/index.asp?id=x!#$&name='()*+,/:;=?@[ ]-_.~%^{}\"|<>`\\y";
		std::string_view dst = "http://127.0.0.1/index.asp?id=x%21%23%24&name=%27%28%29*%2B%2C%2F%3A%3B=?%40%5B%20%5D-_.%7E%25%5E%7B%7D%22%7C%3C%3E%60%5Cy";
		
		ASIO2_CHECK(http::url_encode(url) == dst);
		ASIO2_CHECK(http::url_decode(dst) == url);

		ASIO2_CHECK(http::has_unencode_char(url) == true);
		ASIO2_CHECK(http::has_undecode_char(url) == true); // "+"
		ASIO2_CHECK(http::has_unencode_char(dst) == false);
		ASIO2_CHECK(http::has_undecode_char(dst) == true);

		http::web_request req = http::make_request(url);
		ASIO2_CHECK(!asio2::get_last_error());

		ASIO2_CHECK(req.get_schema() == "http");
		ASIO2_CHECK(req.get_host  () == "127.0.0.1");
		ASIO2_CHECK(req.get_port  () == "80");
		ASIO2_CHECK(req.get_path  () == "/index.asp");
		ASIO2_CHECK(req.get_query () == "id=x%21%23%24&name=%27%28%29*%2B%2C%2F%3A%3B=?%40%5B%20%5D-_.%7E%25%5E%7B%7D%22%7C%3C%3E%60%5Cy");
		ASIO2_CHECK(req.target() == "/index.asp?id=x%21%23%24&name=%27%28%29*%2B%2C%2F%3A%3B=?%40%5B%20%5D-_.%7E%25%5E%7B%7D%22%7C%3C%3E%60%5Cy");

		ASIO2_CHECK(req.url().schema() == req.schema());
		ASIO2_CHECK(req.url().host  () == req.host  ());
		ASIO2_CHECK(req.url().port  () == req.port  ());
		ASIO2_CHECK(req.url().path  () == req.path  ());
		ASIO2_CHECK(req.url().query () == req.query ());
		ASIO2_CHECK(req.url().target() == req.target());

		ASIO2_CHECK(http::url_to_host (url) == "127.0.0.1");
		ASIO2_CHECK(http::url_to_port (url) == "80");
		ASIO2_CHECK(http::url_to_path (url) == "/index.asp");

		ASIO2_CHECK(http::url_to_host (dst) == req.host  ());
		ASIO2_CHECK(http::url_to_port (dst) == req.port  ());
		ASIO2_CHECK(http::url_to_path (dst) == req.path  ());
		ASIO2_CHECK(http::url_to_query(dst) == req.query ());
	}

	// test http url encode decode
	{
		std::string_view url = "https://github.com/search?q=utf8 user:zhllxt";
		std::string_view dst = "https://github.com/search?q=utf8%20user%3Azhllxt";

		ASIO2_CHECK(http::url_encode(url) == dst);
		ASIO2_CHECK(http::url_decode(dst) == url);

		ASIO2_CHECK(http::has_unencode_char(url) == true);
		ASIO2_CHECK(http::has_undecode_char(url) == false);
		ASIO2_CHECK(http::has_unencode_char(dst) == false);
		ASIO2_CHECK(http::has_undecode_char(dst) == true);

		http::web_request req = http::make_request(url);
		ASIO2_CHECK(!asio2::get_last_error());

		ASIO2_CHECK(req.schema() == "https");
		ASIO2_CHECK(req.host  () == "github.com");
		ASIO2_CHECK(req.port  () == "443");
		ASIO2_CHECK(req.path  () == "/search");
		ASIO2_CHECK(req.query () == "q=utf8%20user%3Azhllxt");
		ASIO2_CHECK(req.target() == "/search?q=utf8%20user%3Azhllxt");

		ASIO2_CHECK(req.url().schema() == req.schema());
		ASIO2_CHECK(req.url().host  () == req.host  ());
		ASIO2_CHECK(req.url().port  () == req.port  ());
		ASIO2_CHECK(req.url().path  () == req.path  ());
		ASIO2_CHECK(req.url().query () == req.query ());
		ASIO2_CHECK(req.url().target() == req.target());

		ASIO2_CHECK(http::url_to_host (url) == "github.com");
		ASIO2_CHECK(http::url_to_port (url) == "443");
		ASIO2_CHECK(http::url_to_path (url) == "/search");
		ASIO2_CHECK(http::url_to_query(url) == "q=utf8 user:zhllxt");

		ASIO2_CHECK(http::url_to_host (dst) == req.host  ());
		ASIO2_CHECK(http::url_to_port (dst) == req.port  ());
		ASIO2_CHECK(http::url_to_path (dst) == req.path  ());
		ASIO2_CHECK(http::url_to_query(dst) == req.query ());
	}

	// test http url encode decode
	{
		std::string_view url = "http://127.0.0.1:8080/search?q=c++ module user:zhllxt&s=f48fcb6f-f7d2-4ac0-b688-9931785f16bb";
		std::string_view dst = "http://127.0.0.1:8080/search?q=c%2B%2B%20module%20user%3Azhllxt&s=f48fcb6f-f7d2-4ac0-b688-9931785f16bb";
		
		ASIO2_CHECK(http::url_encode(url) == dst);
		ASIO2_CHECK(http::url_decode(dst) == url);

		ASIO2_CHECK(http::has_unencode_char(url) == true);
		ASIO2_CHECK(http::has_undecode_char(url) == true); // "+"
		ASIO2_CHECK(http::has_unencode_char(dst) == false);
		ASIO2_CHECK(http::has_undecode_char(dst) == true);

		http::web_request req = http::make_request(url);
		ASIO2_CHECK(!asio2::get_last_error());

		ASIO2_CHECK(req.schema() == "http");
		ASIO2_CHECK(req.host  () == "127.0.0.1");
		ASIO2_CHECK(req.port  () == "8080");
		ASIO2_CHECK(req.path  () == "/search");
		ASIO2_CHECK(req.query () == "q=c%2B%2B%20module%20user%3Azhllxt&s=f48fcb6f-f7d2-4ac0-b688-9931785f16bb");
		ASIO2_CHECK(req.target() == "/search?q=c%2B%2B%20module%20user%3Azhllxt&s=f48fcb6f-f7d2-4ac0-b688-9931785f16bb");

		ASIO2_CHECK(req.url().schema() == req.schema());
		ASIO2_CHECK(req.url().host  () == req.host  ());
		ASIO2_CHECK(req.url().port  () == req.port  ());
		ASIO2_CHECK(req.url().path  () == req.path  ());
		ASIO2_CHECK(req.url().query () == req.query ());
		ASIO2_CHECK(req.url().target() == req.target());

		ASIO2_CHECK(http::url_to_host (url) == "127.0.0.1");
		ASIO2_CHECK(http::url_to_port (url) == "8080");
		ASIO2_CHECK(http::url_to_path (url) == "/search");

		ASIO2_CHECK(http::url_to_host (dst) == req.host  ());
		ASIO2_CHECK(http::url_to_port (dst) == req.port  ());
		ASIO2_CHECK(http::url_to_path (dst) == req.path  ());
		ASIO2_CHECK(http::url_to_query(dst) == req.query ());
	}

	// test url match
	{
		ASIO2_CHECK(http::url_match("*", "/index.asp") == true);
		ASIO2_CHECK(http::url_match("/*", "/index.asp") == true);
		ASIO2_CHECK(http::url_match("/*", "index.asp") == true);
		ASIO2_CHECK(http::url_match("/index.asp", "/index.asp") == true);
		ASIO2_CHECK(http::url_match("/index.asp", "index.asp") == false);
		ASIO2_CHECK(http::url_match("index.asp", "/index.asp") == true);
		ASIO2_CHECK(http::url_match("index.asp", "/admin/index.asp") == true);
		ASIO2_CHECK(http::url_match("index.asp", "index.asp") == true);
		ASIO2_CHECK(http::url_match("/index*", "/index.asp") == true);
		ASIO2_CHECK(http::url_match("/index.*", "/index.asp") == true);
		ASIO2_CHECK(http::url_match("/index*", "index.asp") == false);
		ASIO2_CHECK(http::url_match("/index.*", "index.asp") == false);
		ASIO2_CHECK(http::url_match("/api/index*", "/index.asp") == false);
		ASIO2_CHECK(http::url_match("/api/index.*", "/index.asp") == false);
		ASIO2_CHECK(http::url_match("/api/index*", "/api/index.asp") == true);
		ASIO2_CHECK(http::url_match("/api/index.*", "/api/index.asp") == true);
		ASIO2_CHECK(http::url_match("/api/index*", "/api/index.") == true);
		ASIO2_CHECK(http::url_match("/api/index.*", "/api/index.") == true);
		ASIO2_CHECK(http::url_match("/api/index*", "/api/index/") == true);
		ASIO2_CHECK(http::url_match("/api/index.*", "/api/index/") == false);
		ASIO2_CHECK(http::url_match("/api/index*", "/api/index/admin") == true);
		ASIO2_CHECK(http::url_match("/api/index.*", "/api/index/admin") == false);
		ASIO2_CHECK(http::url_match("/api/index/*", "/api/index/admin") == true);
		ASIO2_CHECK(http::url_match("/api/index/*", "/api/index/admin/user") == true);
		ASIO2_CHECK(http::url_match("/api/index/*/user", "/api/index/admin/user") == true);
		ASIO2_CHECK(http::url_match("/api/index/*/user", "/api/index/admin/crud/user") == true);
		ASIO2_CHECK(http::url_match("/api/index/*/user", "/api/index/admin/person") == false);
		ASIO2_CHECK(http::url_match("/api/index/*/user", "/api/index/admin/crud/person") == false);
		ASIO2_CHECK(http::url_match("/*/api/index/user", "/admin/api/index/user") == true);
		ASIO2_CHECK(http::url_match("/*/api/index/user", "/admin/crud/api/index/user") == true);
		ASIO2_CHECK(http::url_match("/*/api/index/user", "/admin/api/index/person") == false);
		ASIO2_CHECK(http::url_match("/*/api/index/user", "/admin/crud/api/index/person") == false);
	}

	// test http_client::execute
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
			ASIO2_CHECK_VALUE(asio2::last_error_msg(), asio2::get_last_error() == asio::error::connection_refused ||
				ec == asio::error::timed_out || ec == http::error::end_of_stream);
		}
		else
		{
			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(rep.version() == 11);
			ASIO2_CHECK(rep.result() == http::status::bad_request);
		}

		rep = asio2::http_client::execute("www.baidu.com", "80", req1, std::chrono::seconds(5));
		ec = asio2::get_last_error();
		ASIO2_CHECK(!asio2::get_last_error());
		ASIO2_CHECK(rep.version() == 11);
		ASIO2_CHECK(rep.result() == http::status::bad_request);

		rep = asio2::http_client::execute("www.baidu.com", "80", req1);
		ec = asio2::get_last_error();
		ASIO2_CHECK(!asio2::get_last_error());
		ASIO2_CHECK(rep.version() == 11);
		ASIO2_CHECK(rep.result() == http::status::bad_request);

		http::web_request req2 = http::make_request(url);
		ec = asio2::get_last_error();
		ASIO2_CHECK(!asio2::get_last_error());

		rep = asio2::http_client::execute(req2, std::chrono::seconds(5));
		ec = asio2::get_last_error();
		ASIO2_CHECK(!asio2::get_last_error());
		ASIO2_CHECK(rep.version() == 11);
		ASIO2_CHECK(rep.result() == http::status::not_found);

		rep = asio2::http_client::execute(req2);
		ec = asio2::get_last_error();
		ASIO2_CHECK(!asio2::get_last_error());
		ASIO2_CHECK(rep.version() == 11);
		ASIO2_CHECK(rep.result() == http::status::not_found);

		url = "http://www.baidu.com";

		rep = asio2::http_client::execute(url, std::chrono::seconds(5));
		ec = asio2::get_last_error();
		ASIO2_CHECK(!asio2::get_last_error());
		ASIO2_CHECK(rep.version() == 11);
		ASIO2_CHECK(rep.result() == http::status::ok);

		rep = asio2::http_client::execute(url);
		ec = asio2::get_last_error();
		ASIO2_CHECK(!asio2::get_last_error());
		ASIO2_CHECK(rep.version() == 11);
		ASIO2_CHECK(rep.result() == http::status::ok);

		rep = asio2::http_client::execute("www.baidu.com", "80", "/", std::chrono::seconds(5));
		ec = asio2::get_last_error();
		ASIO2_CHECK(!asio2::get_last_error());
		ASIO2_CHECK(rep.version() == 11);
		ASIO2_CHECK(rep.result() == http::status::ok);

		rep = asio2::http_client::execute("www.baidu.com", "80", "/");
		ec = asio2::get_last_error();
		ASIO2_CHECK(!asio2::get_last_error());
		ASIO2_CHECK(rep.version() == 11);
		ASIO2_CHECK(rep.result() == http::status::ok);

		req1.target("/");
		req1.method(http::verb::get);
		rep = asio2::http_client::execute("www.baidu.com", "80", req1, sock5_option);
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

		rep = asio2::http_client::execute(req2, std::chrono::seconds(5), sock5_option);
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
			ASIO2_CHECK(rep.result() == http::status::not_found);
		}
	
		rep = asio2::http_client::execute(req2, sock5_option);
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
			ASIO2_CHECK(rep.result() == http::status::not_found);
		}
		
		rep = asio2::http_client::execute(url, std::chrono::seconds(5), sock5_option);
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
		
		rep = asio2::http_client::execute(url, sock5_option);
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
		
		rep = asio2::http_client::execute("www.baidu.com", "80", "/", std::chrono::seconds(5), sock5_option);
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
		
		rep = asio2::http_client::execute("www.baidu.com", "80", "/", sock5_option);
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

	// test http_client::execute
	{
		asio2::socks5::option<asio2::socks5::method::anonymous>
			sock5_option{ "127.0.0.1",10808 };
		//asio2::socks5::option<asio2::socks5::method::anonymous, asio2::socks5::method::password>
		//	sock5_option{ "s5.doudouip.cn",1088,"zjww-1","aaa123" };

		asio::error_code ec;
		http::request_t<http::string_body> req1;

		auto rep = asio2::http_client::execute("www.baidu.com", 80, req1, std::chrono::seconds(5), sock5_option);
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
			ASIO2_CHECK(rep.result() == http::status::bad_request);
		}

		rep = asio2::http_client::execute("www.baidu.com", 80, req1, std::chrono::seconds(5));
		ec = asio2::get_last_error();
		ASIO2_CHECK(!asio2::get_last_error());
		ASIO2_CHECK(rep.version() == 11);
		ASIO2_CHECK(rep.result() == http::status::bad_request);

		rep = asio2::http_client::execute("www.baidu.com", 80, req1);
		ec = asio2::get_last_error();
		ASIO2_CHECK(!asio2::get_last_error());
		ASIO2_CHECK(rep.version() == 11);
		ASIO2_CHECK(rep.result() == http::status::bad_request);

		rep = asio2::http_client::execute("www.baidu.com", 80, "/", std::chrono::seconds(5));
		ec = asio2::get_last_error();
		ASIO2_CHECK(!asio2::get_last_error());
		ASIO2_CHECK(rep.version() == 11);
		ASIO2_CHECK(rep.result() == http::status::ok);

		rep = asio2::http_client::execute("www.baidu.com", 80, "/");
		ec = asio2::get_last_error();
		ASIO2_CHECK(!asio2::get_last_error());
		ASIO2_CHECK(rep.version() == 11);
		ASIO2_CHECK(rep.result() == http::status::ok);

		req1.target("/");
		req1.method(http::verb::get);
		rep = asio2::http_client::execute("www.baidu.com", 80, req1, sock5_option);
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

		rep = asio2::http_client::execute("www.baidu.com", 80, "/", std::chrono::seconds(5), sock5_option);
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
		
		rep = asio2::http_client::execute("www.baidu.com", 80, "/", sock5_option);
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
		ASIO2_CHECK(!asio2::get_last_error());
		ASIO2_CHECK(rep2.version() == 11);
		ASIO2_CHECK(rep2.result() == http::status::forbidden);

		// POST
		auto req4 = http::make_request("POST / HTTP/1.1\r\nHost: 192.168.0.1\r\n\r\n");
		ASIO2_CHECK(!asio2::get_last_error());
		ASIO2_CHECK(req4.method() == http::verb::post);
		ASIO2_CHECK(req4.version() == 11);
		ASIO2_CHECK(req4.at("Host") == "192.168.0.1");
		ASIO2_CHECK(req4.at(http::field::host) == "192.168.0.1");

		auto rep4 = asio2::http_client::execute("www.baidu.com", "80", req4, std::chrono::seconds(3));
		ec = asio2::get_last_error();
		ASIO2_CHECK(!asio2::get_last_error());
		ASIO2_CHECK(rep4.version() == 11);
		ASIO2_CHECK(rep4.result() == http::status::forbidden);

		// POST
		http::request_t<http::string_body> req5(http::verb::post, "/", 11);
		auto rep5 = asio2::http_client::execute("www.baidu.com", "80", req5);
		ec = asio2::get_last_error();
		ASIO2_CHECK(!asio2::get_last_error());
		ASIO2_CHECK(rep5.version() == 11);
		ASIO2_CHECK(rep5.result() == http::status::found);

		// POST
		http::request_t<http::string_body> req6;
		req6.method(http::verb::post);
		req6.target("/");
		auto rep6 = asio2::http_client::execute("www.baidu.com", "80", req6);
		ec = asio2::get_last_error();
		ASIO2_CHECK(!asio2::get_last_error());
		ASIO2_CHECK(rep6.version() == 11);
		ASIO2_CHECK(rep6.result() == http::status::found);

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
		ASIO2_CHECK(std::distance(rep7.begin(), rep7.end()) != 0);
		ASIO2_CHECK(!asio2::get_last_error());
		ASIO2_CHECK(rep7.version() == 11);
		ASIO2_CHECK(rep7.result() == http::status::found);

		// convert the response body to string
		std::stringstream ss1;
		ss1 << rep7.body();
		auto body = ss1.str();
		ASIO2_CHECK(!body.empty());
		ASIO2_CHECK(body.find("<html>") != std::string::npos);

		// convert the whole response to string
		std::stringstream ss2;
		ss2 << rep7;
		auto text = ss2.str();
		ASIO2_CHECK(!text.empty());
		ASIO2_CHECK(text.find("<html>") != std::string::npos);
		ASIO2_CHECK(text.find("HTTP/1.1") != std::string::npos);
	}

	{
		asio2::http_server server;

		server.support_websocket(true);

		// set the root directory, here is:  /asio2/example/wwwroot
		std::filesystem::path root = std::filesystem::current_path()
			.parent_path().parent_path().parent_path()
			.append("example").append("wwwroot");
		server.set_root_directory(std::move(root));

		server.bind_recv([&](http::web_request& req, http::web_response& rep)
		{
			asio2::ignore_unused(req, rep);
		}).bind_connect([](auto & session_ptr)
		{
			asio2::ignore_unused(session_ptr);
			//session_ptr->set_response_mode(asio2::response_mode::manual);
		}).bind_disconnect([](auto & session_ptr)
		{
			asio2::ignore_unused(session_ptr);
		}).bind_start([&]()
		{
		}).bind_stop([&]()
		{
		});

		server.bind<http::verb::get, http::verb::post>("/", [](http::web_request& req, http::web_response& rep)
		{
			asio2::ignore_unused(req, rep);

			rep.fill_file("index.html");
			rep.chunked(true);

		}, aop_log{});

		// If no method is specified, GET and POST are both enabled by default.
		server.bind("*", [](http::web_request& req, http::web_response& rep)
		{
			rep.fill_file(req.target());
		}, aop_check{});

		// Test http multipart
		server.bind<http::verb::get, http::verb::post>("/multipart", [](http::web_request& req, http::web_response& rep)
		{
			auto& body = req.body();
			auto target = req.target();

			std::stringstream ss;
			ss << req;
			auto str_req = ss.str();

			asio2::ignore_unused(body, target, str_req);

			for (auto it = req.begin(); it != req.end(); ++it)
			{
			}

			auto mpbody = req.multipart();

			for (auto it = mpbody.begin(); it != mpbody.end(); ++it)
			{
			}

			if (req.has_multipart())
			{
				http::multipart_fields multipart_body = req.multipart();

				auto& field_username = multipart_body["username"];
				auto username = field_username.value();

				auto& field_password = multipart_body["password"];
				auto password = field_password.value();

				std::string str = http::to_string(multipart_body);

				std::string type = "multipart/form-data; boundary="; type += multipart_body.boundary();

				rep.fill_text(str, http::status::ok, type);

				http::request_t<http::string_body> re;
				re.method(http::verb::post);
				re.set(http::field::content_type, type);
				re.keep_alive(true);
				re.target("/api/user/");
				re.body() = str;
				re.prepare_payload();

				std::stringstream ress;
				ress << re;
				auto restr = ress.str();

				asio2::ignore_unused(username, password, restr);
			}
			else
			{
				rep.fill_page(http::status::ok);
			}
		}, aop_log{});

		server.bind<http::verb::get>("/del_user",
			[](std::shared_ptr<asio2::http_session>& session_ptr, http::web_request& req, http::web_response& rep)
		{
			asio2::ignore_unused(session_ptr, req, rep);

			rep.fill_page(http::status::ok, "del_user successed.");

		}, aop_check{});

		server.bind<http::verb::get, http::verb::post>("/api/user/*", [](http::web_request& req, http::web_response& rep)
		{
			asio2::ignore_unused(req, rep);

			//rep.fill_text("the user name is hanmeimei, .....");

		}, aop_log{}, aop_check{});

		server.bind<http::verb::get>("/defer", [](http::web_request& req, http::web_response& rep)
		{
			asio2::ignore_unused(req, rep);

			// use defer to make the reponse not send immediately, util the derfer shared_ptr
			// is destroyed, then the response will be sent.
			std::shared_ptr<http::response_defer> rep_defer = rep.defer();

			std::thread([rep_defer, &rep]() mutable
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(50));

				rep = asio2::http_client::execute("http://www.baidu.com");
			}).detach();

		}, aop_log{}, aop_check{});

		bool ws_open_flag = false, ws_close_flag = false;
		server.bind("/ws", websocket::listener<asio2::http_session>{}.
			on("message", [](std::shared_ptr<asio2::http_session>& session_ptr, std::string_view data)
		{
			session_ptr->async_send(data);

		}).on("open", [&](std::shared_ptr<asio2::http_session>& session_ptr)
		{
			ws_open_flag = true;
			session_ptr->post([]() {}, std::chrono::milliseconds(10));
			// how to set custom websocket response data : 
			session_ptr->ws_stream().set_option(websocket::stream_base::decorator(
				[](websocket::response_type& rep)
			{
				rep.set(http::field::authorization, " http-server-coro");
			}));

		}).on("close", [&](std::shared_ptr<asio2::http_session>& session_ptr)
		{
			ws_close_flag = true;
			asio2::ignore_unused(session_ptr);
		}).on_ping([](std::shared_ptr<asio2::http_session>& session_ptr)
		{
			asio2::ignore_unused(session_ptr);
		}).on_pong([](std::shared_ptr<asio2::http_session>& session_ptr)
		{
			asio2::ignore_unused(session_ptr);
		}));

		server.bind_not_found([](http::web_request& req, http::web_response& rep)
		{
			asio2::ignore_unused(req);

			rep.fill_page(http::status::not_found);
		});

		server.start("127.0.0.1", 8080);

		asio2::http_client::execute("127.0.0.1", "8080", "/", std::chrono::seconds(5));

		asio2::http_client::execute("127.0.0.1", "8080", "/defer");

		asio2::http_client http_client;

		http_client.bind_recv([&](http::web_request& req, http::web_response& rep)
		{
			// convert the response body to string
			std::stringstream ss;
			ss << rep.body();

			// Remove all fields
			req.clear();

			req.set(http::field::user_agent, "Chrome");
			req.set(http::field::content_type, "text/html");

			req.method(http::verb::get);
			req.keep_alive(true);
			req.target("/get_user?name=abc");
			req.body() = "Hello World.";
			req.prepare_payload();

			http_client.async_send(std::move(req));

		}).bind_connect([&]()
		{
			// connect success, send a request.
			if (!asio2::get_last_error())
			{
				const char * msg = "GET / HTTP/1.1\r\n\r\n";
				http_client.async_send(msg);
			}
		});

		bool http_client_ret = http_client.start("127.0.0.1", 8080);
		ASIO2_CHECK(http_client_ret);

		asio2::ws_client ws_client;

		ws_client.set_connect_timeout(std::chrono::seconds(5));

		ws_client.bind_init([&]()
		{
			// how to set custom websocket request data : 
			ws_client.ws_stream().set_option(websocket::stream_base::decorator(
				[](websocket::request_type& req)
			{
				req.set(http::field::authorization, " websocket-authorization");
			}));

		}).bind_connect([&]()
		{
			std::string s;
			s += '<';
			int len = 128 + std::rand() % 512;
			for (int i = 0; i < len; i++)
			{
				s += (char)((std::rand() % 26) + 'a');
			}
			s += '>';

			ws_client.async_send(std::move(s));

		}).bind_upgrade([&]()
		{
			// this send will be failed, because connection is not fully completed
			ws_client.async_send("abc", [](std::size_t bytes)
			{
				ASIO2_CHECK(asio2::get_last_error() == asio::error::not_connected);
				ASIO2_CHECK(bytes == 0);
			});

		}).bind_recv([&](std::string_view data)
		{
			ws_client.async_send(data);
		});

		bool ws_client_ret = ws_client.start("127.0.0.1", 8080, "/ws");

		ASIO2_CHECK(ws_client_ret);

		ASIO2_CHECK(ws_open_flag);

		std::this_thread::sleep_for(std::chrono::milliseconds(50 + std::rand() % 50));

		ws_client.stop();
		ASIO2_CHECK(ws_close_flag);
	}

	{
		// try open http://localhost:8080 in your browser
		asio2::http_server server;

		server.set_support_websocket(true);

		// set the root directory, here is:  /asio2/example/wwwroot
		std::filesystem::path root = std::filesystem::current_path().parent_path().parent_path().append("wwwroot");
		server.set_root_directory(std::move(root));

		server.bind_recv([&](http::web_request& req, http::web_response& rep)
		{
			asio2::ignore_unused(req, rep);
			// all http and websocket request will goto here first.
		}).bind_connect([](auto & session_ptr)
		{
			asio2::ignore_unused(session_ptr);
			//session_ptr->set_response_mode(asio2::response_mode::manual);
		}).bind_disconnect([](auto & session_ptr)
		{
			asio2::ignore_unused(session_ptr);
		}).bind_start([&]()
		{
		}).bind_stop([&]()
		{
		});

		server.bind<http::verb::get, http::verb::post>("/", [](http::web_request& req, http::web_response& rep)
		{
			asio2::ignore_unused(req, rep);

			rep.fill_file("index.html");
			rep.chunked(true);

		}, aop_log{});

		// If no method is specified, GET and POST are both enabled by default.
		server.bind("*", [](http::web_request& req, http::web_response& rep)
		{
			rep.fill_file(req.target());
		}, aop_check{});

		// Test http multipart
		server.bind<http::verb::get, http::verb::post>("/multipart", [](http::web_request& req, http::web_response& rep)
		{
			auto& body = req.body();
			auto target = req.target();

			std::stringstream ss;
			ss << req;
			auto str_req = ss.str();

			asio2::ignore_unused(body, target, str_req);

			for (auto it = req.begin(); it != req.end(); ++it)
			{
				//std::cout << it->name_string() << " " << it->value() << std::endl;
			}

			auto mpbody = req.multipart();

			for (auto it = mpbody.begin(); it != mpbody.end(); ++it)
			{
				//std::cout
				//	<< "filename:" << it->filename() << " name:" << it->name()
				//	<< " content_type:" << it->content_type() << std::endl;
			}

			if (req.has_multipart())
			{
				http::multipart_fields multipart_body = req.multipart();

				auto& field_username = multipart_body["username"];
				auto username = field_username.value();

				auto& field_password = multipart_body["password"];
				auto password = field_password.value();

				std::string str = http::to_string(multipart_body);

				std::string type = "multipart/form-data; boundary="; type += multipart_body.boundary();

				rep.fill_text(str, http::status::ok, type);

				http::request_t<http::string_body> re;
				re.method(http::verb::post);
				re.set(http::field::content_type, type);
				re.keep_alive(true);
				re.target("/api/user/");
				re.body() = str;
				re.prepare_payload();

				std::stringstream ress;
				ress << re;
				auto restr = ress.str();

				asio2::ignore_unused(username, password, restr);
			}
			else
			{
				rep.fill_page(http::status::ok);
			}
		}, aop_log{});

		server.bind<http::verb::get>("/del_user",
			[](std::shared_ptr<asio2::http_session>& session_ptr, http::web_request& req, http::web_response& rep)
		{
			asio2::ignore_unused(session_ptr, req, rep);

			rep.fill_page(http::status::ok, "del_user successed.");

		}, aop_check{});

		server.bind<http::verb::get, http::verb::post>("/api/user/*", [](http::web_request& req, http::web_response& rep)
		{
			asio2::ignore_unused(req, rep);

			//rep.fill_text("the user name is hanmeimei, .....");

		}, aop_log{}, aop_check{});

		server.bind<http::verb::get>("/defer", [](http::web_request& req, http::web_response& rep)
		{
			asio2::ignore_unused(req, rep);

			// use defer to make the reponse not send immediately, util the derfer shared_ptr
			// is destroyed, then the response will be sent.
			std::shared_ptr<http::response_defer> rep_defer = rep.defer();

			std::thread([rep_defer, &rep]() mutable
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));

				rep = asio2::http_client::execute("http://www.baidu.com");
			}).detach();

		}, aop_log{}, aop_check{});

		// the /ws is the websocket upgraged target
		server.bind("/ws", websocket::listener<asio2::http_session>{}.
			on("message", [](std::shared_ptr<asio2::http_session>& session_ptr, std::string_view data)
		{
			session_ptr->async_send(data);

		}).on("open", [](std::shared_ptr<asio2::http_session>& session_ptr)
		{
			// print the websocket request header.
			//std::cout << session_ptr->request() << std::endl;

			// Set the binary message write option.
			session_ptr->ws_stream().binary(true);

			// Set the text message write option.
			//session_ptr->ws_stream().text(true);

			// how to set custom websocket response data : 
			session_ptr->ws_stream().set_option(websocket::stream_base::decorator(
				[](websocket::response_type& rep)
			{
				rep.set(http::field::authorization, " http-server-coro");
			}));

		}).on("close", [](std::shared_ptr<asio2::http_session>& session_ptr)
		{
			asio2::ignore_unused(session_ptr);

		}).on_ping([](std::shared_ptr<asio2::http_session>& session_ptr)
		{
			asio2::ignore_unused(session_ptr);

		}).on_pong([](std::shared_ptr<asio2::http_session>& session_ptr)
		{
			asio2::ignore_unused(session_ptr);

		}));

		server.bind_not_found([](http::web_request& req, http::web_response& rep)
		{
			asio2::ignore_unused(req);

			rep.fill_page(http::status::not_found);
		});

		server.start("0.0.0.0", 8080);

		//-----------------------------------------------------------------------------------------

		asio2::iopool iopool(4);

		iopool.start();

		std::vector<std::shared_ptr<asio2::ws_client>> ws_clients;

		std::atomic<int> client_connect_counter = 0;

		for (int i = 0; i < test_client_count; i++)
		{
			std::shared_ptr<asio2::ws_client> client_ptr =
				std::make_shared<asio2::ws_client>(iopool.get(i % iopool.size()));

			ws_clients.emplace_back(client_ptr);

			asio2::ws_client& client = *client_ptr;

			client.set_connect_timeout(std::chrono::seconds(5));
			client.set_auto_reconnect(false);

			client.bind_init([&]()
			{
				// Set the binary message write option.
				client.ws_stream().binary(true);

				// Set the text message write option.
				//client.ws_stream().text(true);

				// how to set custom websocket request data : 
				client.ws_stream().set_option(websocket::stream_base::decorator(
					[](websocket::request_type& req)
				{
					req.set(http::field::authorization, " websocket-client-authorization");
				}));

			}).bind_connect([&]()
			{
				if (asio2::get_last_error())
				{
				}
				else
				{
					client_connect_counter++;
				}

				client.start_timer(1, 100, [&]()
				{
					client.async_send("<0123456789abcdefg0123456789abcdefg0123456789abcdefg0123456789abcdefg>");
				});
				//std::string s;
				//s += '<';
				//int len = 128 + std::rand() % 512;
				//for (int i = 0; i < len; i++)
				//{
				//	s += (char)((std::rand() % 26) + 'a');
				//}
				//s += '>';

				//client.async_send(std::move(s), [](std::size_t bytes_sent) { std::ignore = bytes_sent; });

			}).bind_upgrade([&]()
			{
				//if (asio2::get_last_error())
				//	std::cout << "upgrade failure : " << asio2::last_error_val() << " " << asio2::last_error_msg() << std::endl;
				//else
				//	std::cout << "upgrade success : " << client.get_upgrade_response() << std::endl;

				// this send will be failed, because connection is not fully completed
				//client.async_send("abc", []()
				//{
				//	ASIO2_CHECK(asio2::get_last_error());
				//	std::cout << "send failed : " << asio2::last_error_msg() << std::endl;
				//});

			}).bind_recv([&](std::string_view data)
			{
				ASIO2_CHECK(!data.empty());
				//client.async_send(data);
			}).bind_disconnect([&]()
			{
				ASIO2_CHECK(asio2::get_last_error());
				client_connect_counter--;
			});

			// the /ws is the websocket upgraged target
			bool ws_client_ret = client.start("127.0.0.1", 8080, "/ws");
			ASIO2_CHECK(ws_client_ret);
			ASIO2_CHECK(!asio2::get_last_error());
		}

		while (server.get_session_count() < std::size_t(test_client_count))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		while (client_connect_counter < test_client_count)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		auto session_count = server.get_session_count();
		ASIO2_CHECK_VALUE(session_count, session_count == std::size_t(test_client_count));

		ASIO2_CHECK_VALUE(client_connect_counter.load(), client_connect_counter == test_client_count);

		asio2::timer timer(iopool.get(loop % iopool.size()));
		timer.start_timer(1, 10, 1, [&]()
		{
			ASIO2_CHECK(timer.get_thread_id() == std::this_thread::get_id());
			for (auto& client_ptr : ws_clients)
			{
				// if the client.stop function is not called in the client's io_context thread,
				// the stop is async, it is not blocking, and will return immediately. 
				client_ptr->stop();

				// if the client.stop function is not called in the client's io_context thread,
				// after client.stop, the client must be stopped completed already.
				if (client_ptr->get_thread_id() != std::this_thread::get_id())
				{
					ASIO2_CHECK(client_ptr->is_stopped());
				}

				// at here, the client state maybe not stopped, beacuse the stop maybe async.
				// but this async_start event chain must will be executed after all stop event
				// chain is completed. so when the async_start's push_event is executed, the
				// client must be stopped already.
				client_ptr->async_start("127.0.0.1", 8080, "/ws");
			}
		});

		while (timer.is_timer_exists(1))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		while (server.get_session_count() < std::size_t(test_client_count))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		while (client_connect_counter < test_client_count)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		session_count = server.get_session_count();
		ASIO2_CHECK_VALUE(session_count, session_count == std::size_t(test_client_count));

		ASIO2_CHECK_VALUE(client_connect_counter.load(), client_connect_counter == test_client_count);

		timer.stop();
		iopool.stop();
	}

	ASIO2_TEST_END_LOOP;
}


ASIO2_TEST_SUITE
(
	"http",
	ASIO2_TEST_CASE(http_test)
)
