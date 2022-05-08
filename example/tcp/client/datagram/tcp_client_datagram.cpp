//#include <asio2/asio2.hpp>
#include <iostream>
#include <asio2/tcp/tcp_client.hpp>

int main()
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	std::string_view host = "127.0.0.1";
	std::string_view port = "8027";

	asio2::tcp_client* clients = new asio2::tcp_client[10];

	for (int i = 0; i < 10; i++)
	{
	asio2::tcp_client& client = clients[i];

	client.bind_connect([&]()
	{
		if (asio2::get_last_error())
			printf("connect failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
			printf("connect success : %s %u\n", client.local_address().c_str(), client.local_port());

		//std::string s;
		//s += '#';
		//s += char(1);
		//s += 'a';

		//// Beacuse the server specify the "max recv buffer size" to 1024, so if we
		//// send a too long packet, then this client will be disconnect .
		////s.resize(1500);

		//client.async_send(s);

		client.async_call("03defhijk", []([[maybe_unused]] std::string_view data)
		{
			if (asio2::get_last_error())
				return;
			ASIO2_ASSERT(data == "03defhijk");
		}, std::chrono::seconds(3));

	}).bind_disconnect([]()
	{
		printf("disconnect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
	}).bind_recv([&](std::string_view sv)
	{
		printf("recv : %.*s\n", (int)sv.size(), sv.data());

		//std::string s;
		//s += '#';
		//uint8_t len = uint8_t(100 + (std::rand() % 100));
		//s += char(len);
		//for (uint8_t i = 0; i < len; i++)
		//{
		//	s += (char)((std::rand() % 26) + 'a');
		//}

		//client.async_send(std::move(s));

	});

	asio2::rdc::option rdc_option
	{
		[](std::string_view data)
		{
			int id = std::strtol(data.substr(0, 2).data(), nullptr, 10);
			return id;
		}
	};

	client.start(host, port, asio2::use_dgram, std::move(rdc_option));

	client.async_call("01abc", []([[maybe_unused]] std::string_view data)
	{
		if (asio2::get_last_error())
			return;
		ASIO2_ASSERT(data == "01abc");
	});

	client.async_call("02defhijk", []([[maybe_unused]] std::string_view data)
	{
		if (asio2::get_last_error())
			return;
		ASIO2_ASSERT(data == "02defhijk");
	}, std::chrono::seconds(3));

	std::string str = client.call<std::string>("04abcdddddd");
	if (!asio2::get_last_error())
	{
		ASIO2_ASSERT(str == "04abcdddddd");
	}

	str = client.call<std::string>("05abcddddddx", std::chrono::seconds(3));
	if (!asio2::get_last_error())
	{
		ASIO2_ASSERT(str == "05abcddddddx");
	}

	client.call<std::string>("88abcddddddx");
	client.async_call("06ddabc");

	client.async_call("07xddabc").response([]([[maybe_unused]] std::string_view data)
	{
		if (asio2::get_last_error())
			return;
		ASIO2_ASSERT(data == "07xddabc");
	});

	str = client.timeout(std::chrono::seconds(3)).call<std::string>("08yabcddddddx");
	if (!asio2::get_last_error())
	{
		ASIO2_ASSERT(str == "08yabcddddddx");
	}

	str = client.set_timeout(std::chrono::seconds(3)).call<std::string>("09yabcddddddx");
	if (!asio2::get_last_error())
	{
		ASIO2_ASSERT(str == "09yabcddddddx");
	}

	client.async_call("10jxddabc").timeout(std::chrono::seconds(3)).response([]([[maybe_unused]] std::string_view data)
	{
		if (asio2::get_last_error())
			return;
		ASIO2_ASSERT(data == "10jxddabc");
	});

	client.set_timeout(std::chrono::seconds(3)).async_call("11gjxddabc").response([]([[maybe_unused]] std::string_view data)
	{
		if (asio2::get_last_error())
			return;
		ASIO2_ASSERT(data == "11gjxddabc");
	});

	client.response([]([[maybe_unused]] std::string_view data)
	{
		if (asio2::get_last_error())
			return;
		ASIO2_ASSERT(data == "12gggjxddabc");
	}).set_timeout(std::chrono::seconds(3)).async_call("12gggjxddabc");

	client.response([]([[maybe_unused]] std::string_view data)
	{
		if (asio2::get_last_error())
			return;
		ASIO2_ASSERT(data == "13yygggjxddabc");
	}).set_timeout(std::chrono::milliseconds(10)).async_call("13yygggjxddabc");

	client.start_timer("timer111", std::chrono::seconds(5), [&]()
	{
		client.async_call("01abc", []([[maybe_unused]] std::string_view data)
		{
			if (asio2::get_last_error())
				return;
			ASIO2_ASSERT(data == "01abc");
		});

		client.async_call("02defhijk", []([[maybe_unused]] std::string_view data)
		{
			if (asio2::get_last_error())
				return;
			ASIO2_ASSERT(data == "02defhijk");
		}, std::chrono::seconds(3));

		std::string str = client.call<std::string>("04abcdddddd");
		if (!asio2::get_last_error())
		{
			ASIO2_ASSERT(str == "04abcdddddd");
		}

		str = client.call<std::string>("05abcddddddx", std::chrono::seconds(3));
		if (!asio2::get_last_error())
		{
			ASIO2_ASSERT(str == "05abcddddddx");
		}

		client.call<std::string>("88abcddddddx");
		client.async_call("06ddabc");

		client.async_call("07xddabc").response([]([[maybe_unused]] std::string_view data)
		{
			if (asio2::get_last_error())
				return;
			ASIO2_ASSERT(data == "07xddabc");
		});

		str = client.timeout(std::chrono::seconds(3)).call<std::string>("08yabcddddddx");
		if (!asio2::get_last_error())
		{
			ASIO2_ASSERT(str == "08yabcddddddx");
		}

		str = client.set_timeout(std::chrono::seconds(3)).call<std::string>("09yabcddddddx");
		if (!asio2::get_last_error())
		{
			ASIO2_ASSERT(str == "09yabcddddddx");
		}

		client.async_call("10jxddabc").timeout(std::chrono::seconds(3)).response([]([[maybe_unused]] std::string_view data)
		{
			if (asio2::get_last_error())
				return;
			ASIO2_ASSERT(data == "10jxddabc");
		});

		client.set_timeout(std::chrono::seconds(3)).async_call("11gjxddabc").response([]([[maybe_unused]] std::string_view data)
		{
			if (asio2::get_last_error())
				return;
			ASIO2_ASSERT(data == "11gjxddabc");
		});

		client.response([]([[maybe_unused]] std::string_view data)
		{
			if (asio2::get_last_error())
				return;
			ASIO2_ASSERT(data == "12gggjxddabc");
		}).set_timeout(std::chrono::seconds(3)).async_call("12gggjxddabc");

		client.response([]([[maybe_unused]] std::string_view data)
		{
			if (asio2::get_last_error())
				return;
			ASIO2_ASSERT(data == "13yygggjxddabc");
		}).set_timeout(std::chrono::milliseconds(10)).async_call("13yygggjxddabc");

	});
	}

	while (std::getchar() != '\n');

	delete[]clients;

	return 0;
}
