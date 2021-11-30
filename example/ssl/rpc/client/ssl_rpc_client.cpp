#ifndef ASIO2_USE_SSL
#define ASIO2_USE_SSL
#endif

#include <asio2/rpc/rpc_client.hpp>

class userinfo
{
public:
	std::string name;
	int age;
	std::map<int, std::string> purview;

	// User defined object types require serialized the members like this:
	template <class Archive>
	void serialize(Archive & ar)
	{
		ar(name);
		ar(age);
		ar(purview);
	}
};

std::function<void()> sender;

int main()
{
	std::string_view host = "127.0.0.1";
	std::string_view port = "8011";

	asio2::rpcs_client client;

	sender = [&]()
	{
		client.async_call([&](asio::error_code ec, std::string v)
		{
			printf("cat : %s err : %d %s\n", v.c_str(), ec.value(), ec.message().c_str());

			if (!ec)
			{
				ASIO2_ASSERT(v == "abc123xxx");
				sender();
			}

		}, "cat", std::string("abc"), std::string("123xxx"));
	};

	// set default rpc call timeout
	client.default_timeout(std::chrono::seconds(3));

	//------------------------------------------------------------------------------
	// beacuse the server did not specify the verify_fail_if_no_peer_cert flag, so
	// our client may not load the ssl certificate.
	//------------------------------------------------------------------------------
	//client.set_verify_mode(asio::ssl::verify_peer);
	//client.set_cert_file("../../cert/ca.crt", "../../cert/client.crt", "../../cert/client.key", "123456");

	client.bind_connect([&](asio::error_code ec)
	{
		asio2::detail::ignore_unused(ec);

		// the type of the callback's second parameter is auto, so you have to specify 
		// the return type in the template function like 'async_call<int>'
		client.async_call<int>([](asio::error_code ec, auto v)
		{
			if (!ec)
			{
				ASIO2_ASSERT(v == 44 + 11);
			}
			printf("sum1 : %d err : %d %s\n", v, ec.value(), ec.message().c_str());
		}, "add", 44, 11);

		sender();

	}).bind_disconnect([](asio::error_code ec)
	{
		printf("disconnect : %d %s\n", ec.value(), ec.message().c_str());
	});

	client.bind("sub", [](int a, int b) { return a - b; });

	client.async_start(host, port);

	client.start_timer("123", std::chrono::seconds(1), [&]()
	{
		client.async_call([](asio::error_code ec, std::string v)
		{
			if (!ec)
			{
				ASIO2_ASSERT(v == "abc123");
			}
			printf("cat : %s err : %d %s\n", v.c_str(), ec.value(), ec.message().c_str());

		}, "cat", std::string("abc"), std::string("123"));
	});

	while (std::getchar() != '\n');

	return 0;
}
