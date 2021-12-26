#include <asio2/3rd/asio.hpp>
#include <iostream>

struct userinfo
{
	int id;
	char name[20];
	int8_t age;
};

namespace asio
{
	inline asio::const_buffer buffer(const userinfo& u) noexcept
	{
		return asio::const_buffer(&u, sizeof(userinfo));
	}
}

#include <asio2/tcp/tcp_client.hpp>

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



int main()
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	std::string_view host = "127.0.0.1";
	std::string_view port = "8028";

	asio2::tcp_client clients[20];

	for (int i = 0; i < 20; i++)
	{
		//listener listen;

		auto& client = clients[i];

		//// default reconnect option is "enable"
		//client.auto_reconnect(false); // disable auto reconnect
		//client.auto_reconnect(true); // enable auto reconnect and use the default delay
		client.auto_reconnect(true, std::chrono::milliseconds(1000)); // enable auto reconnect and use custom delay

		client.start_timer(1, std::chrono::seconds(1), []() {}); // test timer
		client.post([]() {}, std::chrono::seconds(3));

		client.bind_connect([&](asio::error_code ec)
		{
			asio2::detail::ignore_unused(ec);

			if (asio2::get_last_error())
				printf("connect failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
			else
				printf("connect success : %s %u\n", client.local_address().c_str(), client.local_port());

			const char * p1 = "1abcdefghijklmnopqrxtovwxyz";
			char buf[50] = "2abcdefghijklmnopqrxtovwxyz";
			char * p2 = buf;

			client.async_send(p1);
			client.async_send(buf);
			client.async_send(p2);
			client.async_send("3abcdefghijklmnopqrxtovwxyz");

			// call send in the communication thread, it will degenerates into async_send
			// and the return value is 0(success) or -1(failure).
			if (!ec)
			{
				std::size_t sent_bytes;

				sent_bytes = client.send(p1);
				ASIO2_ASSERT(asio2::get_last_error() == asio::error::in_progress && sent_bytes == 0);

				sent_bytes = client.send(buf);
				ASIO2_ASSERT(asio2::get_last_error() == asio::error::in_progress && sent_bytes == 0);

				sent_bytes = client.send(p2);
				ASIO2_ASSERT(asio2::get_last_error() == asio::error::in_progress && sent_bytes == 0);

				sent_bytes = client.send("3abcdefghijklmnopqrxtovwxyz");
				ASIO2_ASSERT(asio2::get_last_error() == asio::error::in_progress && sent_bytes == 0);
			}
			else
			{
				std::size_t sent_bytes;

				sent_bytes = client.send(p1);
				ASIO2_ASSERT(asio2::get_last_error() == asio::error::not_connected && sent_bytes == (std::size_t)-1);

				sent_bytes = client.send(buf);
				ASIO2_ASSERT(asio2::get_last_error() == asio::error::not_connected && sent_bytes == (std::size_t)-1);

				sent_bytes = client.send(p2);
				ASIO2_ASSERT(asio2::get_last_error() == asio::error::not_connected && sent_bytes == (std::size_t)-1);

				sent_bytes = client.send("3abcdefghijklmnopqrxtovwxyz");
				ASIO2_ASSERT(asio2::get_last_error() == asio::error::not_connected && sent_bytes == (std::size_t)-1);
			}

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

			// ## All of the following ways of async_send operation are correct.
			client.async_send(s);
			client.async_send(s, []() {});
			client.async_send((uint8_t*)"<abcdefghijklmnopqrstovuxyz0123456789>", 10);
			client.async_send((const uint8_t*)"<abcdefghijklmnopqrstovuxyz0123456789>", 10);
			client.async_send(s.data(), int(s.size()));
			client.async_send(s.data(), []() {});
			client.async_send(s.c_str(), size_t(s.size()));
			client.async_send(std::move(s));
			int narys[2] = { 1,2 };
			client.async_send(narys);
			client.async_send(narys, []() {std::cout << asio2::last_error_msg() << std::endl; }); // callback with no params
			client.async_send(narys, [](std::size_t bytes) {std::ignore = bytes; }); // callback with param

		}).bind_disconnect([&](asio::error_code ec)
		{
			printf("disconnect : %d %s\n", ec.value(), ec.message().c_str());
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
			client.async_send(std::move(s), []() {});

		})
			//.bind_recv(on_recv)//bind global function
			//.bind_recv(std::bind(&listener::on_recv, &listen, std::placeholders::_1))//bind member function
			//.bind_recv(&listener::on_recv, listen)//bind member function
			//.bind_recv(&listener::on_recv, &listen)//bind member function
			;

		//client.async_start(host, port);
		client.start(host, port);

		// ##Use this to check whether the code is running in the io strand thread.
		//if (client.io().strand().running_in_this_thread())
		//{
		//}

		// ## All of the following ways of async_send operation are correct.
		std::string s;
		s += '#';
		s += char(1);
		s += 'a';

		if (client.is_started())
		{
			client.async_send(s);
			client.async_send(s, []() {});
			client.async_send((uint8_t*)"<abcdefghijklmnopqrstovuxyz0123456789>", 10);
			client.async_send((const uint8_t*)"<abcdefghijklmnopqrstovuxyz0123456789>", 10);
			client.async_send(s.data(), int(s.size()));
			client.async_send(s.data(), []() {});
			client.async_send(s.c_str(), size_t(s.size()));
			client.async_send(s, asio::use_future);
			client.async_send("<abcdefghijklmnopqrstovuxyz0123456789>", asio::use_future);
			client.async_send(s.data(), asio::use_future);
			client.async_send(s.c_str(), asio::use_future);
			client.async_send(std::move(s));
			int narys[2] = { 1,2 };
			client.async_send(narys);
			client.async_send(narys, []() {});
			client.async_send(narys, [](std::size_t bytes) {std::ignore = bytes; });
			auto future = client.async_send(narys, asio::use_future);
			auto[ec, bytes] = future.get();
			printf("sent result : %s %d\n", ec.message().c_str(), int(bytes));


			// "async_send" use asio::buffer to avoid memory allocation, the underlying
			// buffer must be persistent, like the static pointer "msg" below
			const char * msg = "<abcdefghijklmnopqrstovuxyz0123456789>";
			asio::const_buffer buffer = asio::buffer(msg);
			client.async_send(buffer);

			// Example for Synchronous send data. The return value is the sent bytes.
			// You can use asio2::get_last_error() to check whether some error occured.
			std::size_t sent_bytes = client.send(s); std::ignore = sent_bytes;
			sent_bytes = client.send((uint8_t*)"<abcdefghijklmnopqrstovuxyz0123456789>", 10);
			sent_bytes = client.send((const uint8_t*)"<abcdefghijklmnopqrstovuxyz0123456789>", 10);
			sent_bytes = client.send(s.data(), int(s.size()));
			sent_bytes = client.send(s.c_str(), size_t(s.size()));
			sent_bytes = client.send(std::move(s));
			sent_bytes = client.send(narys);
			if (asio2::get_last_error())
			{
				printf("send data failed : %s\n", asio2::last_error_msg().data());
			}

			const char * p1 = "4abcdefghijklmnopqrxtovwxyz";
			char buf[50] = "5abcdefghijklmnopqrxtovwxyz";
			char * p2 = buf;

			sent_bytes = client.send(p1);
			sent_bytes = client.send(buf);
			sent_bytes = client.send(p2);
			sent_bytes = client.send("6abcdefghijklmnopqrxtovwxyz");

			// "send" use asio::buffer to avoid memory allocation, the underlying
			// buffer must be persistent, like the static pointer "msg" below
			client.send(buffer);

			//// ##Example how to send a struct directly:
			userinfo u;
			u.id = 11;
			memset(u.name, 0, sizeof(u.name));
			memcpy(u.name, "abc", 3);
			u.age = 20;
			client.async_send(u);
		}

		s += '#';

		// 
		client.send(s);
		client.send(std::move(s));
		client.send("abc");
	}
	while (std::getchar() != '\n');
		
	return 0;
}
