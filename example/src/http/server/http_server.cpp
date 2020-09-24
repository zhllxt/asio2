// When compiling with vs under linux, you need to copy the "asio,beast,ceral,fmt" folders to
// the /usr/local/include directory first, and copy the "libcrypto.a,libssl.a" files to 
// /usr/local/lib directory first. "libcrypto.a,libssl.a" is in "asio2/lib/x64".

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
	bool after(std::shared_ptr<asio2::http_session>& session_ptr, http::request& req, http::response& rep)
	{
		asio2::detail::ignore_unused(session_ptr, req, rep);
		printf("aop_log after\n");
		return true;
	}
};

struct aop_check
{
	bool before(std::shared_ptr<asio2::http_session>& session_ptr, http::request& req, http::response& rep)
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
	std::string_view port = "8080";

	asio2::http_server server;

	server.bind_recv([&](http::request& req, http::response& rep)
	{
		// all http and websocket request will goto here first.
		std::cout << req.path() << std::endl;
		std::cout << req.query() << std::endl;

	}).bind_connect([](auto & session_ptr)
	{
		printf("client enter : %s %u %s %u\n", session_ptr->remote_address().c_str(), session_ptr->remote_port(),
			session_ptr->local_address().c_str(), session_ptr->local_port());
	}).bind_disconnect([](auto & session_ptr)
	{
		printf("client leave : %s %u %s\n", session_ptr->remote_address().c_str(),
			session_ptr->remote_port(), asio2::last_error_msg().c_str());
	}).bind_start([&](asio::error_code ec)
	{
		printf("start http server : %s %u %d %s\n", server.listen_address().c_str(), server.listen_port(),
			ec.value(), ec.message().c_str());
	}).bind_stop([&](asio::error_code ec)
	{
		printf("stop http server : %d %s\n", ec.value(), ec.message().c_str());
	});

	server.bind<http::verb::get, http::verb::post>("/index.*", [](http::request& req, http::response& rep)
	{
		rep.fill_file("../../../index.html");
		rep.chunked(true);

	}, aop_log{});

	server.bind<http::verb::get>("/del_user",
		[](std::shared_ptr<asio2::http_session>& session_ptr, http::request& req, http::response& rep)
	{
		printf("del_user ip : %s\n", session_ptr->remote_address().data());

		rep.fill_page(http::status::ok, "del_user successed.");

	}, aop_check{});

	server.bind<http::verb::get>("/api/user/*", [](http::request& req, http::response& rep)
	{
		rep.fill_text("the user name is hanmeimei, .....");

	}, aop_log{}, aop_check{});

	server.bind<http::verb::get>("/defer", [](http::request& req, http::response& rep)
	{
		// use defer to make the reponse not send immediately, util the derfer shared_ptr
		// is destroyed, then the response will be sent.
		std::shared_ptr<http::response_defer> rep_defer = rep.defer();

		std::thread([rep_defer, &rep]() mutable
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(3000));

			asio::error_code ec;
			auto newrep = asio2::http_client::execute("http://www.baidu.com", ec);

			rep = std::move(newrep);

		}).detach();

	}, aop_log{}, aop_check{});

	server.bind("/ws", websocket::listener<asio2::http_session>{}.
		on("message", [](std::shared_ptr<asio2::http_session>& session_ptr, std::string_view data)
	{
		printf("ws msg : %u %.*s\n", (unsigned)data.size(), (int)data.size(), data.data());

		session_ptr->send(data);

	}).on("open", [](std::shared_ptr<asio2::http_session>& session_ptr)
	{
		printf("ws open\n");

		// print the websocket request header.
		std::cout << session_ptr->request() << std::endl;

		// how to set custom websocket response data : 
		session_ptr->ws_stream().set_option(websocket::stream_base::decorator(
			[](websocket::response_type& rep)
		{
			rep.set(http::field::authorization, " http-server-coro");
		}));

	}).on("close", [](std::shared_ptr<asio2::http_session>& session_ptr)
	{
		printf("ws close\n");

	}));

	server.bind_not_found([](http::request& req, http::response& rep)
	{
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
