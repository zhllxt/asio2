#ifndef ASIO2_INCLUDE_RATE_LIMIT
#define ASIO2_INCLUDE_RATE_LIMIT
#endif

#include "unit_test.hpp"
#include <asio2/asio2.hpp>
#include <asio2/external/fmt.hpp>

void rate_limit_rpc_test()
{
	ASIO2_TEST_BEGIN_LOOP(test_loop_times);

	// rpc 
	{
		asio2::rpc_rate_server server;

		server.bind_connect([](std::shared_ptr<asio2::rpc_rate_session>& session_ptr)
		{
			// set the send rate
			session_ptr->socket().rate_policy().write_limit(256);

			// set the recv rate
			session_ptr->socket().rate_policy().read_limit(256);

		}).bind_recv([](auto& session_ptr, std::string_view data)
		{
			asio2::ignore_unused(session_ptr, data);
		});

		server.bind("echo", [](std::string str)
		{
			auto session = asio2::get_current_caller<std::shared_ptr<asio2::rpc_rate_session>>();
			session->async_call("echo", str);
			return str;
		});

		server.start("127.0.0.1", 18102);

		// "fn" should be declared before "client", beacuse when the client destroying, the client
		// async_call's response callback maybe still be invoked, so if the "fn" is declared after
		// "client", when "fn" is called in the response callback, the "fn" maybe destroyed already.
		// of course, you can call "client.stop" manual to avoid this problem.
		std::function<void()> fn;

		asio2::rpc_rate_client client;

		client.bind("echo", [](std::string str)
		{
			return str;
		});

		client.bind_connect([&client, &fn]()
		{
			// set the send rate
			client.socket().rate_policy().write_limit(512);

			ASIO2_CHECK(!asio2::get_last_error());

			if (!asio2::get_last_error())
			{
				fn = [&client, &fn]()
				{
					std::string str;

					for (int i = 0; i < 300; i++)
					{
						str += '0' + std::rand() % 64;
					}

					client.async_call("echo", str).response([str, &fn](std::string s)
					{
						if (!asio2::get_last_error())
						{
							ASIO2_CHECK(s == str);

							fn();
						}
					});
				};

				fn();
			}
		}).bind_recv([](std::string_view data)
		{
			asio2::ignore_unused(data);
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
	"rate_limit_rpc",
	ASIO2_TEST_CASE(rate_limit_rpc_test)
)
