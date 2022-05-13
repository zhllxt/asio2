#include "unit_test.hpp"
#include <asio2/base/timer.hpp>

void timer_test()
{
	ASIO2_TEST_BEGIN_LOOP(test_loop_times / 100);

#define diff 25ll

#undef ASIO2_ENABLE_TIMER_CALLBACK_WHEN_ERROR

	asio2::timer timer1;

	int c1 = 0, c2 = 0, c3 = 0, c4 = 0, c5 = 0, c6 = 0;

	auto t1 = std::chrono::high_resolution_clock::now();
	timer1.start_timer(1, 100, [&c1, &t1]() mutable
	{
		c1++;
		ASIO2_CHECK(!asio2::get_last_error());
		auto elapse1 = std::abs(std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::high_resolution_clock::now() - t1).count() - 100);
		ASIO2_CHECK_VALUE(elapse1, elapse1 < diff);
		t1 = std::chrono::high_resolution_clock::now();
	});

	auto t2 = std::chrono::high_resolution_clock::now();
	timer1.start_timer("id2", 500, 9, [&c2, &t2]() mutable
	{
		c2++;
		ASIO2_CHECK(!asio2::get_last_error());
		auto elapse2 = std::abs(std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::high_resolution_clock::now() - t2).count() - 500);
		ASIO2_CHECK_VALUE(elapse2, elapse2 < diff);
		t2 = std::chrono::high_resolution_clock::now();
	});

	auto t3 = std::chrono::high_resolution_clock::now();
	timer1.start_timer(3, std::chrono::milliseconds(1100), [&c3, &t3]() mutable
	{
		c3++;
		ASIO2_CHECK(!asio2::get_last_error());
		auto elapse3 = std::abs(std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::high_resolution_clock::now() - t3).count() - 1100);
		ASIO2_CHECK_VALUE(elapse3, elapse3 < diff);
		t3 = std::chrono::high_resolution_clock::now();
	});

	auto t4 = std::chrono::high_resolution_clock::now();
	timer1.start_timer(4, std::chrono::milliseconds(600), 7, [&c4, &t4]() mutable
	{
		c4++;
		ASIO2_CHECK(!asio2::get_last_error());
		auto elapse4 = std::abs(std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::high_resolution_clock::now() - t4).count() - 600);
		ASIO2_CHECK_VALUE(elapse4, elapse4 < diff);
		t4 = std::chrono::high_resolution_clock::now();
	});

	bool f5 = true;
	auto t5 = std::chrono::high_resolution_clock::now();
	timer1.start_timer(5, std::chrono::milliseconds(1000), std::chrono::milliseconds(2300), [&c5, &t5, &f5]() mutable
	{
		c5++;
		ASIO2_CHECK(!asio2::get_last_error());
		if (f5)
		{
			f5 = false;
			auto elapse5 = std::abs(std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::high_resolution_clock::now() - t5).count() - 2300);
			ASIO2_CHECK_VALUE(elapse5, elapse5 < diff);
		}
		else
		{
			auto elapse5 = std::abs(std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::high_resolution_clock::now() - t5).count() - 1000);
			ASIO2_CHECK_VALUE(elapse5, elapse5 < diff);
		}
		t5 = std::chrono::high_resolution_clock::now();
	});

	bool f6 = true;
	auto t6 = std::chrono::high_resolution_clock::now();
	timer1.start_timer(6, std::chrono::milliseconds(400), 6, std::chrono::milliseconds(2200), [&c6, &t6, &f6]() mutable
	{
		c6++;
		ASIO2_CHECK(!asio2::get_last_error());
		if (f6)
		{
			f6 = false;
			auto elapse6 = std::abs(std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::high_resolution_clock::now() - t6).count() - 2200);
			ASIO2_CHECK_VALUE(elapse6, elapse6 < diff);
		}
		else
		{
			auto elapse6 = std::abs(std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::high_resolution_clock::now() - t6).count() - 400);
			ASIO2_CHECK_VALUE(elapse6, elapse6 < diff);
		}
		t6 = std::chrono::high_resolution_clock::now();
	});

	std::this_thread::sleep_for(std::chrono::milliseconds(5000));

	// The actual timeout of the timer is about 10 milliseconds more than the preset timeout
	ASIO2_CHECK_VALUE(c1, c1 >= (5000 - (5000 / 100 * 10)) / 100 && c1 <= 5000 / 100);
	ASIO2_CHECK_VALUE(c2, c2 == 9);
	ASIO2_CHECK_VALUE(c3, c3 >= 4 && c3 < 5);
	ASIO2_CHECK_VALUE(c4, c4 == 7);
	ASIO2_CHECK_VALUE(c5, c5 >= 3 && c5 < 4);
	ASIO2_CHECK_VALUE(c6, c6 == 6);

	timer1.stop_all_timers();

	ASIO2_TEST_END_LOOP;
}


ASIO2_TEST_SUITE
(
	"timer",
	ASIO2_TEST_CASE(timer_test)
)
