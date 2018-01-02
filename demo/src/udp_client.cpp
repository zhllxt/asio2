// compile application on linux system can use below command :
// g++ -std=c++11 -lpthread -lrt -ldl main.cpp -o main.exe -I /usr/local/include -I ../../asio2 -L /usr/local/lib -l boost_system -Wall

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

#include <asio2/asio2.hpp>

volatile bool run_flag = true;

class main_frame
{
public:
	void on_recv(asio2::buffer_ptr & buf_ptr, int user_data)
	{
		std::printf("user_data : %d recv : %.*s\n", user_data, (int)buf_ptr->size(), (const char*)buf_ptr->data());
	}
};

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

	main_frame _main_frame;

	const int client_count = 1;

	while (run_flag)
	{
		std::shared_ptr<asio2::client> udp_client[client_count];
		for (int i = 0; i < client_count; i++)
		{
			udp_client[i] = std::make_shared<asio2::client>("udp://localhost:9010");
			udp_client[i]->bind_recv(std::bind(&main_frame::on_recv, &_main_frame, std::placeholders::_1, 100));

			if (!udp_client[i]->start(false))
				std::printf("connect to udp server failed : %d - %s.\n %d", asio2::get_last_error(), asio2::get_last_error_desc().c_str(), i);
			else
				std::printf("connect to udp server successed : %s - %u\n", udp_client[i]->get_remote_address().c_str(), udp_client[i]->get_local_port());
		}


		//-----------------------------------------------------------------------------------------
		std::thread([&]()
		{
			while (run_flag)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(2000));

				for (int i = 0; i < client_count; i++)
				{
					std::string s;
					s += '<';
					int len = 33 + std::rand() % (126 - 33);
					s += (char)len;
					for (int i = 0; i < len; i++)
					{
						s += (char)(std::rand() % 26) + 'a';
					}
					s += '>';
					len += 3;

					udp_client[i]->send(s.c_str());
				}
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
