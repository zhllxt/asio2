#include <asio2/icmp/ping.hpp>
#include <iostream>

void ping_test1()
{
	asio2::ping ping;
	ping.timeout(std::chrono::seconds(3))
		.interval(std::chrono::seconds(1))
		.body("abc")
		.bind_recv([](asio2::icmp_rep& rep)
	{
		if (rep.is_timeout())
			std::cout << "request timed out" << std::endl;
		else
			std::cout << rep.total_length() - rep.header_length()
			<< " bytes from " << rep.source_address()
			<< ": icmp_seq=" << rep.sequence_number()
			<< ", ttl=" << rep.time_to_live()
			<< ", time=" << std::chrono::duration_cast<std::chrono::milliseconds>(rep.lag).count() << "ms"
			<< std::endl;
	}).start("151.101.193.69");

	while (std::getchar() != '\n');
}

class ping_test
{
	asio2::ping ping;
public:
	ping_test() : ping(1000)
	{
		ping.post([]() {}, std::chrono::seconds(3));
		ping.timeout(std::chrono::seconds(3));
		ping.interval(std::chrono::seconds(1));
		ping.body("");
		ping.bind_recv(&ping_test::on_recv, this)
			.bind_start(std::bind(&ping_test::on_start, this, std::placeholders::_1))
			.bind_stop([this](asio::error_code ec) { this->on_stop(ec); });
	}
	void on_recv(asio2::icmp_rep& rep)
	{
		if (rep.is_timeout())
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
		printf("start : %d %s\n", ec.value(), ec.message().c_str());
	}
	void on_stop(asio::error_code ec)
	{
		printf("stop : %d %s\n", ec.value(), ec.message().c_str());
	}
	void run()
	{
		//if (!ping.start("123.45.67.89"))
		//if (!ping.start("fe80::59f9:25af:4427:fab7"))
		if (!ping.start("stackoverflow.com"))
			printf("start failure : %s\n", asio2::last_error_msg().c_str());

		while (std::getchar() != '\n');

		ping.stop();

		printf("loss rate : %.0lf%% average time : %ldms\n", ping.plp(),
			long(std::chrono::duration_cast<std::chrono::milliseconds>(ping.avg_lag()).count()));
	}
};

int main()
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	ping_test ping;
	ping.run();

	//ping_test1();

	while (std::getchar() != '\n');

	return 0;
}

