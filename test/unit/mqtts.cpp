#ifndef ASIO2_USE_SSL
#define ASIO2_USE_SSL
#endif

#include "unit_test.hpp"

#include <asio2/mqtt/mqtts_client.hpp>
#include <asio2/mqtt/mqtts_server.hpp>

#include <iostream>
#include <asio2/external/fmt.hpp>

void mqtts_test()
{
	{
		std::string_view host = "test.mosquitto.org";
		//std::string_view host = "127.0.0.1";
		std::string_view port = "8883";

		asio2::mqtts_client client;

		client.bind_connect([&]()
		{
		}).bind_disconnect([]()
		{
		});

		bool on_connack_flag = false;
		client.on_connack([&on_connack_flag]([[maybe_unused]] mqtt::v5::connack& connack)
		{
			on_connack_flag = true;
		});

		std::promise<void> on_suback_promise;
		std::future<void> on_suback_future = on_suback_promise.get_future();
		bool on_suback_flag = false;
		client.on_suback([&on_suback_flag, &on_suback_promise]([[maybe_unused]] mqtt::v5::suback& msg)
		{
			on_suback_flag = true;
			on_suback_promise.set_value();
		});

		std::promise<void> on_publish_promise;
		std::future<void> on_publish_future = on_publish_promise.get_future();
		bool on_publish_flag = false;
		client.on_publish([&on_publish_flag, &on_publish_promise](mqtt::v5::publish& msg, mqtt::message& rep)
		{
			asio2::ignore_unused(msg, rep);
			on_publish_flag = true;
			on_publish_promise.set_value();
		});

		std::promise<void> on_puback_promise;
		std::future<void> on_puback_future = on_puback_promise.get_future();
		bool on_puback_flag = false;
		client.on_puback([&on_puback_flag, &on_puback_promise]([[maybe_unused]] mqtt::v5::puback& puback)
		{
			on_puback_flag = true;
			on_puback_promise.set_value();
		});

		mqtt::v5::connect connect;
		connect.client_id(u8"37792738@qq.com");

		if (!client.start(host, port, connect))
			return;

		ASIO2_CHECK(on_connack_flag);

		std::promise<void> subscribe0_promise;
		std::future<void> subscribe0_future = subscribe0_promise.get_future();
		bool subscribe0 = false;
		client.subscribe("/asio2/mqtt/qos0", 0, [&subscribe0, &subscribe0_promise]([[maybe_unused]] mqtt::message& msg) mutable
		{
			ASIO2_CHECK(!msg.empty());
			mqtt::v5::publish* p = msg.get_if<mqtt::v5::publish>();
			if (p)
			{
				subscribe0 = true;
			}
			subscribe0_promise.set_value();
		});
		on_suback_future.wait();
		ASIO2_CHECK(on_suback_flag);

		mqtt::v5::publish pub;
		pub.qos(mqtt::qos_type::at_least_once);
		pub.packet_id(3000);
		pub.topic_name("/asio2/mqtt/qos0");
		pub.payload(fmt::format("{}", 3000));
		client.async_send(std::move(pub), [pid = 3000]()
		{
		});
		on_puback_future.wait();
		on_publish_future.wait();
		subscribe0_future.wait();
		ASIO2_CHECK(on_puback_flag);
		ASIO2_CHECK(on_publish_flag);
		ASIO2_CHECK(subscribe0);

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}


ASIO2_TEST_SUITE
(
	"mqtts",
	ASIO2_TEST_CASE(mqtts_test)
)
