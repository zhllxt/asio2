# 基于c++和asio的网络编程框架asio2教程基础篇：3、各个回调函数的触发线程以及多线程总结
 - 关于asio的多线程的知识点感觉挺多的，需要对“服务端，客户端，tcp，udp”分别来总结。
 - 而且了解各个函数分别在哪个线程中执行的，对于多线程编程以及和做具体业务时变量要不要用锁来保护，非常重要。
 - <font color=#ff0000>asio2是“one io_context per cpu”的多线程模式。</font>这个概念主要是asio本身的概念，不了解的可以搜索一下，资料还是挺多的。
## 服务端：
### tcp_server：
 - 由于tcp是面向连接的协议，所以服务端大都设计成多线程的，把多个连接(比如1千个连接)平均分布到各个线程中去处理。
 - asio2的tcp_server构造函数的默认参数如下：

```cpp
explicit tcp_server_impl_t(
	std::size_t init_buffer_size = tcp_frame_size,
	std::size_t max_buffer_size  = (std::numeric_limits<std::size_t>::max)(),
	std::size_t concurrency      = std::thread::hardware_concurrency() * 2
)
```

 - 参数1表示接收缓冲区的初始大小(即默认给每个连接分配这个大小的buffer来接收数据)
 - 参数2表示接收缓冲区的最大大小(当接收的数据越来越多时，asio会自动扩大buffer，默认是可以无限制扩大)
 - 参数3表示服务端启动多少个线程，默认值是cpu*2

```cpp

int main()
{
	std::string_view host = "0.0.0.0";
	std::string_view port = "8028";

	// 由于tcp_server默认会启动cpu*2个数量的线程，假定cpu核数为4，那就是8个线程，
	// 假定为“线程0，线程1，...到...线程7”
	// 这里main函数的线程假定为“线程main”，（新版本asio2没有任何事件会在main线程中触发）

	asio2::tcp_server server;

	// 针对server对象启动一个定时器
	server.start_timer(123, std::chrono::seconds(1), []()
	{
		// 这个定时器的回调函数固定在“线程0”中触发
	});

	// 针对这个session_ptr投递一个异步事件
	server.post([]()
	{
		// 对这个server对象的投递的异步事件固定在“线程0”中触发
		printf("投递的异步事件被执行了\n");
	});


	server.bind_init([]()
	{
		// 固定在“线程0”中触发

	}).bind_start([&]()
	{
		// 固定在“线程0”中触发

	}).bind_accept([](std::shared_ptr<asio2::tcp_session>& session_ptr)
	{
		// 固定在“线程0”中触发

	}).bind_connect([&](auto & session_ptr)
	{
		// 固定在“线程0”中触发

		// 连接成功以后，可以给这个连接(即session_ptr)启动一个定时器
		session_ptr->start_timer("123", std::chrono::seconds(1), []()
		{
			// 这个session_ptr的定时器的回调函数和bind_recv的触发线程是同一个线程
			// (见下方bind_recv中的说明)
		});

		// 针对这个session_ptr投递一个异步事件
		session_ptr->post([]()
		{
			// 对这个session_ptr投递的异步事件的回调函数和bind_recv的触发线程是
			// 同一个线程(见下方bind_recv中的说明)
			printf("投递的异步事件被执行了\n");
		});

	}).bind_recv([&](std::shared_ptr<asio2::tcp_session> & session_ptr, std::string_view data)
	{
		// 平均分布在“线程0，线程1，...到...线程7”中触发

		// 假如session A的bind_recv在“线程2”中触发，session B的bind_recv在“线程3”中触发，
		// 那么session A的bind_recv将永远只在“线程2”中触发，不会出现一会儿在“线程2”中触发
		// 一会儿在“线程3”中触发这种情况，session B同理。

		// 对于async_send发送数据函数来说，真正发送数据时所在的线
		// 程和bind_recv的触发线程是同一个线程。

		// 假定session A的bind_recv的触发线程是“线程2”，那么session A的async_send函数也是在“线程2”
		// 中发送的数据的(不管async_send函数在哪里调用，不管在哪个线程调用，最终都是投递到“线程2”中
		// 去发送的)。
		session_ptr->async_send(data, [](std::size_t bytes_sent)
		{
			// async_send函数可以设置一个回调函数，当发送数据结束以后(不管发送成功还是发送失败)，这个
			// 回调函数就会被调用。
			// 这个回调函数也是在“线程2”中触发的。
			if (asio2::get_last_error())
				printf("发送数据失败,失败原因:%s\n", asio2::last_error_msg().c_str());
			else
				printf("发送数据成功,共发送了%d个字节\n", int(bytes_sent));
		});

	}).bind_disconnect([&](auto & session_ptr)
	{
		// 固定在“线程0”中触发

	}).bind_stop([&]()
	{
		// 固定在“线程0”中触发

	});

	server.start(host, port);

	while (std::getchar() != '\n');  // press enter to exit this program

	server.stop();

	return 0;
}

```
 - http,websocket,rpc都是基于tcp协议实现的，所以这些组件的服务端回调函数触发线程和tcp完全一样。
 - 但有点不同的是ssl有握手事件,websocket有协商事件，如下：

```cpp
// 对于ssl有：
server.bind_handshake([&](auto & session_ptr)
{
	// 固定在“线程0”中触发
})
```

```cpp
// 对于websocket有：
server.bind_upgrade([&](auto & session_ptr)
{
	// 固定在“线程0”中触发
})
```

### udp_server：
- asio2的udp_server构造函数的默认参数如下：

```cpp
explicit udp_server_impl_t(
	std::size_t init_buffer_size = udp_frame_size,
	std::size_t max_buffer_size  = (std::numeric_limits<std::size_t>::max)()
)
```
 - 这里只有参数1和参数2的缓冲区大小设置，没有启动多少个线程这个参数了，这是因为udp是无连接的，所以服务端通信线程在这里设置为固定的1了。启动多个线程没有意义。
```cpp
#include <asio2/asio2.hpp>

int main()
{
	std::string_view host = "0.0.0.0";
	std::string_view port = "8035";

	asio2::udp_server server;

	// 因为udp_server只有一个线程，所有的事件，包括recv,timer,post事件等，都是在“线程0”中触发的。

	server.bind_init([&]()
	{
		// 固定在“线程0”中触发

	}).bind_start([&]()
	{
		// 固定在“线程0”中触发

	}).bind_connect([](auto & session_ptr)
	{
		// 固定在“线程0”中触发

	}).bind_handshake([](auto & session_ptr)
	{
		// 固定在“线程0”中触发

		// 注意bind_handshake是针对可靠UDP的，即使用KCP时才有handshake事件，普通UDP
		// 是没有handshake事件的。但不管有没有用KCP，你调用不调用bind_handshake都
		// 可以，无所谓的，大不了不会触发而已。

	}).bind_recv([](std::shared_ptr<asio2::udp_session> & session_ptr, std::string_view s)
	{
		// 固定在“线程0”中触发

		session_ptr->async_send(s, []() {});

	}).bind_disconnect([](auto & session_ptr)
	{
		// 固定在“线程0”中触发

	}).bind_stop([&]()
	{
		// 固定在“线程0”中触发
	});

	server.start(host, port);

	while (std::getchar() != '\n');

	return 0;
}
```

## 客户端
 - asio2的tcp_client构造函数的默认参数如下：

```cpp
explicit tcp_client_impl_t(
	std::size_t init_buffer_size = tcp_frame_size,
	std::size_t max_buffer_size  = (std::numeric_limits<std::size_t>::max)()
)
```
 - 同样没有启动多少个线程这个参数了，因为所有客户端，不管是tcp还是udp，都将通信线程设置为固定的1个了。
```cpp
#include <asio2/asio2.hpp>

int main()
{
	std::string_view host = "127.0.0.1";
	std::string_view port = "8028";

	asio2::tcp_client client;

	// 因为所有客户端只有一个线程，所有的事件，包括recv,timer,post事件等，都是在“线程0”中触发的。
	
	// 注意：客户端没有start和stop事件了，实际上已经被connect和disconnect
	// 事件代替了。

	client.start_timer(1, std::chrono::seconds(1), []() {});

	client.bind_init([&]()
	{
		// 固定在“线程0”中触发

	}).bind_connect([&]()
	{
		// 固定在“线程0”中触发

		client.async_send("abc", [](std::size_t bytes) {});

	}).bind_recv([&](std::string_view sv)
	{
		// 固定在“线程0”中触发

	}).bind_disconnect([&]()
	{
		// 固定在“线程0”中触发
	});

	client.start(host, port);

	while (std::getchar() != '\n');

	return 0;
}
```
## 项目地址：
github : [https://github.com/zhllxt/asio2](https://github.com/zhllxt/asio2)
码云 : [https://gitee.com/zhllxt/asio2](https://gitee.com/zhllxt/asio2)

### 最后编辑于2022-06-23
