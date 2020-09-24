// When compiling with vs under linux, you need to copy the "asio,beast,ceral,fmt" folders to
// the /usr/local/include directory first, and copy the "libcrypto.a,libssl.a" files to 
// /usr/local/lib directory first. "libcrypto.a,libssl.a" is in "asio2/lib/x64".

#ifndef ASIO2_USE_SSL
#define ASIO2_USE_SSL
#endif

#include <asio2/asio2.hpp>

class user
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

	//bool loop = false;
	bool loop = true;
	while (loop) // use infinite loop and sleep 2 seconds to test start and stop
	{
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
		//client.set_cert_file("../../../cert/ca.crt", "../../../cert/client.crt", "../../../cert/client.key", "client");

		client.bind_connect([&](asio::error_code ec)
		{
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
			printf("disconnect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
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

		if (!loop)
			while (std::getchar() != '\n');
		else
			std::this_thread::sleep_for(std::chrono::seconds(2));
	}

	return 0;
}
