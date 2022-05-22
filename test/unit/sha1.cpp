#include "unit_test.hpp"
#include <asio2/util/sha1.hpp>
#include <asio2/util/base64.hpp>

void sha1_test()
{
	std::string src = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

	{
		ASIO2_CHECK(asio2::sha1(src).str(true ) == "ECCB8B0E82E3B9907D52FEB72ABAF813B539C77D");
		ASIO2_CHECK(asio2::sha1(src).str(false) == "eccb8b0e82e3b9907d52feb72abaf813b539c77d");
	}

	//ASIO2_TEST_BEGIN_LOOP(test_loop_times);

	//ASIO2_TEST_END_LOOP;
}


ASIO2_TEST_SUITE
(
	"sha1",
	ASIO2_TEST_CASE(sha1_test)
)
