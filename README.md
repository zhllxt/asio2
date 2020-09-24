# asio2
### [README in English](https://github.com/zhllxt/asio2/blob/master/README.en.md) 
Header only c++ network library, based on asio,support tcp,udp,http,websocket,rpc,ssl,icmp,serial_port.

<a href="https://996.icu"><img src="https://img.shields.io/badge/link-996.icu-red.svg" alt="996.icu" /></a>
[![996.icu](https://img.shields.io/badge/link-996.icu-red.svg)](https://996.icu)
[![LICENSE](https://img.shields.io/badge/license-Anti%20996-blue.svg)](https://github.com/996icu/996.ICU/blob/master/LICENSE)

* header only,不依赖boost库,不需要单独编译,在工程的Include目录中添加asio2路径,在源码中#include <asio2/asio2.hpp>即可使用;
* 支持tcp, udp, http, websocket, rpc, ssl, icmp, serial_port;
* 支持可靠UDP(基于KCP),支持SSL,支持从内存字符串加载SSL证书;
* TCP支持各种数据拆包功能(单个字符或字符串或用户自定义协议等);实现了数据报模式的TCP(类似WEBSOCKET);
* 跨平台,支持windows,linux,32位,64位;在msvc(vs2017 vs2019 ) gcc8 clang10 下编译通过;
* 基于C++17,基于asio (asio 的 standalone 版本);
* example目录包含大量的示例工程(工程基于VS2017创建),各种使用方法请参考示例代码;

## v2.6重要更新:
* 完全移除对boost库的依赖,以前使用http和websocket时需要依赖boost库,现在所有功能都不需要boost了;
* rpc组件添加了链式调用功能,以前版本中调用rpc函数时,"用户回调函数,超时时长,rpc函数名,rpc函数参数"这些参数都要写在同一个函数中,很容易搞糊涂,现在支持链式调用可避免这个问题了;
* 重写了http接口,http接口更简单易用了;

## 与其它框架的一点区别:
```c++
目前看到的很多基于asio的框架的模式大都如下:
tcp_server server; // 声明一个server
server.run();      // 调用run函数,run函数是阻塞的,run之后怎么退出却不知道.
这种模式需要用户自己去处理程序退出后的逻辑,包括连接的正常关闭,
资源释放等问题,而这些问题自己处理起来是很烦琐的.
asio2框架已经处理过了这些问题,你可以在如MFC的OnInitDialog等地方调用server.start(...),
start(...)函数是非阻塞的,什么时候想退出了只需要server.stop()即可.stop()是阻塞的,
stop时如果有未发送完的数据,会保证一定在数据发送完之后才会退出,
tcp下也会保证所有连接都正常关闭以后才会退出,你不用考虑资源的正确释放等一系列琐碎的问题.
```

## 一点简单的性能测试:
```c++
测试办法:本机127.0.0.1,和rest_rpc进行对比.测试rpc的qps,只测了单连接.字符串
大小为128时:rest_rpc的qps约为30000/秒,asio2的qps约为37000/秒,此时asio2
比rest_rpc性能约高19%.字符串越来越大时(测试了16K和64K),两者性能基本相同.
```
rpc测试的和说明代码请看:[rpc性能测试代码](https://github.com/zhllxt/asio2/wiki/rpc%E6%80%A7%E8%83%BD%E6%B5%8B%E8%AF%95%E4%BB%A3%E7%A0%81)

## TCP:
##### 服务端:
```c++
asio2::tcp_server server;
server.bind_recv([&](std::shared_ptr<asio2::tcp_session> & session_ptr, std::string_view s)
{
	printf("recv : %u %.*s\n", (unsigned)s.size(), (int)s.size(), s.data());
	// 异步发送(所有发送操作都是异步且线程安全的)
	session_ptr->send(s);
	// 发送时指定一个回调函数,当发送完成后会调用此回调函数,bytes_sent表示实际发送的字节数,
	// 发送是否有错误可以用asio2::get_last_error()函数来获取错误码
	// session_ptr->send(s, [](std::size_t bytes_sent) {});
}).bind_connect([&](auto & session_ptr)
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
}).bind_disconnect([&](auto & session_ptr)
{
	printf("client leave : %s %u %s\n",
		session_ptr->remote_address().c_str(),
		session_ptr->remote_port(), asio2::last_error_msg().c_str());
});
server.start("0.0.0.0", "8080");

// 按\n自动拆包(可以指定任意字符)
//server.start("0.0.0.0", "8080", '\n');

// 按\r\n自动拆包(可以指定任意字符串)
//server.start("0.0.0.0", "8080", "\r\n");

// 按自定义规则自动拆包(match_role请参考example代码)(用于对用户自定义的协议拆包)
// 对自定义协议拆包时,match_role如何使用的详细说明请看:https://blog.csdn.net/zhllxt/article/details/104772948
//server.start("0.0.0.0", "8080", match_role('#'));

// 每次接收固定的100字节
//server.start("0.0.0.0", "8080", asio::transfer_exactly(100));

// 数据报模式的TCP,无论发送多长的数据,双方接收的一定是相应长度的整包数据
//server.start("0.0.0.0", "8080", asio2::use_dgram);
```
##### 客户端:
```c++
asio2::tcp_client client;
// 客户端在断开时默认会自动重连

// 禁止自动重连
//client.auto_reconnect(false);

// 启用自动重连 默认在断开连接后延时1秒就会开始重连
//client.auto_reconnect(true);

// 启用自动重连 并设置自定义的延时
client.auto_reconnect(true, std::chrono::seconds(3));

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
	//// 绑定全局函数
	//.bind_recv(on_recv)
	//// 绑定成员函数(具体请查看example代码)
	//.bind_recv(std::bind(&listener::on_recv, &lis, std::placeholders::_1)) 
	//// 按lis对象的引用来绑定成员函数(具体请查看example代码)
	//.bind_recv(&listener::on_recv, lis) 
	//// 按lis对象的指针来绑定成员函数(具体请查看example代码)
	//.bind_recv(&listener::on_recv, &lis) 
	;
// 异步连接服务端
client.async_start("0.0.0.0", "8080");

// 同步连接服务端
//client.start("0.0.0.0", "8080");

// 按\n自动拆包(可以指定任意字符)
//client.async_start("0.0.0.0", "8080", '\n');

// 按\r\n自动拆包(可以指定任意字符串)
//client.async_start("0.0.0.0", "8080", "\r\n"); 

// 按自定义规则自动拆包(match_role请参考example代码)(用于对用户自定义的协议拆包)
// 对自定义协议拆包时,match_role如何使用的详细说明请看:https://blog.csdn.net/zhllxt/article/details/104772948
//client.async_start("0.0.0.0", "8080", match_role); 

// 每次接收固定的100字节
//client.async_start("0.0.0.0", "8080", asio::transfer_exactly(100)); 

// 数据报模式的TCP,无论发送多长的数据,双方接收的一定是相应长度的整包数据
//client.start("0.0.0.0", "8080", asio2::use_dgram); 

// 发送时也可以指定use_future参数,然后通过返回值future来阻塞等待直到发送完成,发送结果的错误码和发送字节数
// 保存在返回值future中(注意,不能在通信线程中用future去等待,这会阻塞通信线程进而导致死锁)
// std::future<std::pair<asio::error_code, std::size_t>> future = client.send("abc", asio::use_future); 
```

## UDP:
##### 服务端:
```c++
asio2::udp_server server;
// ... 绑定监听器(请查看example代码)
server.start("0.0.0.0", "8080"); // 常规UDP
//server.start("0.0.0.0", "8080", asio2::use_kcp); // 可靠UDP
```
##### 客户端:
```c++
asio2::udp_client client;
// ... 绑定监听器(请查看example代码)
client.start("0.0.0.0", "8080");
//client.async_start("0.0.0.0", "8080", asio2::use_kcp); // 可靠UDP
```

## RPC:
##### 服务端:
```c++
// 全局函数示例，当服务端的RPC函数被调用时，如果想知道是哪个客户端调用的，将这个RPC函数的第一
// 个参数设置为连接对象的智能指针即可(如果不关心是哪个客户端调用的,第一个参数可以不要),如下:
int add(std::shared_ptr<asio2::rpc_session>& session_ptr, int a, int b)
{
	return a + b;
}

// rpc默认是按照"数据长度+数据内容"的格式来发送数据的,因此客户端可能会恶意组包,导致解析出的
// "数据长度"非常长,此时就会分配大量内存来接收完整数据包.避免此问题的办法就是是指定缓冲区最
// 大值,如果发送的数据超过缓冲区最大值,就会将该连接直接关闭.所有tcp udp http websocket,server
// client 等均支持这个功能.
asio2::rpc_server server(
	512,  // 接收缓冲区的初始大小
	1024, // 接收缓冲区的最大大小
	4     // 多少个并发线程
);

// ... 绑定监听器(请查看example代码)

// 绑定RPC全局函数
server.bind("add", add);

// 绑定RPC成员函数
server.bind("mul", &A::mul, a);

// 绑定lambda表达式
server.bind("cat", [&](const std::string& a, const std::string& b) { return a + b; });

// 绑定成员函数(按引用) a的定义请查看example代码
server.bind("get_user", &A::get_user, a);

// 绑定成员函数(按指针) a的定义请查看example代码
server.bind("del_user", &A::del_user, &a);

// 服务端也可以调用客户端的RPC函数(通过连接对象session_ptr)
session_ptr->async_call([](asio::error_code ec, int v)
{
	printf("sub : %d err : %d %s\n", v, ec.value(), ec.message().c_str());
}, std::chrono::seconds(10), "sub", 15, 6);

//server.start("0.0.0.0", "8080");
```
##### 客户端:
```c++
asio2::rpc_client client;
// ... 绑定监听器(请查看example代码)
// 不仅server可以绑定RPC函数给client调用，同时client也可以绑定RPC函数给server调用。请参考example代码。
client.start("0.0.0.0", "8080");

asio::error_code ec;

// 同步调用RPC函数
int sum = client.call<int>(ec, std::chrono::seconds(3), "add", 11, 2);
printf("sum : %d err : %d %s\n", sum, ec.value(), ec.message().c_str());

// 异步调用RPC函数,
// 第一个参数是回调函数,当调用完成或超时会自动调用该回调函数,如果超时或其它错误,错误码保存在ec中
// 第二个参数是调用超时,可以不填,如果不填则使用默认超时
// 第三个参数是rpc函数名,之后的参数是rpc函数的参数
client.async_call([](asio::error_code ec, int v)
{
	printf("sum : %d err : %d %s\n", v, ec.value(), ec.message().c_str());
}, "add", 10, 20);

// 上面的调用方式的参数位置很容易搞混,因此也支持链式调用,如下(其它示例请查看example):
client.timeout(std::chrono::seconds(5)).async_call("mul", 2.5, 2.5).response(
	[](asio::error_code ec, double v)
{
	std::cout << "mul1 " << v << std::endl;
});
int sum = client.timeout(std::chrono::seconds(3)).errcode(ec).call<int>("add", 11, 32);

// 返回值为用户自定义数据类型(user类型的定义请查看example代码)
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

// 如果RPC函数的返回值为void,则用户回调函数只有一个参数ec即可
client.async_call([](asio::error_code ec)
{
}, "del_user", std::move(u));

// 只调用rpc函数，不需要返回结果
client.async_call("del_user", std::move(u));

```

## HTTP:
##### 服务端:
```c++
// http 请求拦截器
struct aop_log
{
	bool before(http::request& req, http::response& rep)
	{
		asio2::detail::ignore_unused(rep);
		printf("aop_log before %s\n", req.method_string().data());
		// 返回true则后续的拦截器会接着调用,返回false则后续的拦截器不会被调用
		return true;
	}
	bool after(std::shared_ptr<asio2::http_session>& session_ptr, http::request& req, http::response& rep)
	{
		asio2::detail::ignore_unused(session_ptr, req, rep);
		printf("aop_log after\n");
		return true;
	}
};

struct aop_check
{
	bool before(std::shared_ptr<asio2::http_session>& session_ptr, http::request& req, http::response& rep)
	{
		asio2::detail::ignore_unused(session_ptr, req, rep);
		printf("aop_check before\n");
		return true;
	}
	bool after(http::request& req, http::response& rep)
	{
		asio2::detail::ignore_unused(req, rep);
		printf("aop_check after\n");
		return true;
	}
};

asio2::http_server server;

server.bind<http::verb::get, http::verb::post>("/index.*", [](http::request& req, http::response& rep)
{
	std::cout << req.path() << std::endl;
	std::cout << req.query() << std::endl;

	rep.fill_file("../../../index.html");
	rep.chunked(true);

}, aop_log{});

server.bind<http::verb::get>("/del_user",
	[](std::shared_ptr<asio2::http_session>& session_ptr, http::request& req, http::response& rep)
{
	// 回调函数的第一个参数可以是会话指针session_ptr(这个参数也可以不要)
	printf("del_user ip : %s\n", session_ptr->remote_address().data());

	// fill_page函数用给定的错误代码构造一个简单的标准错误页,<html>...</html>这样
	rep.fill_page(http::status::ok, "del_user successed.");

}, aop_check{});

server.bind<http::verb::get>("/api/user/*", [](http::request& req, http::response& rep)
{
	rep.fill_text("the user name is hanmeimei, .....");

}, aop_log{}, aop_check{});

server.bind<http::verb::get>("/defer", [](http::request& req, http::response& rep)
{
	// 使用defer让http响应延迟发送,defer的智能指针销毁时,才会自动发送response
	std::shared_ptr<http::response_defer> rep_defer = rep.defer();

	std::thread([rep_defer, &rep]() mutable
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(3000));

		asio::error_code ec;
		auto newrep = asio2::http_client::execute("http://www.baidu.com", ec);

		rep = std::move(newrep);

	}).detach();

}, aop_log{}, aop_check{});

// 对websocket的支持
server.bind("/ws", websocket::listener<asio2::http_session>{}.
	on("message", [](std::shared_ptr<asio2::http_session>& session_ptr, std::string_view data)
{
	printf("ws msg : %u %.*s\n", (unsigned)data.size(), (int)data.size(), data.data());

	session_ptr->send(data);

}).on("open", [](std::shared_ptr<asio2::http_session>& session_ptr)
{
	printf("ws open\n");

	// 打印websocket的http请求头
	std::cout << session_ptr->request() << std::endl;

	// 如何给websocket响应头填充额外信息
	session_ptr->ws_stream().set_option(websocket::stream_base::decorator(
		[](websocket::response_type& rep)
	{
		rep.set(http::field::authorization, " http-server-coro");
	}));

}).on("close", [](std::shared_ptr<asio2::http_session>& session_ptr)
{
	printf("ws close\n");

}));

server.bind_not_found([](http::request& req, http::response& rep)
{
	// fill_page函数可以构造一个简单的标准错误页
	rep.fill_page(http::status::not_found);
});
```
##### 客户端:
```c++
asio2::error_code ec;

// 通过URL字符串生成一个http请求对象
auto req1 = http::make_request("http://www.baidu.com/get_user?name=abc");
// 通过URL字符串直接请求某个网址,返回结果在rep1中,如果有错误,错误码保存在ec中
auto rep1 = asio2::http_client::execute("http://www.baidu.com/get_user?name=abc", ec);
if (ec)
	std::cout << ec.message() << std::endl;
else
	std::cout << rep1 << std::endl; // 打印http请求结果

// 通过http协议字符串生成一个http请求对象
auto req2 = http::make_request("GET / HTTP/1.1\r\nHost: 192.168.0.1\r\n\r\n");
// 通过请求对象发送http请求
auto rep2 = asio2::http_client::execute("www.baidu.com", "80", req2, std::chrono::seconds(3), ec);
if (ec)
	std::cout << ec.message() << std::endl;
else
	std::cout << rep2 << std::endl;

std::stringstream ss;
ss << rep2;
std::string result = ss.str(); // 通过这种方式将http请求结果转换为字符串

// 获取url中的path部分的值
auto path = asio2::http::url_to_path("/get_user?name=abc");
std::cout << path << std::endl;

// 获取url中的query部分的值
auto query = asio2::http::url_to_query("/get_user?name=abc");
std::cout << query << std::endl;

std::cout << std::endl;

auto rep3 = asio2::http_client::execute("www.baidu.com", "80", "/api/get_user?name=abc", ec);
if (ec)
	std::cout << ec.message() << std::endl;
else
	std::cout << rep3 << std::endl;

// URL编解码
std::string en = http::url_encode(R"(http://www.baidu.com/json={"qeury":"name like '%abc%'","id":1})");
std::cout << en << std::endl;
std::string de = http::url_decode(en);
std::cout << de << std::endl;

// 其它的更多用法请查看example示例代码

```

##### 其它的HTTP使用方式以及WEBSOCKET使用方式请参考example代码

## ICMP:
```c++
asio2::ping ping;
ping.timeout(std::chrono::seconds(3))  // 设置ping超时 默认3秒
	.interval(std::chrono::seconds(1)) // 设置ping间隔 默认1秒
	.bind_recv([](asio2::icmp_rep& rep)
{
	if (rep.is_timeout())
		std::cout << "request timed out" << std::endl;
	else
		std::cout << rep.total_length() - rep.header_length()
		<< " bytes from " << rep.source_address()
		<< ": icmp_seq=" << rep.sequence_number()
		<< ", ttl=" << rep.time_to_live()
		<< ", time=" << std::chrono::duration_cast<std::chrono::milliseconds>(rep.lag).count() << "ms"
		<< std::endl;
}).start("151.101.193.69");
```

## SSL:
##### TCP/HTTP/WEBSOCKET均支持SSL功能(需要在config.hpp中将#define ASIO2_USE_SSL宏定义放开)
```c++
asio2::tcps_server server;
// 设置证书模式
// 如果是 verify_peer | verify_fail_if_no_peer_cert 则客户端必须要使用证书否则握手失败
// 如果是 verify_peer 或者是 verify_fail_if_no_peer_cert 则客户端用不用证书都可以
server.set_verify_mode(asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert);
// 从内存字符串加载SSL证书(具体请查看example代码) 字符串的具体定义请查看example代码
server.set_cert_buffer(ca_crt, server_crt, server_key, "server"); // use memory string for cert
server.set_dh_buffer(dh);
// 从文件加载SSL证书(注意编译后把example/cert目录下的证书拷贝到exe目录下 否则会提示加载证书失败)
server.set_cert_file("ca.crt", "server.crt", "server.key", "server"); // use file for cert
server.set_dh_file("dh1024.pem");
// 如何制作自己的证书：
// 1. 生成服务端私有密钥
// openssl genrsa -des3 -out server.key 1024
// 2. 生成服务端证书请求文件
// openssl req -new -key server.key -out server.csr -config openssl.cnf
// 3. 生成客户端私有密钥
// openssl genrsa -des3 -out client.key 1024
// 4. 生成客户端证书请求文件
// openssl req -new -key client.key -out client.csr -config openssl.cnf
// 5. 生成CA私有密钥
// openssl genrsa -des3 -out ca.key 2048
// 6. 生成CA证书文件
// openssl req -new -x509 -key ca.key -out ca.crt -days 3650 -config openssl.cnf
// 7. 生成服务端证书
// openssl ca -in server.csr -out server.crt -cert ca.crt -keyfile ca.key -config openssl.cnf
// 8. 生成客户端证书
// openssl ca -in client.csr -out client.crt -cert ca.crt -keyfile ca.key -config openssl.cnf
// 9. 生成dh文件
// openssl dhparam -out dh1024.pem 1024
// 说明:openssl是个exe文件,在tool/openssl/x64/bin目录下 openssl.cnf在tool/openssl/x64/ssl目录下
// 生成证书过程中的其它细节百度搜索即可找到相关说明
```
##### TCP/HTTP/WEBSOCKET服务端、客户端等SSL功能请到example代码中查看。

## 串口:
```c++
std::string_view device = "COM1"; // windows
//std::string_view device = "/dev/ttyS0"; // linux
std::string_view baud_rate = "9600";

asio2::scp sp;
sp.bind_init([&]()
{
	// 设置串口参数
	sp.socket().set_option(asio::serial_port::flow_control(asio::serial_port::flow_control::type::none));
	sp.socket().set_option(asio::serial_port::parity(asio::serial_port::parity::type::none));
	sp.socket().set_option(asio::serial_port::stop_bits(asio::serial_port::stop_bits::type::one));
	sp.socket().set_option(asio::serial_port::character_size(8));

}).bind_recv([&](std::string_view sv)
{
	printf("recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());

	// 接收串口数据
	std::string s;
	uint8_t len = uint8_t(10 + (std::rand() % 20));
	s += '<';
	for (uint8_t i = 0; i < len; i++)
	{
		s += (char)((std::rand() % 26) + 'a');
	}
	s += '>';

	sp.send(s, []() {});

});

// 没有指定如何解析串口数据,需要用户自己去解析串口数据
//sp.start(device, baud_rate);

// 按照单个字符'>'作为数据分隔符自动解析串口数据
sp.start(device, baud_rate, '>');

// 按照字符串"\r\n"作为数据分隔符自动解析串口数据
//sp.start(device, baud_rate, "\r\n");

// 按照用户自定义的协议自动解析,关于match_role如何使用请参考tcp部分说明
//sp.start(device, baud_rate, match_role);

//sp.start(device, baud_rate, asio::transfer_at_least(1));
//sp.start(device, baud_rate, asio::transfer_exactly(10));
```

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


