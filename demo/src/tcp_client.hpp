#pragma once

#include <asio2/asio2.hpp>

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

struct info
{
	int id;
	char name[20];
	int8_t age;
};

#ifdef ASIO_STANDALONE
namespace asio {
#else
namespace boost::asio {
#endif
	inline asio::const_buffer buffer(const info& u) noexcept
	{
		return asio::const_buffer(&u, sizeof(info));
	}
} // namespace asio

void run_tcp_client(std::string_view host, std::string_view port)
{
	int count = 1;
	std::unique_ptr<asio2::tcp_client[]> clients = std::make_unique<asio2::tcp_client[]>(count);
	//while (1) // use infinite loop and sleep 2 seconds to test start and stop
	//for (int i = 0; i < 1000; i++)
	{
		//listener lis;
		for (int i = 0; i < count; ++i)
		{
			auto & client = clients[i];
			//// == default reconnect option is "enable" ==
			//client.reconnect(false); // disable auto reconnect
			//client.reconnect(true); // enable auto reconnect and use the default delay
			client.reconnect(true, std::chrono::milliseconds(100)); // enable auto reconnect and use custom delay
			client.start_timer(1, std::chrono::seconds(1), []() {}); // test timer
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
				client.send(narys, []() {std::cout << asio2::last_error_msg() << std::endl; }); // callback with no params
				client.send(narys, [](std::size_t bytes) {}); // callback with param
				client.send(narys, asio::use_future);

				//asio::write(client.socket(), asio::buffer(s));
			}).bind_disconnect([&](asio::error_code ec)
			{
				printf("disconnect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
			}).bind_recv([&](std::string_view sv)
			{
				printf("recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());

				std::string s;
				s += '#';
				uint8_t len = uint8_t(100 + (std::rand() % 100));
				s += char(len);
				for (uint8_t i = 0; i < len; i++)
				{
					s += (char)((std::rand() % 26) + 'a');
				}
				client.send(std::move(s), []() {});

				//asio::write(client.socket(), asio::buffer(sv));
			})
				//.bind_recv(on_recv)//bind global function
				//.bind_recv(std::bind(&listener::on_recv, &lis, std::placeholders::_1))//bind member function
				//.bind_recv(&listener::on_recv, lis)//bind member function
				//.bind_recv(&listener::on_recv, &lis)//bind member function
				;
			//client.async_start(host, port);
			client.start(host, port);

			// ##Use this to check whether the send operation is running in current thread.
			//if (client.io().strand().running_in_this_thread())
			//{
			//}

			// ## All of the following ways of send operation are correct.
			// (beacuse these send operations is not running in the io.strand thread,
			// then the content will be sent asynchronous)
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

				//// ##Example how to send a struct directly:
				//info u;
				//client.send(u);

				// ##Thread-safe send operation example :
				//client.post([&client]()
				//{
				//	if (client.is_started())
				//		asio::write(client.stream(), asio::buffer(std::string("abcdefghijklmn")));
				//});
			}
		}

		while (std::getchar() != '\n');
		//std::this_thread::sleep_for(std::chrono::milliseconds(500));
		
		for (int i = 0; i < count; ++i)
		{
			auto & client = clients[i];
			client.stop();
		}
	}
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
	for (int i = 0; i < 1000; i++)
	{
		size_t count = 1;
		std::unique_ptr<asio2::tcps_client[]> clients = std::make_unique<asio2::tcps_client[]>(count);
		for (size_t i = 0; i < count; i++)
		{
			auto & client = clients[i];
			client.set_cert(cer);
			client.connect_timeout(std::chrono::seconds(10));
			client.reconnect(true, std::chrono::milliseconds(100));
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

				client.send(std::move(s));

			}).bind_disconnect([](asio::error_code ec)
			{
				printf("disconnect : %d %s\n", ec.value(), ec.message().c_str());
			}).bind_recv([&](std::string_view sv)
			{
				//printf("recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());

				client.send(sv);
			}).bind_handshake([&](asio::error_code ec)
			{
				printf("handshake : %d %s\n", ec.value(), ec.message().c_str());
			});
			if (client.start(host, port))
				client.send(std::string("<0123456789>"));
			//client.async_start(host, port);
			//client.async_start(host, port, '>');
			//client.async_start(host, port, "\r\n");
			//client.async_start(host, port, asio::transfer_at_least(1));
			//client.async_start(host, port, asio::transfer_exactly(100));
			//client.async_start(host, port, asio2::use_dgram);
		}
		//while (std::getchar() != '\n');
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		for (size_t i = 0; i < count; i++)
		{
			auto & client = clients[i];
			client.stop();
		}
	}
#endif // ASIO2_USE_SSL
}
