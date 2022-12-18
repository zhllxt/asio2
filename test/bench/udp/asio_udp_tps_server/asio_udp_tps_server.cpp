
#include <asio2/base/detail/push_options.hpp>

#include <asio2/asio2.hpp>

using asio::ip::udp;

decltype(std::chrono::steady_clock::now()) time1 = std::chrono::steady_clock::now();
decltype(std::chrono::steady_clock::now()) time2 = std::chrono::steady_clock::now();
std::size_t recvd_bytes = 0;
bool first_flag = true;

class server
{
public:
	server(asio::io_context& io_context, short port)
		: socket_(io_context, udp::endpoint(udp::v4(), port))
	{
		socket_.async_receive_from(asio::buffer(data_, max_length), sender_endpoint_,
			[this](const asio::error_code& error, size_t bytes_recvd)
		{
			handle_receive_from(error, bytes_recvd);
		});
	}

	void handle_receive_from(const asio::error_code& error,
		size_t bytes_recvd)
	{
		if (!error && bytes_recvd > 0)
		{
			if (first_flag)
			{
				first_flag = false;
				time1 = std::chrono::steady_clock::now();
				time2 = std::chrono::steady_clock::now();
			}

			recvd_bytes += bytes_recvd;

			decltype(std::chrono::steady_clock::now()) time3 = std::chrono::steady_clock::now();
			auto ms = std::chrono::duration_cast<std::chrono::seconds>(time3 - time2).count();
			if (ms > 1)
			{
				time2 = time3;
				ms = std::chrono::duration_cast<std::chrono::seconds>(time3 - time1).count();
				double speed = (double)recvd_bytes / (double)ms / double(1024) / double(1024);
				printf("%.1lf MByte/Sec\n", speed);
			}

			socket_.async_send_to(
				asio::buffer(data_, bytes_recvd), sender_endpoint_,
				[this](const asio::error_code& error, size_t bytes_recvd)
			{
				handle_send_to(error, bytes_recvd);
			});
		}
		else
		{
			socket_.async_receive_from(asio::buffer(data_, max_length), sender_endpoint_,
				[this](const asio::error_code& error, size_t bytes_recvd)
			{
				handle_receive_from(error, bytes_recvd);
			});
		}
	}

	void handle_send_to(const asio::error_code& /*error*/,
		size_t /*bytes_sent*/)
	{
		socket_.async_receive_from(asio::buffer(data_, max_length), sender_endpoint_,
			[this](const asio::error_code& error, size_t bytes_recvd)
		{
			handle_receive_from(error, bytes_recvd);
		});
	}

private:
	udp::socket socket_;
	udp::endpoint sender_endpoint_;
	enum { max_length = 1500 };
	char data_[max_length];
};

int main()
{
	asio::io_context io_context;

	server s(io_context, 8115);

	io_context.run();

	while (std::getchar() != '\n'); // press enter to exit this program

	return 0;
}
