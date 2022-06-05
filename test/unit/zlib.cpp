#include "unit_test.hpp"
#include <asio2/util/zlib.hpp>
#include <asio2/util/base64.hpp>
#include <fmt/format.h>

void zlib_test()
{
	beast::zlib::impl zliber;

	std::string src = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	std::string dst = "MjA0MjYxNTO3sExMSk5JTUvPyMzKzsnNyy8oLCouKS0rr6iscnRydnF1c/fw9PL28fXzDwgMCg4JDQuPiIwCAAAA//8=";

	std::string en = zliber.compress(src);

	//if (!(en[0] == 0x50 && en[1] == 0x4b && en[2] == 0x03 && en[3] == 0x04))
	//{
	//	fmt::print("{:02X} {:02X} {:02X} {:02X}\n", en[0], en[1], en[2], en[3]);
	//}

	std::string base64 = asio2::base64_encode(en);

	ASIO2_CHECK(base64 == dst);
	ASIO2_CHECK(zliber.uncompress(asio2::base64_decode(base64)) == src);
}


ASIO2_TEST_SUITE
(
	"zlib",
	ASIO2_TEST_CASE(zlib_test)
)
