# asio2
A open source cross-platform c++ library for network programming based on asio,support for tcp,udp,http,rpc,ssl and so on.

* Support TCP,UDP,HTTP,WEBSOCKET,RPC,ICMP,SERIAL_PORT;
* Support reliable UDP (based on KCP), support SSL, support loading SSL certificates from memory strings;
* TCP supports data unpacking (character or string or user defined protocol), and implements the datagram mode of TCP (similar to WEBSOCKET);
* Support windows, linux, 32 bits, 64 bits;
* Dependence on C++ 17,dependence on asio (boost::asio or asio standalone). If HTTP functions are required, can only use boost::asio.
* The demo directory contains a large number of sample projects (projects based on VS2017 creation), and a variety of use methods refer to the sample code.

## TCP:
##### server:
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
//server.start("0.0.0.0", 8080, '\n'); // Automatic unpacking by \n (arbitrary characters can be specified)
//server.start("0.0.0.0", 8080, "\r\n"); // Automatic unpacking by \r\n (arbitrary string can be specified)
//server.start("0.0.0.0", 8080, match_role('#')); // Automatic unpacking according to the rules specified by match_role (see demo code for match_role) (for user-defined protocol unpacking)
//server.start("0.0.0.0", 8080, asio::transfer_exactly(100)); // Receive a fixed 100 bytes at a time
//server.start("0.0.0.0", 8080, asio2::use_dgram); // TCP in datagram mode, no matter how long the data is sent, the whole package data of the corresponding length must be received by both sides.
```
##### client:
```c++
asio2::tcp_client client;
// The client will automatically reconnect when it disconnects
//// [ default reconnect option is "enable" ]
//client.reconnect(false); // disable auto reconnect
//client.reconnect(true); // enable auto reconnect and use the default delay
client.reconnect(true, std::chrono::milliseconds(100)); // enable auto reconnect and use custom delay
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
	//.bind_recv(on_recv) // Binding global functions
	//.bind_recv(std::bind(&listener::on_recv, &lis, std::placeholders::_1)) // Binding member functions (see demo code for details)
	//.bind_recv(&listener::on_recv, lis) // Bind member functions by reference to lis object (see demo code for details)
	//.bind_recv(&listener::on_recv, &lis) // Bind member functions by pointers to lis object (see demo code for details)
	;
client.async_start("0.0.0.0", 8080); // Asynchronous connection to server
//client.start("0.0.0.0", 8080); // Synchronized connection to server
//client.async_start("0.0.0.0", 8080, '\n');
//client.async_start("0.0.0.0", 8080, "\r\n");
//client.async_start("0.0.0.0", 8080, match_role);
//client.async_start("0.0.0.0", 8080, asio::transfer_exactly(100));
//client.start("0.0.0.0", 8080, asio2::use_dgram);
```

## UDP:
##### server:
```c++
asio2::udp_server server;
// ... Binding listener (see demo code)
server.start("0.0.0.0", 8080); // general UDP
//server.start("0.0.0.0", 8080, asio2::use_kcp); // Reliable UDP
```
##### client:
```c++
asio2::udp_client client;
// ... Binding listener (see demo code)
client.start("0.0.0.0", 8080);
//client.async_start("0.0.0.0", 8080, asio2::use_kcp); // Reliable UDP
```

## RPC:
##### server:
```c++
asio2::rpc_server server;
// ... Binding listener (see demo code)
A a; // For the definition of A, see the demo code
server.bind("add", add); // Binding RPC global functions
server.bind("mul", &A::mul, a); // Binding RPC member functions
server.bind("cat", [&](const std::string& a, const std::string& b) { return a + b; }); // Binding lambda
server.bind("get_user", &A::get_user, a); // Binding member functions (by reference)
server.bind("del_user", &A::del_user, &a); // Binding member functions (by pointer)
//server.start("0.0.0.0", 8080, asio2::use_dgram); // Using TCP datagram mode as the underlying support of RPC communication, the use_dgram parameter must be used when starting the server.
server.start("0.0.0.0", 8080); // Using websocket as the underlying support of RPC communication(You need to go to the end code of the rcp_server.hpp file and choose to use websocket)
```
##### client:
```c++
asio2::rpc_client client;
// ... Binding listener (see demo code)
//client.start("0.0.0.0", 8080, asio2::use_dgram);
client.start("0.0.0.0", 8080);
asio::error_code ec;
// Synchronized invoke RPC functions
int sum = client.call<int>(ec, std::chrono::seconds(3), "add", 11, 2);
printf("sum : %d err : %d %s\n", sum, ec.value(), ec.message().c_str());
// Asynchronous invocation of RPC function, the first parameter is the callback function, when the call is completed or timeout, the callback function automatically called, if timeout or other errors,
// error codes are stored in ec, where async_call does not specify the result value type, the second parameter of the lambda expression must specify the type.
client.async_call([](asio::error_code ec, int v)
{
	printf("sum : %d err : %d %s\n", v, ec.value(), ec.message().c_str());
}, "add", 10, 20);
// Here async_call specifies the result value type, the second parameter of the lambda expression can be auto type.
client.async_call<int>([](asio::error_code ec, auto v)
{
	printf("sum : %d err : %d %s\n", v, ec.value(), ec.message().c_str());
}, "add", 12, 21);
// Result value is user-defined data type (see demo code for the definition of user type)
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
// If the result value of the RPC function is void, then the user callback function has only one parameter.
client.async_call([](asio::error_code ec)
{
}, "del_user", std::move(u));

```

## HTTP and WEBSOCKET:
##### See the sample code http and websocket section

## ICMP:
##### See the sample code ping test section

## serial port:
##### See the sample code serial port section
