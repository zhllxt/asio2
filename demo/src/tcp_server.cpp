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
#ifdef WINDOWS
	// Detected memory leaks on windows system,linux has't these function
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	// the number is the memory leak line num of the vs output window content.
	//_CrtSetBreakAlloc(1640);
#endif

	std::signal(SIGINT, [](int signal) { run_flag = false; });

	asio2::session_ptr session;
	while (run_flag)
	{
		std::string url(" tcp://*:9001/auto?so_sndbuf=1024k & so_rcvbuf=1024K & recv_buffer_size=1024 & io_context_pool_size=4 &auto_reconnect=true");
		asio2::server tcp_server(url);
		tcp_server.bind_recv([&tcp_server](asio2::session_ptr & session_ptr, asio2::buffer_ptr & buf_ptr)
		{
			std::printf("recv : %.*s\n", (int)buf_ptr->size(), (const char*)buf_ptr->data());

			session_ptr->send(buf_ptr);
		}).bind_close([](asio2::session_ptr & session_ptr, int error)
		{
		}).bind_accept([&session](asio2::session_ptr & session_ptr)
		{
			session = session_ptr;
		});
		if (!tcp_server.start())
			std::printf("start tcp server failed : %d - %s\n", asio2::get_last_error(), asio2::get_last_error_desc().data());
		else
			std::printf("start tcp server successed : %s - %u\n", tcp_server.get_listen_address().data(), tcp_server.get_listen_port());

		//-----------------------------------------------------------------------------------------
		std::thread([&]()
		{
			while (run_flag)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		}).join();

		std::printf(">> ctrl + c is pressed,prepare exit...\n");
	}

	std::printf(">> leave main \n");

#ifdef WINDOWS
	//system("pause");
#endif

	return 0;
};
