#define ASIO2_ENABLE_LOG
#include <asio2/mqtt/mqtt_client.hpp>
#include <iostream>
#include <asio2/external/fmt.hpp>
#include <asio2/mqtt/message.hpp>

int main()
{
	std::string_view host = "broker.hivemq.com";
	//std::string_view host = "127.0.0.1";
	std::string_view port = "1883";

	asio2::mqtt_client client;

	mqtt::v5::connect connect;
	connect.client_id(u8"37792738@qq.com");
	client.set_connect_message(std::move(connect));

	client.bind_connect([&]()
	{
		if (asio2::get_last_error())
			printf("connect failure : %d %s\n",
				asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
			printf("connect success : %s %u\n",
				client.local_address().c_str(), client.local_port());

		if (!asio2::get_last_error())
		{
			client.start_timer(1, 1000, 1, [&]()
			{
				mqtt::v5::subscribe sub;
				sub.packet_id(1001);
				sub.add_subscriptions(mqtt::subscription{ "/asio2/mqtt/qos1",mqtt::qos_type::at_least_once });
				client.async_send(std::move(sub), []()
				{
					std::cout << "send v5::subscribe, packet_id :  1001" << std::endl;
				});
			});

			client.start_timer(2, 2000, 5, [&]()
			{
				static std::uint16_t id = 3000; id++;

				mqtt::v5::publish pub;
				pub.qos(mqtt::qos_type::at_least_once);
				pub.packet_id(id);
				pub.topic_name("/asio2/mqtt/qos1");
				pub.payload(fmt::format("{}", id));
				client.async_send(std::move(pub), [pid = id]()
				{
					fmt::print("send v5::publish  , packet id : {:5d}\n", pid);
				});
			});
		}

	}).bind_disconnect([]()
	{
		printf("disconnect : %d %s\n",
			asio2::last_error_val(), asio2::last_error_msg().c_str());
	});

	client.on_connack([](mqtt::v5::connack& connack)
	{
		fmt::print("recv v5::connack  , reason code: {}", int(connack.reason_code()));

		for (auto& vprop : connack.properties().data())
		{
			std::visit([](auto& prop)
			{
				[[maybe_unused]] auto name  = prop.name();
				[[maybe_unused]] auto type  = prop.type();
				[[maybe_unused]] auto value = prop.value();

				fmt::print(" {}:{}", name, value);
			}, vprop.variant());
		}
		fmt::print("\n");
	});

	client.on_suback([](mqtt::v5::suback& msg)
	{
		fmt::print("recv v5::suback   , packet id : {:5d} reason code: {}\n",
			msg.packet_id(), msg.reason_codes().at(0));
	});

	client.on_publish([](mqtt::v5::publish& msg, mqtt::message& rep)
	{
		asio2::ignore_unused(msg, rep);
		fmt::print("recv v5::publish  , packet id : {:5d} QoS : {} topic_name : {} payload : {}\n",
			msg.packet_id(), int(msg.qos()), msg.topic_name(), msg.payload());
	});

	client.on_puback([](mqtt::v5::puback& puback)
	{
		fmt::print("recv v5::puback   , packet id : {:5d} reason code : {}\n", puback.packet_id(), puback.reason_code());
	});

	client.on_pubrec([](mqtt::v5::pubrec& msg, mqtt::v5::pubrel& rep)
	{
		asio2::ignore_unused(msg, rep);
		fmt::print("recv v5::pubrec   , packet id : {:5d} reason_code : {}\n",
			msg.packet_id(), int(msg.reason_code()));
	});

	client.on_pubcomp([](mqtt::v5::pubcomp& msg)
	{
		fmt::print("recv v5::pubcomp  , packet id : {:5d} reason_code : {}\n",
			msg.packet_id(), int(msg.reason_code()));
	});

	client.on_pingresp([](mqtt::message& msg)
	{
		std::ignore = msg;
		printf("recv v5::pingresp\n");
	});

	client.on_disconnect([](mqtt::message& msg)
	{
		asio2::ignore_unused(msg);
		if (mqtt::v5::disconnect* p = static_cast<mqtt::v5::disconnect*>(msg); p)
			printf("recv v5::disconnect, reason code : %u\n", p->reason_code());
	});

	client.start(host, port);

	client.subscribe("/asio2/mqtt/qos1", 1, [](mqtt::v5::publish& msg) mutable
	{
		fmt::print("recv v5::publish  , packet id : {:5d} QoS : {} topic_name : {} payload : {}\n",
			msg.packet_id(), int(msg.qos()), msg.topic_name(), msg.payload());
	});

	client.subscribe("/asio2/mqtt/qos0", 0, [](mqtt::message& msg) mutable
	{
		ASIO2_ASSERT(!msg.empty());
		mqtt::v5::publish* p = msg.get_if<mqtt::v5::publish>();
		if (p)
		{
			fmt::print("recv v5::publish  , packet id : {:5d} QoS : {} topic_name : {} payload : {}\n",
				p->packet_id(), int(p->qos()), p->topic_name(), p->payload());
		}
	});

	bool ret1 = client.subscribe<bool>("/asio2/mqtt/qos1", 1, [](mqtt::v5::publish& msg) mutable
	{
		fmt::print("recv v5::publish  , packet id : {:5d} QoS : {} topic_name : {} payload : {}\n",
			msg.packet_id(), int(msg.qos()), msg.topic_name(), msg.payload());
	});

	bool ret2 = client.subscribe<bool>("/asio2/mqtt/qos0", 0, [](mqtt::message& msg) mutable
	{
		ASIO2_ASSERT(!msg.empty());
		mqtt::v5::publish* p = msg.get_if<mqtt::v5::publish>();
		if (p)
		{
			fmt::print("recv v5::publish  , packet id : {:5d} QoS : {} topic_name : {} payload : {}\n",
				p->packet_id(), int(p->qos()), p->topic_name(), p->payload());
		}
	});

	mqtt::message msg1 = client.subscribe<mqtt::message>("/asio2/mqtt/qos1", 1, [](mqtt::v5::publish& msg) mutable
	{
		fmt::print("recv v5::publish  , packet id : {:5d} QoS : {} topic_name : {} payload : {}\n",
			msg.packet_id(), int(msg.qos()), msg.topic_name(), msg.payload());
	});
	mqtt::v5::suback* psuback1 = msg1.get_if<mqtt::v5::suback>();

	mqtt::message msg2 = client.subscribe<mqtt::message>("/asio2/mqtt/qos0", 0, [](mqtt::message& msg) mutable
	{
		ASIO2_ASSERT(!msg.empty());
		mqtt::v5::publish* p = msg.get_if<mqtt::v5::publish>();
		if (p)
		{
			fmt::print("recv v5::publish  , packet id : {:5d} QoS : {} topic_name : {} payload : {}\n",
				p->packet_id(), int(p->qos()), p->topic_name(), p->payload());
		}
	});
	mqtt::v5::suback* psuback2 = msg2.get_if<mqtt::v5::suback>();

	client.unsubscribe("/asio2/mqtt/qos1");

	bool unret = client.unsubscribe<bool>("/asio2/mqtt/qos1");

	mqtt::message unmsg = client.unsubscribe<mqtt::message>("/asio2/mqtt/qos1");
	mqtt::v5::unsuback* punsuback = unmsg.get_if<mqtt::v5::unsuback>();

	client.publish("/asio2/mqtt/qos0", "0", 0);

	bool pubret = client.publish<bool>("/asio2/mqtt/qos1", "1", 1);

	mqtt::message pubmsg = client.publish<mqtt::message>("/asio2/mqtt/qos2", "2", 2);
	mqtt::v5::pubcomp* ppubcomp = pubmsg.get_if<mqtt::v5::pubcomp>();

	asio2::ignore_unused(ret1, ret2, msg1, msg2, psuback1, psuback2, unret, unmsg, punsuback, pubret, ppubcomp);

	while (std::getchar() != '\n');

	return 0;
}
