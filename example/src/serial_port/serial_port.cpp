// When compiling with vs under linux, you need to copy the "asio,beast,ceral,fmt" folders to
// the /usr/local/include directory first, and copy the "libcrypto.a,libssl.a" files to 
// /usr/local/lib directory first. "libcrypto.a,libssl.a" is in "asio2/lib/x64".

#include <asio2/asio2.hpp>

int main()
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	std::string_view device = "COM1"; // for windows
	//std::string_view device = "/dev/ttyS0"; // for linux
	std::string_view baud_rate = "9600";

	asio2::scp sp;

	sp.start_timer(1, std::chrono::seconds(1), []() {});

	sp.bind_init([&]()
	{
		// Set other serial port parameters at here
		sp.socket().set_option(asio::serial_port::flow_control(asio::serial_port::flow_control::type::none));
		sp.socket().set_option(asio::serial_port::parity(asio::serial_port::parity::type::none));
		sp.socket().set_option(asio::serial_port::stop_bits(asio::serial_port::stop_bits::type::one));
		sp.socket().set_option(asio::serial_port::character_size(8));

	}).bind_recv([&](std::string_view sv)
	{
		printf("recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());

		std::string s;
		uint8_t len = uint8_t(10 + (std::rand() % 20));
		s += '<';
		for (uint8_t i = 0; i < len; i++)
		{
			s += (char)((std::rand() % 26) + 'a');
		}
		s += '>';
		sp.send(s, []() {});

	}).bind_start([&](asio::error_code ec)
	{
		printf("start : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	}).bind_stop([&](asio::error_code ec)
	{
		printf("stop : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	});

	//sp.start(device, baud_rate);
	sp.start(device, baud_rate, '>');
	//sp.start(device, baud_rate, "\r\n");
	//sp.start(device, baud_rate, match_role);
	//sp.start(device, baud_rate, asio::transfer_at_least(1));
	//sp.start(device, baud_rate, asio::transfer_exactly(10));

	sp.send("abc", [](std::size_t bytes_sent)
	{
		printf("send : %u %d %s\n", (unsigned)bytes_sent,
			asio2::last_error_val(), asio2::last_error_msg().c_str());
	});

	while (std::getchar() != '\n');

	return 0;
}
