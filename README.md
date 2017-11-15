# asio2
A open source cross-platform c++ library for network programming based on boost::asio,support for tcp,udp,http,ssl and so on.

```c++
asio2::server tcp_pack_server(" tcp://*:8099/pack?pool_buffer_size=1024");
tcp_pack_server.bind_recv([&tcp_pack_server](asio2::session_ptr session_ptr, asio2::buffer_ptr data_ptr)
{
	std::printf("recv : %.*s\n", (int)data_ptr->size(), (const char*)data_ptr->data());
}).set_pack_parser(std::bind(pack_parser, std::placeholders::_1));
tcp_pack_server.start();
