# rpc性能测试

测试办法:本机127.0.0.1,和rest_rpc进行对比.测试rpc的qps,只测了单连接.

字符串大小为128时:asio2比rest_rpc性能约高14%.字符串越来越大时(测试了16K和64K),两者性能基本相同.


```cpp
测试rpc的远程函数为:std::string echo(std::string a)

参数std::string为128长度的字符:std::string str(128, 'A');

rpc测试的硬件参数:内存8G,cpu(i5 4590)4核3.30GHZ.

rest_rpc和asio2测试所用的asio均为1.12.2版本.

rest_rpc在这里:https://github.com/qicosmos/rest_rpc

测试的时间为2020-01-18 直接下载的rest_rpc的master版本,
rest_rpc的版本为:https://github.com/qicosmos/rest_rpc/tree/e91d5f783888b148d58afd6eb45722507f95b803
asio2的版本为:https://github.com/zhllxt/asio2/tree/9f10555cfdd58026614907e5737f70a6608aa34c

这里只做了单连接的本地测试,测试的具体代码工程在这里: /asio2/test/bench/rpc/
```

## asio2的测试代码:
##### server:
```cpp
#include <asio2/asio2.hpp>

decltype(std::chrono::steady_clock::now()) t1 = std::chrono::steady_clock::now();
decltype(std::chrono::steady_clock::now()) t2 = std::chrono::steady_clock::now();
std::size_t qps = 0;
bool first = true;
std::string echo(std::string a)
{
	if (first)
	{
		first = false;
		t1 = std::chrono::steady_clock::now();
		t2 = std::chrono::steady_clock::now();
	}

	qps++;
	decltype(std::chrono::steady_clock::now()) t3 = std::chrono::steady_clock::now();
	auto ms = std::chrono::duration_cast<std::chrono::seconds>(t3 - t2).count();
	if (ms > 1)
	{
		t2 = t3;
		ms = std::chrono::duration_cast<std::chrono::seconds>(t3 - t1).count();
		double speed = (double)qps / (double)ms;
		printf("%.1lf\n", speed);
	}
	return a;
}

int main(int argc, char *argv[])
{
	asio2::rpc_server server;

	server.bind("echo", echo);
	server.start("0.0.0.0", "8080", asio2::use_dgram);

	while (std::getchar() != '\n');

	return 0;
};
```

##### client:
```cpp
#include <asio2/asio2.hpp>

std::string str(128, 'A');

std::function<void()> sender;

int main(int argc, char *argv[])
{
	asio2::rpc_client client;

	sender = [&]()
	{
		client.async_call([](asio::error_code ec, std::string v)
		{
			if (!ec)
				sender();
		}, "echo", str);
	};

	client.bind_connect([&](asio::error_code ec)
	{
		sender();
	});

	client.start("127.0.0.1", "8080", asio2::use_dgram);

	while (std::getchar() != '\n');

	return 0;
};
```

## rest_rpc的测试代码:
##### server:
```cpp
#include <rpc_server.h>
using namespace rest_rpc;
using namespace rpc_service;
#include <fstream>

decltype(std::chrono::steady_clock::now()) t1 = std::chrono::steady_clock::now();
decltype(std::chrono::steady_clock::now()) t2 = std::chrono::steady_clock::now();
std::size_t qps = 0;
bool first = true;

std::string echo(rpc_conn conn, std::string a) {

	if (first)
	{
		first = false;
		t1 = std::chrono::steady_clock::now();
		t2 = std::chrono::steady_clock::now();
	}

	qps++;
	decltype(std::chrono::steady_clock::now()) t3 = std::chrono::steady_clock::now();
	auto ms = std::chrono::duration_cast<std::chrono::seconds>(t3 - t2).count();
	if (ms > 1)
	{
		t2 = t3;
		ms = std::chrono::duration_cast<std::chrono::seconds>(t3 - t1).count();
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
```

##### client:
```cpp
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
```
