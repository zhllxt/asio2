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

// head 1 byte <
// len  1 byte content len,not include head and tail,just include the content len
// ...  content
// tail 1 byte >

std::size_t pack_parser(asio2::buffer_ptr data_ptr)
{
	if (data_ptr->size() < 3)
		return NEED_MORE_DATA;

	uint8_t * data = data_ptr->data();
	if (data[0] == '<')
	{
		std::size_t total_len = data[1] + 3;
		if (data_ptr->size() < total_len)
			return NEED_MORE_DATA;
		if (data[total_len - 1] == '>')
			return total_len;
	}

	return INVALID_DATA;
}

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
		asio2::server tcp_pack_server(" tcp://*:8099/pack?send_buffer_size=1024k & recv_buffer_size=1024K & pool_buffer_size=1024 & io_service_pool_size=3");
		tcp_pack_server.bind_recv([&tcp_pack_server](std::shared_ptr<asio2::session> session_ptr, asio2::buffer_ptr data_ptr)
		{
			std::printf("recv : %.*s\n", (int)data_ptr->size(), (const char*)data_ptr->data());

			int send_len = std::rand() % ((int)data_ptr->size() / 2);
			int already_send_len = 0;
			while (true)
			{
				session_ptr->send((const uint8_t *)(data_ptr->data() + already_send_len), (std::size_t)send_len);
				already_send_len += send_len;

				if ((std::size_t)already_send_len >= data_ptr->size())
					break;

				send_len = std::rand() % ((int)data_ptr->size() / 2);
				if (send_len + already_send_len > (int)data_ptr->size())
					send_len = (int)data_ptr->size() - already_send_len;

				// send for several packets,and sleep for a moment after each send is completed
				// the client will recv a full packet
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}

		}).set_pack_parser(std::bind(pack_parser, std::placeholders::_1));

		if (!tcp_pack_server.start())
			std::printf("start tcp server failed : %d - %s.\n", asio2::get_last_error(), asio2::get_last_error_desc().c_str());
		else
			std::printf("start tcp server successed : %s - %u\n", tcp_pack_server.get_listen_address().c_str(), tcp_pack_server.get_listen_port());


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
