#include <asio2/tcp/tcp_server.hpp>

decltype(std::chrono::steady_clock::now()) time1 = std::chrono::steady_clock::now();
decltype(std::chrono::steady_clock::now()) time2 = std::chrono::steady_clock::now();
std::atomic<std::size_t> recvd_bytes = 0;
bool first = true;

int main()
{
	asio2::tcp_server server;

	server.bind_recv([&](std::shared_ptr<asio2::tcp_session>& session_ptr, std::string_view data)
	{
		if (first)
		{
			first = false;
			time1 = std::chrono::steady_clock::now();
			time2 = std::chrono::steady_clock::now();
		}

		recvd_bytes += data.size();

		decltype(std::chrono::steady_clock::now()) time3 = std::chrono::steady_clock::now();
		auto sec = std::chrono::duration_cast<std::chrono::seconds>(time3 - time2).count();
		if (sec > 1)
		{
			time2 = time3;
			sec = std::chrono::duration_cast<std::chrono::seconds>(time3 - time1).count();
			double speed = (double)recvd_bytes / (double)sec / double(1024) / double(1024);
			printf("%.1lf MByte/Sec connection_count:%zu\n", speed, server.get_session_count());
		}

		session_ptr->async_send(asio::buffer(data)); // no allocate memory
		//session_ptr->async_send(data); // allocate memory
	});

	if (!server.start("0.0.0.0", "18081"))
		printf("start failed: %s\n", asio2::last_error_msg().data());

	while (std::getchar() != '\n');

	return 0;
}
