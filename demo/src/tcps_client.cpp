// compile application on linux system can use below command :
// g++ -std=c++11 -lpthread -lrt -ldl client.cpp -o tcps_client.exe -I /usr/local/include -I ../../../ -L /usr/local/lib -l boost_system -Wall

#include <clocale>
#include <climits>
#include <csignal>
#include <ctime>
#include <locale>
#include <limits>
#include <thread>
#include <chrono>
#include <iostream>

#if defined(_MSC_VER)
#	pragma warning(disable:4996)
#endif

#define ASIO2_USE_SSL
#include <asio2/asio2.hpp>

volatile bool run_flag = true;

int main(int argc, char *argv[])
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system,linux has't these function
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// the number is the memory leak line num of the vs output window content.
	//_CrtSetBreakAlloc(1640);
#endif

	std::signal(SIGINT, [](int signal) { run_flag = false; });

	std::srand(static_cast<unsigned int>(std::time(nullptr)));

	while (run_flag)
	{
		std::string cer =
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

		while (run_flag)
		{
			asio2::client tcps_client("tcps://127.0.0.1:19443/");
			tcps_client
				//.set_certificate_file("server.crt");
				.set_certificate(cer);

			tcps_client.bind_recv([](asio2::buffer_ptr & buf_ptr)
			{
				//std::printf("recv : %.*s\n", (int)buf_ptr->size(), (const char*)buf_ptr->data());
			}).bind_connect([&tcps_client](int error)
			{
				if (error == 0)
					std::printf("start tcps client successed : %s - %u\n", tcps_client.get_remote_address().data(), tcps_client.get_remote_port());
				else
					std::printf("start tcps client failed : %d - %s.\n", asio2::get_last_error(), asio2::get_last_error_desc().data());
			});
			if (!tcps_client.start(false))
				std::printf("start tcps client failed : %d - %s.\n", asio2::get_last_error(), asio2::get_last_error_desc().data());

			std::string s;
			s += '<';
			int len = 33 + std::rand() % (126 - 33);
			for (int i = 0; i < len; i++)
			{
				s += (char)(std::rand() % 26) + 'a';
			}
			s += '>';

			tcps_client.send(s.data());

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		//-----------------------------------------------------------------------------------------
		std::thread([&]()
		{
			while (run_flag)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(2000));

				//std::string s;
				//s += '<';
				//int len = 33 + std::rand() % (126 - 33);
				//for (int i = 0; i < len; i++)
				//{
				//	s += (char)(std::rand() % 26) + 'a';
				//}
				//s += '>';

				//tcps_client.send(s.data());
			}
		}).join();

		std::printf(">> ctrl + c is pressed,prepare exit...\n");
	}

	std::printf(">> leave main \n");

#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	//system("pause");
#endif

	return 0;
};
