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

void http1_test()
{
	ASIO2_TEST_BEGIN_LOOP(test_loop_times);

	// test http url encode decode
	{
		std::string_view url = "http://www.baidu.com/query?key=x!#$&'()*,/:;=?@[ ]-_.~%^{}\"|<>`\\y";
		std::string_view dst = "http://www.baidu.com/query?key=x%21%23%24&%27%28%29*%2C/%3A%3B=?%40%5B%20%5D-_.%7E%25%5E%7B%7D%22%7C%3C%3E%60%5Cy";

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
		ASIO2_CHECK(req.query () == "key=x%21%23%24&%27%28%29*%2C/%3A%3B=?%40%5B%20%5D-_.%7E%25%5E%7B%7D%22%7C%3C%3E%60%5Cy");
		ASIO2_CHECK(req.target() == "/query?key=x%21%23%24&%27%28%29*%2C/%3A%3B=?%40%5B%20%5D-_.%7E%25%5E%7B%7D%22%7C%3C%3E%60%5Cy");

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
		std::string_view url = R"(http://www.baidu.com/childpathtest/json={"qeury":"name like '%abc%'","id":1})";
		std::string_view dst = R"(http://www.baidu.com/childpathtest/json=%7B%22qeury%22%3A%22name%20like%20%27%25abc%25%27%22%2C%22id%22%3A1%7D)";

		ASIO2_CHECK(http::url_encode(url) == dst);
		ASIO2_CHECK(http::url_decode(dst) == url);

		ASIO2_CHECK(asio2::http::url_to_path("/get_user?name=abc") == "/get_user");
		ASIO2_CHECK(asio2::http::url_to_query("/get_user?name=abc") == "name=abc");
	}

	// test http url encode decode
	{
		std::string_view url = "http://127.0.0.1/index.asp?id=x!#$&name='()*+,/:;=?@[ ]-_.~%^{}\"|<>`\\y";
		std::string_view dst = "http://127.0.0.1/index.asp?id=x%21%23%24&name=%27%28%29*%2B%2C/%3A%3B=?%40%5B%20%5D-_.%7E%25%5E%7B%7D%22%7C%3C%3E%60%5Cy";
		
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
		ASIO2_CHECK(req.get_query () == "id=x%21%23%24&name=%27%28%29*%2B%2C/%3A%3B=?%40%5B%20%5D-_.%7E%25%5E%7B%7D%22%7C%3C%3E%60%5Cy");
		ASIO2_CHECK(req.target() == "/index.asp?id=x%21%23%24&name=%27%28%29*%2B%2C/%3A%3B=?%40%5B%20%5D-_.%7E%25%5E%7B%7D%22%7C%3C%3E%60%5Cy");

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

	{
		http::multipart_fields mf;
		mf.set_boundary("--7115EB26E91C48FB99C067C640EB629C");

		http::multipart_field f1;
		f1.set_content_disposition("form-data");
		f1.set_name("filename");
		f1.set_content_transfer_encoding("binary");
		f1.set_value("header.png");
		mf.insert(std::move(f1));

		http::multipart_field f2;
		f2.set_content_disposition("form-data");
		f2.set_name("filesize");
		f2.set_content_transfer_encoding("binary");
		f2.set_value("1024");
		mf.insert(std::move(f2));

		http::multipart_field f3;
		f3.set_content_disposition("form-data");
		f3.set_name("filedata");
		f3.set_content_transfer_encoding("binary");
		f3.set_content_type("image/png");
		f3.set_filename("header.png");
		f3.set_value(".... eg: here is the file data ....");
		mf.insert(std::move(f3));

		auto mfstr = http::to_string(mf);

		http::web_request req;
		req.target("/update_header_img.html");
		req.method(http::verb::post);
		req.set(http::field::content_type, "multipart/form-data; boundary=--7115EB26E91C48FB99C067C640EB629C");
		req.body() = mfstr;
		req.prepare_payload();

		std::stringstream ss;
		ss << req;
		auto str = ss.str();

		ASIO2_CHECK(!str.empty());

		// or 
		http::basic_multipart_fields<std::string> mfs;
		http::basic_multipart_field<std::string> fs;

		asio2::ignore_unused(mfs, fs);

		// ......
	}

	ASIO2_TEST_END_LOOP;
}


ASIO2_TEST_SUITE
(
	"http1",
	ASIO2_TEST_CASE(http1_test)
)
