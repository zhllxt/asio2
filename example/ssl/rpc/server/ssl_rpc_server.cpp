#ifndef ASIO2_ENABLE_SSL
#define ASIO2_ENABLE_SSL
#endif

#include <asio2/rpc/rpcs_server.hpp>
#include <iostream>

int add(int a, int b)
{
	return a + b;
}

int main()
{
	std::string_view host = "0.0.0.0";
	std::string_view port = "8011";

	asio2::rpcs_server server;

	// use file for cert
	server.set_cert_file(
		"../../example/cert/ca.crt",
		"../../example/cert/server.crt",
		"../../example/cert/server.key",
		"123456");

	if (asio2::get_last_error())
		std::cout << "load cert files failed: " << asio2::last_error_msg() << std::endl;

	server.bind_connect([&](auto & session_ptr)
	{
		printf("client enter : %s %u %s %u\n",
			session_ptr->remote_address().c_str(), session_ptr->remote_port(),
			session_ptr->local_address().c_str(), session_ptr->local_port());

		session_ptr->async_call([](int v)
		{
			if (!asio2::get_last_error())
			{
				ASIO2_ASSERT(v == 15 - 6);
			}
			printf("sub : %d - %d %s\n", v,
				asio2::last_error_val(), asio2::last_error_msg().c_str());
		}, std::chrono::seconds(10), "sub", 15, 6);

	}).bind_start([&]()
	{
		printf("start rpcs server : %s %u %d %s\n",
			server.listen_address().c_str(), server.listen_port(),
			asio2::last_error_val(), asio2::last_error_msg().c_str());
	});

	server.bind("add", add);

	server.bind("cat", [&](std::shared_ptr<asio2::rpcs_session>& session_ptr,
		const std::string& a, const std::string& b)
	{
		int x = std::rand(), y = std::rand();
		// Nested call rpc function in business function is ok.
		session_ptr->async_call([x, y](int v)
		{
			asio2::ignore_unused(x, y);
			if (!asio2::get_last_error())
			{
				ASIO2_ASSERT(v == x - y);
			}
			printf("sub : %d - %d %s\n", v,
				asio2::last_error_val(), asio2::last_error_msg().c_str());
		}, "sub", x, y);

		return a + b;
	});

	server.start(host, port);

	while (std::getchar() != '\n');

	server.stop();

	return 0;
}
