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

std::size_t pack_parser(asio2::buffer_ptr & buf_ptr)
{
	if (buf_ptr->size() < 3)
		return asio2::need_more_data;

	uint8_t * data = buf_ptr->data();
	if (data[0] == '<')
	{
		std::size_t pack_len = (data[1] - '0') + 3;
		if (buf_ptr->size() < pack_len)
			return asio2::need_more_data;
		if (data[pack_len - 1] == '>')
			return pack_len;
	}

	return asio2::invalid_data;
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
		while (run_flag)
		{
			int i = 0;
			const int client_count = 10;

			std::shared_ptr<asio2::client> tcp_pack_client[client_count];
			for (i = 0; i < client_count; i++)
			{
				tcp_pack_client[i] = std::make_shared<asio2::client>("tcp://localhost:8099/pack");
				tcp_pack_client[i]->bind_recv([](asio2::buffer_ptr & buf_ptr)
				{
					std::printf("recv : %.*s\n", (int)buf_ptr->size(), (const char*)buf_ptr->data());
				}).bind_send([&tcp_pack_client](asio2::buffer_ptr & buf_ptr, int error)
				{
				}).set_pack_parser(std::bind(pack_parser, std::placeholders::_1));

				if (!tcp_pack_client[i]->start(false))
					std::printf("connect to tcp server failed : %d - %s. %d\n", asio2::get_last_error(), asio2::get_last_error_desc().data(), i);
				else
					std::printf("connect to tcp server successed : %s - %u\n", tcp_pack_client[i]->get_remote_address().data(), tcp_pack_client[i]->get_remote_port());
			}
			for (i = 0; i < client_count; i++)
			{
				std::string s;
				s += '<';
				int len = 33 + std::rand() % (126 - 33);
				s += ((char)len + '0');
				for (int i = 0; i < len; i++)
				{
					s += (char)(std::rand() % 26) + 'a';
				}
				s += '>';
				len += 3;

				int packet_send_times = 0;
				int send_len = std::rand() % (len / 2);
				int already_send_len = 0;
				while (true)
				{
					packet_send_times++;
					tcp_pack_client[i]->send((const uint8_t *)(s.data() + already_send_len), (std::size_t)send_len);
					already_send_len += send_len;

					if (already_send_len >= len)
						break;

					send_len = std::rand() % (len / 2);
					if (send_len + already_send_len > len)
						send_len = len - already_send_len;

					// send for several packets,and sleep for a moment after each send is completed
					// the server will recv a full packet
					//std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}

				//std::printf("split packet count : %d\n", packet_send_times);
			}
		}

		//-----------------------------------------------------------------------------------------
		std::thread([&]()
		{
			while (run_flag)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(2000));

				//for (i = 0; i < client_count; i++)
				//{
				//	std::string s;
				//	s += '<';
				//	int len = 33 + std::rand() % (126 - 33);
				//	s += ((char)len + '0');
				//	for (int i = 0; i < len; i++)
				//	{
				//		s += (char)(std::rand() % 26) + 'a';
				//	}
				//	s += '>';
				//	len += 3;

				//	int packet_send_times = 0;
				//	int send_len = std::rand() % (len / 2);
				//	int already_send_len = 0;
				//	while (true)
				//	{
				//		packet_send_times++;
				//		tcp_pack_client[i]->send((const uint8_t *)(s.data() + already_send_len), (std::size_t)send_len);
				//		already_send_len += send_len;

				//		if (already_send_len >= len)
				//			break;

				//		send_len = std::rand() % (len / 2);
				//		if (send_len + already_send_len > len)
				//			send_len = len - already_send_len;

				//		// send for several packets,and sleep for a moment after each send is completed
				//		// the server will recv a full packet
				//		std::this_thread::sleep_for(std::chrono::milliseconds(100));
				//	}

				//	//std::printf("split packet count : %d\n", packet_send_times);
				//}
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
