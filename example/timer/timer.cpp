#include <asio2/asio2.hpp>
#include <asio2/tcp/tcp_server.hpp>
#include <asio2/tcp/tcp_client.hpp>

int main()
{
	asio2::timer timer;

	// 1    - timer id
	// 1000 - timer inteval, 1000 milliseconds
	timer.start_timer(1, 1000, [&]()
	{
		printf("timer 1, loop infinite\n");

		timer.reset_timer_interval(1, 2000);
		// or:
		// timer.reset_timer_interval(1, std::chrono::milliseconds(2000));
	});

	// "id2"- timer id, this timer id is a string
	// 2000 - timer inteval, 2000 milliseconds
	// 1    - timer repeat times
	timer.start_timer("id2", 2000, 1, []()
	{
		printf("timer id2, loop once\n");
	});

	// 3    - timer id
	// 3000 - timer inteval, 3000 milliseconds
	timer.start_timer(3, std::chrono::milliseconds(3000), []()
	{
		printf("timer 3, loop infinite\n");
	});

	// 4    - timer id
	// 4000 - timer inteval, 4000 milliseconds
	// 4    - timer repeat times
	timer.start_timer(4, std::chrono::milliseconds(4000), 4, []()
	{
		printf("timer 4, loop 4 times\n");
	});

	// 5    - timer id
	// std::chrono::milliseconds(1000) - timer inteval, 1000 milliseconds
	// std::chrono::milliseconds(5000) - timer delay for first execute, 5000 milliseconds
	timer.start_timer(5, std::chrono::milliseconds(1000), std::chrono::seconds(5), []()
	{
		printf("timer 5, loop infinite, delay 5 seconds\n");
	});

	// 6    - timer id
	// std::chrono::milliseconds(1000) - timer inteval, 1000 milliseconds
	// 6    - timer repeat times
	// std::chrono::milliseconds(6000) - timer delay for first execute, 6000 milliseconds
	timer.start_timer(6, std::chrono::seconds(1), 6, std::chrono::milliseconds(6000), []()
	{
		printf("timer 6, loop 6 times, delay 6 seconds\n");
	});

	// Start an asynchronous task with delay
	timer.post([&]()
	{
		ASIO2_ASSERT(timer.io().running_in_this_thread());
		printf("execute some task after 3 seconds for timer\n");
	}, std::chrono::seconds(3));

	//---------------------------------------------------------------------------------------------

	{
		std::shared_ptr<asio2::timer> timer_ptr = std::make_shared<asio2::timer>();

		timer_ptr->start_timer("timer_ptr1", 1000, []()
		{
			printf("timer_ptr1, loop infinite\n");
		});

		// Note : 
		// if the timer is create as "std::shared_ptr<asio2::timer>", not "asio2::timer", you must
		// call the "timer::stop()" function manually, otherwise it maybe cause crash.
		// all the "server, client" need do the same like this.
		timer_ptr->stop();
	}

	//---------------------------------------------------------------------------------------------

	asio2::tcp_server server;

	// test timer
	server.start_timer(1, std::chrono::seconds(1), [&]()
	{
		ASIO2_ASSERT(server.io().running_in_this_thread());
		printf("execute timer for tcp server \n");
	});

	// Start an asynchronous task with delay
	server.post([&]()
	{
		ASIO2_ASSERT(server.io().running_in_this_thread());
		printf("execute some task after 3 seconds\n");
	}, std::chrono::seconds(3));

	// Start a synchronization task
	server.dispatch([&]()
	{
		ASIO2_ASSERT(server.io().running_in_this_thread());
		printf("execute some task in server's io_context thread\n");
	});

	server.bind_connect([&](auto & session_ptr)
	{
		session_ptr->post([session_ptr]()
		{
			printf("execute some task after 3 seconds, session key : %zu\n", session_ptr->hash_key());
		}, std::chrono::seconds(3));

		session_ptr->start_timer(1, 1000, 5, [session_ptr]()
		{
			printf("execute timer for 5 times, session key : %zu\n", session_ptr->hash_key());
		});
	});

	server.start("127.0.0.1", 3990);

	//---------------------------------------------------------------------------------------------

	asio2::tcp_client client;

	std::shared_ptr<asio2::condition_event> evt_ptr = client.post_condition_event([&]()
	{
		ASIO2_ASSERT(client.io().running_in_this_thread());
		printf("execute manual event for tcp client \n");
	});

	// test timer
	client.start_timer(1, std::chrono::seconds(1), 10, [&, evt_ptr]()
	{
		ASIO2_ASSERT(client.io().running_in_this_thread());
		printf("execute timer 10 times for tcp client \n");

		static int counter = 0;
		if (counter++ > 5)
		{
			client.stop_timer(1);
			evt_ptr->notify();
		}
	});

	// Start an asynchronous task with delay
	client.post([&]()
	{
		ASIO2_ASSERT(client.io().running_in_this_thread());
		printf("execute some task immediately for tcp client\n");
	});

	// Start a synchronization task
	client.dispatch([&]()
	{
		ASIO2_ASSERT(client.io().running_in_this_thread());
		printf("execute some task in client's io_context thread\n");
	});

	client.start("127.0.0.1", 3990);

	while (std::getchar() != '\n');

	return 0;
}
