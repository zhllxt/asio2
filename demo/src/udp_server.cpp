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

class user_udp_server_listener : public asio2::udp_server_listener
{
public:
	virtual void on_send(asio2::session_ptr & session_ptr, asio2::buffer_ptr & buf_ptr, int error) override
	{
	}
	virtual void on_recv(asio2::session_ptr & session_ptr, asio2::buffer_ptr & buf_ptr) override
	{
		std::printf("recv : %.*s\n", (int)buf_ptr->size(), (const char*)buf_ptr->data());

		session_ptr->send(buf_ptr);
	}
	virtual void on_close(asio2::session_ptr & session_ptr, int error) override
	{
	}
	virtual void on_listen() override
	{
	}
	virtual void on_accept(asio2::session_ptr & session_ptr) override
	{
	}
	virtual void on_shutdown(int error) override
	{
	}
};

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
		asio2::server udp_server("udp://*:9010/?so_sndbuf=2m&so_rcvbuf=3m&recv_buffer_size=1024");
		udp_server.bind_listener(std::make_shared<user_udp_server_listener>());
		if (!udp_server.start())
			std::printf("start udp server failed : %d - %s.\n", asio2::get_last_error(), asio2::get_last_error_desc().data());
		else
			std::printf("start udp server successed : %s - %u\n", udp_server.get_listen_address().data(), udp_server.get_listen_port());


		//asio2::sender udp_sender("udp://*:0");
		//udp_sender.start();

		//-----------------------------------------------------------------------------------------
		std::thread([&]()
		{
			while (run_flag)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				//udp_sender.send("127.0.0.1", "40001", "abc");
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
