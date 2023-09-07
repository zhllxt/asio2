#ifndef ASIO2_INCLUDE_RATE_LIMIT
#define ASIO2_INCLUDE_RATE_LIMIT
#endif

#include "unit_test.hpp"
#include <asio2/asio2.hpp>
#include <asio2/external/fmt.hpp>

void rate_limit_ws_test()
{
	ASIO2_TEST_BEGIN_LOOP(test_loop_times);

	// websocket 
	{
		asio2::ws_rate_server server;

		server.bind_connect([](std::shared_ptr<asio2::ws_rate_session>& session_ptr)
		{
			// set the send rate
			session_ptr->socket().rate_policy().write_limit(256);

			// set the recv rate
			session_ptr->socket().rate_policy().read_limit(256);

		}).bind_recv([](auto& session_ptr, std::string_view data)
		{
			ASIO2_CHECK(data.size() == 1024);

			session_ptr->async_send(data);
		});

		server.start("127.0.0.1", 18102);


		asio2::ws_rate_client client;

		client.bind_connect([&client]()
		{
			// set the send rate
			client.socket().rate_policy().write_limit(512);

			ASIO2_CHECK(!asio2::get_last_error());

			if (!asio2::get_last_error())
			{
				std::string str;

				for (int i = 0; i < 1024; i++)
				{
					str += '0' + std::rand() % 64;
				}

				client.async_send(str);
			}
		}).bind_recv([&client](std::string_view data)
		{
            ASIO2_CHECK(data.size() == 1024);

			client.async_send(data);
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
	"rate_limit_ws",
	ASIO2_TEST_CASE(rate_limit_ws_test)
)
