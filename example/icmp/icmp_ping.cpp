#include <asio2/icmp/ping.hpp>
#include <iostream>

int main()
{
	std::cout << asio2::ping::execute("www.baidu.com").milliseconds() << std::endl;;


	asio2::ping ping;
	ping.set_timeout(std::chrono::seconds(4))
		.set_interval(std::chrono::seconds(1))
		.set_body("abc")
		.bind_recv([](asio2::icmp_rep& rep)
	{
		if (rep.is_timeout())
			std::cout << "request timed out" << std::endl;
		else
			std::cout << rep.total_length() - rep.header_length()
			<< " bytes from " << rep.source_address()
			<< ": icmp_seq=" << rep.sequence_number()
			<< ", ttl=" << rep.time_to_live()
			<< ", time=" << rep.milliseconds() << "ms"
			<< std::endl;
	}).start("stackoverflow.com");

	while (std::getchar() != '\n');

	ping.stop();

	printf("loss rate : %.0lf%% average time : %ldms\n", ping.plp(),
		long(std::chrono::duration_cast<std::chrono::milliseconds>(ping.avg_lag()).count()));

	while (std::getchar() != '\n');

	return 0;
}

