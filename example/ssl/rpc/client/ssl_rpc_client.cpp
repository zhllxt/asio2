#ifndef ASIO2_ENABLE_SSL
#define ASIO2_ENABLE_SSL
#endif

#include <asio2/rpc/rpcs_client.hpp>

int main()
{
	std::string_view host = "127.0.0.1";
	std::string_view port = "8011";

	asio2::rpcs_client client;

	// set default rpc call timeout
	client.set_default_timeout(std::chrono::seconds(3));

	//------------------------------------------------------------------------------
	// beacuse the server did not specify the verify_fail_if_no_peer_cert flag, so
	// our client may not load the ssl certificate.
	//------------------------------------------------------------------------------
	//client.set_verify_mode(asio::ssl::verify_peer);
	//client.set_cert_file(
	//	"../../example/cert/ca.crt",
	//	"../../example/cert/client.crt",
	//	"../../example/cert/client.key",
	//	"123456");

	client.bind_connect([&]()
	{
		if (asio2::get_last_error())
			return;

		// the type of the callback's second parameter is auto, so you have to specify 
		// the return type in the template function like 'async_call<int>'
		int x = std::rand(), y = std::rand();
		client.async_call<int>([x, y](auto v)
		{
			asio2::ignore_unused(x, y);
			if (!asio2::get_last_error())
			{
				ASIO2_ASSERT(v == x + y);
			}
			printf("sum1 : %d - %d %s\n", v,
				asio2::last_error_val(), asio2::last_error_msg().c_str());
		}, "add", x, y);

	}).bind_disconnect([]()
	{
		printf("disconnect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	});

	client.bind("sub", [](int a, int b) { return a - b; });

	client.async_start(host, port);

	client.start_timer("timer_id1", std::chrono::milliseconds(500), [&]()
	{
		std::string s1;
		s1 += '<';
		for (int i = 100 + std::rand() % (100); i > 0; i--)
		{
			s1 += (char)((std::rand() % 26) + 'a');
		}

		std::string s2;
		for (int i = 100 + std::rand() % (100); i > 0; i--)
		{
			s2 += (char)((std::rand() % 26) + 'a');
		}
		s2 += '>';

		client.async_call([s1, s2](std::string v)
		{
			if (!asio2::get_last_error())
			{
				ASIO2_ASSERT(v == s1 + s2);
			}
			printf("cat : %s - %d %s\n", v.c_str(),
				asio2::last_error_val(), asio2::last_error_msg().c_str());

		}, "cat", s1, s2);
	});

	while (std::getchar() != '\n');

	return 0;
}
