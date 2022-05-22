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
	// try open https://localhost:8443 in your browser
	std::string_view host = "127.0.0.1";
	std::string_view port = "8443";

	asio2::https_server server;

	server.support_websocket(true);

	// set the root directory, here is:  /asio2/example/wwwroot
	std::filesystem::path root = std::filesystem::current_path().parent_path().parent_path().append("wwwroot");
	server.set_root_directory(std::move(root));

	server.set_cert_file(
		"../../cert/ca.crt",
		"../../cert/server.crt",
		"../../cert/server.key",
		"123456");
	server.set_dh_file("../../cert/dh1024.pem");

	server.bind_accept([](std::shared_ptr<asio2::https_session> & session_ptr)
	{
		// close the invalid client
		if (session_ptr->remote_address().find("192") != std::string::npos)
			session_ptr->stop();

	}).bind_start([&]()
	{
		printf("start https server : %s %u %d %s\n",
			server.listen_address().c_str(), server.listen_port(),
			asio2::last_error_val(), asio2::last_error_msg().c_str());
	}).bind_stop([&]()
	{
		printf("stop https server : %d %s\n",
			asio2::last_error_val(), asio2::last_error_msg().c_str());
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

	// the /ws is the websocket upgraged target
	server.bind("/ws", websocket::listener<asio2::https_session>{}.
		on("message", [](std::shared_ptr<asio2::https_session>& session_ptr, std::string_view data)
	{
		printf("ws msg : %zu %.*s\n", data.size(), (int)data.size(), data.data());

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
