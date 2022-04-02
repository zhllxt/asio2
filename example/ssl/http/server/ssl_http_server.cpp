#ifndef ASIO2_USE_SSL
#define ASIO2_USE_SSL
#endif

#include <filesystem>
#include <iostream>
#include <asio2/http/https_server.hpp>

struct aop_log
{
	bool before(http::web_request& req, http::web_response& rep)
	{
		asio2::ignore_unused(rep);
		printf("aop_log before %s\n", req.method_string().data());
		return true;
	}
	bool after(std::shared_ptr<asio2::https_session>& session_ptr, http::web_request& req, http::web_response& rep)
	{
		asio2::ignore_unused(session_ptr, req, rep);
		printf("aop_log after\n");
		return true;
	}
};

struct aop_check
{
	bool before(std::shared_ptr<asio2::https_session>& session_ptr, http::web_request& req, http::web_response& rep)
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
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	std::string_view host = "127.0.0.1";
	std::string_view port = "8443";

	asio2::https_server server;

	server.support_websocket(true);

	server.set_cert_file(
		"../../cert/ca.crt",
		"../../cert/server.crt",
		"../../cert/server.key",
		"123456");
	server.set_dh_file("../../cert/dh1024.pem");

	server.bind_recv([&](http::web_request& req, http::web_response& rep)
	{
		// all http and websocket request will goto here first.
		std::cout << req.path() << std::endl;
		std::cout << req.query() << std::endl;

		auto& body = req.body();
		auto target = req.target();

		std::stringstream ss;
		ss << req;
		auto str_req = ss.str();


		asio2::ignore_unused(body, target, str_req);

		for (auto it = req.begin(); it != req.end(); ++it)
		{
			std::cout << it->name_string() << " : " << it->value() << std::endl;
		}

		auto mpbody = req.multipart();

		for (auto it = mpbody.begin(); it != mpbody.end(); ++it)
		{
			std::cout << "filename:" << it->filename() << " name:" << it->name() << " content_type:" << it->content_type() << std::endl;
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

	}).bind_accept([](std::shared_ptr<asio2::https_session> & session_ptr)
	{
		// close the invalid client
		if (session_ptr->remote_address().find("192") != std::string::npos)
			session_ptr->stop();

	}).bind_disconnect([](std::shared_ptr<asio2::https_session> & session_ptr)
	{
		printf("client leave : %s %u %s\n", session_ptr->remote_address().c_str(),
			session_ptr->remote_port(), asio2::last_error_msg().c_str());

	}).bind_start([&]()
	{
		printf("start https server : %s %u %d %s\n", server.listen_address().c_str(),
			server.listen_port(), asio2::last_error_val(), asio2::last_error_msg().c_str());

	}).bind_stop([&]()
	{
		printf("stop https server : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	});

	server.bind<http::verb::get, http::verb::post>("/index.*", [](http::web_request& req, http::web_response& rep)
	{
		asio2::ignore_unused(req);

		rep.fill_file("../../index.html");

	}, aop_log{});

	server.bind<http::verb::get>("/del_user",
		[](std::shared_ptr<asio2::https_session>& session_ptr, http::web_request& req, http::web_response& rep)
	{
		asio2::ignore_unused(req);

		printf("del_user ip : %s\n", session_ptr->remote_address().data());

		rep.fill_page(http::status::ok, "del_user successed.");

	}, aop_check{});

	server.bind<http::verb::get>("/api/user/*", [](http::web_request& req, http::web_response& rep)
	{
		asio2::ignore_unused(req);

		rep.fill_text("the user name is hanmeimei, .....");

	}, aop_log{}, aop_check{});

	server.bind("/ws", websocket::listener<asio2::https_session>{}.
		on("message", [](std::shared_ptr<asio2::https_session>& session_ptr, std::string_view data)
	{
		printf("ws msg : %u %.*s\n", (unsigned)data.size(), (int)data.size(), data.data());

		session_ptr->async_send(data);

	}).on("open", [](std::shared_ptr<asio2::https_session>& session_ptr)
	{
		asio2::ignore_unused(session_ptr);

		printf("ws open\n");

	}).on("close", [](std::shared_ptr<asio2::https_session>& session_ptr)
	{
		asio2::ignore_unused(session_ptr);

		printf("ws close\n");

	}).on("ping", [](std::shared_ptr<asio2::https_session>& session_ptr, std::string_view data)
	{
		asio2::ignore_unused(session_ptr);

		printf("ws ping %d\n", int(data.size()));

	}).on("pong", [](std::shared_ptr<asio2::https_session>& session_ptr, std::string_view data)
	{
		asio2::ignore_unused(session_ptr);

		printf("ws pong %d\n", int(data.size()));

	}));

	server.bind_not_found([](/*std::shared_ptr<asio2::http_session>& session_ptr, */
		http::web_request& req, http::web_response& rep)
	{
		asio2::ignore_unused(req);

		rep.fill_page(http::status::not_found);
	});

	server.start(host, port);

	while (std::getchar() != '\n');

	return 0;
}
