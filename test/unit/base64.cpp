#include "unit_test.hpp"
#include <asio2/util/base64.hpp>

void base64_test()
{
	std::string src = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	std::string dst = "MDEyMzQ1Njc4OWFiY2RlZmdoaWprbG1ub3BxcnN0dXZ3eHl6QUJDREVGR0hJSktMTU5PUFFSU1RVVldYWVo=";

	std::string en = asio2::base64().encode(src);
	std::string de = asio2::base64().decode(dst);

	ASIO2_CHECK(en == dst);
	ASIO2_CHECK(de == src);

	ASIO2_TEST_BEGIN_LOOP(test_loop_times);

	std::string str;
	int len = 500 + std::rand() % (500);
	for (int i = 0; i < len; i++)
	{
		str += (char)(std::rand() % 255);
	}

	ASIO2_CHECK(asio2::base64_decode(asio2::base64_encode(str)) == str);

	ASIO2_TEST_END_LOOP;
}


ASIO2_TEST_SUITE
(
	"base64",
	ASIO2_TEST_CASE(base64_test)
)
