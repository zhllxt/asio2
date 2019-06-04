# asio2
### [README in English](https://github.com/zhllxt/asio2/blob/master/README.en.md) 
A open source cross-platform c++ library for network programming based on asio,support for tcp,udp,http,rpc,ssl and so on.

* 支持TCP,UDP,HTTP,WEBSOCKET,RPC,ICMP,SERIAL_PORT等;
* 支持可靠UDP(基于KCP),支持SSL,支持从内存字符串加载SSL证书;
* TCP支持数据拆包功能(按指定的分隔符对数据自动进行拆包,保证用户收到的数据是一个完整的数据包);实现了TCP的数据报模式(类似WEBSOCKET);
* 支持windows,linux,32位,64位;
* 依赖asio(boost::asio或独立asio均可,若需要HTTP功能必须使用boost::asio),依赖C++17;
* 代码采用hpp头文件方式,以源码级链入,无需编译,只需在工程的Include包含目录中添加asio2路径,然后在源码中#include <asio2/asio2.hpp>包含头文件即可;
* demo目录包含大量的示例工程(工程基于VS2017创建),各种使用方法请参考示例代码;

## TCP:
##### 服务端:
```c++
asio2::tcp_server server;
server.bind_recv([&server](std::shared_ptr<asio2::tcp_session> & session_ptr, std::string_view s)
{
	session_ptr->no_delay(true);

	printf("recv : %u %.*s\n", (unsigned)s.size(), (int)s.size(), s.data());
	session_ptr->send(s, [](std::size_t bytes_sent) {});
}).bind_connect([&server](auto & session_ptr)
{
	printf("client enter : %s %u %s %u\n",
		session_ptr->remote_address().c_str(), session_ptr->remote_port(),
		session_ptr->local_address().c_str(), session_ptr->local_port());
}).bind_disconnect([&server](auto & session_ptr)
{
	printf("client leave : %s %u %s\n",
		session_ptr->remote_address().c_str(),
		session_ptr->remote_port(), asio2::last_error_msg().c_str());
});
server.start("0.0.0.0", 8080);
//server.start("0.0.0.0", 8080, '\n'); // 按\n自动拆包(可以指定任意字符)
//server.start("0.0.0.0", 8080, "\r\n"); // 按\r\n自动拆包(可以指定任意字符串)
//server.start("0.0.0.0", 8080, match_role('#')); // 按match_role指定的规则自动拆包(match_role请参考demo代码)(用于对用户自定义的协议拆包)
//server.start("0.0.0.0", 8080, asio::transfer_exactly(100)); // 每次接收固定的100字节
//server.start("0.0.0.0", 8080, asio2::use_dgram); // 数据报模式的TCP,无论发送多长的数据,双方接收的一定是相应长度的整包数据
```
##### 客户端:
```c++
asio2::tcp_client client;
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
client.async_start("0.0.0.0", 8080); // 异步连接服务端
//client.start("0.0.0.0", 8080); // 同步连接服务端
//client.async_start("0.0.0.0", 8080, '\n'); // 按\n自动拆包(可以指定任意字符)
//client.async_start("0.0.0.0", 8080, "\r\n"); // 按\r\n自动拆包(可以指定任意字符串)
//client.async_start("0.0.0.0", 8080, match_role); // 按match_role指定的规则自动拆包(match_role请参考demo代码)(用于对用户自定义的协议拆包)
//client.async_start("0.0.0.0", 8080, asio::transfer_exactly(100)); // 每次接收固定的100字节
//client.start("0.0.0.0", 8080, asio2::use_dgram); // 数据报模式的TCP,无论发送多长的数据,双方接收的一定是相应长度的整包数据
```

## UDP:
##### 服务端:
```c++
asio2::udp_server server;
// ... 绑定监听器(请查看demo代码)
server.start("0.0.0.0", 8080); // 常规UDP
//server.start("0.0.0.0", 8080, asio2::use_kcp); // 可靠UDP
```
##### 客户端:
```c++
asio2::udp_client client;
// ... 绑定监听器(请查看demo代码)
client.start("0.0.0.0", 8080);
//client.async_start("0.0.0.0", 8080, asio2::use_kcp); // 可靠UDP
```

## RPC:
##### 服务端:
```c++
asio2::rpc_server server;
// ... 绑定监听器(请查看demo代码)
A a; // A的定义请查看demo代码
server.bind("add", add); // 绑定RPC全局函数
server.bind("mul", &A::mul, a); // 绑定RPC成员函数
server.bind("cat", [&](const std::string& a, const std::string& b) { return a + b; }); // 绑定lambda表达式
server.bind("get_user", &A::get_user, a); // 绑定成员函数(按引用)
server.bind("del_user", &A::del_user, &a); // 绑定成员函数(按指针)
//server.start("0.0.0.0", 8080, asio2::use_dgram); // 使用TCP数据报模式作为RPC通信底层支撑,启动服务端时必须要使用use_dgram参数
server.start("0.0.0.0", 8080); // 使用websocket作为RPC通信底层支撑(需要到rcp_server.hpp文件末尾代码中选择使用websocket)
```
##### 客户端:
```c++
asio2::rpc_client client;
// ... 绑定监听器(请查看demo代码)
//client.start("0.0.0.0", 8080, asio2::use_dgram); // 使用TCP数据报模式作为RPC通信底层支撑,启动服务端时必须要使用use_dgram参数
client.start("0.0.0.0", 8080); // 使用websocket作为RPC通信底层支撑
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
client.async_call<int>([](asio::error_code ec, auto v)
{
	printf("sum : %d err : %d %s\n", v, ec.value(), ec.message().c_str());
}, "add", 12, 21);
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

## ICMP:
##### 请查看demo示例代码ping test 部分

## 串口:
##### 请查看demo示例代码serial port 部分
