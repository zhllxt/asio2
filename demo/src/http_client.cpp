// compile application on linux system can use below command :
// g++ -std=c++11 -lpthread -lrt -ldl main.cpp -o main.exe -I /usr/local/include -I ../../asio2 -L /usr/local/lib -l boost_system -Wall

#include <csignal>
#include <ctime>

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

	//std::string get = 
	//	"GET /DataSvr/api/anchor/AddAnchor?json=%7b%22id%22:4990560701320869680,%22name%22:%22anchor222%22,%22ip%22:%22192.168.0.101%22%7d HTTP/1.1\r\n"\
	//	"Host: localhost:8443\r\n"\
	//	"Connection: keep-alive\r\n"\
	//	"User-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/61.0.3163.100 Safari/537.36\r\n"\
	//	"Upgrade-Insecure-Requests: 1\r\n"\
	//	"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8\r\n"\
	//	"Accept-Encoding: gzip, deflate, br\r\n"\
	//	"Accept-Language: zh-CN,zh;q=0.8\r\n"\

	while (run_flag)
	{
		//GET /DataSvr/api/anchor/AddAnchor?json=%7b%22id%22:4990560701320869680,%22name%22:%22anchor222%22,%22ip%22:%22192.168.0.101%22%7d HTTP/1.1
		//Host: localhost:8443
		//Connection: keep-alive
		//User-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/61.0.3163.100 Safari/537.36
		//Upgrade-Insecure-Requests: 1
		//Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8
		//Accept-Encoding: gzip, deflate, br
		//Accept-Language: zh-CN,zh;q=0.8

		auto t1 = std::chrono::steady_clock::now();
		asio2::client http_client("http://bbs.csdn.net:80");
		http_client.bind_recv([](asio2::response_ptr response_ptr)
		{

		}).bind_close([&t1](int error)
		{
			printf("http_client is closed %d\n", std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - t1).count());
		});
		if (!http_client.start(false))
			std::printf("connect to http server failed : %d - %s\n", asio2::get_last_error(), asio2::get_last_error_desc().c_str());
		{
			asio2::http_request req;
			req
				.method(asio2::http::HTTP_GET)
				.http_version(1, 1)
				.add_header("Host", "bbs.csdn.net:80")
				.add_header("Connection", "keep-alive")
				.add_header("User-Agent", "Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/61.0.3163.100 Safari/537.36")
				.add_header("Upgrade-Insecure-Requests", "1")
				.add_header("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8")
				.add_header("Accept-Encoding", "gzip, deflate, br")
				.add_header("Accept-Language", "zh-CN,zh;q=0.8")
				.uri("/");

			auto msg = req.data();

			http_client.send(msg.data());
		}

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
