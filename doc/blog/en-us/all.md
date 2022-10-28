# asio2
### [中文](README.md) | English
Header only c++ network library, based on asio,support tcp,udp,http,websocket,rpc,ssl,icmp,serial_port.

* header only, do not rely on the Boost library;
* Support tcp, udp, http, websocket, rpc, ssl, icmp, serial_port;
* Support reliable udp (based on KCP), support SSL, support loading SSL certificates from memory strings;
* TCP supports data unpacking (character or string or user defined protocol), and implements the datagram mode of TCP (similar to WEBSOCKET);
* Support windows,linux,macos,arm,android, 32 bits, 64 bits;Compiled under msvc gcc clang ndk mingw;
* Dependence on C++ 17,dependence on asio (standalone asio or boost::asio). 
* The example directory contains a large number of sample code, and a variety of use methods refer to the sample code.

## TCP:
##### server:
```c++
asio2::tcp_server server;
server.bind_recv([&server](std::shared_ptr<asio2::tcp_session> & session_ptr, std::string_view s)
{
	session_ptr->no_delay(true);

	printf("recv : %zu %.*s\n", s.size(), (int)s.size(), s.data());
    
	session_ptr->async_send(s);
}).bind_connect([&server](auto & session_ptr)
{
	printf("client enter : %s %u %s %u\n",
		session_ptr->remote_address().c_str(), session_ptr->remote_port(),
		session_ptr->local_address().c_str(), session_ptr->local_port());
}).bind_disconnect([&server](auto & session_ptr)
{
	printf("client leave : %s %u %s\n",
		session_ptr->remote_address().c_str(), session_ptr->remote_port(),
		asio2::last_error_msg().c_str());
});
server.start("0.0.0.0", 8080);

// Automatic unpacking by \n (arbitrary characters can be specified)
//server.start("0.0.0.0", 8080, '\n');

// Automatic unpacking by \r\n (arbitrary string can be specified)
//server.start("0.0.0.0", 8080, "\r\n"); 

// Automatic unpacking according to the rules specified by match_role
// (see demo code for match_role) (for user-defined protocol unpacking)
//server.start("0.0.0.0", 8080, match_role('#')); 

// Receive a fixed 100 bytes at a time
//server.start("0.0.0.0", 8080, asio::transfer_exactly(100)); 

// TCP in datagram mode, no matter how long the data is sent, the whole 
// package data of the corresponding length must be received by both sides.
//server.start("0.0.0.0", 8080, asio2::use_dgram); 
```
##### client:
```c++
asio2::tcp_client client;

// The client will automatically reconnect when it disconnects
// [ default reconnect option is "enable" ]

// disable auto reconnect
//client.auto_reconnect(false); 

// enable auto reconnect and use the default delay
//client.auto_reconnect(true); 

// enable auto reconnect and use custom delay
client.auto_reconnect(true, std::chrono::seconds(3));

client.bind_connect([&]()
{
	if (asio2::get_last_error())
		printf("connect failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	else
		printf("connect success : %s %u\n", client.local_address().c_str(), client.local_port());

	client.async_send("<abcdefghijklmnopqrstovuxyz0123456789>");
}).bind_disconnect([]()
{
	printf("disconnect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
}).bind_recv([&](std::string_view sv)
{
	printf("recv : %zu %.*s\n", sv.size(), (int)sv.size(), sv.data());

	client.async_send(sv);
})
	//// Binding global functions
	//.bind_recv(on_recv) 
	//// Binding member functions (see demo code for details)
	//.bind_recv(std::bind(&listener::on_recv, &lis, std::placeholders::_1))
	//// Bind member functions by reference to lis object (see demo code for details)
	//.bind_recv(&listener::on_recv, lis) 
	//// Bind member functions by pointers to lis object (see demo code for details)
	//.bind_recv(&listener::on_recv, &lis) 
	;
// Asynchronous connection to server
client.async_start("0.0.0.0", 8080); 

// Synchronized connection to server
//client.start("0.0.0.0", 8080); 

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
// If you want to know which client called this function, set the first parameter
// to std::shared_ptr<asio2::rpc_session>& session_ptr, If you don't want to,keep 
// it empty is ok.
int add(std::shared_ptr<asio2::rpc_session>& session_ptr, int a, int b)
{
	return a + b;
}

// Specify the "max recv buffer size" to avoid malicious packets, if some client
// sent data packets size is too long to the "max recv buffer size", then the
// client will be disconnect automatic .
asio2::rpc_server server(
	512,  // the initialize recv buffer size : 
	1024, // the max recv buffer size :
	4     // the thread count : 
);

// ... Binding listener (see demo code)

A a; // For the definition of A, see the demo code

// Binding RPC global functions
server.bind("add", add);

// Binding RPC member functions
server.bind("mul", &A::mul, a);

// Binding lambda
server.bind("cat", [&](const std::string& a, const std::string& b) { return a + b; });

// Binding member functions (by reference)
server.bind("get_user", &A::get_user, a);

// Binding member functions (by pointer)
server.bind("del_user", &A::del_user, &a);

server.start("0.0.0.0", 8080);
```
##### client:
```c++
asio2::rpc_client client;
// ... Binding listener (see demo code)
client.start("0.0.0.0", 8080);

// Synchronized invoke RPC functions
int sum = client.call<int>(std::chrono::seconds(3), "add", 11, 2);
printf("sum : %d err : %d %s\n", sum, asio2::last_error_val(), asio2::last_error_msg().c_str());

// Asynchronous invocation of RPC function, the first parameter is the callback function,
// when the call is completed or timeout, the callback function automatically called.
client.async_call([](int v)
{
	// if timeout or other errors, you can get the error info by asio2::get_last_error.
	printf("sum : %d err : %d %s\n", v, asio2::last_error_val(), asio2::last_error_msg().c_str());
}, "add", 10, 20);

// Result value is user-defined data type (see demo code for the definition of user type)
user u = client.call<user>("get_user");
printf("%s %d ", u.name.c_str(), u.age);
for (auto &[k, v] : u.purview)
{
	printf("%d %s ", k, v.c_str());
}
printf("\n");

u.name = "hanmeimei";
u.age = ((int)time(nullptr)) % 100;
u.purview = { {10,"get"},{20,"set"} };
// If the result value of the RPC function is void, then the user callback 
// function has only one parameter.
client.async_call([]()
{
}, "del_user", std::move(u));
// just call rpc function, don't need result
client.async_call("del_user", std::move(u));
```

## HTTP and WEBSOCKET:
##### server:
```c++

struct aop_log
{
	bool before(http::request& req, http::response& rep)
	{
		asio2::detail::ignore_unused(rep);
		printf("aop_log before %s\n", req.method_string().data());
		return true;
	}
	bool after(std::shared_ptr<asio2::http_session>& session_ptr,
		http::request& req, http::response& rep)
	{
		asio2::detail::ignore_unused(session_ptr, req, rep);
		printf("aop_log after\n");
		return true;
	}
};

struct aop_check
{
	bool before(std::shared_ptr<asio2::http_session>& session_ptr,
		http::request& req, http::response& rep)
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

server.bind_recv([&](http::request& req, http::response& rep)
{
	std::cout << req.path() << std::endl;
	std::cout << req.query() << std::endl;

}).bind_connect([](auto & session_ptr)
{
	printf("client enter : %s %u %s %u\n",
	    session_ptr->remote_address().c_str(), session_ptr->remote_port(),
		session_ptr->local_address().c_str(), session_ptr->local_port());
}).bind_disconnect([](auto & session_ptr)
{
	printf("client leave : %s %u %s\n",
		session_ptr->remote_address().c_str(), session_ptr->remote_port(),
		asio2::last_error_msg().c_str());
}).bind_start([&]()
{
	printf("start http server : %s %u %d %s\n",
	server.listen_address().c_str(), server.listen_port(),
		asio2::last_error_val(), asio2::last_error_msg().c_str());
}).bind_stop([&]()
{
	printf("stop : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
});

server.bind<http::verb::get, http::verb::post>("/index.*",
 [](http::request& req, http::response& rep)
{
	rep.fill_file("../../../index.html");
	rep.chunked(true);

}, aop_log{});

server.bind<http::verb::get>("/del_user",
	[](std::shared_ptr<asio2::http_session>& session_ptr,
		http::request& req, http::response& rep)
{
	printf("del_user ip : %s\n", session_ptr->remote_address().data());

	rep.fill_page(http::status::ok, "del_user successed.");

}, aop_check{});

server.bind<http::verb::get>("/api/user/*", [](http::request& req, http::response& rep)
{
	rep.fill_text("the user name is hanmeimei, .....");

}, aop_log{}, aop_check{});

server.bind<http::verb::get>("/defer", [](http::request& req, http::response& rep)
{
	// use defer to make the reponse not send immediately, util the derfer shared_ptr
	// is destroyed, then the response will be sent.
	std::shared_ptr<http::response_defer> rep_defer = rep.defer();

	std::thread([rep_defer, &rep]() mutable
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        
		auto newrep = asio2::http_client::execute("http://www.baidu.com");

		rep = std::move(newrep);

	}).detach();

}, aop_log{}, aop_check{});

server.bind("/ws", websocket::listener<asio2::http_session>{}.
	on("message", [](std::shared_ptr<asio2::http_session>& session_ptr, std::string_view data)
{
	printf("ws msg : %zu %.*s\n", data.size(), (int)data.size(), data.data());

	session_ptr->async_send(data);

}).on("open", [](std::shared_ptr<asio2::http_session>& session_ptr)
{
	printf("ws open\n");

	// print the websocket request header.
	std::cout << session_ptr->request() << std::endl;

	// how to set custom websocket response data : 
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
	rep.fill_page(http::status::not_found);
});

server.start(host, port);
```
##### client:
```c++

auto req1 = http::make_request("http://www.baidu.com/get_user?name=abc");
auto rep1 = asio2::http_client::execute("http://www.baidu.com/get_user?name=abc");
if (asio2::get_last_error())
	std::cout << asio2::last_error_msg() << std::endl;
else
	std::cout << rep1 << std::endl;


auto req2 = http::make_request("GET / HTTP/1.1\r\nHost: 192.168.0.1\r\n\r\n");
auto rep2 = asio2::http_client::execute("www.baidu.com", "80", req2, std::chrono::seconds(3));
if (asio2::get_last_error())
	std::cout << asio2::last_error_msg() << std::endl;
else
	std::cout << rep2 << std::endl;


auto path = asio2::http::url_to_path("/get_user?name=abc");
std::cout << path << std::endl;

auto query = asio2::http::url_to_query("/get_user?name=abc");
std::cout << query << std::endl;

std::cout << std::endl;

auto rep3 = asio2::http_client::execute("www.baidu.com", "80", "/api/get_user?name=abc");
if (asio2::get_last_error())
	std::cout << asio2::last_error_msg() << std::endl;
else
	std::cout << rep3 << std::endl;

std::string en = http::url_encode(R"(http://www.baidu.com/json={"qeury":"name like '%abc%'","id":1})");
std::cout << en << std::endl;
std::string de = http::url_decode(en);
std::cout << de << std::endl;

```


## ICMP:
```c++
asio2::ping ping;
ping.timeout(std::chrono::seconds(3))
	.interval(std::chrono::seconds(1))
	.body("abc")
	.bind_recv([](asio2::icmp_rep& rep)
{
	if (rep.is_timeout())
		std::cout << "request timed out" << std::endl;
	else
		std::cout << rep.total_length() - rep.header_length()
		<< " bytes from " << rep.source_address()
		<< ": icmp_seq=" << rep.sequence_number()
		<< ", ttl=" << rep.time_to_live()
		<< ", time=" << rep.milliseconds() << "ms"
		<< std::endl;
}).start("151.101.193.69");
```
```c++
std::cout << asio2::ping::execute("www.google.com").milliseconds() << std::endl;
```
## SSL:
##### TCP/HTTP/WEBSOCKET all support SSL(config.hpp uncomment #define ASIO2_USE_SSL)
```c++
asio2::tcps_server server;
// server set_verify_mode :
//   "verify_peer", ca_cert_buffer can be empty.
//      Whether the client has a certificate or not is ok.
//   "verify_fail_if_no_peer_cert", ca_cert_buffer can be empty.
//      Whether the client has a certificate or not is ok.
//   "verify_peer | verify_fail_if_no_peer_cert", ca_cert_buffer cannot be empty.
//      Client must use certificate, otherwise handshake will be failed.
// client set_verify_mode :
//   "verify_peer", ca_cert_buffer cannot be empty.
//   "verify_none", ca_cert_buffer can be empty.
server.set_verify_mode(asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert);
// 
server.set_cert_buffer(ca_crt, server_crt, server_key, "server"); // use memory string for cert
server.set_dh_buffer(dh);
// 
server.set_cert_file("ca.crt", "server.crt", "server.key", "server"); // use file for cert
server.set_dh_file("dh1024.pem");
// >> openssl create your certificates and sign them
// ------------------------------------------------------------------------------------------------
// // 1. Generate Server private key
// openssl genrsa -des3 -out server.key 1024
// // 2. Generate Server Certificate Signing Request(CSR)
// openssl req -new -key server.key -out server.csr -config openssl.cnf
// // 3. Generate Client private key
// openssl genrsa -des3 -out client.key 1024
// // 4. Generate Client Certificate Signing Request(CSR)
// openssl req -new -key client.key -out client.csr -config openssl.cnf
// // 5. Generate CA private key
// openssl genrsa -des3 -out ca.key 2048
// // 6. Generate CA Certificate file
// openssl req -new -x509 -key ca.key -out ca.crt -days 3650 -config openssl.cnf
// // 7. Generate Server Certificate file
// openssl ca -in server.csr -out server.crt -cert ca.crt -keyfile ca.key -config openssl.cnf
// // 8. Generate Client Certificate file
// openssl ca -in client.csr -out client.crt -cert ca.crt -keyfile ca.key -config openssl.cnf
// // 9. Generate dhparam file
// openssl dhparam -out dh1024.pem 1024
```

## serial port:
```c++
std::string_view device = "COM1"; // for windows
//std::string_view device = "/dev/ttyS0"; // for linux
std::string_view baud_rate = "9600";

asio2::scp sp;

sp.bind_init([&]()
{
	// Set other serial port parameters at here
	sp.socket().set_option(asio::serial_port::flow_control(asio::serial_port::flow_control::type::none));
	sp.socket().set_option(asio::serial_port::parity(asio::serial_port::parity::type::none));
	sp.socket().set_option(asio::serial_port::stop_bits(asio::serial_port::stop_bits::type::one));
	sp.socket().set_option(asio::serial_port::character_size(8));

}).bind_recv([&](std::string_view sv)
{
	printf("recv : %zu %.*s\n", sv.size(), (int)sv.size(), sv.data());

	std::string s;
	uint8_t len = uint8_t(10 + (std::rand() % 20));
	s += '<';
	for (uint8_t i = 0; i < len; i++)
	{
		s += (char)((std::rand() % 26) + 'a');
	}
	s += '>';

	sp.async_send(std::move(s));

});

//sp.start(device, baud_rate);
sp.start(device, baud_rate, '>');
//sp.start(device, baud_rate, "\r\n");
//sp.start(device, baud_rate, match_role);
//sp.start(device, baud_rate, asio::transfer_at_least(1));
//sp.start(device, baud_rate, asio::transfer_exactly(10));

```


##### timer
```c++
asio2::timer timer;
timer.start_timer(1, std::chrono::seconds(1), [&]()
{
	printf("timer 1\n");
	if (true)
		timer.stop_timer(1);
});
timer.start_timer("id2", 2000, 5, []()
{
	printf("timer id2, loop 5 times\n");
});
timer.start_timer(5, std::chrono::milliseconds(1000), std::chrono::milliseconds(5000), []()
{
	printf("timer 5, loop infinite, delay 5 seconds\n");
});
```
##### Manually triggered events
```c++
asio2::tcp_client client;

// Post an asynchronous condition event that is never executed unless it is manually triggered
std::shared_ptr<asio2::condition_event> event_ptr = client.post_condition_event([]()
{
	// do something.
});

client.bind_recv([&](std::string_view data)
{
	// For example, to achieve a certain condition
	if (data == "some_condition")
	{
		// Trigger the event to start execution
		event_ptr->notify();
	}
});
```
