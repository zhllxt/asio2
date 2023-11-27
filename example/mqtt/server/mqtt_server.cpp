#include <asio2/mqtt/mqtt_server.hpp>
#include <iostream>

int main()
{
	std::string_view host = "0.0.0.0";
	std::string_view port = "1883";

	asio2::mqtt_server server;

	asio2::mqtt_options options;
	//mqtt::options options;

	server.set_mqtt_options(options);

	server.bind_accept([](std::shared_ptr<asio2::mqtt_session>& session_ptr)
	{
		asio2::ignore_unused(session_ptr);

	}).bind_recv([](auto & session_ptr, std::string_view s)
	{
		asio2::ignore_unused(session_ptr, s);

		//printf("recv : %zu %.*s\n", s.size(), (int)s.size(), s.data());

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
	}).bind_start([&]()
	{
		printf("start mqtt server : %s %u %d %s\n", 
			server.listen_address().c_str(), server.listen_port(),
			asio2::last_error_val(), asio2::last_error_msg().c_str());
	}).bind_stop([&]()
	{
		printf("stop : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	});

	server.on_publish([](std::shared_ptr<asio2::mqtt_session>& session_ptr, mqtt::message& msg, mqtt::message& rep)
	{
		asio2::ignore_unused(session_ptr, msg, rep);
	});

	server.on_subscribe(
	[](std::shared_ptr<asio2::mqtt_session>& session_ptr, mqtt::message& msg, mqtt::message& rep) mutable
	{
		asio2::ignore_unused(session_ptr, msg, rep);

		mqtt::subscriptions_set* subset = nullptr;

		msg.invoke_if<mqtt::v3::subscribe, mqtt::v4::subscribe, mqtt::v5::subscribe>([&subset](auto& msg) mutable
		{
			// if the msg is v3::subscribe or v4::subscribe or v5::subscribe, then this lambda will 
			// be called, otherwise this lambda will can't be called
			subset = std::addressof(msg.subscriptions());
		});

		//if (subset)
		//{
		//	rep.invoke_if<mqtt::v3::suback, mqtt::v4::suback, mqtt::v5::suback>([](auto& msg) mutable
		//	{
		//		// if the msg is v3::suback or v4::suback or v5::suback, then this lambda
		//		// will be called, otherwise this lambda will can't be called
		//		msg.reason_codes().clear();
		//	});
		//}
	});

	server.start(host, port);

	while (std::getchar() != '\n');

	return 0;
}
