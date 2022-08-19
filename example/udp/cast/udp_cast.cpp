#include <asio2/udp/udp_cast.hpp>

int main()
{
	std::string_view host = "0.0.0.0";
	std::string_view port = "8022";

	asio2::udp_cast udp_cast;

	udp_cast.bind_recv([&](asio::ip::udp::endpoint& endpoint, std::string_view data)
	{
		printf("recv : %s %u %zu %.*s\n",
			endpoint.address().to_string().c_str(), endpoint.port(),
			data.size(), (int)data.size(), data.data());

		udp_cast.async_send(endpoint, data);
	}).bind_start([&]()
	{
		printf("start : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	}).bind_stop([&]()
	{
		printf("stop : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	}).bind_init([&]()
	{
		//// Join the multicast group.
		//udp_cast.socket().set_option(
		//	asio::ip::multicast::join_group(asio::ip::make_address("239.255.0.1")));
	});

	udp_cast.start(host, port);

	std::string str("<0123456789abcdefghijklmnopqrstowvxyz>");

	// send to self
	udp_cast.send("127.0.0.1", 8022, str);

	asio::ip::udp::endpoint ep(asio::ip::make_address("127.0.0.1"), 8020);
	udp_cast.async_send(ep, str);

	// this is a multicast address
	udp_cast.async_send("239.255.0.1", "8030", str);

	// the resolve function is a time-consuming operation
	//asio::ip::udp::resolver resolver(udp_cast.io().context());
	//asio::ip::udp::resolver::query query("www.baidu.com", "18080");
	//asio::ip::udp::endpoint ep2 = *resolver.resolve(query);
	//udp_cast.async_send(ep2, std::move(str));


	// stop the udp cast after 10 seconds. 
	udp_cast.start_timer(1, 10000, 1, [&]()
	{
		// note : the stop is called in the io_context thread is ok.
		// of course you can call the udp_cast.stop() function anywhere.
		udp_cast.stop();
	});

	// the udp_cast.wait_stop() will be blocked forever until the udp_cast.stop() is called.
	udp_cast.wait_stop();

	// Or you can call the wait_for function directly to block for 10 seconds
	//udp_cast.wait_for(std::chrono::seconds(10));

	return 0;
}
