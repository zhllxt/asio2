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
		std::string url(" tcp://*:9001/?send_buffer_size=1024k & recv_buffer_size=1024K & pool_buffer_size=1024 & io_service_pool_size=3 ");
		std::shared_ptr<asio2::server> tcp_server = std::make_shared<asio2::server>(url);
		tcp_server->bind_recv([&tcp_server](asio2::session_ptr session_ptr, asio2::buffer_ptr data_ptr)
		{
			std::printf("recv : %.*s\n", (int)data_ptr->size(), (const char*)data_ptr->data());

			session_ptr->send(data_ptr);
		}).bind_accept([](asio2::session_ptr session_ptr)
		{
			session_ptr->set_user_data(session_ptr);
		}).bind_close([](asio2::session_ptr session_ptr, int error)
		{
			session_ptr->set_user_data(nullptr);
		});
		if (!tcp_server->start())
			std::printf("start tcp server failed : %d - %s\n", asio2::get_last_error(), asio2::get_last_error_desc().c_str());
		else
			std::printf("start tcp server successed : %s - %u\n", tcp_server->get_listen_address().c_str(), tcp_server->get_listen_port());

		//-----------------------------------------------------------------------------------------
		std::thread([&]()
		{
			while (run_flag)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(2000));
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
