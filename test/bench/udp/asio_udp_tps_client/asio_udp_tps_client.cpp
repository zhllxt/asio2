
#include <asio2/base/detail/push_options.hpp>

#include <asio.hpp>

using asio::ip::udp;

std::string strmsg(1024, 'A');

class client
{
public:
	client(asio::io_context& io_context, std::string ip, short port)
		: socket_(io_context, udp::endpoint(udp::v4(), 0))
	{
		udp::resolver resolver(io_context);
		udp::resolver::results_type endpoints =
			resolver.resolve(udp::v4(), ip, std::to_string(port));

		dest_endpoint_ = *endpoints.begin();

		socket_.async_send_to(
			asio::buffer(strmsg), dest_endpoint_,
			[this](const asio::error_code& error, size_t bytes_recvd)
		{
			handle_send_to(error, bytes_recvd);
		});
	}

	void handle_receive_from(const asio::error_code& error,
		size_t bytes_recvd)
	{
		if (!error && bytes_recvd > 0)
		{
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
	udp::endpoint dest_endpoint_;
	enum { max_length = 1500 };
	char data_[max_length];
};

int main()
{
	asio::io_context io_context;

	client s(io_context, "127.0.0.1", 8115);

	io_context.run();

	while (std::getchar() != '\n');

	return 0;
}
