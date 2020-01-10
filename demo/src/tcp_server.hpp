#pragma once

#include <asio2/asio2.hpp>

void run_tcp_server(std::string_view host, std::string_view port)
{
	//while (1) // use infinite loop and sleep 1 seconds to test start and stop
	//for (int i = 0; i < 1000; i++)
	{
		asio2::tcp_server server;
		printf("\n");
		server.start_timer(1, std::chrono::seconds(1), []() {}); // test timer
		server.bind_recv([&server](std::shared_ptr<asio2::tcp_session> & session_ptr, std::string_view s)
		{
			printf("recv : %u %.*s\n", (unsigned)s.size(), (int)s.size(), s.data());

			session_ptr->send(s, [](std::size_t bytes_sent) {});

			// ##Thread-safe send operation example:
			//session_ptr->post([session_ptr]()
			//{
			//	asio::write(session_ptr->stream(), asio::buffer(std::string("abcdefghijklmn")));
			//});

			// ##Use this to check whether the send operation is running in current thread.
			//if (session_ptr->io().strand().running_in_this_thread())
			//{
			//}

		}).bind_connect([&server](auto & session_ptr)
		{
			session_ptr->no_delay(true);
			session_ptr->start_timer(2, std::chrono::seconds(1), []() {}); // test timer
			//session_ptr->stop(); // You can close the connection directly here.
			printf("client enter : %s %u %s %u\n",
				session_ptr->remote_address().c_str(), session_ptr->remote_port(),
				session_ptr->local_address().c_str(), session_ptr->local_port());
		}).bind_disconnect([&server](auto & session_ptr)
		{
			printf("client leave : %s %u %s\n",
				session_ptr->remote_address().c_str(),
				session_ptr->remote_port(), asio2::last_error_msg().c_str());
		}).bind_start([&server](asio::error_code ec)
		{
			printf("start tcp server : %s %u %d %s\n", server.listen_address().c_str(), server.listen_port(),
				ec.value(), ec.message().c_str());
		}).bind_stop([&server](asio::error_code ec)
		{
			printf("stop : %d %s\n", ec.value(), ec.message().c_str());
		});

		server.start(host, port);
		//server.start(host, port, asio::transfer_at_least(100));
		//server.start(host, port, asio::transfer_exactly(100));

		while (std::getchar() != '\n');  // press enter to exit this program
		//std::this_thread::sleep_for(std::chrono::milliseconds(500));

		server.stop();
	}
}

void run_tcps_server(std::string_view host, std::string_view port)
{
#ifdef ASIO2_USE_SSL
	std::string_view cer =
		"-----BEGIN CERTIFICATE-----\r\n"\
		"MIICcTCCAdoCCQDYl7YrsugMEDANBgkqhkiG9w0BAQsFADB9MQswCQYDVQQGEwJD\r\n"\
		"TjEOMAwGA1UECAwFSEVOQU4xEjAQBgNVBAcMCVpIRU5HWkhPVTENMAsGA1UECgwE\r\n"\
		"SE5aWDENMAsGA1UECwwESE5aWDEMMAoGA1UEAwwDWkhMMR4wHAYJKoZIhvcNAQkB\r\n"\
		"Fg8zNzc5MjczOEBxcS5jb20wHhcNMTcxMDE1MTQzNjI2WhcNMjcxMDEzMTQzNjI2\r\n"\
		"WjB9MQswCQYDVQQGEwJDTjEOMAwGA1UECAwFSEVOQU4xEjAQBgNVBAcMCVpIRU5H\r\n"\
		"WkhPVTENMAsGA1UECgwESE5aWDENMAsGA1UECwwESE5aWDEMMAoGA1UEAwwDWkhM\r\n"\
		"MR4wHAYJKoZIhvcNAQkBFg8zNzc5MjczOEBxcS5jb20wgZ8wDQYJKoZIhvcNAQEB\r\n"\
		"BQADgY0AMIGJAoGBAMc2Svpl4UgxCVKGwoYJBxNWObXvQzw74ksY6Zoiq5tJNJzf\r\n"\
		"q9ZCJigwjx3vAFF7tELRxsgAf6l7AvReu1O6difjdpMkEic0W7acZtldislDjUbu\r\n"\
		"qitfHsWeKTucBu3+3TUawvv+fdeWgeN54jMoL+Oo3CV7d2gFRV2fD5z4tryXAgMB\r\n"\
		"AAEwDQYJKoZIhvcNAQELBQADgYEAwDIC3xYmYJ6kLI8NgmX89re0scSWCcA8VgEZ\r\n"\
		"u8roYjYauCLkp1aXNlZtJFQjwlfo+8FLzgp3dP8Y75YFwQ5zy8fFaLQSQ/0syDbx\r\n"\
		"sftKSVmxDo3S27IklEyJAIdB9eKBTeVvrT96R610j24t1eYENr59Vk6A/fKTWJgU\r\n"\
		"EstmrAs=\r\n"\
		"-----END CERTIFICATE-----\r\n";

	std::string_view key =
		"-----BEGIN RSA PRIVATE KEY-----\r\n"\
		"Proc-Type: 4,ENCRYPTED\r\n"\
		"DEK-Info: DES-EDE3-CBC,EC5314BD06CD5FB6\r\n"\
		"\r\n"\
		"tP93tjR4iOGfOLHjIBQA0aHUE5wQ7EDcUeKacFfuYrtlYbYpbRzhQS+vGtoO1wGg\r\n"\
		"h/s9DbEN1XaiV9aE+N3E54zu2LuVO1lYDtCf3L26cd1Bu6gj0cWiAMco1Vm7RV9j\r\n"\
		"vmgmeOYkqbOiAbiIa4HCmDkEaHY4nCPlW+cdYxrozkAQCAiTpFQR8taRB0lsly0i\r\n"\
		"lUQitYLz3nhEMucLffcwAXN9IOnXFoURVZnLc53CX857iizOXeP9XeNE63UwDZ4v\r\n"\
		"1wnglnGUJA6vCxnxk6KvptF9rSdCD/sz1Y+J5mAVr+2y4vPLO4YOCL6HSFY6285M\r\n"\
		"RyGNVVx3vX0u6FbWJC3qt5yj6tMdVJ4O7U4XgqOKnS5jVLk+fKcTVyNySB5yAT2b\r\n"\
		"qwWCZcRPP2M+qlsSWhgzsucyz0eVOPVJxAJ4Vp/X6saO4xyRPsFV3USbRKlOMS7+\r\n"\
		"SEJ/7ANU9mEgLIQRKEfSKXWpQtm95pCVlajWQ7/3nXNjdV7mNi42ukdItBvOtdv+\r\n"\
		"oUiN8MkP/e+4SsGmJayNT7HvBC9DjoyDQIK6sZOgtsbAu/bDBhPnjnNsZcsgxJ/O\r\n"\
		"ijnj+0HyNS/Vr6emAkxTFgryUdBTuoY7019vcNWTYPDS3ugpe3goRHE0FTOwNdUe\r\n"\
		"dk+KM4bYAa0+1z1QEZTEoNqdT7WYwMD1QzgSWukYHemsWqoAvW5f4PrdoVA21W9D\r\n"\
		"L8I1YZf8ZHBnkuGX0oHi5w/4DkVNOT5BaZRmqXinZgFPwduYGVCh04x7ohuOQ5m0\r\n"\
		"etrTAVwJd2mcI7rDTaKCPT528/QWxZxXpHzggRoDil/5T7fn35ixRg==\r\n"\
		"-----END RSA PRIVATE KEY-----\r\n";

	std::string_view dh =
		"-----BEGIN DH PARAMETERS-----\r\n"\
		"MEYCQQCdoJif7jYqTh5+vLgt3q1FZvG+7WymoAoMKWMNOtqLZ+uFhZH3e9vFhV7z\r\n"\
		"NgWnHCe/vsGJok2wHS4R/laH6MQTAgEC\r\n"\
		"-----END DH PARAMETERS-----\r\n";

	//while (1)
	//for (int i = 0; i < 1000; i++)
	{
		printf("\n");
		bool stopped = false;
		asio2::tcps_server server;
		server.set_cert("test", cer, key, dh); // use memory string for cert
		//server.set_cert_file("test", "server.crt", "server.key", "dh512.pem"); // use file for cert
		server.start_timer(1, std::chrono::seconds(1), []() {});
		server.bind_recv([&server](auto & session_ptr, std::string_view s)
		{
			printf("recv : %u %.*s\n", (unsigned)s.size(), (int)s.size(), s.data());

			session_ptr->send(s, []() {});
		}).bind_connect([&](auto & session_ptr)
		{
			printf("client enter : %s %u %s %u\n", session_ptr->remote_address().c_str(), session_ptr->remote_port(),
				session_ptr->local_address().c_str(), session_ptr->local_port());
		}).bind_disconnect([&](auto & session_ptr)
		{
			// Used to test that all sessions must be closed before entering the on_stop(bind_stop) function.
			if (stopped)
			{
				ASIO2_ASSERT(false);
			}
			printf("client leave : %s %u %s\n", session_ptr->remote_address().c_str(),
				session_ptr->remote_port(), asio2::last_error_msg().c_str());
		}).bind_handshake([&](auto & session_ptr, asio::error_code ec)
		{
			if (asio2::get_last_error())
				printf("handshake failure : %d %s\n", ec.value(), ec.message().c_str());
			else
				printf("handshake success : %s %u\n", session_ptr->remote_address().c_str(), session_ptr->remote_port());
		}).bind_start([&](asio::error_code ec)
		{
			if (asio2::get_last_error())
				printf("start tcps server failure : %d %s\n", ec.value(), ec.message().c_str());
			else
				printf("start tcps server success : %s %u\n", server.listen_address().c_str(), server.listen_port());
			//server.stop();
		}).bind_stop([&](asio::error_code ec)
		{
			stopped = true;
			printf("stop : %d %s\n", ec.value(), ec.message().c_str());
		});
		server.start(host, port);

		while (std::getchar() != '\n');
		std::this_thread::sleep_for(std::chrono::milliseconds(200));

		server.stop();
	}
#endif // ASIO2_USE_SSL
}
