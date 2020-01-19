# asio2
### [README in English](https://github.com/zhllxt/asio2/blob/master/README.en.md) 
A open source cross-platform c++ library for network programming based on asio,support for tcp,udp,http,rpc,ssl and so on.

<a href="https://996.icu"><img src="https://img.shields.io/badge/link-996.icu-red.svg" alt="996.icu" /></a>
[![996.icu](https://img.shields.io/badge/link-996.icu-red.svg)](https://996.icu)
[![LICENSE](https://img.shields.io/badge/license-Anti%20996-blue.svg)](https://github.com/996icu/996.ICU/blob/master/LICENSE)

* 支持TCP,UDP,HTTP,WEBSOCKET,RPC,ICMP,SERIAL_PORT等;
* 支持可靠UDP(基于KCP),支持SSL,支持从内存字符串加载SSL证书;
* TCP支持各种数据拆包功能(单个字符或字符串或用户自定义协议等);实现了TCP的数据报模式(类似WEBSOCKET);
* 支持windows,linux,32位,64位;
* 基于C++17,基于asio(boost::asio或独立asio均可,若需要HTTP功能必须使用boost::asio);
* header only 方式,无需编译,只需在工程的Include包含目录中添加asio2路径,然后在源码中#include <asio2/asio2.hpp>包含头文件即可;
* demo目录包含大量的示例工程(工程基于VS2017创建),各种使用方法请参考示例代码;

## 与其它框架的一点区别:
```c++
目前看到的很多基于asio的框架的模式大都如下:
tcp_server server; // 声明一个server
server.run();      // 调用run函数,run函数是阻塞的
这种模式需要用户自己去处理程序退出后的逻辑,包括连接的正常关闭,
资源释放等问题,而这些问题自己处理起来是很烦琐的.
asio2框架已经处理过了这些问题,你可以在如MFC的OnInitDialog等地方调用server.start(...),
start(...)函数是非阻塞的,什么时候想退出了只需要server.stop()即可.stop()是阻塞的,
stop时如果有未发送完的数据,会保证一定在数据发送完之后才会退出,
tcp下也会保证所有连接都正常关闭以后才会退出,你不用考虑资源的正确释放等一系列琐碎的问题.
```

## 一点简单的性能测试:
```c++
测试办法:本机127.0.0.1,和rest_rpc进行对比.测试rpc的qps,只测了单连接.
字符串大小为128时:rest_rpc的qps约为30000/秒,asio2的qps约为37000/秒,此时asio2比rest_rpc性能约高19%.
字符串越来越大时(测试了16K和64K),两者性能基本相同.
```
rpc测试的和说明代码请看:[rpc性能测试代码](https://github.com/zhllxt/asio2/wiki/rpc%E6%80%A7%E8%83%BD%E6%B5%8B%E8%AF%95%E4%BB%A3%E7%A0%81)

## TCP:
##### 服务端:
```c++
asio2::tcp_server server;
server.bind_recv([&server](std::shared_ptr<asio2::tcp_session> & session_ptr, std::string_view s)
{
	printf("recv : %u %.*s\n", (unsigned)s.size(), (int)s.size(), s.data());
	// 异步发送(所有发送操作都是异步且线程安全的)
	session_ptr->send(s);
	// 发送时指定一个回调函数,当发送完成后会调用此回调函数,bytes_sent表示实际发送的字节数,
	// 发送是否有错误可以用asio2::get_last_error()函数来获取错误码
	// session_ptr->send(s, [](std::size_t bytes_sent) {});
}).bind_connect([&server](auto & session_ptr)
{
	session_ptr->no_delay(true);
	printf("client enter : %s %u %s %u\n",
		session_ptr->remote_address().c_str(), session_ptr->remote_port(),
		session_ptr->local_address().c_str(), session_ptr->local_port());
	// 可以用session_ptr这个会话启动一个定时器,这个定时器是在这个session_ptr会话的数据收
	// 发线程中执行的,这对于连接状态的判断或其它需求很有用(尤其在UDP这种无连接的协议中,有
	// 时需要在数据处理过程中使用一个定时器来延时做某些操作,而且这个定时器还需要和数据处理
	// 在同一个线程中安全触发)
	//session_ptr->start_timer(1, std::chrono::seconds(1), []() {});
}).bind_disconnect([&server](auto & session_ptr)
{
	printf("client leave : %s %u %s\n",
		session_ptr->remote_address().c_str(),
		session_ptr->remote_port(), asio2::last_error_msg().c_str());
});
server.start("0.0.0.0", "8080");
//server.start("0.0.0.0", "8080", '\n'); // 按\n自动拆包(可以指定任意字符)
//server.start("0.0.0.0", "8080", "\r\n"); // 按\r\n自动拆包(可以指定任意字符串)
//server.start("0.0.0.0", "8080", match_role('#')); // 按match_role指定的规则自动拆包(match_role请参考demo代码)(用于对用户自定义的协议拆包)
//server.start("0.0.0.0", "8080", asio::transfer_exactly(100)); // 每次接收固定的100字节
//server.start("0.0.0.0", "8080", asio2::use_dgram); // 数据报模式的TCP,无论发送多长的数据,双方接收的一定是相应长度的整包数据
```
##### 客户端:
```c++
asio2::tcp_client client;
// 客户端在断开时默认会自动重连
//client.reconnect(false); // 禁止自动重连
//client.reconnect(true); // 启用自动重连 默认在断开连接后延时1秒就会开始重连
client.reconnect(true, std::chrono::milliseconds(100)); // 启用自动重连 并设置自定义的延时
client.bind_connect([&](asio::error_code ec)
{
	if (asio2::get_last_error())
		printf("connect failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	else
		printf("connect success : %s %u\n", client.local_address().c_str(), client.local_port());

	client.send("<abcdefghijklmnopqrstovuxyz0123456789>");
}).bind_disconnect([](asio::error_code ec)
{
	printf("disconnect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
}).bind_recv([&](std::string_view sv)
{
	printf("recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());

	client.send(sv);
})
	//.bind_recv(on_recv) // 绑定全局函数
	//.bind_recv(std::bind(&listener::on_recv, &lis, std::placeholders::_1)) // 绑定成员函数(具体请查看demo代码)
	//.bind_recv(&listener::on_recv, lis) // 按lis对象的引用来绑定成员函数(具体请查看demo代码)
	//.bind_recv(&listener::on_recv, &lis) // 按lis对象的指针来绑定成员函数(具体请查看demo代码)
	;
client.async_start("0.0.0.0", "8080"); // 异步连接服务端
//client.start("0.0.0.0", "8080"); // 同步连接服务端
//client.async_start("0.0.0.0", "8080", '\n'); // 按\n自动拆包(可以指定任意字符)
//client.async_start("0.0.0.0", "8080", "\r\n"); // 按\r\n自动拆包(可以指定任意字符串)
//client.async_start("0.0.0.0", "8080", match_role); // 按match_role指定的规则自动拆包(match_role请参考demo代码)(用于对用户自定义的协议拆包)
//client.async_start("0.0.0.0", "8080", asio::transfer_exactly(100)); // 每次接收固定的100字节
//client.start("0.0.0.0", "8080", asio2::use_dgram); // 数据报模式的TCP,无论发送多长的数据,双方接收的一定是相应长度的整包数据

// 发送时也可以指定use_future参数,然后通过返回值future来阻塞等待直到发送完成,发送结果的错误码和发送字节数
// 保存在返回值future中(注意,不能在通信线程中用future去等待,这会阻塞通信线程进而导致死锁)
// std::future<std::pair<asio::error_code, std::size_t>> future = client.send("abc", asio::use_future); 
```

## UDP:
##### 服务端:
```c++
asio2::udp_server server;
// ... 绑定监听器(请查看demo代码)
server.start("0.0.0.0", "8080"); // 常规UDP
//server.start("0.0.0.0", "8080", asio2::use_kcp); // 可靠UDP
```
##### 客户端:
```c++
asio2::udp_client client;
// ... 绑定监听器(请查看demo代码)
client.start("0.0.0.0", "8080");
//client.async_start("0.0.0.0", "8080", asio2::use_kcp); // 可靠UDP
```

## RPC:
##### 服务端:
```c++
// 全局函数示例，当服务端的RPC函数被调用时，如果想知道是哪个客户端调用的，将这个RPC函数的第一个参数
// 设置为连接对象的智能指针即可（如果不关心是哪个客户端调用的，删除这第一个参数即可），如下：
int add(std::shared_ptr<asio2::rpc_session>& session_ptr, int a, int b)
{
	return a + b;
}
asio2::rpc_server server;
// ... 绑定监听器(请查看demo代码)
A a; // A的定义请查看demo代码
server.bind("add", add); // 绑定RPC全局函数
server.bind("mul", &A::mul, a); // 绑定RPC成员函数
server.bind("cat", [&](const std::string& a, const std::string& b) { return a + b; }); // 绑定lambda表达式
server.bind("get_user", &A::get_user, a); // 绑定成员函数(按引用)
server.bind("del_user", &A::del_user, &a); // 绑定成员函数(按指针)
server.start("0.0.0.0", "8080", asio2::use_dgram); // 使用TCP数据报模式作为RPC通信底层支撑,启动服务端时必须要使用use_dgram参数
//server.start("0.0.0.0", "8080"); // 使用websocket作为RPC通信底层支撑(需要到rcp_server.hpp文件末尾代码中选择使用websocket)
```
##### 客户端:
```c++
asio2::rpc_client client;
// ... 绑定监听器(请查看demo代码)
// 不仅server可以绑定RPC函数给client调用，同时client也可以绑定RPC函数给server调用。请参考demo代码。
client.start("0.0.0.0", "8080", asio2::use_dgram); // 使用TCP数据报模式作为RPC通信底层支撑,启动服务端时必须要使用use_dgram参数
//client.start("0.0.0.0", "8080"); // 使用websocket作为RPC通信底层支撑
asio::error_code ec;
// 同步调用RPC函数
int sum = client.call<int>(ec, std::chrono::seconds(3), "add", 11, 2);
printf("sum : %d err : %d %s\n", sum, ec.value(), ec.message().c_str());
// 异步调用RPC函数,第一个参数是回调函数,当调用完成或超时会自动调用该回调函数,如果超时或其它错误,
// 错误码保存在ec中,这里async_call没有指定返回值类型,则lambda表达式的第二个参数必须要指定类型
client.async_call([](asio::error_code ec, int v)
{
	printf("sum : %d err : %d %s\n", v, ec.value(), ec.message().c_str());
}, "add", 10, 20);
// 这里async_call指定了返回值类型,则lambda表达式的第二个参数可以为auto类型
// 也可以指定异步RPC的超时，如下：std::chrono::seconds(3)
client.async_call<int>([](asio::error_code ec, auto v)
{
	printf("sum : %d err : %d %s\n", v, ec.value(), ec.message().c_str());
}, std::chrono::seconds(3),  "add", 12, 21);
// 返回值为用户自定义数据类型(user类型的定义请查看demo代码)
user u = client.call<user>(ec, "get_user");
printf("%s %d ", u.name.c_str(), u.age);
for (auto &[k, v] : u.purview)
{
	printf("%d %s ", k, v.c_str());
}
printf("\n");

u.name = "hanmeimei";
u.age = ((int)time(nullptr)) % 100;
u.purview = { {10,"get"},{20,"set"} };
// 如果RPC函数的返回值为void,则用户回调函数只有一个参数即可
client.async_call([](asio::error_code ec)
{
}, "del_user", std::move(u));

```

## HTTP:
##### 服务端:
```c++
asio2::http_server server;
server.bind_recv([&](std::shared_ptr<asio2::http_session> & session_ptr, http::request<http::string_body>& req)
{
	// 在收到http请求时尝试发送一个文件到对端
	{
		// 如果请求是非法的,直接发送错误信息到对端并返回
		if (req.target().empty() ||
			req.target()[0] != '/' ||
			req.target().find("..") != beast::string_view::npos)
		{
			session_ptr->send(http::make_response(http::status::bad_request, "Illegal request-target"));
			session_ptr->stop(); // 同时直接断开这个连接
			return;
		}

		// Build the path to the requested file
		std::string path(req.target().data(), req.target().size());
		path.insert(0, std::filesystem::current_path().string());
		if (req.target().back() == '/')
			path.append("index.html");

		// 打开文件
		beast::error_code ec;
		http::file_body::value_type body;
		body.open(path.c_str(), beast::file_mode::scan, ec);

		// 如果打开文件失败,直接发送错误信息到对端并直接返回
		if (ec == beast::errc::no_such_file_or_directory)
		{
			session_ptr->send(http::make_response(http::status::not_found,
				std::string_view{ req.target().data(), req.target().size() }));
			return;
		}

		// Cache the size since we need it after the move
		auto const size = body.size();

		// 生成一个文件形式的http响应对象,然后发送给对端
		http::response<http::file_body> res{
			std::piecewise_construct,
			std::make_tuple(std::move(body)),
			std::make_tuple(http::status::ok, req.version()) };
		res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(http::field::content_type, http::extension_to_mimetype(path));
		res.content_length(size);
		res.keep_alive(req.keep_alive()); 
		res.chunked(true);
		// Specify a callback function when sending
		//session_ptr->send(std::move(res));
		session_ptr->send(std::move(res), [&res](std::size_t bytes_sent)
		{
			auto opened = res.body().is_open(); std::ignore = opened;
			auto err = asio2::get_last_error(); std::ignore = err;
		});
		//session_ptr->send(std::move(res), asio::use_future);
		return;
	}

	std::cout << req << std::endl;
	if (true)
	{
		// 用make_response生成一个http响应对象,状态码200表示操作成功,"suceess"是HTTP消息的body部分内容
		auto rep = http::make_response(200, "suceess");
		session_ptr->send(rep, []()
		{
			auto err = asio2::get_last_error(); std::ignore = err;
		});
	}
	else
	{
		// 也可以直接发送一个http标准响应字符串,内部会将这个字符串自动转换为http响应对象再发送出去
		std::string_view rep =
			"HTTP/1.1 404 Not Found\r\n"\
			"Server: Boost.Beast/181\r\n"\
			"Content-Length: 7\r\n"\
			"\r\n"\
			"failure";
		// test send string sequence, the string will automatically parsed into a standard http request
		session_ptr->send(rep, [](std::size_t bytes_sent)
		{
			auto err = asio2::get_last_error(); std::ignore = err;
		});
	}
});
server.start(host, port);

```
##### 客户端:
```c++
asio2::error_code ec;
auto req1 = http::make_request("http://www.baidu.com/get_user?name=a"); // 通过URL字符串生成一个http请求对象
auto req2 = http::make_request("GET / HTTP/1.1\r\nHost: 127.0.0.1:8443\r\n\r\n"); // 通过http协议字符串生成一个http请求对象
req2.set(http::field::timeout, 5000); // 给请求设置一个超时时间
auto rep1 = asio2::http_client::execute("http://www.baidu.com/get_user?name=a", ec); // 通过URL字符串直接请求某个网址,返回结果在rep1中,如果有错误,错误码保存在ec中
auto rep2 = asio2::http_client::execute("127.0.0.1", "8080", req2); // 通过IP端口以及前面生成的req2请求对象来发送一个http请求
std::cout << rep2 << std::endl; // 显示http请求结果
std::stringstream ss;
ss << rep2;
std::string result = ss.str(); // 通过这种方式将http请求结果转换为字符串
```

##### 其它的HTTP使用方式以及WEBSOCKET使用方式请参考demo代码

## ICMP:
```c++
class ping_test // 模拟在一个类对象中使用ping组件(其它所有如TCP/UDP/HTTP等组件一样可以在类对象中使用)
{
	asio2::ping ping;
public:
	ping_test() : ping(10) // 构造函数传入的10表示只ping 10次后就结束,传入-1表示一直ping
	{
		ping.timeout(std::chrono::seconds(3)); // 设置ping超时
		ping.interval(std::chrono::seconds(1)); // 设置ping间隔
		ping.body("0123456789abcdefghijklmnopqrstovuxyz");
		ping.bind_recv(&ping_test::on_recv, this) // 绑定当前这个类的成员函数作为监听器
			.bind_start(std::bind(&ping_test::on_start, this, std::placeholders::_1)) // 也是绑定成员函数
			.bind_stop([this](asio::error_code ec) { this->on_stop(ec); }); // 绑定lambda
	}
	void on_recv(asio2::icmp_rep& rep)
	{
		if (rep.lag.count() == -1) // 如果延时的值等于-1表示超时了
			std::cout << "request timed out" << std::endl;
		else
			std::cout << rep.total_length() - rep.header_length()
			<< " bytes from " << rep.source_address()
			<< ": icmp_seq=" << rep.sequence_number()
			<< ", ttl=" << rep.time_to_live()
			<< ", time=" << std::chrono::duration_cast<std::chrono::milliseconds>(rep.lag).count() << "ms"
			<< std::endl;
	}
	void on_start(asio::error_code ec)
	{
		printf("start : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	}
	void on_stop(asio::error_code ec)
	{
		printf("stop : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	}
	void run()
	{
		if (!ping.start("127.0.0.1"))
			//if (!ping.start("123.45.67.89"))
			//if (!ping.start("stackoverflow.com"))
			printf("start failure : %s\n", asio2::last_error_msg().c_str());
		while (std::getchar() != '\n');
		ping.stop();
		// ping结束后可以输出统计信息,包括丢包率,平均延时时长等
		printf("loss rate : %.0lf%% average time : %lldms\n", ping.plp(),
			std::chrono::duration_cast<std::chrono::milliseconds>(ping.avg_lag()).count());
	}
};
```

## SSL:
##### TCP/HTTP/WEBSOCKET均支持SSL功能(需要在config.hpp中将#define ASIO2_USE_SSL宏定义放开)
```c++
asio2::tcps_server server;
// 从内存字符串加载SSL证书(具体请查看demo代码)
server.set_cert("test", cer, key, dh); // cer,key,dh这三个字符串的定义请查看demo代码
// 从文件加载SSL证书
//server.set_cert_file("test", "server.crt", "server.key", "dh512.pem");
```
##### TCP/HTTP/WEBSOCKET服务端、客户端等SSL功能请到DEMO代码中查看。

## 串口:
##### 请查看demo示例代码serial port 部分

## 其它:
##### 定时器
```c++
// 框架中提供了定时器功能,使用非常简单,如下:
asio2::timer timer;
// 参数1表示定时器ID,参数2表示定时器间隔,参数3为定时器回调函数
timer.start_timer(1, std::chrono::seconds(1), [&]()
{
	printf("timer 1\n");
	if (true) // 满足某个条件时关闭定时器,当然也可以在其它任意地方关闭定时器
		timer.stop_timer(1);
});
```


