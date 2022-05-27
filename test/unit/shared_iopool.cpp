#include "unit_test.hpp"

#include <asio2/config.hpp>

#ifndef ASIO2_ENABLE_LOG
#define ASIO2_ENABLE_LOG
#endif

#include <asio2/asio2.hpp>

void shared_iopool_test()
{
	ASIO2_TEST_BEGIN_LOOP(test_loop_times);

	asio2::iopool iopool(4);

	// iopool must start first, othwise the server.start will blocked forever.
	iopool.start();

	std::vector<std::shared_ptr<std::thread>> threads;

	//// --------------------------------------------------------------------------------

	// compile test, just for test the constructor sfinae
	asio2::tcp_server  _server1(1024);
	asio2::tcp_server  _server2(1024, 65535);
	asio2::tcp_server  _server3(1024, 65535, 4);
	asio2::tcp_server  _server5(1024, 65535, std::vector{ &iopool.get(0), &iopool.get(1) });
	asio2::tcp_server  _server6(1024, 65535, std::list  { &iopool.get(0), &iopool.get(1) });
	asio2::rpc_server  _server7(1024, 65535, &iopool.get(0));
	asio2::http_server _server8(1024, 65535,  iopool.get(0));
	asio2::udp_server  _servera(std::vector<asio2::io_t*>{ &iopool.get(0), &iopool.get(1) });
	asio2::ws_server   _serverb(std::list  <asio2::io_t*>{ &iopool.get(0), &iopool.get(1) });
	asio2::tcp_server  _serverc(&iopool.get(0));
	asio2::tcp_server  _serverd( iopool.get(0));

	asio2::tcp_client  _client1(1024);
	asio2::tcp_client  _client2(1024, 65535);
	asio2::tcp_client  _client3(1024, 65535, 4);
	asio2::tcp_client  _client5(1024, 65535, std::vector{ &iopool.get(0) });
	asio2::tcp_client  _client6(1024, 65535, std::list  { &iopool.get(0) });
	asio2::tcp_client  _client7(1024, 65535, &iopool.get(0));
	asio2::tcp_client  _client8(1024, 65535,  iopool.get(0));
	asio2::http_client _clienta(std::vector<asio2::io_t*>{ &iopool.get(0) });
	asio2::ws_client   _clientb(std::list  <asio2::io_t*>{ &iopool.get(0) });
	asio2::udp_client  _clientc(&iopool.get(0));
	asio2::rpc_client  _clientd( iopool.get(0));

	asio2::serial_port _sp1  ( iopool.get(1));
	asio2::udp_cast    _cast1(&iopool.get(2));
	asio2::ping        _ping1(&iopool.get(3));

	//// --------------------------------------------------------------------------------

	int index = std::rand() % iopool.size();

#define iothread_check \
	[&iopool, index]() \
	{ \
		ASIO2_CHECK(iopool.running_in_thread(index)); \
	}

#define iothreads_check \
	[&iopool]() \
	{ \
		ASIO2_CHECK(iopool.running_in_threads()); \
	}

	//// --------------------------------------------------------------------------------
#undef ASIO2_ENABLE_TIMER_CALLBACK_WHEN_ERROR

#define timer_check \
	[&iopool, index]() \
	{ \
		ASIO2_CHECK(iopool.running_in_thread(index)); \
		ASIO2_CHECK(!asio2::get_last_error()); \
	}

	asio2::timer t1(iopool.get(index));
	t1.start_timer("3011", std::chrono::milliseconds(10), timer_check);
	t1.start_timer("3012", std::chrono::milliseconds(10), std::chrono::milliseconds(10), timer_check);
	t1.start_timer("3013", std::chrono::milliseconds(10), 3, timer_check);
	t1.start_timer("3014", std::chrono::milliseconds(10), 3, std::chrono::milliseconds(10), timer_check);
	t1.post([&t1, &iopool, index]()
	{
		t1.start_timer("3021", std::chrono::milliseconds(10), timer_check);
		t1.start_timer("3022", std::chrono::milliseconds(10), std::chrono::milliseconds(10), timer_check);
		t1.start_timer("3023", std::chrono::milliseconds(10), 3, timer_check);
		t1.start_timer("3024", std::chrono::milliseconds(10), 3, std::chrono::milliseconds(10), timer_check);
	});

	std::thread([&t1, &iopool, index]()
	{
		t1.start_timer(3031, std::chrono::milliseconds(10), timer_check);
		t1.start_timer(3032, std::chrono::milliseconds(10), std::chrono::milliseconds(10), timer_check);
		t1.start_timer(3033, std::chrono::milliseconds(10), 3, timer_check);
		t1.start_timer(3034, std::chrono::milliseconds(10), 3, std::chrono::milliseconds(10), timer_check);
		t1.post([&t1, &iopool, index]()
		{
			t1.start_timer(3041, std::chrono::milliseconds(10), timer_check);
			t1.start_timer(3042, std::chrono::milliseconds(10), std::chrono::milliseconds(10), timer_check);
			t1.start_timer(3043, std::chrono::milliseconds(10), 3, timer_check);
			t1.start_timer(3044, std::chrono::milliseconds(10), 3, std::chrono::milliseconds(10), timer_check);
		});
	}).join();

	//// --------------------------------------------------------------------------------

	index = std::rand() % iopool.size();

	asio2::udp_cast cast1(iopool.get(index));

	cast1.start_timer(1, std::chrono::seconds(1), timer_check);

	auto ptr = cast1.post_event(iothread_check);

	cast1.post(iothread_check, std::chrono::milliseconds(std::rand() % 50));

	threads.emplace_back(std::make_shared<std::thread>([ptr]() mutable
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(50 + std::rand() % 2));
		ptr->notify();
	}));

	std::this_thread::sleep_for(std::chrono::milliseconds(2 + std::rand() % 5));

	ptr->notify();

	cast1.start("0.0.0.0", 4444);

	std::this_thread::sleep_for(std::chrono::milliseconds(2 + std::rand() % 5));

	cast1.stop();

	cast1.start_timer(1, std::chrono::seconds(1), timer_check);

	cast1.post(iothread_check, std::chrono::milliseconds(std::rand() % 50));

	ptr = cast1.post_event(iothread_check);

	threads.emplace_back(std::make_shared<std::thread>([ptr]() mutable
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(50 + std::rand() % 2));
		ptr->notify();
	}));

	ptr.reset();

	ptr = cast1.post_event(iothread_check);

	//-----------------------------------------------------------------------------------

	index = 0;

	asio2::tcp_server server(std::vector<asio2::io_t*>{
		&iopool.get(0), &iopool.get(1), &iopool.get(2), &iopool.get(3) });

	// the server's io_context must be the user passed io_context.
	for (std::size_t i = 0; i < server.iopool().size(); i++)
	{
		[[maybe_unused]] asio::io_context& ioc1 = iopool.get(i).context();
		[[maybe_unused]] asio::io_context& ioc2 = server.iopool().get(i).context();

		ASIO2_CHECK(&ioc1 == &ioc2);
	}

	server.start_timer(1, std::chrono::seconds(1), timer_check); // test timer

	server.bind_recv([&](std::shared_ptr<asio2::tcp_session> & session_ptr, std::string_view sv)
	{
		ASIO2_CHECK(session_ptr->io().strand().running_in_this_thread());

		iothreads_check();

		session_ptr->async_send(sv);

	}).bind_connect([&](auto & session_ptr)
	{
		ASIO2_CHECK(iopool.running_in_thread(0));

		session_ptr->start_timer(2, std::chrono::seconds(1), iothreads_check); // test timer
	}).bind_disconnect([&](auto & session_ptr)
	{
		asio2::ignore_unused(session_ptr);
		ASIO2_CHECK(iopool.running_in_thread(0));
	}).bind_start([&]()
	{
		ASIO2_CHECK(iopool.running_in_thread(0));
	}).bind_stop([&]()
	{
		ASIO2_CHECK(iopool.running_in_thread(0));
	});

	server.start("127.0.0.1", 39001);

	//-----------------------------------------------------------------------------------

	index = std::rand() % iopool.size();

	// if you create a object and it will destroyed before iopool.stop(), you must create
	// the object as shared_ptr<asio2::tcp_client>, can't be asio2::tcp_client.
	{
		std::shared_ptr<asio2::tcp_client> tcp_client = std::make_shared<asio2::tcp_client>(iopool.get(index));

		tcp_client->start("127.0.0.1", 39001);

		tcp_client->async_send("i love you", iothread_check);

		//tcp_client->stop();

		std::this_thread::sleep_for(std::chrono::milliseconds(2 + std::rand() % 2));
	}

	asio2::tcp_client client(iopool.get(index));

	client.auto_reconnect(true, std::chrono::milliseconds(1000));

	client.start_timer(1, std::chrono::seconds(1), timer_check); // test timer

	client.bind_connect([&, index]()
	{
		ASIO2_CHECK(iopool.running_in_thread(index));

		std::string str(std::size_t(100 + (std::rand() % 300)), char(std::rand() % 255));

		client.async_send(std::move(str), iothread_check);
	}).bind_disconnect([&]()
	{
		ASIO2_CHECK(iopool.running_in_thread(index));
	}).bind_recv([&, index](std::string_view sv)
	{
		ASIO2_CHECK(iopool.running_in_thread(index));

		client.async_send(sv, iothread_check);
	});

	client.async_start("127.0.0.1", 39001);

	std::this_thread::sleep_for(std::chrono::milliseconds(20 + std::rand() % 10));

	cast1.stop();
	client.stop();
	server.stop();
	t1.stop_all_timers();

	iopool.stop();

	ASIO2_CHECK(client.is_stopped());
	ASIO2_CHECK(server.is_stopped());
	ASIO2_CHECK(cast1 .is_stopped());
	ASIO2_CHECK(iopool.stopped());

	//// --------------------------------------------------------------------------------

	// iopool must start first, othwise the server.start will blocked forever.
	iopool.start();

	t1.post([&t1]()
	{
		t1.start_timer(1, std::chrono::seconds(1), []() {});
	});

	cast1.start_timer(1, std::chrono::seconds(1), []() {});

	ptr = cast1.post_event([]() {});

	cast1.post([]()
	{
	}, std::chrono::milliseconds(std::rand() % 50));

	threads.emplace_back(std::make_shared<std::thread>([ptr]() mutable
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10 + std::rand() % 10));
		ptr->notify();
	}));

	std::this_thread::sleep_for(std::chrono::milliseconds(2 + std::rand() % 2));

	ptr->notify();

	cast1.start("0.0.0.0", 4444);

	std::this_thread::sleep_for(std::chrono::milliseconds(10 + std::rand() % 10));

	cast1.stop();

	cast1.start_timer(1, std::chrono::seconds(1), []() {});

	cast1.post([]()
	{
	}, std::chrono::milliseconds(std::rand() % 50));

	ptr = cast1.post_event([]() {});

	threads.emplace_back(std::make_shared<std::thread>([ptr]() mutable
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10 + std::rand() % 10));
		ptr->notify();
	}));

	ptr.reset();

	ptr = cast1.post_event([]() {});

	cast1.start("0.0.0.0", 4444);

	//-----------------------------------------------------------------------------------

	// the server's io_context must be the user passed io_context.
	for (std::size_t i = 0; i < server.iopool().size(); i++)
	{
		[[maybe_unused]] asio::io_context& ioc1 = iopool.get(i).context();
        [[maybe_unused]] asio::io_context& ioc2 = server.iopool().get(i).context();

		ASIO2_CHECK(&ioc1 == &ioc2);
	}

	server.bind_recv([&](std::shared_ptr<asio2::tcp_session>& session_ptr, std::string_view s)
	{
		ASIO2_CHECK(session_ptr->io().strand().running_in_this_thread());

		session_ptr->async_send(s);
	}).bind_connect([&](std::shared_ptr<asio2::tcp_session>&)
	{
		ASIO2_CHECK(iopool.running_in_thread(0));
	}).bind_disconnect([&](std::shared_ptr<asio2::tcp_session>&)
	{
		ASIO2_CHECK(iopool.running_in_thread(0));
	}).bind_start([&]()
	{
		ASIO2_CHECK(iopool.running_in_thread(0));
	}).bind_stop([&]()
	{
		ASIO2_CHECK(iopool.running_in_thread(0));
	});

	server.start("127.0.0.1", 39001);

	//-----------------------------------------------------------------------------------

	client.start_timer(1, std::chrono::seconds(1), []() {}); // test timer

	client.bind_connect([&]()
	{
		std::string s(std::size_t(100 + (std::rand() % 300)), char(std::rand() % 255));

		client.async_send(std::move(s));
	}).bind_disconnect([&]()
	{
	}).bind_recv([&](std::string_view sv)
	{
		client.async_send(sv);
	});

	client.async_start("127.0.0.1", 39001);

	std::this_thread::sleep_for(std::chrono::milliseconds(20 + std::rand() % 80));

	//cast1.stop();
	//client.stop();
	//server.stop();
	//t1.stop_all_timers();

	// the iopool.stop will cal all "server,client,timer,serial_port,ping" objects stop();
	iopool.stop();

	ASIO2_CHECK(client.is_stopped());
	ASIO2_CHECK(server.is_stopped());
	ASIO2_CHECK(cast1 .is_stopped());
	ASIO2_CHECK(iopool.stopped());

	for (auto& thread : threads)
	{
		//thread->join();
		thread->detach();
	}

	ASIO2_TEST_END_LOOP;
}


ASIO2_TEST_SUITE
(
	"shared_iopool",
	ASIO2_TEST_CASE(shared_iopool_test)
)
