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
		const int client_count = 1;
		std::shared_ptr<asio2::client> tcp_client[client_count];
		for (int i = 0; i < client_count; i++)
		{
			tcp_client[i] = std::move(std::make_shared<asio2::client>("tcp://127.0.0.1:9001/auto"));
		}
		for (int i = 0; i < client_count; i++)
		{
			tcp_client[i]->bind_recv([&tcp_client, i](asio2::buffer_ptr & buf_ptr)
			{
				std::printf("recv : %.*s\n", (int)buf_ptr->size(), (const char*)buf_ptr->data());

				//tcp_client[i]->send(std::move(buf_ptr), false);
			}).bind_connect([&tcp_client, i](int error)
			{
				if (error == 0)
					std::printf("connect to tcp server successed : %s - %u\n", tcp_client[i]->get_remote_address().data(), tcp_client[i]->get_remote_port());
				else
				{
					std::printf("connect to tcp server failed : %d - %s\n", asio2::get_last_error(), asio2::get_last_error_desc().data());
					tcp_client[i]->start();
				}
			}).bind_close([&tcp_client,i](int error)
			{
				std::printf("connection is disconnected : %d - %s\n", asio2::get_last_error(), asio2::get_last_error_desc().data());
			});
			if (!tcp_client[i]->start(false))
				std::printf("connect to tcp server failed : %d - %s\n", asio2::get_last_error(), asio2::get_last_error_desc().data());
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

					tcp_client[i]->send(s.data());
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
