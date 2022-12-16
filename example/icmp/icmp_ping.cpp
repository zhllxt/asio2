#include <asio2/icmp/ping.hpp>
#include <iostream>

int main()
{
	// under linux maybe failed always:
	// https://github.com/chriskohlhoff/asio/issues/781
	std::cout << asio2::ping::execute("www.baidu.com").milliseconds() << std::endl;;

	// execute with error, print the error message.
	if (asio2::get_last_error())
		std::cout << asio2::last_error_msg() << std::endl;

	asio2::ping ping;
	ping.set_timeout(std::chrono::seconds(4));
	ping.set_interval(std::chrono::seconds(1));
	ping.bind_recv([](asio2::icmp_rep& rep)
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
	}).bind_start([]()
	{
		if (asio2::get_last_error())
			std::cout << asio2::last_error_msg() << std::endl;
	});
	ping.start("stackoverflow.com");

	while (std::getchar() != '\n');

	ping.stop();

	printf("loss rate : %.0lf%% average time : %ldms\n", ping.plp(),
		long(std::chrono::duration_cast<std::chrono::milliseconds>(ping.avg_lag()).count()));

	while (std::getchar() != '\n');

	return 0;
}

