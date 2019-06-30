/*
g++ -c -x c++ /root/projects/client/demo/src/client.cpp -I /usr/local/include -I /root/projects/client/ -g2 -gdwarf-2 -o "/root/projects/client/obj/x64/Debug/client/client.o" -Wall -Wswitch -W"no-deprecated-declarations" -W"empty-body" -Wconversion -W"return-type" -Wparentheses -W"no-format" -Wuninitialized -W"unreachable-code" -W"unused-function" -W"unused-value" -W"unused-variable" -O0 -fno-strict-aliasing -fno-omit-frame-pointer -fthreadsafe-statics -fexceptions -frtti -std=c++17
g++ -o "/root/projects/client/bin/x64/Debug/client.out" -Wl,--no-undefined -Wl,-L/usr/local/lib -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -lpthread -lrt -ldl /root/projects/client/obj/x64/Debug/client/client.o -lboost_system -lssl -lcrypto
*/
#include <asio2/asio2.hpp>
#include <iostream>

using iterator = asio::buffers_iterator<asio::streambuf::const_buffers_type>;
std::pair<iterator, bool> match_role(iterator b, iterator e)
{
	iterator i = b;
	while (i != e)
	{
		if (*i++ == '#' && i != e)
		{
			unsigned length = std::uint8_t(*i++);
			if (length <= e - i)
				return std::pair(i + length, true);
		}
	}
	return std::pair(b, false);
}

void on_recv(std::string_view sv)
{
	printf("1recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());
}

class listener
{
public:
	void on_recv(std::string_view sv)
	{
		printf("2recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());
	}
};

//struct info
//{
//	int id;
//	char name[20];
//	int8_t age;
//};
//
//#ifdef ASIO_STANDALONE
//namespace asio {
//#else
//namespace boost::asio {
//#endif
//	inline asio::const_buffer buffer(const info& u) noexcept
//	{
//		return asio::const_buffer(&u, sizeof(info));
//	}
//} // namespace asio

void run_tcp_client(std::string_view host, std::string_view port)
{
	//while (1) // use infinite loop and sleep 2 seconds to test start and stop
	{
		//listener lis;
		int count = 1;
		std::unique_ptr<asio2::tcp_client[]> clients = std::make_unique<asio2::tcp_client[]>(count);
		for (int i = 0; i < count; ++i)
		{
			auto & client = clients[i];
			client.start_timer(1, std::chrono::seconds(1), []() {});
			client.bind_connect([&](asio::error_code ec)
			{
				if (asio2::get_last_error())
					printf("connect failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
				else
					printf("connect success : %s %u\n", client.local_address().c_str(), client.local_port());

				std::string s;
				//s += '<';
				//int len = 128 + std::rand() % (300);
				//for (int i = 0; i < len; i++)
				//{
				//	s += (char)((std::rand() % 26) + 'a');
				//}
				//s += '>';

				//s += 'x';
				s += '#';
				s += char(1);
				s += 'a';

				// ## All of the following ways of send operation are correct.
				// (send operation is running in the io.strand thread, the content will be sent directly)
				client.send(s);
				client.send(s, []() {});
				client.send((uint8_t*)"<abcdefghijklmnopqrstovuxyz0123456789>", 10);
				client.send((const uint8_t*)"<abcdefghijklmnopqrstovuxyz0123456789>", 10);
				client.send(s.data(), int(s.size()));
				client.send(s.data(), []() {});
				client.send(s.c_str(), size_t(s.size()));
				client.send(s, asio::use_future);
				client.send("<abcdefghijklmnopqrstovuxyz0123456789>", asio::use_future);
				client.send(s.data(), asio::use_future);
				client.send(s.c_str(), asio::use_future);
				int narys[2] = { 1,2 };
				client.send(narys);
				client.send(narys, []() {});
				client.send(narys, [](std::size_t bytes) {});
				client.send(narys, asio::use_future);

				//// ##Example how to send a struct directly:
				//info u;
				//client.send(u);

				// ##Thread-safe send operation example:
				client.post([&client]()
				{
					asio::write(client.stream(), asio::buffer(std::string("abcdefghijklmn")));
				});

				//asio::write(client.socket(), asio::buffer(s));
			}).bind_disconnect([](asio::error_code ec)
			{
				printf("disconnect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
			}).bind_recv([&](std::string_view sv)
			{
				//asio::write(client.socket(), asio::buffer(sv));
				//printf("3recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());

				//if (sv.front() != '<' || sv.back() != '>')
				//{
				//	printf("%s\n", asio2::last_error_msg().c_str());
				//	while (std::getchar() != '\n');
				//}

				std::string s;
				s += '#';
				uint8_t len = uint8_t(100 + (std::rand() % 100));
				s += char(len);
				for (uint8_t i = 0; i < len; i++)
				{
					s += (char)((std::rand() % 26) + 'a');
				}
				// demo of force a packet of data to be sent twice in "use_dgram" mode
				client.send(s.substr(0, s.size() / 2), []() {});
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				client.send(s.substr(s.size() / 2), [](std::size_t bytes_sent) {});
			})
				//.bind_recv(on_recv)//bind global function
				//.bind_recv(std::bind(&listener::on_recv, &lis, std::placeholders::_1))//bind member function
				//.bind_recv(&listener::on_recv, lis)//bind member function
				//.bind_recv(&listener::on_recv, &lis)//bind member function
				;
			//client.async_start(host, port);
			client.start(host, port);
			//client.async_start(host, port, '>');
			//client.async_start(host, port, "\r\n");
			//client.async_start(host, port, match_role);
			//client.async_start(host, port, asio::transfer_at_least(1));
			//client.async_start(host, port, asio::transfer_exactly(100));
			//client.start(host, port, asio2::use_dgram);

			// ##Use this to check whether the send operation is running in current thread.
			//if (client.io().strand().running_in_this_thread())
			//{
			//}

			// ## All of the following ways of send operation are correct.
			// (send operation is not running in the io.strand thread, the content will be sent asynchronous)
			std::string s;
			s += '#';
			s += char(1);
			s += 'a';
			if (client.is_started())
			{
				client.send(s);
				client.send(s, []() {});
				client.send((uint8_t*)"<abcdefghijklmnopqrstovuxyz0123456789>", 10);
				client.send((const uint8_t*)"<abcdefghijklmnopqrstovuxyz0123456789>", 10);
				client.send(s.data(), int(s.size()));
				client.send(s.data(), []() {});
				client.send(s.c_str(), size_t(s.size()));
				client.send(s, asio::use_future);
				client.send("<abcdefghijklmnopqrstovuxyz0123456789>", asio::use_future);
				client.send(s.data(), asio::use_future);
				client.send(s.c_str(), asio::use_future);
				int narys[2] = { 1,2 };
				client.send(narys);
				client.send(narys, []() {});
				client.send(narys, [](std::size_t bytes) {});
				client.send(narys, asio::use_future);
			}
		}

		while (std::getchar() != '\n');
		//std::this_thread::sleep_for(std::chrono::seconds(2));

		for (int i = 0; i < count; ++i)
		{
			auto & client = clients[i];
			client.stop();
		}
	}
}

void run_tcp_client_group(std::string_view host, std::string_view port)
{
	static constexpr std::size_t count = 2;
	std::unique_ptr<asio2::tcp_client[]> clients = std::make_unique<asio2::tcp_client[]>(count);
	while (1)
	{
		for (std::size_t i = 0; i < count; i++)
		{
			auto & client = clients[i];
			client.start_timer(1, std::chrono::seconds(1), []() {});
			client.bind_connect([&](asio::error_code ec)
			{
				if (asio2::get_last_error())
					printf("connect failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
				else
					printf("connect success : %s %u\n", client.local_address().c_str(), client.local_port());

				std::string s;
				s += '<';
				int len = 128 + std::rand() % (300);
				for (int i = 0; i < len; i++)
				{
					s += (char)((std::rand() % 26) + 'a');
				}
				s += '>';

				client.send(std::move(s));
			}).bind_disconnect([](asio::error_code ec)
			{
				printf("disconnect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
			}).bind_recv([&](std::string_view sv)
			{
				client.send(sv);
			});
			client.async_start(host, port);
		}

		//while (std::getchar() != '\n');
		std::this_thread::sleep_for(std::chrono::seconds(1));

		for (std::size_t i = 0; i < count; i++)
		{
			auto & client = clients[i];
			client.stop();
		}
	}
}

void run_udp_client(std::string_view host, std::string_view port)
{
	//while (1)
	{
		printf("\n");
		asio2::udp_client client;
		client.connect_timeout(std::chrono::seconds(3));
		//client.local_endpoint(asio::ip::address_v4::from_string("127.0.0.1"), 9876);
		client.local_endpoint(asio::ip::udp::v4(), 15678);
		//std::string msg;
		//msg.resize(15000, 'a');
		client.bind_connect([&](asio::error_code ec)
		{
			if (asio2::get_last_error())
				printf("connect failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
			else
				printf("connect success : %s %u\n", client.local_address().c_str(), client.local_port());

			//client.send(msg);
			client.send(std::string("<abcdefghijklmnopqrstovuxyz0123456789>"));
		}).bind_disconnect([](asio::error_code ec)
		{
			printf("disconnect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		}).bind_recv([&](std::string_view sv)
		{
			printf("recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());

			//std::this_thread::sleep_for(std::chrono::milliseconds(100));

			std::string s;
			s += '<';
			int len = 33 + std::rand() % (126 - 33);
			for (int i = 0; i < len; i++)
			{
				s += (char)((std::rand() % 26) + 'a');
			}
			s += '>';

			//client.send(std::move(s));
			//client.post([&client, s = std::string(sv)]()
			//{
			//	client.stream().send(asio::buffer(std::move(s)));
			//});
			//client.send(sv, asio::use_future);
			//client.send(sv, [](std::size_t bytes_sent) {});
			//asio::write(client.get_socket(), asio::buffer(s));

		}).bind_handshake([&](asio::error_code ec)
		{
			if (asio2::get_last_error())
				printf("handshake failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
			else
				printf("handshake success : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());

			//client.send(msg);
			if (!asio2::get_last_error())
				client.send(std::string("<abcdefghijklmnopqrstovuxyz0123456789abcdefghijklmnopqrstovuxyz0123456789>"));
		});
		//client.async_start(host, port, asio2::use_kcp);
		if (!client.start(host, port))
			printf("start failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		//auto * kp = client.kcp();

		while (std::getchar() != '\n');
		//std::this_thread::sleep_for(std::chrono::seconds(1));

		client.stop();
	}
}

void run_http_client(std::string_view host, std::string_view port)
{
#ifndef ASIO_STANDALONE
	//GET /DataSvr/api/anchor/AddAnchor?json=%7b%22id%22:4990560701320869680,%22name%22:%22anchor222%22,%22ip%22:%22192.168.0.101%22%7d HTTP/1.1
	//Host: localhost:8443
	//Connection: keep-alive
	//User-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/61.0.3163.100 Safari/537.36
	//Upgrade-Insecure-Requests: 1
	//Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8
	//Accept-Encoding: gzip, deflate, br
	//Accept-Language: zh-CN,zh;q=0.8

	asio2::error_code ec;
	//auto req1 = http::make_request("http://www.baidu.com/get_user?name=a");
	//auto req2 = http::make_request("http://www.baidu.com");
	//auto req3 = http::make_request("GET / HTTP/1.1\r\nHost: 127.0.0.1:8443\r\n\r\n");
	//req3.set(http::field::timeout, 5000); // Used to Setting Read Timeout, 5000 milliseconds, default is 3000 milliseconds
	//auto req4 = http::make_request("GET / HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n");
	//auto rep1 = asio2::http_client::execute("http://www.baidu.com/get_user?name=a", ec);
	//auto path = asio2::http::url_to_path("/get_user?name=a");
	//auto rep2 = asio2::http_client::execute("127.0.0.1", "8080", req3);
	//auto rep3 = asio2::http_client::execute("http://127.0.0.1:8080/get_user?name=a", ec);
	//if (ec) std::cout << ec.message() << std::endl;
	//std::cout << rep3 << std::endl;

	auto rep = asio2::http_client::execute(host, port, "/api/get_user?name=zhl");
	std::cout << rep << std::endl << "--------------------\n";

	std::string en = http::url_encode("http://www.baidu.com/get_user?name=zhl");
	std::cout << en << std::endl;
	std::string de = http::url_decode(en);
	std::cout << de << std::endl;

	asio2::http_client client;
	client.bind_recv([&](http::response<http::string_body>& rep)
	{
		std::cout << std::endl << "--------------------\n" << rep;
	}).bind_connect([&](asio::error_code ec)
	{
		if (asio2::get_last_error())
			printf("connect failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
			printf("connect success : %s %u\n", client.local_address().c_str(), client.local_port());

		auto * msg = "GET / HTTP/1.1\r\n\r\n";
		client.send(msg, [](std::size_t bytes_sent) {});
	}).bind_disconnect([](asio::error_code ec)
	{
		printf("disconnect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	});
	client.start(host, port, "/api/get_user?name=zhl");
	while (std::getchar() != '\n');
#endif
}

void run_ws_client(std::string_view host, std::string_view port)
{
#ifndef ASIO_STANDALONE
	asio2::ws_client client;
	client.connect_timeout(std::chrono::seconds(10));
	client.bind_connect([&](asio::error_code ec)
	{
		if (asio2::get_last_error())
			printf("connect failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
			printf("connect success : %s %u\n", client.local_address().c_str(), client.local_port());
		std::string s;
		s += '<';
		int len = 128 + std::rand() % 512;
		for (int i = 0; i < len; i++)
		{
			s += (char)((std::rand() % 26) + 'a');
		}
		s += '>';
		client.send(std::move(s), [](std::size_t bytes_sent) {});
	}).bind_upgrade([&](asio::error_code ec)
	{
		std::cout << "upgrade " << (ec ? "failure : " : "success : ") << ec.value() << " "
			<< ec.message() << std::endl << client.upgrade_response() << std::endl;
		client.send("abc"); // this send will failed, because connection is not fully completed
	}).bind_disconnect([](asio::error_code ec)
	{
		printf("disconnect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	}).bind_recv([&](std::string_view sv)
	{
		printf("recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());

		//std::this_thread::sleep_for(std::chrono::milliseconds(100));

		client.send(sv, []() {});
	});
	if (!client.start(host, port))
	{
		printf("start failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	}

	while (std::getchar() != '\n');
	//std::this_thread::sleep_for(std::chrono::seconds(20));

	client.stop();
#endif
}

void run_tcps_client(std::string_view host, std::string_view port)
{
#ifdef ASIO2_USE_SSL
	std::string_view cer =
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
	//while (1)
	{
		int count = 1;
		std::unique_ptr<asio2::tcps_client[]> clients = std::make_unique<asio2::tcps_client[]>(count);
		for (size_t i = 0; i < count; i++)
		{
			auto & client = clients[i];
			client.set_cert(cer);
			client.connect_timeout(std::chrono::seconds(10));
			//client.set_cert_file("server.crt");
			client.start_timer(1, std::chrono::seconds(1), []() {});
			client.bind_connect([&](asio::error_code ec)
			{
				printf("connect : %s %u %d %s\n", client.local_address().c_str(), client.local_port(),
					ec.value(), ec.message().c_str());
				std::string s;
				s += '<';
				int len = 128 + std::rand() % (300);
				for (int i = 0; i < len; i++)
				{
					s += (char)((std::rand() % 26) + 'a');
				}
				s += '>';

				//client.send(std::move(s));
			}).bind_disconnect([](asio::error_code ec)
			{
				printf("disconnect : %d %s\n", ec.value(), ec.message().c_str());
			}).bind_recv([&](std::string_view sv)
			{
				printf("recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());

				//if (sv.front() != '<' || sv.back() != '>')
				//{
				//	printf("%s\n", asio2::last_error_msg().c_str());
				//	while (std::getchar() != '\n');
				//}

				std::this_thread::sleep_for(std::chrono::milliseconds(10));

				client.send(sv);
			}).bind_handshake([&](asio::error_code ec)
			{
				printf("handshake : %d %s\n", ec.value(), ec.message().c_str());
			});
			//client.async_start(host, port);
			if (client.start(host, port)) client.send(std::string("<0123456789>"));
			//client.async_start(host, port, '>');
			//client.async_start(host, port, "\r\n");
			//client.async_start(host, port, match_role);
			//client.async_start(host, port, asio::transfer_at_least(1));
			//client.async_start(host, port, asio::transfer_exactly(100));
			//client.async_start(host, port, asio2::use_dgram);
		}
		while (std::getchar() != '\n');
		//std::this_thread::sleep_for(std::chrono::seconds(2));
		for (size_t i = 0; i < count; i++)
		{
			auto & client = clients[i];
			client.stop();
		}
	}
#endif // ASIO2_USE_SSL
}

void run_https_client(std::string_view host, std::string_view port)
{
#if !defined(ASIO_STANDALONE) && defined(ASIO2_USE_SSL)
	//auto rep = asio2::http_client::execute(host, port, "/api/get_user?name=zhl");
	//std::cout << rep << std::endl << "--------------------\n";

	asio2::https_client client;
	client.set_cert_file("server.crt");
	client.connect_timeout(std::chrono::seconds(10));
	client.bind_recv([&](http::response<http::string_body>& rep)
	{
		std::cout << std::endl << "--------------------\n" << rep << std::endl;
	}).bind_connect([&](asio::error_code ec)
	{
		if (asio2::get_last_error())
			printf("connect failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
			printf("connect success : %s %u\n", client.local_address().c_str(), client.local_port());

		auto * msg = "GET / HTTP/1.1\r\n\r\n";
		client.send(msg);
	}).bind_disconnect([](asio::error_code ec)
	{
		printf("disconnect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	}).bind_handshake([&](asio::error_code ec)
	{
		printf("handshake : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	});
	client.start(host, port, "/api/get_user?name=zhl");
	while (std::getchar() != '\n');
	client.stop();
#endif // ASIO2_USE_SSL
}

void run_wss_client(std::string_view host, std::string_view port)
{
#if !defined(ASIO_STANDALONE) && defined(ASIO2_USE_SSL)
	//while (1)
	{
		asio2::wss_client client;
		client.connect_timeout(std::chrono::seconds(300));
		client.set_cert_file("server.crt");
		client.bind_recv([&](std::string_view s)
		{
			printf("recv : %u %.*s\n", (unsigned)s.size(), (int)s.size(), s.data());
			client.send(s);
		}).bind_connect([&](asio::error_code ec)
		{
			if (asio2::get_last_error())
				printf("connect failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
			else
				printf("connect success : %s %u\n", client.local_address().c_str(), client.local_port());

			std::string s;
			s += '<';
			int len = 128 + std::rand() % (300);
			for (int i = 0; i < len; i++)
			{
				s += (char)((std::rand() % 26) + 'a');
			}
			s += '>';

			client.send(std::move(s), [](std::size_t bytes_sent) {});
		}).bind_disconnect([](asio::error_code ec)
		{
			printf("disconnect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		}).bind_handshake([&](asio::error_code ec)
		{
			printf("handshake : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		}).bind_upgrade([&](asio::error_code ec)
		{
			std::cout << "upgrade " << (ec ? "failure : " : "success : ") << ec.value() << " "
				<< ec.message() << std::endl << client.upgrade_response() << std::endl;
		});
		client.async_start(host, port);
		while (std::getchar() != '\n');
		//std::this_thread::sleep_for(std::chrono::seconds(2));
		//client.stop();
	}
#endif // ASIO2_USE_SSL
}

class ping_test
{
	asio2::ping ping;
public:
	ping_test() : ping(10)
	{
		ping.timeout(std::chrono::seconds(3));
		ping.interval(std::chrono::seconds(1));
		ping.body("0123456789abcdefghijklmnopqrstovuxyz");
		ping.bind_recv(&ping_test::on_recv, this)
			.bind_start(std::bind(&ping_test::on_start, this, std::placeholders::_1))
			.bind_stop([this](asio::error_code ec) { this->on_stop(ec); });
	}
	void on_recv(asio2::icmp_rep& rep)
	{
		if (rep.lag.count() == -1)
			std::cout << "request timed out" << std::endl;
		else
			std::cout << rep.total_length() - rep.header_length()
			<< " bytes from " << rep.source_address()
			<< ": icmp_seq=" << rep.sequence_number()
			<< ", ttl=" << rep.time_to_live()
			<< ", time=" << std::chrono::duration_cast<std::chrono::milliseconds>(rep.lag).count() << "ms"
			<< std::endl;
	}
	void on_start(asio::error_code ec)
	{
		printf("start : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	}
	void on_stop(asio::error_code ec)
	{
		printf("stop : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	}
	void run()
	{
		if (!ping.start("127.0.0.1"))
			//if (!ping.start("123.45.67.89"))
			//if (!ping.start("stackoverflow.com"))
			printf("start failure : %s\n", asio2::last_error_msg().c_str());
		while (std::getchar() != '\n');
		ping.stop();
		printf("loss rate : %.0lf%% average time : %lldms\n", ping.plp(),
			std::chrono::duration_cast<std::chrono::milliseconds>(ping.avg_lag()).count());
	}
};

void run_ping_test()
{
	ping_test ping;
	ping.run();
}

void run_serial_port(const std::string& device, unsigned int baud_rate)
{
	//while (1) // use infinite loop and sleep 2 seconds to test start and stop
	{
		asio2::scp sp;
		sp.start_timer(1, std::chrono::seconds(1), []() {});
		sp.bind_init([&]()
		{
			// Set other serial port parameters at here
			sp.socket().set_option(asio::serial_port::flow_control(asio::serial_port::flow_control::type::none));
			sp.socket().set_option(asio::serial_port::parity(asio::serial_port::parity::type::none));
			sp.socket().set_option(asio::serial_port::stop_bits(asio::serial_port::stop_bits::type::one));
			sp.socket().set_option(asio::serial_port::character_size(8));

		}).bind_recv([&](std::string_view sv)
		{
			printf("recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());

			std::string s;
			uint8_t len = uint8_t(10 + (std::rand() % 20));
			s += '<';
			for (uint8_t i = 0; i < len; i++)
			{
				s += (char)((std::rand() % 26) + 'a');
			}
			s += '>';
			sp.send(s, []() {});

		}).bind_start([&](asio::error_code ec)
		{
			printf("start : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		}).bind_stop([&](asio::error_code ec)
		{
			printf("stop : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		});
		//sp.start(device, baud_rate);
		sp.start(device, baud_rate, '>');
		//sp.start(device, baud_rate, "\r\n");
		//sp.start(device, baud_rate, match_role);
		//sp.start(device, baud_rate, asio::transfer_at_least(1));
		//sp.start(device, baud_rate, asio::transfer_exactly(10));
		sp.send("abc", [](std::size_t bytes_sent) {
			printf("%llu %d %s\n", bytes_sent, asio2::last_error_val(), asio2::last_error_msg().c_str()); });

		while (std::getchar() != '\n');
		//std::this_thread::sleep_for(std::chrono::seconds(2));

	}
}

void run_udp_cast(std::string_view host, std::string_view port)
{
	//while (1)
	{
		host = "0.0.0.0"; port = "8088";
		printf("\n");
		asio2::udp_cast sender;
		sender.bind_recv([&](asio::ip::udp::endpoint& endpoint, std::string_view sv)
		{
			printf("recv : %s %u %u %.*s\n", endpoint.address().to_string().c_str(),
				endpoint.port(), (unsigned)sv.size(), (int)sv.size(), sv.data());
		}).bind_start([&](asio::error_code ec)
		{
			printf("start : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		}).bind_stop([&](asio::error_code ec)
		{
			printf("stop : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		}).bind_init([&]()
		{
			//// Join the multicast group.
			//sender.socket().set_option(
			//	asio::ip::multicast::join_group(asio::ip::make_address("239.255.0.1")));
		});
		sender.start(host, port);

		std::string s("<0123456789abcdefghijklmnopqrstowvxyz>");
		asio::ip::udp::endpoint ep1(asio::ip::make_address("::FFFF:127.0.0.1"), 18080); // ipv6 address
		sender.send(ep1, s, [](std::size_t bytes_sent)
		{
			printf("send : %llu %s\n", bytes_sent, asio2::last_error_msg().c_str());
		});
		sender.send("239.255.0.1", "8080", s, []() // this is a multicast address
		{
			printf("send : %s\n", asio2::last_error_msg().c_str());
		});

		std::string shost("239.255.0.1"), sport("8080");

		int narys[2] = { 1,2 };
		sender.send(shost, sport, s);
		sender.send(shost, sport, s, []() {});
		sender.send(shost, sport, (uint8_t*)"<abcdefghijklmnopqrstovuxyz0123456789>", 10);
		sender.send(shost, sport, (const uint8_t*)"<abcdefghijklmnopqrstovuxyz0123456789>", 10);
		sender.send(shost, sport, s.data(), 5);
		sender.send(shost, sport, s.data(), []() {});
		sender.send(shost, sport, s.c_str(), size_t(5));
		sender.send(shost, sport, s, asio::use_future);
		sender.send(shost, sport, "<abcdefghijklmnopqrstovuxyz0123456789>", asio::use_future);
		sender.send(shost, sport, s.data(), asio::use_future);
		sender.send(shost, sport, s.c_str(), asio::use_future);
		sender.send(shost, sport, narys);
		sender.send(shost, sport, narys, []() {});
		sender.send(shost, sport, narys, [](std::size_t bytes) {});
		sender.send(shost, sport, narys, asio::use_future);

		sender.send(ep1, s);
		sender.send(ep1, s, []() {});
		sender.send(ep1, (uint8_t*)"<abcdefghijklmnopqrstovuxyz0123456789>", 10);
		sender.send(ep1, (const uint8_t*)"<abcdefghijklmnopqrstovuxyz0123456789>", 10);
		sender.send(ep1, s.data(), 5);
		sender.send(ep1, s.data(), []() {});
		sender.send(ep1, s.c_str(), size_t(5));
		sender.send(ep1, s, asio::use_future);
		sender.send(ep1, "<abcdefghijklmnopqrstovuxyz0123456789>", asio::use_future);
		sender.send(ep1, s.data(), asio::use_future);
		sender.send(ep1, s.c_str(), asio::use_future);
		sender.send(ep1, narys);
		sender.send(ep1, narys, []() {});
		sender.send(ep1, narys, [](std::size_t bytes) {});
		sender.send(ep1, narys, asio::use_future);

		// the resolve function is a time-consuming operation
		//asio::ip::udp::resolver resolver(sender.io().context());
		//asio::ip::udp::resolver::query query("www.baidu.com", "18080");
		//asio::ip::udp::endpoint ep2 = *resolver.resolve(query);
		//sender.send(ep2, std::move(s));

		while (std::getchar() != '\n');
		//std::this_thread::sleep_for(std::chrono::seconds(2));

		sender.stop();
	}
}

class user
{
public:
	std::string name;
	int age;
	std::map<int, std::string> purview;

	template <class Archive>
	void serialize(Archive & ar)
	{
		ar(name);
		ar(age);
		ar(purview);
	}
};

void run_rpc_client(std::string_view host, std::string_view port)
{
	//while (1) // use infinite loop and sleep 2 seconds to test start and stop
	{
		int count = 1;
		std::unique_ptr<asio2::rpc_client[]> clients = std::make_unique<asio2::rpc_client[]>(count);
		for (int i = 0; i < count; ++i)
		{
			auto & client = clients[i];
			client.start_timer(1, std::chrono::seconds(1), []() {});
			client.timeout(std::chrono::seconds(3));
			client.bind_connect([&](asio::error_code ec)
			{
				printf("connect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
				client.async_call<int>([](asio::error_code ec, auto v)
				{
					printf("sum : %d err : %d %s\n", v, ec.value(), ec.message().c_str());
				}, "add", 120, 11);
			}).bind_disconnect([](asio::error_code ec)
			{
				printf("disconnect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
			}).bind_recv([&](std::string_view sv)
			{
			}).bind_send([&](std::string_view sv)
			{
			});

			client.bind("sub", [](int a, int b) { return a - b; });

			// Using tcp dgram mode as the underlying communication support(This is the default setting)
			// Then must use "use_dgram" parameter.
			client.start(host, port, asio2::use_dgram);

			// Using websocket as the underlying communication support.
			// Need to goto the tail of the (rpc_client.hpp rpc_server.hpp rpc_session.hpp) files,
			// and modified to use websocket.
			//client.start(host, port);

			//for (;;)
			{
				asio::error_code ec;
				int sum = client.call<int>(ec, std::chrono::seconds(3), "add", 11, 2);
				printf("sum : %d err : %d %s\n", sum, ec.value(), ec.message().c_str());

				client.async_call([](asio::error_code ec, int v)
				{
					printf("sum : %d err : %d %s\n", v, ec.value(), ec.message().c_str());
				}, "add", 10, 20);

				client.async_call<int>([](asio::error_code ec, auto v)
				{
					printf("sum : %d err : %d %s\n", v, ec.value(), ec.message().c_str());
				}, "add", 12, 21);

				try
				{
					double mul = client.call<double>("mul", 2.5, 2.5);
					printf("mul : %lf err : %d %s\n", mul, ec.value(), ec.message().c_str());
				}
				catch (asio2::system_error& e) { printf("mul : %d %s\n", e.code().value(), e.code().message().c_str()); }

				client.async_call([](asio::error_code ec, std::string v)
				{
					printf("cat : %s err : %d %s\n", v.c_str(), ec.value(), ec.message().c_str());
				}, "cat", std::string("abc"), std::string("123"));

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
				client.async_call([](asio::error_code ec)
				{
				}, "del_user", std::move(u));

				//std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		}

		while (std::getchar() != '\n');
		//std::this_thread::sleep_for(std::chrono::seconds(2));

		for (int i = 0; i < count; ++i)
		{
			auto & client = clients[i];
			client.stop();
		}
	}
}

int main(int argc, char *argv[])
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system,linux has't these function
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	// the number is the memory leak line num of the vs output window content.
	//_CrtSetBreakAlloc(1640);
#endif

	//std::srand(static_cast<unsigned int>(std::time(nullptr)));

	//auto t1 = std::chrono::steady_clock::now();
	//auto t2 = std::chrono::steady_clock::now();
	//auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
	//printf("%lld %llu\n", ms, sum);

	std::string_view host = "127.0.0.1", port = "8080";
	//std::string_view host = "192.168.1.146", port = "8080"; 
	//port = argv[1];
	run_tcp_client(host, port);
	//run_tcp_client_group(host, port);
	//run_udp_client(host, port);
	//run_http_client(host, port);
	//run_ws_client(host, port);
	//run_tcps_client(host, port);
	//run_https_client(host, port);
	//run_wss_client(host, port);
	//run_ping_test();
	//run_serial_port("COM1", 9600);
	//run_serial_port("/dev/ttyS0", 9600);
	//run_udp_cast(host, port);
	//run_rpc_client(host, port);

#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	//system("pause");
#endif

	return 0;
};
