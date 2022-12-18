#include <asio2/tcp/tcp_server.hpp>

decltype(std::chrono::steady_clock::now()) time1 = std::chrono::steady_clock::now();
decltype(std::chrono::steady_clock::now()) time2 = std::chrono::steady_clock::now();
std::size_t qps = 0;
bool _first = true;

int main()
{
	asio2::tcp_server server;

	server.bind_recv([](auto& session_ptr, std::string_view data)
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

		session_ptr->async_send(data);
	});

	server.start("0.0.0.0", "8110", '\n');

	while (std::getchar() != '\n');

	return 0;
}
