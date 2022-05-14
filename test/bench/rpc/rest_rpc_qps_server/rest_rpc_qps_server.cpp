#include <asio2/base/detail/push_options.hpp>

#define ASIO_STANDALONE

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
