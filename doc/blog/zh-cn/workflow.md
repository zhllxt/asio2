# 基于c++和asio的网络编程框架asio2教程基础篇：2、各个回调函数的触发顺序和执行流程

## 以tcp举例：
### tcp服务端流程：
```cpp

#include <asio2/asio2.hpp>

int main()
{
	std::string_view host = "0.0.0.0";
	std::string_view port = "8028";

	asio2::tcp_server server;

	server.bind_init([]()
	{
		// 第1步：触发init回调函数
		// 当监听socket打开成功(即socket(AF_INET, SOCK_STREAM, 0)函数调用结束)之后，这里被触发。
		// 这个函数只会在服务端启动时触发1次。
		// 可以在这里给监听socket设置一些选项什么的。

	}).bind_start([&]()
	{
		// 第2步：触发start回调函数
		// 当监听socket监听成功(即bind,listen函数调用结束)之后，这里被触发。
		// 这个函数只会在服务端启动时触发1次。
		// 可以在这里记录服务端启动成功的日志。

		printf("start tcp server : %s %u\n",
			server.listen_address().c_str(), server.listen_port());

	}).bind_accept([](std::shared_ptr<asio2::tcp_session>& session_ptr)
	{
		// 第3步：触发accept回调函数
		// 当某个客户端刚连接上服务端之后，这里被触发。
		// 这个函数会多次触发，每有1个客户端连接上来之后，就会触发1次。但1个客户端只
		// 触发1次。由于客户端有掉线自动重连机制，所以客户端每重连1次，就会触发1次。
		// 可以在这里判断该客户端是不是在黑名单中，如果是，直接调用stop将该客户端断开。
		// 此时还不能给这个客户端发送数据，会由于连接还未完全建立而导致发送失败。

		if (session_ptr->remote_address() == "192.168.1.100")
			session_ptr->stop();

	}).bind_connect([&](auto & session_ptr)
	{
		// 第4步：触发connect回调函数
		// 当某个客户端和服务端的连接完全建立成功之后，这里被触发。
		// 这个函数会多次触发，每有1个客户端连接完成之后，就会触发1次。但1个客户端只
		// 触发1次。
		// 可以在这里给该客户端发送数据了。

		// 可能有人会问，对于tcp来说，只要连接建立了，不就立即可以发数据了吗？为什么在上面的
		// bind_accept中不能发，在这里的bind_connect中才能发呢？

		// 这是因为asio2框架人为的给这个连接做了一个状态标识(status)，在bind_accept中该status
		// 是starting，表示连接还未完全建立，当触发到bind_connect时才会给该连接的状态标识设置
		// 为started，表示连接建立全部完成了。所以如果在bind_accept中发送数据的话，框架会判断
		// 到状态标识不是started所以就不允许发送数据。

		// 那为什么要做这样一个状态标识呢？
		// 因为对于ssl来说，连接建立之后，还有ssl握手，只有ssl握手成功以后才能收发数据。对于
		// websocket来说，连接建立之后，还有upgrade升级协商，只有upgrade升级之后才能收发数据，
		// 所以为了整体框架流程统一，人为的做了一个规定，只有触发了bind_connect的回调函数之后
		// 才表示整个连接完全建立结束了。

		session_ptr->no_delay(true);

		printf("client enter : %s %u %s %u\n",
			session_ptr->remote_address().c_str(), session_ptr->remote_port(),
			session_ptr->local_address().c_str(), session_ptr->local_port());

	}).bind_recv([&](std::shared_ptr<asio2::tcp_session> & session_ptr, std::string_view data)
	{
		// 第5步：触发recv回调函数
		// 当收到某个客户端发送过来的数据之后，这里被触发。
		// 这个函数会多次触发，每有1个客户端发送数据过来之后，就会触发1次。且1个客户端也会
		// 触发多次，即发送数据就触发1次。
		// 在这里对接收到的数据进行解析处理。

		printf("recv : %u %.*s\n", (unsigned)data.size(), (int)data.size(), data.data());

		session_ptr->async_send(data, [](std::size_t bytes_sent) {});

	}).bind_disconnect([&](auto & session_ptr)
	{
		// 第6步：触发disconnect回调函数
		// 当某个客户端关闭了，服务端收到该客户端的连接断开事件之后，
		// 在该连接对应的socket关闭之前(即closesocket函数调用之前)，这里被触发。
		// 这个函数会多次触发，每有1个客户端连接完成之后，就会触发1次。但1个客户端只
		// 触发1次。
		// 可以在这里记录客户端的断开日志。

		printf("client leave : %s %u %s\n",
			session_ptr->remote_address().c_str(),
			session_ptr->remote_port(), asio2::last_error_msg().c_str());

	}).bind_stop([&]()
	{
		// 第7步：触发stop回调函数
		// 当关闭服务端时，在所有的连接都全部断开了之后，在监听socket关闭之前，这里被触发。
		// 注意：必须所有连接全部断开了，且所有的session_ptr的引用计数为0了，这里才会触发，
		// 因此如果你将session_ptr保存在了其它地方，一定要记得将你保存的session_ptr删除，
		// 否则server.stop()函数会一直阻塞无法结束，这里的回调函数也无法触发。
		// 这个函数只会在服务端关闭时触发1次。
		// 可以在这里记录服务端的停止日志。

		printf("stop tcp server : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());

	});

	server.start(host, port);

	while (std::getchar() != '\n');  // press enter to exit this program

	server.stop();

	return 0;
}
```
### tcp客户端流程：

```cpp

#include <asio2/asio2.hpp>

int main()
{
	std::string_view host = "127.0.0.1";
	std::string_view port = "8028";

	asio2::tcp_client client;

	client.bind_init([]()
	{
		// 第1步：触发init回调函数
		// 当客户端socket打开成功(即socket(AF_INET, SOCK_STREAM, 0)函数调用结束)之后，这里被触发。
		// 这个函数只会在客户端启动时触发1次。
		// 可以在这里给socket设置一些选项什么的。

	}).bind_connect([&]()
	{
		// 第2步：触发connect回调函数
		// 当客户端连接服务端完成之后，这里被触发。
		// 这个函数只会在客户端连接服务端时触发1次，不管连接成功还是连接失败都会触发。连接
		// 成功还是失败，可以使用asio2::get_last_error()来进行判断。
		// 注意：客户端有掉线自动重连机制，因此，如果客户端连接服务端失败，那么客户端过几秒后
		// 会再次自动连接服务端，每连接一次服务端，不管连接成功还是失败，此函数就会被触发1次。
		// 可以在这里给向服务端发送数据了。

		if (asio2::get_last_error())
			printf("connect failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
			printf("connect success : %s %u\n", client.local_address().c_str(), client.local_port());

		// 如果没有错误，就表示连接成功，可以向服务端发送数据了。
		if (!asio2::get_last_error())
			client.async_send("<abcdefghijklmnopqrstovuxyz0123456789>");

	}).bind_recv([&](std::string_view data)
	{
		// 第3步：触发recv回调函数
		// 当收到服务端发送过来的数据之后，这里被触发。
		// 这个函数会多次触发，每接收到1次数据，就会触发1次。
		// 可以在这里对接收到的数据进行解析处理。

		printf("recv : %u %.*s\n", (unsigned)data.size(), (int)data.size(), data.data());

		client.async_send(data);

	}).bind_disconnect([&]()
	{
		// 第4步：触发disconnect回调函数
		// 当客户端关闭时(或者服务端关闭了)，在socket即将关闭之前，这里被触发。
		// 这个函数只会在客户端关闭时触发1次。且只有在客户端连接成功之后才会被触发，就是
		// 说如果客户端没有成功连接上服务端，那么客户端在关闭时是不会触发disconnect的。
		// 可以在这里记录客户端的关闭日志。

		printf("disconnect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());

	});

	client.start(host, port);

	while (std::getchar() != '\n');
		
	return 0;
}

```

### 所有回调函数都必须要写吗？
 - 服务端大约有至少7个回调函数可以写，客户端的回调函数少一些，那是不是所有的回调函数都必须要写呢？
 - 不是的，想写几个就写几个即可。
 - 比如只想要接收数据的功能，其它都不要，那就只写一个bind_recv就行了。如下：
```cpp
#include <asio2/asio2.hpp>

int main()
{
	std::string_view host = "0.0.0.0";
	std::string_view port = "8028";

	asio2::tcp_server server;

	server.bind_recv([&](std::shared_ptr<asio2::tcp_session> & session_ptr, std::string_view data)
	{
		printf("recv : %u %.*s\n", (unsigned)data.size(), (int)data.size(), data.data());

		session_ptr->async_send(data, [](std::size_t bytes_sent) {});

	});

	server.start(host, port);

	while (std::getchar() != '\n');  // press enter to exit this program

	server.stop();

	return 0;
}
```
 - 但要注意的是，不管你有没有绑定事件回调函数，实际上所有事件都是执行过了的，只不过你没有绑定回调函数，在执行到该事件时asio2框架就没有通知你罢了。比如，如果你没有调用bind_recv来绑定一个接收数据的回调函数，那asio2内部还是照样在接收数据的，只是收到数据时没有通知你。
## 项目地址：
github : [https://github.com/zhllxt/asio2](https://github.com/zhllxt/asio2)
码云 : [https://gitee.com/zhllxt/asio2](https://gitee.com/zhllxt/asio2)

### 最后编辑于2022-06-23
