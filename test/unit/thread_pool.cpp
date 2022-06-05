#include "unit_test.hpp"
#include <asio2/util/thread_pool.hpp>

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

	ASIO2_TEST_END_LOOP;
}


ASIO2_TEST_SUITE
(
	"thread_pool",
	ASIO2_TEST_CASE(thread_pool_test)
)
