#include <asio2/external/asio.hpp>

struct userinfo
{
	int id;
	char name[20];
	int8_t age;
};

#ifdef ASIO_STANDALONE
namespace asio
#else
namespace boost::asio
#endif
{
	inline asio::const_buffer buffer(const userinfo& u) noexcept
	{
		return asio::const_buffer(&u, sizeof(userinfo));
	}
}

#include <asio2/tcp/tcp_client.hpp>

int main()
{
	std::string_view host = "127.0.0.1";
	//std::string_view host = "fe80::dc05:d962:f568:39b7"; // ipv6 windows
	//std::string_view host = "fe80::bb00:5a10:d713:d293%eth0"; // ipv6 linux
	std::string_view port = "8028";

	asio2::tcp_client client;

	// disable auto reconnect, default reconnect option is "enable"
	//client.set_auto_reconnect(false);

	// enable auto reconnect and use custom delay, default delay is 1 seconds
	client.set_auto_reconnect(true, std::chrono::milliseconds(2000));

	client.bind_connect([&]()
	{
		if (asio2::get_last_error())
			printf("connect failure : %d %s\n",
				asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
			printf("connect success : %s %u\n",
				client.local_address().c_str(), client.local_port());

		// has no error, it means connect success, we can send data at here
		if (!asio2::get_last_error())
		{
			client.async_send("<abcdefghijklmnopqrstovuxyz0123456789>");
		}
	}).bind_disconnect([&]()
	{
		printf("disconnect : %d %s\n",
			asio2::last_error_val(), asio2::last_error_msg().c_str());
	}).bind_recv([&](std::string_view data)
	{
		printf("recv : %zu %.*s\n", data.size(), (int)data.size(), data.data());

		client.async_send(data);
	});

	// Asynchronous connect to the server
	//client.async_start(host, port);

	// Synchronously connect to the server
	client.start(host, port);

	std::string str = "[abcdefghijklmnopqrstovuxyz0123456789]";

	if (client.is_started())
	{
		// ## All of the following ways of send operation are correct.
		client.async_send(str);

		client.async_send(str.data(), str.size() / 2);

		int intarray[2] = { 1, 2 };

		// callback with no params
		client.async_send(intarray, []()
		{
			if (asio2::get_last_error())
				printf("send failed : %s\n", asio2::last_error_msg().data());
		});

		// callback with param
		client.async_send(intarray, [](std::size_t sent_bytes)
		{
			std::ignore = sent_bytes;
		});

		// use future to wait util the send is finished.
		std::future<std::pair<asio::error_code, std::size_t>> future =
			client.async_send(str, asio::use_future);
		auto[ec, bytes] = future.get();
		printf("sent result : %s %zu\n", ec.message().c_str(), bytes);

		// use asio::buffer to avoid memory allocation, the underlying
		// buffer must be persistent, like the static pointer "msg" below
		const char * msg = "<abcdefghijklmnopqrstovuxyz0123456789>";
		asio::const_buffer buffer = asio::buffer(msg);
		client.async_send(buffer);

		// Example for Synchronous send data. The return value is the sent bytes.
		// You can use asio2::get_last_error() to check whether some error occured.
		std::size_t sent_bytes = client.send(std::move(str));
		if (asio2::get_last_error())
		{
			printf("send data failed : %zu %s\n", sent_bytes, asio2::last_error_msg().data());
		}

		// send vector, array, .... and others
		std::vector<uint8_t> data{ 1,2,3 };
		client.send(std::move(data));

		// ##Example how to send a struct directly:
		userinfo u;
		u.id = 11;
		memset(u.name, 0, sizeof(u.name));
		memcpy(u.name, "abc", 3);
		u.age = 20;
		client.async_send(u);
	}

	while (std::getchar() != '\n');
		
	return 0;
}
