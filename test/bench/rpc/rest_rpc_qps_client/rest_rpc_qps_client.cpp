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

#if defined(_MSC_VER)
#pragma warning(disable:4189)
#pragma warning(disable:4244)
#endif

#include <thread>
#include <iostream>
#include <rpc_client.hpp>
#include <chrono>
#include <fstream>
#include "codec.h"
#include <string>
using namespace rest_rpc;
using namespace rest_rpc::rpc_service;

std::string str(128, 'A');

void test_performance1() {
	rpc_client client("127.0.0.1", 8080);
	bool r = client.connect();
	if (!r) {
		std::cout << "connect timeout" << std::endl;
		return;
	}

	for (;;) {
		auto future = client.async_call<FUTURE>("echo", str);
		auto status = future.wait_for(std::chrono::seconds(2));
		if (status == std::future_status::deferred) {
			std::cout << "deferred\n";
		}
		else if (status == std::future_status::timeout) {
			std::cout << "timeout\n";
		}
		else if (status == std::future_status::ready) {
		}
	}
}

int main() {
	test_performance1();
	return 0;
}

#include <asio2/base/detail/pop_options.hpp>
