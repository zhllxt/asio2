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

class main_app : public asio2::http_server_listener
{
public:
	/// construct  
	main_app() : http_server("http://*:9080")
	{
		http_server.bind_listener(this);

		if (!http_server.start())
			std::printf("start http server failed : %d - %s.\n", asio2::get_last_error(), asio2::get_last_error_desc().c_str());
		else
			std::printf("start http server successed : %s - %u\n", http_server.get_listen_address().c_str(), http_server.get_listen_port());
	}

public:
	virtual void on_send(asio2::session_ptr & session_ptr, asio2::response_ptr & response_ptr, int error) override
	{
	}
	virtual void on_recv(asio2::session_ptr & session_ptr, asio2::request_ptr & request_ptr) override
	{
		auto iska = request_ptr->is_keepalive();
		auto host = request_ptr->host();
		auto port = request_ptr->port();
	}
	virtual void on_close(asio2::session_ptr & session_ptr, int error) override
	{
		printf("http session leave %s %u %s\n", session_ptr->get_remote_address().c_str(), session_ptr->get_remote_port(), asio2::get_error_desc(error).data());
	}
	virtual void on_listen() override
	{
	}
	virtual void on_accept(asio2::session_ptr & session_ptr) override
	{
		printf("http session enter %s %u\n", session_ptr->get_remote_address().c_str(), session_ptr->get_remote_port());
	}
	virtual void on_shutdown(int error) override
	{
	}

protected:
	asio2::server http_server;

};
#include <strstream>
int main(int argc, char *argv[])
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system,linux has't these function
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// the number is the memory leak line num of the vs output window content.
	//_CrtSetBreakAlloc(1640); 
#endif

	std::signal(SIGINT, [](int signal) { run_flag = false; });

	while (run_flag)
	{
		main_app app;

		// show the use of http_parser_parse_url function
		const char * url = "http://localhost:8080/engine/api/user/adduser?json=%7b%22id%22:4990560701320869680,%22name%22:%22admin%22%7d";
		//const char * url = "http://localhost:8080/engine/api/user/adduser?json=[\"id\":4990560701320869680,\"name\":\"admin\"]";
		//const char * url = "http://localhost:8080/engine/api/user/adduser";
		asio2::http::http_parser_url u;
		if (0 == asio2::http::http_parser_parse_url(url, strlen(url), 0, &u))
		{
			std::string s;
			uint16_t port = 0;
			if (u.field_set & (1 << asio2::http::UF_PORT))
				port = u.port;
			else
				port = 80;

			if (u.field_set & (1 << asio2::http::UF_SCHEMA))
			{
				s.resize(u.field_data[asio2::http::UF_SCHEMA].len, '\0');
				strncpy((char *)s.data(), url + u.field_data[asio2::http::UF_SCHEMA].off, u.field_data[asio2::http::UF_SCHEMA].len);
			}

			if (u.field_set & (1 << asio2::http::UF_HOST))
			{
				s.resize(u.field_data[asio2::http::UF_HOST].len, '\0');
				strncpy((char *)s.data(), url + u.field_data[asio2::http::UF_HOST].off, u.field_data[asio2::http::UF_HOST].len);
			}

			if (u.field_set & (1 << asio2::http::UF_PATH))
			{
				s.resize(u.field_data[asio2::http::UF_PATH].len, '\0');
				strncpy((char *)s.data(), url + u.field_data[asio2::http::UF_PATH].off, u.field_data[asio2::http::UF_PATH].len);
			}

			if (u.field_set & (1 << asio2::http::UF_QUERY))
			{
				s.resize(u.field_data[asio2::http::UF_QUERY].len, '\0');
				strncpy((char *)s.data(), url + u.field_data[asio2::http::UF_QUERY].off, u.field_data[asio2::http::UF_QUERY].len);
			}
		}

		//-----------------------------------------------------------------------------------------
		std::thread([&]()
		{
			while (run_flag)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
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
