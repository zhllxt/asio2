# asio2
A open source cross-platform c++ library for network programming based on boost::asio,support for tcp,udp,http,ssl and so on.

* Support TCP, UDP, HTTP, SSL, support load SSL certificates from the memory string; support windows, Linux, 32 bit, 64 bit;
* The code is based on boost::asio construction, using C++11, using URL string to establish server or client connections;
* boost::asio code has been picked out and introduced, no need to install boost and OpenSSL libraries, all the code is the hpp file, just need to add the asio2 path to the Include inclusion directory of the project, and then use #include <asio2/asio2.hpp> to contains the header file in the source code;
* The demo directory contains a large number of sample projects (projects based on VS2017 creation), and a variety of use methods refer to the sample code.

## TCP:
#### :small_orange_diamond:PACK mode:
used for "packet header,packet body,packet tail" or other similar packet formats, you need to setting up a packet parsing function, when data is recvd, the packet parse function will be invoked automatically, and user custom data listener will recvd a completed packet;
##### server:
```c++
// head 1 byte <
// len  1 byte content len,not include head and tail,just include the content len
// ...  content
// tail 1 byte >

std::size_t pack_parser(asio2::buffer_ptr & data_ptr)
{
	if (data_ptr->size() < 3) // recvd data len is less than min packet len:packet head(one byte) + packet size(one byte) + packet tail(one byte)
		return asio2::need_more_data; // return need_more_data

	uint8_t * data = data_ptr->data();
	if (data[0] == '<') // packet head is correct
	{
		std::size_t pack_len = data[1] + 3;
		if (data_ptr->size() < pack_len) // recvd data len is less than completed packet len
			return asio2::need_more_data; // return need_more_data
		if (data[pack_len - 1] == '>') // packet tail is correct
			return pack_len; // return this completed packet total len
	}

	return asio2::invalid_data; // find invalid data, return invalid_data, then this session will be closed automatically
}
asio2::server tcp_pack_server(" tcp://*:8099/pack?pool_buffer_size=1024");
tcp_pack_server.bind_recv([](asio2::session_ptr & session_ptr, asio2::buffer_ptr & data_ptr) // set recvd data listener
{
	std::printf("recv : %.*s\n", (int)data_ptr->size(), (const char*)data_ptr->data());
}).set_pack_parser(std::bind(pack_parser, std::placeholders::_1)); // setting up a recvd data parser function
tcp_pack_server.start();
```
##### client:
```c++
asio2::client tcp_pack_client("tcp://localhost:8099/pack");
tcp_pack_client.bind_recv([](asio2::buffer_ptr & data_ptr) // set recvd data listener
{
	std::printf("recv : %.*s\n", (int)data_ptr->size(), (const char*)data_ptr->data());
}).set_pack_parser(std::bind(pack_parser, std::placeholders::_1)); // set recvd data packet parser
tcp_pack_client.start();
```
#### :small_orange_diamond:AUTO mode:
server or client can sent arbitrary data, the recver will ensure recvd data len is equal to the sent data len, and then call user custom data listener; note : when send data with AUTO mode, it will automatically insert extra four bytes data of sent data len into the packet head internal, the recver need the four bytes to calc the recvd data len first.
##### server:
```c++
asio2::server tcp_auto_server("tcp://*:8098/auto");
tcp_auto_server.bind_recv([](asio2::session_ptr & session_ptr, asio2::buffer_ptr & data_ptr)
{
	std::printf("recv : %.*s\n", (int)data_ptr->size(), (const char*)data_ptr->data());
});
tcp_auto_server.start();
```
##### client:
```c++
asio2::client tcp_auto_client("tcp://127.0.0.1:8098/auto");
tcp_auto_client.bind_recv([](asio2::buffer_ptr & data_ptr) // set recvd data listener
{
	std::printf("recv : %.*s\n", (int)data_ptr->size(), (const char*)data_ptr->data());
});
tcp_auto_client.start();
```
## SSL TCP:
##### server:
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
tcps_server.set_certificate("test", cer, key, dh); // loading certificate from memory buffer
//tcps_server.set_certificate_file("test", "server.crt", "server.key", "dh512.pem"); // loading certificate from file

tcps_server.bind_recv([](asio2::session_ptr & session_ptr, asio2::buffer_ptr & data_ptr)
{
	std::printf("recv : %.*s\n", (int)data_ptr->size(), (const char*)data_ptr->data());

	session_ptr->send(data_ptr);
});
tcps_server.start();
```
##### client:
Please refer to the sample code under the demo directory;
