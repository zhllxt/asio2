#pragma once

#include <asio2/asio2.hpp>

class ping_test
{
	asio2::ping ping;
public:
	ping_test() : ping(1000)
	{
		ping.timeout(std::chrono::seconds(3));
		ping.interval(std::chrono::seconds(1));
		ping.body("");
		ping.bind_recv(&ping_test::on_recv, this)
			.bind_start(std::bind(&ping_test::on_start, this, std::placeholders::_1))
			.bind_stop([this](asio::error_code ec) { this->on_stop(ec); });
	}
	void on_recv(asio2::icmp_rep& rep)
	{
		if (rep.lag.count() == -1)
			std::cout << "request timed out" << std::endl;
		else
			std::cout << rep.total_length() - rep.header_length()
			<< " bytes from " << rep.source_address()
			<< ": icmp_seq=" << rep.sequence_number()
			<< ", ttl=" << rep.time_to_live()
			<< ", time=" << std::chrono::duration_cast<std::chrono::milliseconds>(rep.lag).count() << "ms"
			<< std::endl;
	}
	void on_start(asio::error_code ec)
	{
		printf("start : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	}
	void on_stop(asio::error_code ec)
	{
		printf("stop : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	}
	void run()
	{
		if (!ping.start("fe80::59f9:25af:4427:fab7"))
			//if (!ping.start("123.45.67.89"))
			//if (!ping.start("stackoverflow.com"))
			printf("start failure : %s\n", asio2::last_error_msg().c_str());

		while (std::getchar() != '\n');

		ping.stop();
		printf("loss rate : %.0lf%% average time : %lldms\n", ping.plp(),
			std::chrono::duration_cast<std::chrono::milliseconds>(ping.avg_lag()).count());
	}
};

void run_ping_test()
{
	ping_test ping;
	ping.run();
}

