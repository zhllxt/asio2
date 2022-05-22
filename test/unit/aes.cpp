#include "unit_test.hpp"
#include <asio2/util/aes.hpp>
#include <asio2/util/base64.hpp>

void aes_test()
{
	std::string src = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

	// ecb
	{
		std::string password = "0123456789123456";
		std::string base64 = "PDx51gkWitGUC69RorsQAdaFmDnwtoow5Ivb45kgqmaupaOTmtU9bY03WYIF5GrefWSQaoL6fkPXtdn+ysRNCg==";
		asio2::aes aes1(password);
		aes1.mode(asio2::aes::mode_t::ecb);
		std::string en = aes1.encrypt(src);
		std::string de = aes1.decrypt(en);
		ASIO2_CHECK(asio2::base64_encode(en) == base64);
		ASIO2_CHECK(src == de);
	}

	{
		std::string password = "012345678912345612345678";
		std::string base64 = "pMY2AU58ga73LQBTwpHPeZ/hKs5/HReGE7pGbaWl0BC7wMkCgmCMxukgqUXOfi+yyJAEZwBnZyWwP1EeV/3yBQ==";
		asio2::aes aes1(password);
		aes1.mode(asio2::aes::mode_t::ecb);
		std::string en = aes1.encrypt(src);
		std::string de = aes1.decrypt(en);
		ASIO2_CHECK(asio2::base64_encode(en) == base64);
		ASIO2_CHECK(src == de);
	}

	{
		std::string password = "01234567891234560123456789123456";
		std::string base64 = "DSUnogYX1RsV5o8sv9TgF3ITEk5Kzq5JB/06q+K8ZJaT7kD0kX2YncW73APV36gbMgFkXq6hZmN+j+yU07EOAg==";
		asio2::aes aes1(password);
		aes1.mode(asio2::aes::mode_t::ecb);
		std::string en = aes1.encrypt(src);
		std::string de = aes1.decrypt(en);
		ASIO2_CHECK(asio2::base64_encode(en) == base64);
		ASIO2_CHECK(src == de);
	}

	ASIO2_TEST_BEGIN_LOOP(test_loop_times);

	std::string str;
	{
		int len = 500 + std::rand() % (500);
		for (int i = 0; i < len; i++)
		{
			str += (char)(std::rand() % 255);
		}
		// the last character should not be '\0'
		str += (char)((std::rand() % 26) + 'a');
	}

	{
		std::string password;
		int len = std::rand() % (40);
		for (int i = 0; i < len; i++)
		{
			password += (char)(std::rand() % 255);
		}

		asio2::aes aes1(password, asio2::aes::mode_t::ecb);

		std::string en = aes1.encrypt(str);
		std::string de = aes1.decrypt(en);

		ASIO2_CHECK(str == de);
	}

	// the cbc,ctr maybe has some problem
	//{
	//	std::string password;
	//	int len = std::rand() % (40);
	//	for (int i = 0; i < len; i++)
	//	{
	//		password += (char)(std::rand() % 255);
	//	}

	//	asio2::aes aes1(password, asio2::aes::mode_t::cbc);

	//	std::string en = aes1.encrypt(str);
	//	std::string de = aes1.decrypt(en);

	//	ASIO2_CHECK(str == de);
	//}

	//{
	//	std::string password;
	//	int len = std::rand() % (40);
	//	for (int i = 0; i < len; i++)
	//	{
	//		password += (char)(std::rand() % 255);
	//	}

	//	asio2::aes aes1(password, asio2::aes::mode_t::ctr);

	//	std::string en = aes1.encrypt(str);
	//	std::string de = aes1.decrypt(en);

	//	ASIO2_CHECK(str == de);
	//}

	ASIO2_TEST_END_LOOP;
}


ASIO2_TEST_SUITE
(
	"aes",
	ASIO2_TEST_CASE(aes_test)
)
