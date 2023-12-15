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
		ASIO2_ASSERT(asio2::get_current_caller<std::shared_ptr<asio2::http_session>>().get() == session_ptr.get());
		asio2::ignore_unused(session_ptr, req, rep);
		printf("aop_log after\n");
		return true;
	}
};

struct aop_check
{
	bool before(std::shared_ptr<asio2::http_session>& session_ptr, http::web_request& req, http::web_response& rep)
	{
		ASIO2_ASSERT(asio2::get_current_caller<std::shared_ptr<asio2::http_session>>().get() == session_ptr.get());
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

	// root = /asio2/bin/x64
	std::filesystem::path root = std::filesystem::current_path();

	// root = /asio2/example/wwwroot
	root = root.parent_path().parent_path().append("example/wwwroot");

	// set the http server root directory
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
		session_ptr->set_disconnect_timeout(std::chrono::seconds(2));
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

		rep.fill_file("/index.html");
	}, aop_log{}, http::enable_cache);

	// If no method is specified, GET and POST are both enabled by default.
	server.bind("*", [](http::web_request& req, http::web_response& rep)
	{
		rep.fill_file(http::url_decode(req.target()));
		rep.chunked(true);
	}, aop_check{}, http::enable_cache);

	// Test http multipart
	server.bind<http::verb::get, http::verb::post>("/multipart_test", [](http::web_request& req, http::web_response& rep)
	{
		auto& body = req.body();
		auto target = req.target();

		std::stringstream ss;
		ss << req;
		auto str_req = ss.str();

		asio2::ignore_unused(body, target, str_req);

		for (auto it = req.begin(); it != req.end(); ++it)
		{
			std::cout << it->name_string() << ": " << it->value() << std::endl;
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

			http::web_request req_copy;
			req_copy.method(http::verb::post);
			req_copy.set(http::field::content_type, type);
			req_copy.keep_alive(true);
			req_copy.target("/api/user/");
			req_copy.body() = str;
			req_copy.prepare_payload();

			std::stringstream ress;
			ress << req_copy;
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

		// if the url is :
		// /api/user/add_user?name=hanmeimei&age=20&height=168

		[[maybe_unused]] std::string_view t = req.target(); // t == /api/user/add_user?name=hanmeimei&age=20&height=168
		[[maybe_unused]] std::string_view p = req.get_path(); // p == /api/user/add_user
		[[maybe_unused]] std::string_view q = req.get_query(); // q == name=hanmeimei&age=20&height=168

		std::vector<std::string_view> kvs = asio2::split(q, '&');

		for (std::string_view kv : kvs)
		{
			[[maybe_unused]] std::string_view k = kv.substr(0, kv.find('='));
			[[maybe_unused]] std::string_view v = kv.substr(kv.find('=') + 1);
		}

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
		
		// same as above
		auto& req = session_ptr->get_request();
		std::cout << req.get_path() << std::endl;

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

		// how to send a message to client when the websocket connection is connected.
		// can't use session_ptr->async_send(...) directly, because the websocket connection is not ready.
		session_ptr->post_queued_event([session_ptr]()
		{
			session_ptr->async_send("eg: hello websocket");
		});

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
