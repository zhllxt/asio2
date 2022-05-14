#include <asio2/base/detail/push_options.hpp>

#define ASIO_STANDALONE

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
