// When compiling with vs under linux, you need to copy the "asio,beast,ceral,fmt" folders to
// the /usr/local/include directory first, and copy the "libcrypto.a,libssl.a" files to 
// /usr/local/lib directory first. "libcrypto.a,libssl.a" is in "asio2/lib/x64".

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

namespace asio
{
	inline asio::const_buffer buffer(const info& u) noexcept
	{
		return asio::const_buffer(&u, sizeof(info));
	}
}

int main()
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	std::string_view host = "127.0.0.1";
	std::string_view port = "8028";

	asio2::tcp_client client;

	//listener listen;

	//// == default reconnect option is "enable" ==
	//client.auto_reconnect(false); // disable auto reconnect
	//client.auto_reconnect(true); // enable auto reconnect and use the default delay
	client.auto_reconnect(true, std::chrono::milliseconds(1000)); // enable auto reconnect and use custom delay

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
		client.send(std::move(s));
		int narys[2] = { 1,2 };
		client.send(narys);
		client.send(narys, []() {std::cout << asio2::last_error_msg() << std::endl; }); // callback with no params
		client.send(narys, [](std::size_t bytes) {}); // callback with param

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
		//.bind_recv(std::bind(&listener::on_recv, &listen, std::placeholders::_1))//bind member function
		//.bind_recv(&listener::on_recv, listen)//bind member function
		//.bind_recv(&listener::on_recv, &listen)//bind member function
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
		client.send(std::move(s));
		int narys[2] = { 1,2 };
		client.send(narys);
		client.send(narys, []() {});
		client.send(narys, [](std::size_t bytes) {});
		auto future = client.send(narys, asio::use_future);
		auto[ec, bytes] = future.get();
		printf("sent result : %s %d\n", ec.message().c_str(), int(bytes));


		// "send" use asio::buffer to avoid memory allocation, the underlying
		// buffer must be persistent, like the static pointer "msg" below
		const char * msg = "<abcdefghijklmnopqrstovuxyz0123456789>";
		asio::const_buffer buffer = asio::buffer(std::string_view{ msg });
		client.send(buffer);


		//// ##Example how to send a struct directly:
		//info u;
		//client.send(u);

		// ##Thread-safe send operation example :
		//client.post([&]()
		//{
		//	if (client.is_started())
		//		asio::write(client.stream(), asio::buffer(std::string("abcdefghijklmn")));
		//});
	}

	while (std::getchar() != '\n');
		
	return 0;
}
