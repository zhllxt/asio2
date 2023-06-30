#include <asio2/asio2.hpp>
#include <iostream>

int main()
{
	// 4 threads
	asio2::iopool iopool(4);

	// iopool must start first, othwise the server.start will blocked forever.
	iopool.start();

	std::vector<std::future<void>> futures;

	auto future1 = iopool.post([]()
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		std::cout << "task 1:" << std::this_thread::get_id() << std::endl;
	});
	auto future2 = iopool.post([]()
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		std::cout << "task 2:" << std::this_thread::get_id() << std::endl;
	});
	auto future3 = iopool.post(2, [&iopool]()
	{
		std::cout << "task 3:" << std::this_thread::get_id() << std::endl;
		ASIO2_ASSERT(iopool.get_thread_id(2) == std::this_thread::get_id());
		ASIO2_ASSERT(iopool.get(2)->get_thread_id() == std::this_thread::get_id());
		asio2::ignore_unused(iopool);
	});

	futures.emplace_back(std::move(future1));
	futures.emplace_back(std::move(future2));
	futures.emplace_back(std::move(future3));

	for (auto& future : futures)
	{
		future.wait();
	}

	//// --------------------------------------------------------------------------------

	asio2::timer timer(iopool.get(std::rand() % iopool.size()));

	timer.start_timer(1, std::chrono::milliseconds(1000), [&]()
	{
		printf("timer 1, loop infinite\n");
	});

	//-----------------------------------------------------------------------------------

	// 256 - init recv buffer size, 2048 - max recv buffer size, see the constructor of tcp_server
	asio2::tcp_server server(256, 2048, std::vector<std::shared_ptr<asio2::io_t>>{ iopool.get(0), iopool.get(1) });

	server.bind_recv([&](std::shared_ptr<asio2::tcp_session>& session_ptr, std::string_view data)
	{
		ASIO2_ASSERT(session_ptr->io().running_in_this_thread());

		printf("recv : %zu %.*s\n", data.size(), (int)data.size(), data.data());

		// delay 3 seconds, then send the response
		session_ptr->post([session_ptr, s = std::string{ data }]() mutable
		{
			session_ptr->async_send(std::move(s));
		}, std::chrono::milliseconds(3000));

	}).bind_connect([&](auto& session_ptr)
	{
		ASIO2_ASSERT(iopool.running_in_thread(0));

		printf("client enter : %s %u %s %u\n",
			session_ptr->remote_address().c_str(), session_ptr->remote_port(),
			session_ptr->local_address().c_str(), session_ptr->local_port());

	}).bind_disconnect([&](auto & session_ptr)
	{
		ASIO2_ASSERT(iopool.running_in_thread(0));

		printf("client leave : %s %u %s\n",
			session_ptr->remote_address().c_str(), session_ptr->remote_port(),
			asio2::last_error_msg().c_str());
	}).bind_start([&]()
	{
		ASIO2_ASSERT(iopool.running_in_thread(0));

		printf("start tcp server : %s %u %d %s\n",
			server.listen_address().c_str(), server.listen_port(),
			asio2::last_error_val(), asio2::last_error_msg().c_str());
	}).bind_stop([&]()
	{
		ASIO2_ASSERT(iopool.running_in_thread(0));

		printf("stop : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	});

	server.start("127.0.0.1", 4567);

	//-----------------------------------------------------------------------------------

	// if you create a object and it will destroyed before iopool.stop(), you must create
	// the object as shared_ptr<asio2::tcp_client>, can't be asio2::tcp_client.
	{
		std::shared_ptr<asio2::tcp_client> client = std::make_shared<asio2::tcp_client>(iopool.get(0));

		client->start("127.0.0.1", 4567);

		client->async_send("i love you");

		std::this_thread::sleep_for(std::chrono::seconds(2));
	}

	//-----------------------------------------------------------------------------------

	std::vector<std::shared_ptr<asio2::tcp_client>> clients;

	for (int i = 0; i < 10; i++)
	{
		std::shared_ptr<asio2::tcp_client>& client_ptr = clients.emplace_back(
			std::make_shared<asio2::tcp_client>(iopool.get(i % iopool.size())));

		client_ptr->bind_recv([client_ptr](std::string_view data)
		{
			client_ptr->async_send(data);
		}).bind_connect([client_ptr]()
		{
			client_ptr->async_send("<0123456789abcdefg>");
		});

		client_ptr->async_start("127.0.0.1", 4567);
	}

	while (std::getchar() != '\n');  // press enter to exit this program

	/* call the server.stop and client.stop, or not call the server.stop and client.stop, is OK.*/

	//server.stop();

	//for (auto& client_ptr : clients)
	//{
	//	client_ptr->stop();
	//}

	// must call iopool.stop() before exit.
	iopool.stop();

	return 0;
}
