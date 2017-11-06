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
		asio2::client tcp_client("tcp://localhost:9001/");
		tcp_client.bind_recv([](asio2::buffer_ptr data_ptr)
		{
			std::printf("recv : %.*s\n", (int)data_ptr->size(), (const char*)data_ptr->data());
		}).bind_connect([&tcp_client](int error)
		{
			if (error == 0)
				std::printf("connect to tcp server successed : %s - %u\n", tcp_client.get_remote_address().c_str(), tcp_client.get_remote_port());
			else
				std::printf("connect to tcp server failed : %d - %s\n", asio2::get_last_error(), asio2::get_last_error_desc().c_str());
		});
		if (!tcp_client.start(false))
			std::printf("connect to tcp server failed : %d - %s\n", asio2::get_last_error(), asio2::get_last_error_desc().c_str());

		//-----------------------------------------------------------------------------------------
		std::thread([&]()
		{
			while (run_flag)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(2000));

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

				tcp_client.send(s.c_str());
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
