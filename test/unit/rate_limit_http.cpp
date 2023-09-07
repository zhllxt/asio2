#ifndef ASIO2_INCLUDE_RATE_LIMIT
#define ASIO2_INCLUDE_RATE_LIMIT
#endif

#include "unit_test.hpp"
#include <asio2/asio2.hpp>
#include <asio2/external/fmt.hpp>

void rate_limit_http_test()
{
	ASIO2_TEST_BEGIN_LOOP(test_loop_times);

	// http 
	{
		asio2::http_rate_server server;

		server.bind_connect([](std::shared_ptr<asio2::http_rate_session>& session_ptr)
		{
			// set the send rate
			session_ptr->socket().rate_policy().write_limit(256);

			// set the recv rate
			session_ptr->socket().rate_policy().read_limit(256);

		});

		server.bind("*", [](http::web_request& req, http::web_response& rep)
		{
			rep.fill_text(req.target());
		});

		server.start("127.0.0.1", 18102);


		asio2::http_rate_client client;

		client.bind_connect([&client]()
		{
			// set the send rate
			client.socket().rate_policy().write_limit(512);

			ASIO2_CHECK(!asio2::get_last_error());

			if (!asio2::get_last_error())
			{
				http::web_request req;
				req.version(11);
				req.method(http::verb::get);
				req.target("/1");
				req.set(http::field::host, "127.0.0.1");

				client.async_send(req);
			}
		}).bind_recv([&client](http::web_request& req,http::web_response& rep)
		{
			ASIO2_CHECK(rep.body().text() == "/1");

			req.version(11);
			req.method(http::verb::get);
			req.target("/1");
			req.set(http::field::host, "127.0.0.1");

			client.async_send(req);
		});

		client.start("127.0.0.1", 18102);

		std::thread([&client]()
		{
			// note: if you set the rate in the other thread which is not the io_context
			// thread, you should use post function to make the operation is executed in
			// the io_context thread, otherwise it is not thread safety.
			client.post([&client]()
			{
				// set the recv rate
				client.socket().rate_policy().read_limit(512);
			});
		}).detach();

        std::this_thread::sleep_for(std::chrono::seconds(3));
	}

	ASIO2_TEST_END_LOOP;
}


ASIO2_TEST_SUITE
(
	"rate_limit_http",
	ASIO2_TEST_CASE(rate_limit_http_test)
)
