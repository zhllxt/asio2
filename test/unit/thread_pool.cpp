#include "unit_test.hpp"
#include <asio2/util/thread_pool.hpp>
#include <set>

std::atomic<int> c1 = 0;

void test1()
{
	c1++;
}

void test2(int i)
{
	c1 += i;
}

struct member_test
{
	void test1()
	{
		c1++;
	}

	void test2(int i)
	{
		c1 += i;
	}
};

void thread_pool_test()
{
	ASIO2_TEST_BEGIN_LOOP(test_loop_times);

	{
		c1 = 0;

		{
			asio2::thread_pool thpool(2);
			thpool.post([&]()
			{
				c1++;

				ASIO2_CHECK(thpool.pool_size() == 2);
				ASIO2_CHECK(thpool.task_size() == 0);
				ASIO2_CHECK(thpool.running_in_threads());
			});
		}

		ASIO2_CHECK_VALUE(c1.load(), c1 == 1);
	}

	{
		c1 = 0;

		{
			asio2::thread_pool thpool;
			thpool.post([&]()
			{
				c1++;
				ASIO2_CHECK(thpool.pool_size() == std::thread::hardware_concurrency());
				ASIO2_CHECK(thpool.running_in_threads());
			});
			thpool.post([&]() mutable
			{
				c1++;
				ASIO2_CHECK(thpool.pool_size() == std::thread::hardware_concurrency());
				ASIO2_CHECK(thpool.running_in_threads());
			});
			thpool.post(test1);
			thpool.post(std::bind(test1));
			thpool.post(test2, 1);
			thpool.post(test2, 2);
			thpool.post(std::bind(test2, 1));
			thpool.post(std::bind(test2, 2));

			member_test membertest;
			thpool.post(&member_test::test1, membertest);
			thpool.post(&member_test::test2, membertest, 1);
			thpool.post(&member_test::test2, membertest, 2);
			thpool.post(std::bind(&member_test::test1, membertest));
			thpool.post(std::bind(&member_test::test2, membertest, 1));
			thpool.post(std::bind(&member_test::test2, membertest, 2));
		}

		ASIO2_CHECK_VALUE(c1.load(), c1 == 18);
	}

	{
		asio2::thread_group thpool(4);

		thpool.post(0, [&thpool]()
		{
			ASIO2_CHECK(std::this_thread::get_id() == thpool.thread_id(0));
			ASIO2_CHECK(std::this_thread::get_id() == thpool.get_thread_id(0));
			ASIO2_CHECK(thpool.running_in_thread(0));
			ASIO2_CHECK(thpool.running_in_threads());
		});

		thpool.post(1, [&thpool]()
		{
			ASIO2_CHECK(std::this_thread::get_id() == thpool.thread_id(1));
			ASIO2_CHECK(std::this_thread::get_id() == thpool.get_thread_id(1));
			ASIO2_CHECK(thpool.running_in_thread(1));
			ASIO2_CHECK(thpool.running_in_threads());
		});

		thpool.post(2, [&thpool]()
		{
			ASIO2_CHECK(std::this_thread::get_id() == thpool.thread_id(2));
			ASIO2_CHECK(std::this_thread::get_id() == thpool.get_thread_id(2));
			ASIO2_CHECK(thpool.running_in_thread(2));
			ASIO2_CHECK(thpool.running_in_threads());
		});

		for (std::size_t i = 0; i < thpool.get_pool_size(); ++i)
		{
			thpool.post(i, [&thpool, i]()
			{
				ASIO2_CHECK(std::this_thread::get_id() == thpool.get_thread_id(i));
			});
		}

		for (int i = 0; i < test_loop_times / 10; ++i)
		{
			int thread_index = std::rand();

			thpool.post(thread_index, []()
			{
				std::this_thread::yield();
			});

			thpool.post(thread_index, [&thpool, thread_index]()
			{
				ASIO2_CHECK(std::this_thread::get_id() == thpool.get_thread_id(thread_index % thpool.get_pool_size()));
			});
		}
	}

	for (int n = 0; n < 10; n++)
	{
		asio2::thread_group thpool;

		std::mutex mtx;

		std::set<std::thread::id> thread_ids_set;

		std::vector<std::future<void>> futures;

		for (std::size_t i = 0; i < thpool.get_pool_size(); ++i)
		{
			auto future = thpool.post(i, [&thread_ids_set, &mtx]()
			{
				std::lock_guard g(mtx);
				thread_ids_set.emplace(std::this_thread::get_id());
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			});

			futures.emplace_back(std::move(future));
		}

		for (auto& f : futures)
		{
			f.wait();
		}

		ASIO2_CHECK(thread_ids_set.size() == thpool.get_pool_size());
	}

	ASIO2_TEST_END_LOOP;
}


ASIO2_TEST_SUITE
(
	"thread_pool",
	ASIO2_TEST_CASE(thread_pool_test)
)
