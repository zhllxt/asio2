#include <asio2/udp/udp_server.hpp>

decltype(std::chrono::steady_clock::now()) time1 = std::chrono::steady_clock::now();
decltype(std::chrono::steady_clock::now()) time2 = std::chrono::steady_clock::now();
std::size_t recvd_bytes = 0;
bool first = true;

int main()
{
	asio2::udp_server server(1500, 1500);

	server.bind_recv([&](std::shared_ptr<asio2::udp_session>& session_ptr, std::string_view data)
	{
		if (first)
		{
			first = false;
			time1 = std::chrono::steady_clock::now();
			time2 = std::chrono::steady_clock::now();
		}

		recvd_bytes += data.size();

		decltype(std::chrono::steady_clock::now()) time3 = std::chrono::steady_clock::now();
		auto ms = std::chrono::duration_cast<std::chrono::seconds>(time3 - time2).count();
		if (ms > 1)
		{
			time2 = time3;
			ms = std::chrono::duration_cast<std::chrono::seconds>(time3 - time1).count();
			double speed = (double)recvd_bytes / (double)ms / double(1024) / double(1024);
			printf("%.1lf MByte/Sec\n", speed);
		}

		session_ptr->async_send(asio::buffer(data)); // no allocate memory
		//session_ptr->async_send(data); // allocate memory
	});

	server.start("0.0.0.0", "8116");

	while (std::getchar() != '\n');

	return 0;
}
