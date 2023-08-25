#include <asio2/base/detail/push_options.hpp>

#if defined(__GNUC__) || defined(__GNUG__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wunused-value"
#  pragma GCC diagnostic ignored "-Wreorder"
#endif


#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wunused-value"
#  pragma clang diagnostic ignored "-Wreorder-ctor"
#endif

#include <type_traits>

#define ASIO_STANDALONE

#if (defined(__cplusplus) && __cplusplus >= 202002L) || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L)
namespace std
{
	template <typename> struct result_of;
	template <typename F, typename... Args>
	struct result_of<F(Args...)> : std::invoke_result<F, Args...> {};
}
#endif

// https://github.com/qicosmos/rest_rpc

// first : unzip the "asio2/test/bench/rpc/rest_rpc-master.zip"
// the unziped path is like this : "asio2/test/bench/rpc/rest_rpc-master/include"

#include <fstream>
#include <memory>
#include <chrono>
#include <thread>
#include <rpc_server.h>
using namespace rest_rpc;
using namespace rpc_service;

decltype(std::chrono::steady_clock::now()) time1 = std::chrono::steady_clock::now();
decltype(std::chrono::steady_clock::now()) time2 = std::chrono::steady_clock::now();
std::size_t qps = 0;
bool first = true;

std::string echo(rpc_conn conn, std::string a) {

	if (first)
	{
		first = false;
		time1 = std::chrono::steady_clock::now();
		time2 = std::chrono::steady_clock::now();
	}

	qps++;
	decltype(std::chrono::steady_clock::now()) time3 = std::chrono::steady_clock::now();
	auto ms = std::chrono::duration_cast<std::chrono::seconds>(time3 - time2).count();
	if (ms > 1)
	{
		time2 = time3;
		ms = std::chrono::duration_cast<std::chrono::seconds>(time3 - time1).count();
		double speed = (double)qps / (double)ms;
		printf("%.1lf\n", speed);
	}

	return a;
}


int main() {
	rpc_server server(8080, std::thread::hardware_concurrency());

	server.register_handler("echo", echo);

	server.run();

	return 0;
}

#include <asio2/base/detail/pop_options.hpp>
