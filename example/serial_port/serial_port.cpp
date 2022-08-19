#include <asio2/serial_port/serial_port.hpp>

int main()
{
	std::string_view device = "COM1"; // for windows
	//std::string_view device = "/dev/ttyS0"; // for linux
	std::string_view baud_rate = "9600";

	asio2::serial_port sp;

	sp.bind_init([&]()
	{
		// Set other serial port parameters at here
		sp.set_option(asio::serial_port::flow_control(asio::serial_port::flow_control::type::none));
		sp.set_option(asio::serial_port::parity(asio::serial_port::parity::type::none));
		sp.set_option(asio::serial_port::stop_bits(asio::serial_port::stop_bits::type::one));
		sp.set_option(asio::serial_port::character_size(8));

	}).bind_recv([&](std::string_view data)
	{
		printf("recv : %zu %.*s\n", data.size(), (int)data.size(), data.data());

		sp.async_send(data);

	}).bind_start([&]()
	{
		printf("start : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	}).bind_stop([&]()
	{
		printf("stop : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	});

	//sp.start(device, baud_rate);
	sp.start(device, baud_rate, '\n');
	//sp.start(device, baud_rate, "\r\n");
	//sp.start(device, baud_rate, match_role);
	//sp.start(device, baud_rate, asio::transfer_at_least(1));
	//sp.start(device, baud_rate, asio::transfer_exactly(10));

	sp.async_send("abc0123456789xyz\n", [](std::size_t bytes_sent)
	{
		printf("send : %zu %d %s\n", bytes_sent,
			asio2::last_error_val(), asio2::last_error_msg().c_str());
	});

	while (std::getchar() != '\n');

	return 0;
}
