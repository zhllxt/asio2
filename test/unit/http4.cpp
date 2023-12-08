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

void http4_test()
{
	ASIO2_TEST_BEGIN_LOOP(test_loop_times);

	asio2::http_client http_client_bd;
	asio2::http_client http_client_qq;

	bool has_internet = http_client_bd.start("www.baidu.com", 80) || http_client_qq.start("qq.com", 80);

	{
		// try open http://localhost:8080 in your browser
		asio2::http_server server;

		server.set_support_websocket(true);

		// set the root directory, here is:  /asio2/example/wwwroot
		std::filesystem::path root = std::filesystem::current_path()
			.parent_path().parent_path()
			.append("example").append("wwwroot");
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

			rep.fill_file("/index.html");
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

		server.bind<http::verb::get>("/defer", [has_internet](http::web_request& req, http::web_response& rep)
		{
			asio2::ignore_unused(req, rep);

			// use defer to make the reponse not send immediately, util the derfer shared_ptr
			// is destroyed, then the response will be sent.
			std::shared_ptr<http::response_defer> rep_defer = rep.defer();

			std::thread([has_internet, rep_defer, &rep]() mutable
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));

				if (has_internet)
					rep = asio2::http_client::execute("http://www.baidu.com");
				else
					rep.fill_file("/index.html");
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

		server.start("127.0.0.1", 8080);

		//-----------------------------------------------------------------------------------------

		asio2::iopool iopool(4);

		iopool.start();

		std::vector<std::shared_ptr<asio2::ws_client>> ws_clients;
		std::vector<std::shared_ptr<asio2::http_client>> http_clients;

		std::atomic<int> client_connect_counter = 0;
		std::atomic<int> client_start_failed_counter = 0;

		for (int i = 0; i < test_client_count; i++)
		{
			std::shared_ptr<asio2::ws_client> client_ptr =
				std::make_shared<asio2::ws_client>(iopool.get(i % iopool.size()));
			std::shared_ptr<asio2::http_client> http_client_ptr =
				std::make_shared<asio2::http_client>(iopool.get(i % iopool.size()));

			ws_clients.emplace_back(client_ptr);
			http_clients.emplace_back(http_client_ptr);

			asio2::ws_client& client = *client_ptr;
			asio2::http_client& http_client = *http_client_ptr;

			http::web_request req(http::verb::get, "/", 11);

			http_client.start("127.0.0.1", 8080);
			http_client.async_send(req);

			http_client.async_start("127.0.0.1", 8080);
			http_client.async_send(req);

			http_client.stop();

			http_client.async_start("127.0.0.1", 8080);
			http_client.async_send(req);

			http_client.start("127.0.0.1", 8080);
			http_client.async_send(req);

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
					if (!ws_client_ret)
						client_start_failed_counter++;
					ASIO2_CHECK(ws_client_ret);
					ASIO2_CHECK(!asio2::get_last_error());
		}

		while (server.get_session_count() < std::size_t(test_client_count * 2 - client_start_failed_counter))
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		while (client_connect_counter < test_client_count - client_start_failed_counter)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		auto session_count = server.get_session_count();
		ASIO2_CHECK_VALUE(session_count, session_count == std::size_t(test_client_count * 2));

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
			ASIO2_TEST_WAIT_CHECK();
		}

		while (server.get_session_count() < std::size_t(test_client_count * 2 - client_start_failed_counter))
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		while (client_connect_counter < test_client_count - client_start_failed_counter)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		session_count = server.get_session_count();
		ASIO2_CHECK_VALUE(session_count, session_count == std::size_t(test_client_count * 2));

		ASIO2_CHECK_VALUE(client_connect_counter.load(), client_connect_counter == test_client_count);

		timer.stop();
		iopool.stop();
	}

	ASIO2_TEST_END_LOOP;
}


ASIO2_TEST_SUITE
(
	"http4",
	ASIO2_TEST_CASE(http4_test)
)
