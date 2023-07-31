#include "unit_test.hpp"
#include <asio2/util/uuid.hpp>
#include <asio2/util/string.hpp>
#include <vector>
#include <unordered_set>
#include <ctype.h>
#include <locale>

void uuid_test()
{
	std::string number = "0123456789";
	std::string lowers = "abcdef";
	std::string uppers = "ABCDEF";
	std::string empty = "00000000-0000-0000-0000-000000000000";

	static std::string const short_uuid_chars =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";

	std::vector<int> short_uuid_chars_flags;
	short_uuid_chars_flags.resize(short_uuid_chars.size());

	std::unordered_set<std::string> uuid_set;

	ASIO2_TEST_BEGIN_LOOP(test_loop_times);

	std::uint8_t d1[16]{};
	std::uint8_t d2[16]{};
	std::uint8_t d3[16]{};
	std::uint8_t d4[16]{};

	asio2::uuid u1;

	u1.generate();
	std::memcpy(d1, u1.data, 16);
	ASIO2_CHECK(u1.version() == asio2::uuid::version_type::random_number_based);
	ASIO2_CHECK(u1.variant() == asio2::uuid::variant_type::rfc);

	{
		std::string str = u1.str(false, true);

		ASIO2_CHECK(str.size() == 36);

		bool has_upper = false, has_group = false;

		for (auto c : str)
		{
			if (c == '-')
			{
				has_group = true;
			}
			if (uppers.find(c) != std::string::npos)
			{
				has_upper = true;
			}
		}

		ASIO2_CHECK(has_upper == false);
		ASIO2_CHECK(has_group == true);

		for (std::size_t i = 0; i < empty.size(); ++i)
		{
			if (empty[i] == '-')
			{
				ASIO2_CHECK(str[i] == '-');
			}
			else
			{
				ASIO2_CHECK(lowers.find(str[i]) != std::string::npos || number.find(str[i]) != std::string::npos);
			}
		}

		asio2::replace(str, "-", "");
		ASIO2_CHECK(str.find('-') == std::string::npos);
		ASIO2_CHECK(str.size() == 32);
		bool inserted = uuid_set.emplace(std::move(str)).second;
		ASIO2_CHECK(inserted);
	}

	u1.generate();
	std::memcpy(d2, u1.data, 16);
	ASIO2_CHECK(u1.version() == asio2::uuid::version_type::random_number_based);
	ASIO2_CHECK(u1.variant() == asio2::uuid::variant_type::rfc);

	{
		std::string str = u1.str(true, true);

		ASIO2_CHECK(str.size() == 36);

		bool has_lower = false, has_group = false;

		for (auto c : str)
		{
			if (c == '-')
			{
				has_group = true;
			}
			if (lowers.find(c) != std::string::npos)
			{
				has_lower = true;
			}
		}

		ASIO2_CHECK(has_lower == false);
		ASIO2_CHECK(has_group == true);

		for (std::size_t i = 0; i < empty.size(); ++i)
		{
			if (empty[i] == '-')
			{
				ASIO2_CHECK(str[i] == '-');
			}
			else
			{
				ASIO2_CHECK(uppers.find(str[i]) != std::string::npos || number.find(str[i]) != std::string::npos);
			}
		}

		asio2::replace(str, "-", "");
		ASIO2_CHECK(str.find('-') == std::string::npos);
		ASIO2_CHECK(str.size() == 32);
		std::transform(str.begin(), str.end(), str.begin(), [](char c) { return (char)std::tolower(c); });
		bool inserted = uuid_set.emplace(std::move(str)).second;
		ASIO2_CHECK(inserted);
	}

	u1.generate();
	std::memcpy(d3, u1.data, 16);
	ASIO2_CHECK(u1.version() == asio2::uuid::version_type::random_number_based);
	ASIO2_CHECK(u1.variant() == asio2::uuid::variant_type::rfc);

	{
		std::string str = u1.str(false, false);

		ASIO2_CHECK(str.size() == 32);

		bool has_upper = false, has_group = false;

		for (auto c : str)
		{
			if (c == '-')
			{
				has_group = true;
			}
			if (uppers.find(c) != std::string::npos)
			{
				has_upper = true;
			}
		}

		ASIO2_CHECK(has_upper == false);
		ASIO2_CHECK(has_group == false);

		bool inserted = uuid_set.emplace(std::move(str)).second;
		ASIO2_CHECK(inserted);
	}

	u1.generate();
	std::memcpy(d4, u1.data, 16);
	ASIO2_CHECK(u1.version() == asio2::uuid::version_type::random_number_based);
	ASIO2_CHECK(u1.variant() == asio2::uuid::variant_type::rfc);

	{
		std::string str = u1.str(true, false);

		ASIO2_CHECK(str.size() == 32);

		bool has_lower = false, has_group = false;

		for (auto c : str)
		{
			if (c == '-')
			{
				has_group = true;
			}
			if (lowers.find(c) != std::string::npos)
			{
				has_lower = true;
			}
		}

		ASIO2_CHECK(has_lower == false);
		ASIO2_CHECK(has_group == false);

		std::transform(str.begin(), str.end(), str.begin(), [](char c) { return (char)std::tolower(c); });
		bool inserted = uuid_set.emplace(std::move(str)).second;
		ASIO2_CHECK(inserted);
	}

	while (true)
	{
		int len = 8 + std::rand() % 24;

		std::string s2 = u1.short_uuid(len);

		ASIO2_CHECK(s2.size() == std::size_t(len));

		for (auto c : s2)
		{
			auto pos = short_uuid_chars.find(c);
			ASIO2_CHECK(pos != std::string::npos);
			short_uuid_chars_flags[pos] = 1;
		}

		bool finded_0 = false;

		for (auto i : short_uuid_chars_flags)
		{
			if (i != 1)
			{
				finded_0 = true;
				break;
			}
		}

		if (!finded_0)
			break;
	}

	ASIO2_CHECK(std::memcmp(d1, d2, 16) != 0);
	ASIO2_CHECK(std::memcmp(d1, d3, 16) != 0);
	ASIO2_CHECK(std::memcmp(d1, d4, 16) != 0);
	ASIO2_CHECK(std::memcmp(d2, d3, 16) != 0);
	ASIO2_CHECK(std::memcmp(d2, d4, 16) != 0);
	ASIO2_CHECK(std::memcmp(d3, d4, 16) != 0);

	ASIO2_TEST_END_LOOP;

	for (auto i : short_uuid_chars_flags)
	{
		ASIO2_CHECK(i == 1);
	}
}


ASIO2_TEST_SUITE
(
	"uuid",
	ASIO2_TEST_CASE(uuid_test)
)
