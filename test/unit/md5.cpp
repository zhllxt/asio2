#include "unit_test.hpp"

#include <asio2/util/md5.hpp>

void md5_test()
{
	asio2::md5 md5;

	std::string src = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

	ASIO2_CHECK(asio2::md5(src).str(true ) == "B9B3CC3F3A30D8EF2BB1E2E267ED97DE");
	ASIO2_CHECK(asio2::md5(src).str(false) == "b9b3cc3f3a30d8ef2bb1e2e267ed97de");
}


ASIO2_TEST_SUITE
(
	"md5",
	ASIO2_TEST_CASE(md5_test)
)
