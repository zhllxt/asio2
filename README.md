# asio2
A open source cross-platform c++ library for network programming based on boost::asio,support for tcp,udp,http,ssl and so on.

##### 代码基于boost::asio构建，使用了C++11语法，支持TCP,UDP,HTTP,SSL,支持从内存字符串加载SSL证书，使用URL字符串方式建立server或client对象；已将boost::asio代码单独摘出并引入，无需安装boost和openssl库，所有代码均是hpp文件，以源码级链入，只需在Include包含目录中添加asio2路径，然后#include <asio2/asio2.hpp>包含头文件即可，隐藏通信细节，使用相当简单；

## TCP：
#### PACK模式：
  用于常见的“包头,包体,包尾”等等封包格式，给server或client设置一个封包解析函数后，则server或client会自动进行拆包操作，确保在收到完整的数据包之后才会触发用户的数据接收监听器；
#### 服务端：
```c++
// head 1 byte <
// len  1 byte content len,not include head and tail,just include the content len
// ...  content
// tail 1 byte >

std::size_t pack_parser(asio2::buffer_ptr data_ptr)
{
	if (data_ptr->size() < 3)
		return asio2::need_more_data;

	uint8_t * data = data_ptr->data();
	if (data[0] == '<')
	{
		std::size_t total_len = data[1] + 3;
		if (data_ptr->size() < total_len)
			return asio2::need_more_data;
		if (data[total_len - 1] == '>')
			return total_len;
	}

	return asio2::invalid_data;
}
asio2::server tcp_pack_server(" tcp://*:8099/pack?pool_buffer_size=1024");
tcp_pack_server.bind_recv([](asio2::session_ptr session_ptr, asio2::buffer_ptr data_ptr)
{
	std::printf("recv : %.*s\n", (int)data_ptr->size(), (const char*)data_ptr->data());
}).set_pack_parser(std::bind(pack_parser, std::placeholders::_1));
tcp_pack_server.start();
```
#### 客户端：
```c++
asio2::client tcp_pack_client("tcp://localhost:8099/pack");
tcp_pack_client.bind_recv([](asio2::buffer_ptr data_ptr)
{
	std::printf("recv : %.*s\n", (int)data_ptr->size(), (const char*)data_ptr->data());
}).set_pack_parser(std::bind(pack_parser, std::placeholders::_1));
tcp_pack_client.start();
```
#### AUTO模式：
  server或client可发送任意数据包，接收方会确保收到的数据包是和发送方的数据包完全一致时才会触发用户的数据接收监听器；注意：AUTO模式发送数据时会自动在数据头添加4个字节的额外数据，用于标识发送内容的长度，接收方解析时需要使用该4字节解析出内容长度；
#### 服务端：
```c++
asio2::server tcp_auto_server("tcp://*:8098/auto");
tcp_auto_server.bind_recv([](asio2::session_ptr session_ptr, asio2::buffer_ptr data_ptr)
{
	std::printf("recv : %.*s\n", (int)data_ptr->size(), (const char*)data_ptr->data());
});
tcp_auto_server.start();
```
#### 客户端：
```c++
asio2::client tcp_auto_client("tcp://127.0.0.1:8098/auto");
tcp_auto_client.bind_recv([](asio2::buffer_ptr data_ptr)
{
	std::printf("recv : %.*s\n", (int)data_ptr->size(), (const char*)data_ptr->data());
});
tcp_auto_client.start();
```

