# asio2
A open source cross-platform c++ library for network programming based on boost::asio,support for tcp,udp,http,ssl and so on.

[README in English](https://github.com/zhllxt/asio2/README.en.md) 

* 支持TCP,UDP,HTTP,SSL,支持从内存字符串加载SSL证书；支持windows,linux,32位,64位；
* 代码基于boost::asio构建，使用了C++11，采用URL字符串方式建立server或client连接；
* 已将boost::asio代码单独摘出并引入，无需安装boost和openssl库，所有代码均是hpp文件，以源码级链入，只需在工程的Include包含目录中添加asio2路径，然后在源码中#include <asio2/asio2.hpp>包含头文件即可；
* demo目录包含大量的示例工程（工程基于VS2017创建），各种使用方法请参考示例代码；

## TCP：
#### :small_orange_diamond:PACK模式：
用于常见的“包头,包体,包尾”等等封包格式，给server或client设置一个封包解析函数后，则server或client在收到数据时会自动调用解析函数进行拆包组包等操作，确保投递给用户数据接收监听器的是一个完整的封包；
##### 服务端：
```c++
// head 1 byte <
// len  1 byte content len,not include head and tail,just include the content len
// ...  content
// tail 1 byte >

std::size_t pack_parser(asio2::buffer_ptr & data_ptr)
{
	if (data_ptr->size() < 3) // 接收数据长度小于封包最小长度：包头1个字节，包体长度1个字节，包尾1个字节
		return asio2::need_more_data; // 返回“需要更多数据”

	uint8_t * data = data_ptr->data();
	if (data[0] == '<') // 包头正确
	{
		std::size_t pack_len = data[1] + 3;
		if (data_ptr->size() < pack_len) // 接收的数据总长度小于一个完整包的长度
			return asio2::need_more_data; // 返回“需要更多数据”
		if (data[pack_len - 1] == '>') // 包尾正确
			return pack_len; // 返回这个封包的完整长度
	}

	return asio2::invalid_data; // 如果发现无效数据，返回invalid_data,则此连接会被断开
}
asio2::server tcp_pack_server(" tcp://*:8099/pack?pool_buffer_size=1024");
tcp_pack_server.bind_recv([](asio2::session_ptr & session_ptr, asio2::buffer_ptr & data_ptr) // 设置数据接收监听器
{
	// session_ptr 连接对象智能指针 buffer_ptr 数据对象智能指针 (server端会将连接对象和通信数据一起通知给监听器)
	std::printf("recv : %.*s\n", (int)data_ptr->size(), (const char*)data_ptr->data());
}).set_pack_parser(std::bind(pack_parser, std::placeholders::_1)); // 设置封包格式解析器
tcp_pack_server.start();
```
##### 客户端：
```c++
asio2::client tcp_pack_client("tcp://localhost:8099/pack");
tcp_pack_client.bind_recv([](asio2::buffer_ptr & data_ptr) // 设置数据接收监听器
{
	std::printf("recv : %.*s\n", (int)data_ptr->size(), (const char*)data_ptr->data());
}).set_pack_parser(std::bind(pack_parser, std::placeholders::_1)); // 设置封包格式解析器
tcp_pack_client.start();
```
#### :small_orange_diamond:AUTO模式：
server或client可发送任意数据包，接收方会确保收到的数据长度和发送方发送的数据长度一致时才会触发用户的数据接收监听器；注意：AUTO模式发送数据时会自动在数据头添加4个字节的额外数据，用于标识发送内容的长度，接收方解析时需要使用该4字节解析出内容长度；
##### 服务端：
```c++
asio2::server tcp_auto_server("tcp://*:8098/auto");
tcp_auto_server.bind_recv([](asio2::session_ptr & session_ptr, asio2::buffer_ptr & data_ptr)
{
	std::printf("recv : %.*s\n", (int)data_ptr->size(), (const char*)data_ptr->data());
});
tcp_auto_server.start();
```
##### 客户端：
```c++
asio2::client tcp_auto_client("tcp://127.0.0.1:8098/auto");
tcp_auto_client.bind_recv([](asio2::buffer_ptr & data_ptr) // 设置数据接收监听器
{
	std::printf("recv : %.*s\n", (int)data_ptr->size(), (const char*)data_ptr->data());
});
tcp_auto_client.start();
```
## SSL TCP：
##### 服务端：
```c++
std::string cer =
	"-----BEGIN CERTIFICATE-----\r\n"\
	"MIICcTCCAdoCCQDYl7YrsugMEDANBgkqhkiG9w0BAQsFADB9MQswCQYDVQQGEwJD\r\n"\
	"TjEOMAwGA1UECAwFSEVOQU4xEjAQBgNVBAcMCVpIRU5HWkhPVTENMAsGA1UECgwE\r\n"\
	"SE5aWDENMAsGA1UECwwESE5aWDEMMAoGA1UEAwwDWkhMMR4wHAYJKoZIhvcNAQkB\r\n"\
	"Fg8zNzc5MjczOEBxcS5jb20wHhcNMTcxMDE1MTQzNjI2WhcNMjcxMDEzMTQzNjI2\r\n"\
	"WjB9MQswCQYDVQQGEwJDTjEOMAwGA1UECAwFSEVOQU4xEjAQBgNVBAcMCVpIRU5H\r\n"\
	"WkhPVTENMAsGA1UECgwESE5aWDENMAsGA1UECwwESE5aWDEMMAoGA1UEAwwDWkhM\r\n"\
	"MR4wHAYJKoZIhvcNAQkBFg8zNzc5MjczOEBxcS5jb20wgZ8wDQYJKoZIhvcNAQEB\r\n"\
	"BQADgY0AMIGJAoGBAMc2Svpl4UgxCVKGwoYJBxNWObXvQzw74ksY6Zoiq5tJNJzf\r\n"\
	"q9ZCJigwjx3vAFF7tELRxsgAf6l7AvReu1O6difjdpMkEic0W7acZtldislDjUbu\r\n"\
	"qitfHsWeKTucBu3+3TUawvv+fdeWgeN54jMoL+Oo3CV7d2gFRV2fD5z4tryXAgMB\r\n"\
	"AAEwDQYJKoZIhvcNAQELBQADgYEAwDIC3xYmYJ6kLI8NgmX89re0scSWCcA8VgEZ\r\n"\
	"u8roYjYauCLkp1aXNlZtJFQjwlfo+8FLzgp3dP8Y75YFwQ5zy8fFaLQSQ/0syDbx\r\n"\
	"sftKSVmxDo3S27IklEyJAIdB9eKBTeVvrT96R610j24t1eYENr59Vk6A/fKTWJgU\r\n"\
	"EstmrAs=\r\n"\
	"-----END CERTIFICATE-----\r\n";

std::string key =
	"-----BEGIN RSA PRIVATE KEY-----\r\n"\
	"Proc-Type: 4,ENCRYPTED\r\n"\
	"DEK-Info: DES-EDE3-CBC,EC5314BD06CD5FB6\r\n"\
	"\r\n"\
	"tP93tjR4iOGfOLHjIBQA0aHUE5wQ7EDcUeKacFfuYrtlYbYpbRzhQS+vGtoO1wGg\r\n"\
	"h/s9DbEN1XaiV9aE+N3E54zu2LuVO1lYDtCf3L26cd1Bu6gj0cWiAMco1Vm7RV9j\r\n"\
	"vmgmeOYkqbOiAbiIa4HCmDkEaHY4nCPlW+cdYxrozkAQCAiTpFQR8taRB0lsly0i\r\n"\
	"lUQitYLz3nhEMucLffcwAXN9IOnXFoURVZnLc53CX857iizOXeP9XeNE63UwDZ4v\r\n"\
	"1wnglnGUJA6vCxnxk6KvptF9rSdCD/sz1Y+J5mAVr+2y4vPLO4YOCL6HSFY6285M\r\n"\
	"RyGNVVx3vX0u6FbWJC3qt5yj6tMdVJ4O7U4XgqOKnS5jVLk+fKcTVyNySB5yAT2b\r\n"\
	"qwWCZcRPP2M+qlsSWhgzsucyz0eVOPVJxAJ4Vp/X6saO4xyRPsFV3USbRKlOMS7+\r\n"\
	"SEJ/7ANU9mEgLIQRKEfSKXWpQtm95pCVlajWQ7/3nXNjdV7mNi42ukdItBvOtdv+\r\n"\
	"oUiN8MkP/e+4SsGmJayNT7HvBC9DjoyDQIK6sZOgtsbAu/bDBhPnjnNsZcsgxJ/O\r\n"\
	"ijnj+0HyNS/Vr6emAkxTFgryUdBTuoY7019vcNWTYPDS3ugpe3goRHE0FTOwNdUe\r\n"\
	"dk+KM4bYAa0+1z1QEZTEoNqdT7WYwMD1QzgSWukYHemsWqoAvW5f4PrdoVA21W9D\r\n"\
	"L8I1YZf8ZHBnkuGX0oHi5w/4DkVNOT5BaZRmqXinZgFPwduYGVCh04x7ohuOQ5m0\r\n"\
	"etrTAVwJd2mcI7rDTaKCPT528/QWxZxXpHzggRoDil/5T7fn35ixRg==\r\n"\
	"-----END RSA PRIVATE KEY-----\r\n";

std::string dh =
	"-----BEGIN DH PARAMETERS-----\r\n"\
	"MEYCQQCdoJif7jYqTh5+vLgt3q1FZvG+7WymoAoMKWMNOtqLZ+uFhZH3e9vFhV7z\r\n"\
	"NgWnHCe/vsGJok2wHS4R/laH6MQTAgEC\r\n"\
	"-----END DH PARAMETERS-----\r\n";

asio2::server tcps_server("tcps://*:9443/auto");
tcps_server.set_certificate("test", cer, key, dh); // 从内存字符串加载证书
//tcps_server.set_certificate_file("test", "server.crt", "server.key", "dh512.pem"); // 从文件加载证书

tcps_server.bind_recv([](asio2::session_ptr & session_ptr, asio2::buffer_ptr & data_ptr)
{
	std::printf("recv : %.*s\n", (int)data_ptr->size(), (const char*)data_ptr->data());

	session_ptr->send(data_ptr);
});
tcps_server.start();
```
##### 客户端：
请参考demo目录下的示例代码；
