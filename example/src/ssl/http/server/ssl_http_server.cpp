// When compiling with vs under linux, you need to copy the "asio,beast,ceral,fmt" folders to
// the /usr/local/include directory first, and copy the "libcrypto.a,libssl.a" files to 
// /usr/local/lib directory first. "libcrypto.a,libssl.a" is in "asio2/lib/x64".

#ifndef ASIO2_USE_SSL
#define ASIO2_USE_SSL
#endif

#include <filesystem>
#include <asio2/asio2.hpp>

struct aop_log
{
	bool before(http::request& req, http::response& rep)
	{
		asio2::detail::ignore_unused(rep);
		printf("aop_log before %s\n", req.method_string().data());
		return true;
	}
	bool after(std::shared_ptr<asio2::https_session>& session_ptr, http::request& req, http::response& rep)
	{
		asio2::detail::ignore_unused(session_ptr, req, rep);
		printf("aop_log after\n");
		return true;
	}
};

struct aop_check
{
	bool before(std::shared_ptr<asio2::https_session>& session_ptr, http::request& req, http::response& rep)
	{
		asio2::detail::ignore_unused(session_ptr, req, rep);
		printf("aop_check before\n");
		return true;
	}
	bool after(http::request& req, http::response& rep)
	{
		asio2::detail::ignore_unused(req, rep);
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

	std::string_view host = "0.0.0.0";
	std::string_view port = "8043";

	//bool loop = false;
	bool loop = true;
	while (loop) // use infinite loop and sleep 2 seconds to test start and stop
	{
		asio2::https_server server;

		server.support_websocket(true);

		server.set_cert_file("../../../cert/ca.crt", "../../../cert/server.crt", "../../../cert/server.key", "server");
		server.set_dh_file("../../../cert/dh1024.pem");

		server.bind_accept([](std::shared_ptr<asio2::https_session> & session_ptr)
		{
			// close the invalid client
			if (session_ptr->remote_address().find("192") != std::string::npos)
				session_ptr->stop();

		}).bind_disconnect([](std::shared_ptr<asio2::https_session> & session_ptr)
		{
			printf("client leave : %s %u %s\n", session_ptr->remote_address().c_str(),
				session_ptr->remote_port(), asio2::last_error_msg().c_str());

		}).bind_start([&](asio::error_code ec)
		{
			printf("start https server : %s %u %d %s\n", server.listen_address().c_str(),
				server.listen_port(), ec.value(), ec.message().c_str());

		}).bind_stop([&](asio::error_code ec)
		{
			printf("stop https server : %d %s\n", ec.value(), ec.message().c_str());
		});

		server.bind<http::verb::get, http::verb::post>("/index.*", [](http::request& req, http::response& rep)
		{
			rep.fill_file("../../../index.html");

		}, aop_log{});

		server.bind<http::verb::get>("/del_user",
			[](std::shared_ptr<asio2::https_session>& session_ptr, http::request& req, http::response& rep)
		{
			printf("del_user ip : %s\n", session_ptr->remote_address().data());

			rep.fill_page(http::status::ok, "del_user successed.");

		}, aop_check{});

		server.bind<http::verb::get>("/api/user/*", [](http::request& req, http::response& rep)
		{
			rep.fill_text("the user name is hanmeimei, .....");

		}, aop_log{}, aop_check{});

		server.bind("/ws", websocket::listener<asio2::https_session>{}.
			on("message", [](std::shared_ptr<asio2::https_session>& session_ptr, std::string_view data)
		{
			printf("ws msg : %u %.*s\n", (unsigned)data.size(), (int)data.size(), data.data());

			session_ptr->send(data);

		}).on("open", [](std::shared_ptr<asio2::https_session>& session_ptr)
		{
			printf("ws open\n");

		}).on("close", [](std::shared_ptr<asio2::https_session>& session_ptr)
		{
			printf("ws close\n");

		}).on("ping", [](std::shared_ptr<asio2::https_session>& session_ptr, std::string_view data)
		{
			printf("ws ping %d\n", int(data.size()));

		}).on("pong", [](std::shared_ptr<asio2::https_session>& session_ptr, std::string_view data)
		{
			printf("ws pong %d\n", int(data.size()));

		}));

		server.bind_not_found([](/*std::shared_ptr<asio2::http_session>& session_ptr, */
			http::request& req, http::response& rep)
		{
			rep.fill_page(http::status::not_found);
		});

		server.start(host, port);

		if (!loop)
			while (std::getchar() != '\n');
		else
			std::this_thread::sleep_for(std::chrono::seconds(2));
	}

	return 0;
}
