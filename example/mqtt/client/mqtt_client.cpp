#include <asio2/mqtt/mqtt_client.hpp>
#include <iostream>

int main()
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	//std::string_view host = "broker.hivemq.com";
	std::string_view host = "127.0.0.1";
	std::string_view port = "1883";

	asio2::mqtt_client client;

	asio2::mqtt::v5::connect conn;
	conn.username("3772738@qq.com");
	conn.password("0123456789123456");
	conn.properties(
		asio2::mqtt::v5::payload_format_indicator{ asio2::mqtt::v5::payload_format_indicator::format::string },
		asio2::mqtt::v5::session_expiry_interval{ 60 });
	conn.will_attributes(
		u8"0123456789abcdefg",
		u8"0123456789abcdefg",
		asio2::mqtt::qos_type::at_least_once, true,
		asio2::mqtt::v5::payload_format_indicator{ 0 },
		asio2::mqtt::v5::session_expiry_interval{ 60 });
	conn.will_attributes(
		u8"0123456789abcdefg",
		u8"0123456789abcdefg",
		asio2::mqtt::qos_type::at_least_once, true);

	std::vector<asio::const_buffer> sbuffer;
	std::string sdata;
	std::vector<std::uint8_t> v1data;
	std::vector<char> v2data;
	conn.serialize(sdata);
	conn.serialize(sbuffer);

	std::cout << int(conn.version()) << std::endl;

	char* p1 = (char*)&sbuffer[0];
	char* p2 = (char*)&sbuffer[sbuffer.size() - 1];

	auto ssss = sizeof(asio::const_buffer);
	auto size = p2 - p1;

	std::cout << sdata.size() << std::endl;
	std::cout << ssss << std::endl;
	std::cout << size << std::endl;
	std::cout << conn.required_size() << std::endl;

	client.post([]() {}, std::chrono::seconds(3));

	client.bind_recv([&](std::string_view data)
	{
		asio2::detail::ignore_unused(data);

		//asio2::mqtt::v3::connack cack;
		//cack.deserialize(data);

		//asio2::mqtt::v4::connack cack;
		//cack.deserialize(data);

		//asio2::mqtt::v5::connack cack;
		//cack.deserialize(data);

		//cack.properties().has<asio2::mqtt::v5::receive_maximum>();

		//std::cout << "reason_code value is : " << int(cack.reason_code()) << std::endl;

		//asio2::mqtt::v5::receive_maximum* prm = cack.properties().get_if<asio2::mqtt::v5::receive_maximum>();
		//if (prm)
		//	std::cout << "receive_maximum value is : " << prm->value() << std::endl;

		//asio2::mqtt::v5::topic_alias_maximum* ptam = cack.properties().get_if<asio2::mqtt::v5::topic_alias_maximum>();
		//if(ptam)
		//	std::cout << "topic_alias_maximum value is : " << ptam->value() << std::endl;

		//asio2::mqtt::v5::reason_string* prs = cack.properties().get_if<asio2::mqtt::v5::reason_string>();
		//if(prs)
		//	std::cout << "reason_string value is : " << cack.reason_string() << std::endl;

		//if (cack.reason_code() == asio2::mqtt::v3::connect_reason_code::success)
		//	std::cout << "connect success-----------------------------\n";
		//else
		//	std::cout << "connect failure*****************************\n";

		//std::ignore = cack;

	}).bind_connect([&](asio::error_code ec)
	{
		if (ec)
			printf("connect failure : %d %s\n",
				asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
			printf("connect success : %s %u\n",
				client.local_address().c_str(), client.local_port());

		if (!ec)
		{
			asio2::mqtt::v5::publish publish;
			publish.qos(asio2::mqtt::qos_type::at_least_once);
			//publish.packet_id(1001);
			publish.topic_name("/");
			publish.payload("20210906_payload_test");
			client.async_send(std::move(publish), []()
			{
				std::cout << "send publish" << std::endl;
			});
		}

	}).bind_disconnect([](asio::error_code ec)
	{
		asio2::detail::ignore_unused(ec);

		printf("disconnect : %d %s\n",
			asio2::last_error_val(), asio2::last_error_msg().c_str());
	});

	client.on_connack([](asio2::mqtt::v3::connack& connack)
	{
		std::cout << "connack reason code: " << int(connack.reason_code()) << std::endl;
	});

	client.on_connack([](asio2::mqtt::v4::connack& connack)
	{
		std::cout << "connack reason code: " << int(connack.reason_code()) << std::endl;
	});

	client.on_connack([](asio2::mqtt::v5::connack& connack)
	{
		std::cout << "connack reason code: " << int(connack.reason_code()) << std::endl;

		for (auto& vprop : connack.properties().value())
		{
			std::visit([](auto& prop)
			{
				auto name  = prop.name();
				auto type  = prop.type();
				auto value = prop.value();

				asio2::detail::ignore_unused(name, type, value);
			},vprop);
		}
	});

	client.on_puback([](asio2::mqtt::v3::puback& puback)
	{
		printf("recv v3::puback : packet id : %u\n", puback.packet_id());
	});

	client.on_puback([](asio2::mqtt::v4::puback& puback)
	{
		printf("recv v4::puback : packet id : %u\n", puback.packet_id());
	});

	client.on_puback([](asio2::mqtt::v5::puback& puback)
	{
		printf("recv v5::puback : packet id : %u reason code : %u\n", puback.packet_id(), puback.reason_code());
	});

	client.on_pubrec([](asio2::mqtt::v3::pubrec& pubrec, asio2::mqtt::v3::pubrel& pubrel)
	{
		asio2::detail::ignore_unused(pubrec, pubrel);
		std::cout << "recv v3::pubrec, packet id: " << pubrec.packet_id() << std::endl;
	});

	client.on_pubrec([](asio2::mqtt::v4::pubrec& pubrec, asio2::mqtt::v4::pubrel& pubrel)
	{
		asio2::detail::ignore_unused(pubrec, pubrel);
		std::cout << "recv v4::pubrec, packet id: " << pubrec.packet_id() << std::endl;
	});

	client.on_pubrec([](asio2::mqtt::v5::pubrec& pubrec, asio2::mqtt::v5::pubrel& pubrel)
	{
		asio2::detail::ignore_unused(pubrec, pubrel);
		std::cout << "recv v5::pubrec, packet id: " << pubrec.packet_id() << " reason_code : " << int(pubrec.reason_code()) << std::endl;
	});

	client.on_pubcomp([](asio2::mqtt::v3::pubcomp& pubcomp)
	{
		asio2::detail::ignore_unused(pubcomp);
		std::cout << "recv v3::pubcomp, packet id: " << pubcomp.packet_id() << std::endl;
	});

	client.on_pubcomp([](asio2::mqtt::v4::pubcomp& pubcomp)
	{
		asio2::detail::ignore_unused(pubcomp);
		std::cout << "recv v4::pubcomp, packet id: " << pubcomp.packet_id() << std::endl;
	});

	client.on_pubcomp([](asio2::mqtt::v5::pubcomp& pubcomp)
	{
		asio2::detail::ignore_unused(pubcomp);
		std::cout << "recv v5::pubcomp, packet id: " << pubcomp.packet_id() << " reason_code : " << int(pubcomp.reason_code()) << std::endl;
	});

	client.on_pingresp([](asio2::mqtt::v3::pingresp& pingresp)
	{
		std::ignore = pingresp;
		printf("recv v3::pingresp\n");
	});

	client.on_pingresp([](asio2::mqtt::v4::pingresp& pingresp)
	{
		std::ignore = pingresp;
		printf("recv v4::pingresp\n");
	});

	client.on_pingresp([](asio2::mqtt::v5::pingresp& pingresp)
	{
		std::ignore = pingresp;
		printf("recv v5::pingresp\n");
	});

	client.on_disconnect([](asio2::mqtt::v3::disconnect& disconnect)
	{
		asio2::ignore_unused(disconnect);
		printf("recv v3::disconnect \n");
	});

	client.on_disconnect([](asio2::mqtt::v4::disconnect& disconnect)
	{
		asio2::ignore_unused(disconnect);
		printf("recv v4::disconnect \n");
	});

	client.on_disconnect([](asio2::mqtt::v5::disconnect& disconnect)
	{
		asio2::ignore_unused(disconnect);
		printf("recv v5::disconnect : reason code : %u\n", disconnect.reason_code());
	});

	asio2::mqtt::v5::connect connect;
	connect.client_id(u8"37792738@qq.com");

	client.start(host, port, std::move(connect));

	while (std::getchar() != '\n');

	return 0;
}
