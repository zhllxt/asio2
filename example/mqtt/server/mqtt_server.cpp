#include <asio2/mqtt/mqtt_server.hpp>
#include <iostream>

int main()
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	std::string_view host = "0.0.0.0";
	std::string_view port = "1883";

	asio2::mqtt_server server;

	asio2::mqtt_options options;
	//asio2::mqtt::options options;

	server.set_options(options);

	server.bind_accept([](std::shared_ptr<asio2::mqtt_session>& session_ptr)
	{
		asio2::detail::ignore_unused(session_ptr);

	}).bind_recv([](auto & session_ptr, std::string_view s)
	{
		asio2::detail::ignore_unused(session_ptr, s);

		//printf("recv : %u %.*s\n", (unsigned)s.size(), (int)s.size(), s.data());

		//session_ptr->async_send(std::string(s), [](std::size_t bytes_sent) {});

	}).bind_connect([](auto & session_ptr)
	{
		printf("client enter : %s %u %s %u\n", 
			session_ptr->remote_address().c_str(), session_ptr->remote_port(),
			session_ptr->local_address().c_str(), session_ptr->local_port());
	}).bind_disconnect([](auto & session_ptr)
	{
		printf("client leave : %s %u %s\n", session_ptr->remote_address().c_str(),
			session_ptr->remote_port(), asio2::last_error_msg().c_str());
	}).bind_start([&](asio::error_code ec)
	{
		printf("start mqtt server : %s %u %d %s\n", 
			server.listen_address().c_str(), server.listen_port(),
			ec.value(), ec.message().c_str());
	}).bind_stop([&](asio::error_code ec)
	{
		printf("stop : %d %s\n", ec.value(), ec.message().c_str());
	});

	server.on_publish([](std::shared_ptr<asio2::mqtt_session>& session_ptr,
		asio2::mqtt::v3::publish& publish,
		std::variant<asio2::mqtt::v3::puback, asio2::mqtt::v3::pubrec>& response)
	{
		asio2::detail::ignore_unused(session_ptr, publish, response);
	});

	server.on_publish([](std::shared_ptr<asio2::mqtt_session>& session_ptr,
		asio2::mqtt::v4::publish& publish,
		std::variant<asio2::mqtt::v4::puback, asio2::mqtt::v4::pubrec>& response)
	{
		asio2::detail::ignore_unused(session_ptr, publish, response);
	});

	server.on_publish([](std::shared_ptr<asio2::mqtt_session>& session_ptr,
		asio2::mqtt::v5::publish& publish,
		std::variant<asio2::mqtt::v5::puback, asio2::mqtt::v5::pubrec>& response)
	{
		asio2::detail::ignore_unused(session_ptr, publish, response);
	});

	server.start(host, port);

	while (std::getchar() != '\n');

	return 0;
}
