#include <filesystem>
#include <iostream>
#include <asio2/asio2.hpp>

struct aop_log
{
	bool before(http::web_request& req, http::web_response& rep)
	{
		asio2::ignore_unused(rep);
		printf("aop_log before %s\n", req.method_string().data());
		return true;
	}
	bool after(std::shared_ptr<asio2::http_session>& session_ptr, http::web_request& req, http::web_response& rep)
	{
		asio2::ignore_unused(session_ptr, req, rep);
		printf("aop_log after\n");
		return true;
	}
};

struct aop_check
{
	bool before(std::shared_ptr<asio2::http_session>& session_ptr, http::web_request& req, http::web_response& rep)
	{
		asio2::ignore_unused(session_ptr, req, rep);
		printf("aop_check before\n");
		return true;
	}
	bool after(http::web_request& req, http::web_response& rep)
	{
		asio2::ignore_unused(req, rep);
		printf("aop_check after\n");
		return true;
	}
};

int main()
{
	// try open http://localhost:8080 in your browser
	std::string_view host = "0.0.0.0";
	std::string_view port = "8080";

	asio2::http_server server;

	server.support_websocket(true);

	// set the root directory, here is:  /asio2/example/wwwroot
	std::filesystem::path root = std::filesystem::current_path().parent_path().parent_path().append("wwwroot");
	server.set_root_directory(std::move(root));

	server.bind_recv([&](http::web_request& req, http::web_response& rep)
	{
		asio2::ignore_unused(req, rep);
		// all http and websocket request will goto here first.
		std::cout << req.path() << req.query() << std::endl;
	}).bind_connect([](auto & session_ptr)
	{
		printf("client enter : %s %u %s %u\n",
			session_ptr->remote_address().c_str(), session_ptr->remote_port(),
			session_ptr->local_address().c_str(), session_ptr->local_port());
		//session_ptr->set_response_mode(asio2::response_mode::manual);
	}).bind_disconnect([](auto & session_ptr)
	{
		printf("client leave : %s %u %s\n",
			session_ptr->remote_address().c_str(), session_ptr->remote_port(),
			asio2::last_error_msg().c_str());
	}).bind_start([&]()
	{
		printf("start http server : %s %u %d %s\n",
			server.listen_address().c_str(), server.listen_port(),
			asio2::last_error_val(), asio2::last_error_msg().c_str());
	}).bind_stop([&]()
	{
		printf("stop http server : %d %s\n",
			asio2::last_error_val(), asio2::last_error_msg().c_str());
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
			std::cout << it->name_string() << " " << it->value() << std::endl;
		}

		auto mpbody = req.multipart();

		for (auto it = mpbody.begin(); it != mpbody.end(); ++it)
		{
			std::cout
				<< "filename:" << it->filename() << " name:" << it->name()
				<< " content_type:" << it->content_type() << std::endl;
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
		asio2::ignore_unused(req, rep);

		printf("del_user ip : %s\n", session_ptr->remote_address().data());

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
		printf("ws msg : %zu %.*s\n", data.size(), (int)data.size(), data.data());

		session_ptr->async_send(data);

	}).on("open", [](std::shared_ptr<asio2::http_session>& session_ptr)
	{
		printf("ws open\n");

		// print the websocket request header.
		std::cout << session_ptr->request() << std::endl;

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

		printf("ws close\n");

	}).on_ping([](std::shared_ptr<asio2::http_session>& session_ptr)
	{
		asio2::ignore_unused(session_ptr);

		printf("ws ping\n");

	}).on_pong([](std::shared_ptr<asio2::http_session>& session_ptr)
	{
		asio2::ignore_unused(session_ptr);

		printf("ws pong\n");

	}));

	server.bind_not_found([](http::web_request& req, http::web_response& rep)
	{
		asio2::ignore_unused(req);

		rep.fill_page(http::status::not_found);
	});

	server.start(host, port);

	while (std::getchar() != '\n'); // press enter to exit this program

	return 0;
}

//-------------------------------------------------------------------
// http request protocol sample
//-------------------------------------------------------------------

//GET / HTTP/1.1
//Host: 127.0.0.1:8443
//Connection: keep-alive
//Cache-Control: max-age=0
//User-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/61.0.3163.100 Safari/537.36
//Upgrade-Insecure-Requests: 1
//Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8
//Accept-Encoding: gzip, deflate, br
//Accept-Language: zh-CN,zh;q=0.8

//-------------------------------------------------------------------
// http response protocol sample
//-------------------------------------------------------------------

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
