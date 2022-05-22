#include "unit_test.hpp"
#include <asio2/util/des.hpp>
#include <asio2/util/base64.hpp>

void des_test()
{
	std::string src = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

	// maybe has some problem : the result is not equal to the http://tool.chacuo.net/cryptdes
	{
		std::string password = "12345678";
		std::string base64 = "i7R6DPCpYm0p0XQDeov2tobw/wQnmEOY8b39qad9yVw/D0VIesdH4k/eTIE/iV8rj6zrx2IywoPIkiYhu0/bRQ==";
		asio2::des des1(password);
		std::string en = des1.encrypt(src);
		std::string de = des1.decrypt(en);
		//ASIO2_CHECK(asio2::base64_encode(en) == base64);
		ASIO2_CHECK(src == de);
	}

	ASIO2_TEST_BEGIN_LOOP(test_loop_times);

	std::string str;
	{
		int len = 500 + std::rand() % (500);
		for (int i = 0; i < len; i++)
		{
			// the character should not be '\0'
			str += (char)(1 + std::rand() % 255);
		}
	}

	{
		std::string password;
		int len = std::rand() % (10);
		for (int i = 0; i < len; i++)
		{
			password += (char)(std::rand() % 255);
		}

		asio2::des des1(password);

		std::string en = des1.encrypt(str);
		std::string de = des1.decrypt(en);

		ASIO2_CHECK(str == de);
	}

	ASIO2_TEST_END_LOOP;
}


ASIO2_TEST_SUITE
(
	"des",
	ASIO2_TEST_CASE(des_test)
)
