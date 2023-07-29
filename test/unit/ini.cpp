#include "unit_test.hpp"
#include <asio2/util/ini.hpp>
#include <cmath>
#include <cstdlib>

void ini_test()
{
	ASIO2_TEST_BEGIN_LOOP(test_loop_times);

	asio2::ini ini;

	std::error_code ec;
	std::filesystem::resize_file(ini.filepath(), 0, ec);

	ASIO2_CHECK(std::filesystem::path(ini.filepath()).filename().string() == "test_ini.ini");

	ASIO2_CHECK(ini.get<bool>("number", "boolean",  true ) == true );
	ASIO2_CHECK(ini.get<bool>("number", "boolean",  false) == false);

	ASIO2_CHECK(ini.get<         char>("number", "ichar", 'a') == 'a');
	ASIO2_CHECK(ini.get<unsigned char>("number", "uchar", 250) == 250);

	ASIO2_CHECK(ini.get<std:: int8_t>("number", "i8",  99) ==  99);
	ASIO2_CHECK(ini.get<std:: int8_t>("number", "i8", -99) == -99);
	ASIO2_CHECK(ini.get<std::uint8_t>("number", "u8",  99) ==  99);

	ASIO2_CHECK(ini.get<std:: int16_t>("number", "i16",  9999) ==  9999);
	ASIO2_CHECK(ini.get<std:: int16_t>("number", "i16", -9999) == -9999);
	ASIO2_CHECK(ini.get<std::uint16_t>("number", "u16",  9999) ==  9999);

	ASIO2_CHECK(ini.get<std:: int32_t>("number", "i32",  999999) ==  999999);
	ASIO2_CHECK(ini.get<std:: int32_t>("number", "i32", -999999) == -999999);
	ASIO2_CHECK(ini.get<std::uint32_t>("number", "u32",  999999) ==  999999);

	ASIO2_CHECK(ini.get<std:: int64_t>("number", "i64",  9999999999ll ) ==  9999999999ll );
	ASIO2_CHECK(ini.get<std:: int64_t>("number", "i64", -9999999999ll ) == -9999999999ll );
	ASIO2_CHECK(ini.get<std::uint64_t>("number", "u64",  9999999999llu) ==  9999999999llu);

	ASIO2_CHECK(std::fabs(ini.get<float >("number", "sf",  99.99f) -  99.99f) < 0.0001f);
	ASIO2_CHECK(std::fabs(ini.get<double>("number", "df", 999.99 ) - 999.99 ) < 0.0001 );

	ASIO2_CHECK(ini.get<std::size_t>("number", "su", 99999) == 99999);

	ASIO2_CHECK(ini.get<std::string>("string", "str", "abc!=xyz") == "abc!=xyz");

	//---------------------------------------------------------------------------------------------

	ASIO2_CHECK(ini.get<bool>("", "boolean",  true ) == true );
	ASIO2_CHECK(ini.get<bool>("", "boolean",  false) == false);

	ASIO2_CHECK(ini.get<         char>("", "ichar", 'a') == 'a');
	ASIO2_CHECK(ini.get<unsigned char>("", "uchar", 250) == 250);

	ASIO2_CHECK(ini.get<std:: int8_t>("", "i8",  99) ==  99);
	ASIO2_CHECK(ini.get<std:: int8_t>("", "i8", -99) == -99);
	ASIO2_CHECK(ini.get<std::uint8_t>("", "u8",  99) ==  99);

	ASIO2_CHECK(ini.get<std:: int16_t>("", "i16",  9999) ==  9999);
	ASIO2_CHECK(ini.get<std:: int16_t>("", "i16", -9999) == -9999);
	ASIO2_CHECK(ini.get<std::uint16_t>("", "u16",  9999) ==  9999);

	ASIO2_CHECK(ini.get<std:: int32_t>("", "i32",  999999) ==  999999);
	ASIO2_CHECK(ini.get<std:: int32_t>("", "i32", -999999) == -999999);
	ASIO2_CHECK(ini.get<std::uint32_t>("", "u32",  999999) ==  999999);

	ASIO2_CHECK(ini.get<std:: int64_t>("", "i64",  9999999999ll ) ==  9999999999ll );
	ASIO2_CHECK(ini.get<std:: int64_t>("", "i64", -9999999999ll ) == -9999999999ll );
	ASIO2_CHECK(ini.get<std::uint64_t>("", "u64",  9999999999llu) ==  9999999999llu);

	ASIO2_CHECK(std::fabs(ini.get<float >("", "sf",  99.99f) -  99.99f) < 0.0001f);
	ASIO2_CHECK(std::fabs(ini.get<double>("", "df", 999.99 ) - 999.99 ) < 0.0001 );

	ASIO2_CHECK(ini.get<std::size_t>("", "su", 99999) == 99999);

	ASIO2_CHECK(ini.get<std::string>("", "str", "abc!=xyz") == "abc!=xyz");

	//---------------------------------------------------------------------------------------------

	bool boolean = ((loop % 2) == 0);
	std::int8_t sign = (boolean ? 1 : -1);
	char ichar = (std::rand() % (std::numeric_limits<         char>::max)());
	char uchar = (std::rand() % (std::numeric_limits<unsigned char>::max)());
	std::  int8_t i8  = (std::rand() % (std::numeric_limits<std::  int8_t>::max)()) * sign;
	std:: uint8_t u8  = (std::rand() % (std::numeric_limits<std:: uint8_t>::max)());
	std:: int16_t i16 = (std::rand() % (std::numeric_limits<std:: int16_t>::max)()) * sign;
	std::uint16_t u16 = (std::rand() % (std::numeric_limits<std::uint16_t>::max)());
	std:: int32_t i32 = (std::rand() % (std::numeric_limits<std:: int32_t>::max)()) * sign;
	std::uint32_t u32 = (std::rand() % (std::numeric_limits<std::uint32_t>::max)());
	std:: int64_t i64 = (std::rand() % (std::numeric_limits<std:: int64_t>::max)()) * sign;
	std::uint64_t u64 = (std::rand() % (std::numeric_limits<std::uint64_t>::max)());
	std::size_t su = (std::rand() % (std::numeric_limits<std::size_t>::max)());
	float  sf = ( float(std::rand()) /  float(RAND_MAX) / 2.0f) *  float(sign);
	double df = (double(std::rand()) / double(RAND_MAX) / 2.0f) * double(sign);
	std::string str;
	for (int i = std::rand() % 50 + 5; i >= 0; --i)
		str += '!' + (std::rand() % (126 - 33));

	//---------------------------------------------------------------------------------------------

	ini.set("number", "boolean", boolean);
	ASIO2_CHECK(ini.get<bool>("number", "boolean") == boolean);

	ini.set("number", "boolean", "true");
	ASIO2_CHECK(ini.get<bool>("number", "boolean") == true);

	ini.set("number", "boolean", "false");
	ASIO2_CHECK(ini.get<bool>("number", "boolean") == false);

	ini.set("number", "ichar", ichar);
	ASIO2_CHECK(ini.get<char>("number", "ichar") == ichar);

	ini.set("number", "uchar", uchar);
	ASIO2_CHECK(ini.get<char>("number", "uchar") == uchar);

	ini.set("number", "i8", i8);
	ASIO2_CHECK(ini.get<std::int8_t>("number", "i8") == i8);

	ini.set("number", "u8", u8);
	ASIO2_CHECK(ini.get<std::uint8_t>("number", "u8") == u8);

	ini.set("number", "i16", i16);
	ASIO2_CHECK(ini.get<std::int16_t>("number", "i16") == i16);

	ini.set("number", "u16", u16);
	ASIO2_CHECK(ini.get<std::uint16_t>("number", "u16") == u16);

	ini.set("number", "i32", i32);
	ASIO2_CHECK(ini.get<std::int32_t>("number", "i32") == i32);

	ini.set("number", "u32", u32);
	ASIO2_CHECK(ini.get<std::uint32_t>("number", "u32") == u32);

	ini.set("number", "i64", i64);
	ASIO2_CHECK(ini.get<std::int64_t>("number", "i64") == i64);

	ini.set("number", "u64", u64);
	ASIO2_CHECK(ini.get<std::uint64_t>("number", "u64") == u64);

	ini.set("number", "su", su);
	ASIO2_CHECK(ini.get<std::size_t>("number", "su") == su);

	ini.set("number", "sf", sf);
	ASIO2_CHECK(std::fabs(ini.get<float>("number", "sf") - sf) < 0.0001f);

	ini.set("number", "df", df);
	ASIO2_CHECK(std::fabs(ini.get<double>("number", "df") - df) < 0.0001);

	ini.set("string", "str", str);
	ASIO2_CHECK(ini.get<std::string>("string", "str") == str);

	ini.set("number", "i32", "xyz");
	ASIO2_CHECK(ini.get<std::int32_t>("number", "i32") == std::int32_t{});

	ASIO2_CHECK(ini.get<std::string>("string", "no") == "");

	//---------------------------------------------------------------------------------------------

	ini.set("", "boolean", boolean);
	ASIO2_CHECK(ini.get<bool>("", "boolean") == boolean);

	ini.set("", "boolean", "true");
	ASIO2_CHECK(ini.get<bool>("", "boolean") == true);

	ini.set("", "boolean", "false");
	ASIO2_CHECK(ini.get<bool>("", "boolean") == false);

	ini.set("", "ichar", ichar);
	ASIO2_CHECK(ini.get<char>("", "ichar") == ichar);

	ini.set("", "uchar", uchar);
	ASIO2_CHECK(ini.get<char>("", "uchar") == uchar);

	ini.set("", "i8", i8);
	ASIO2_CHECK(ini.get<std::int8_t>("", "i8") == i8);

	ini.set("", "u8", u8);
	ASIO2_CHECK(ini.get<std::uint8_t>("", "u8") == u8);

	ini.set("", "i16", i16);
	ASIO2_CHECK(ini.get<std::int16_t>("", "i16") == i16);

	ini.set("", "u16", u16);
	ASIO2_CHECK(ini.get<std::uint16_t>("", "u16") == u16);

	ini.set("", "i32", i32);
	ASIO2_CHECK(ini.get<std::int32_t>("", "i32") == i32);

	ini.set("", "u32", u32);
	ASIO2_CHECK(ini.get<std::uint32_t>("", "u32") == u32);

	ini.set("", "i64", i64);
	ASIO2_CHECK(ini.get<std::int64_t>("", "i64") == i64);

	ini.set("", "u64", u64);
	ASIO2_CHECK(ini.get<std::uint64_t>("", "u64") == u64);

	ini.set("", "su", su);
	ASIO2_CHECK(ini.get<std::size_t>("", "su") == su);

	ini.set("", "sf", sf);
	ASIO2_CHECK(std::fabs(ini.get<float>("", "sf") - sf) < 0.0001f);

	ini.set("", "df", df);
	ASIO2_CHECK(std::fabs(ini.get<double>("", "df") - df) < 0.0001);

	ini.set("", "str", str);
	ASIO2_CHECK(ini.get<std::string>("", "str") == str);

	ini.set("", "i32", "xyz");
	ASIO2_CHECK(ini.get<std::int32_t>("", "i32") == std::int32_t{});

	ASIO2_CHECK(ini.get<std::string>("", "no") == "");

	ASIO2_TEST_END_LOOP;
}


ASIO2_TEST_SUITE
(
	"ini",
	ASIO2_TEST_CASE(ini_test)
)
