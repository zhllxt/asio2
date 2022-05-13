#include <asio2/udp/udp_cast.hpp>

int main()
{
	std::string_view host = "0.0.0.0";
	std::string_view port = "8022";

	asio2::udp_cast sender;

	sender.bind_recv([&](asio::ip::udp::endpoint& endpoint, std::string_view data)
	{
		printf("recv : %s %u %zu %.*s\n",
			endpoint.address().to_string().c_str(), endpoint.port(),
			data.size(), (int)data.size(), data.data());

		sender.async_send(endpoint, data);
	}).bind_start([&]()
	{
		printf("start : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	}).bind_stop([&]()
	{
		printf("stop : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	}).bind_init([&]()
	{
		//// Join the multicast group.
		//sender.socket().set_option(
		//	asio::ip::multicast::join_group(asio::ip::make_address("239.255.0.1")));
	});

	sender.start(host, port);

	std::string str("<0123456789abcdefghijklmnopqrstowvxyz>");

	// send to self
	sender.send("127.0.0.1", 8022, str);

	asio::ip::udp::endpoint ep(asio::ip::make_address("127.0.0.1"), 8020);
	sender.async_send(ep, str);

	// this is a multicast address
	sender.async_send("239.255.0.1", "8030", str);

	// the resolve function is a time-consuming operation
	//asio::ip::udp::resolver resolver(sender.io().context());
	//asio::ip::udp::resolver::query query("www.baidu.com", "18080");
	//asio::ip::udp::endpoint ep2 = *resolver.resolve(query);
	//sender.async_send(ep2, std::move(str));

	while (std::getchar() != '\n');

	return 0;
}
