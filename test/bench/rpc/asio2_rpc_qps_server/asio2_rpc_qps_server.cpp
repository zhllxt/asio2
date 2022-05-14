#include <asio2/rpc/rpc_server.hpp>

decltype(std::chrono::steady_clock::now()) time1 = std::chrono::steady_clock::now();
decltype(std::chrono::steady_clock::now()) time2 = std::chrono::steady_clock::now();
std::size_t qps = 0;
bool _first = true;
std::string echo(std::string a)
{
	if (_first)
	{
		_first = false;
		time1 = std::chrono::steady_clock::now();
		time2 = std::chrono::steady_clock::now();
	}

	qps++;
	decltype(std::chrono::steady_clock::now()) time3 = std::chrono::steady_clock::now();
	auto ms = std::chrono::duration_cast<std::chrono::seconds>(time3 - time2).count();
	if (ms > 1)
	{
		time2 = time3;
		ms = std::chrono::duration_cast<std::chrono::seconds>(time3 - time1).count();
		double speed = (double)qps / (double)ms;
		printf("%.1lf\n", speed);
	}
	return a;
}

int main()
{
	asio2::rpc_server server;

	server.bind("echo", echo);
	server.start("0.0.0.0", "8080");

	while (std::getchar() != '\n');

	return 0;
}
