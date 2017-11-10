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

#define USE_SSL
#include <asio2/asio2.hpp>

class user_udp_server_listener : public asio2::udp_server_listener
{
public:
	virtual void on_send(asio2::session_ptr session_ptr, asio2::buffer_ptr data_ptr, int error) override
	{
	}
	virtual void on_recv(asio2::session_ptr session_ptr, asio2::buffer_ptr data_ptr) override
	{
		session_ptr->send(data_ptr);
	}
	virtual void on_close(asio2::session_ptr session_ptr, int error) override
	{
	}
	virtual void on_listen() override
	{
	}
	virtual void on_accept(asio2::session_ptr session_ptr) override
	{
	}
	virtual void on_shutdown(int error) override
	{
	}
};

class user_tcp_client_listener : public asio2::tcp_client_listener_t<uint8_t>
{
public:
	virtual void on_send(asio2::buffer_ptr data_ptr, int error) override
	{
	}
	virtual void on_recv(asio2::buffer_ptr data_ptr) override
	{
	}
	virtual void on_close(int error) override
	{
	}
	virtual void on_connect(int error) override
	{
	}
};

volatile bool run_flag = true;

class main_frame
{
public:

	void on_recv(asio2::session_ptr session_ptr, asio2::buffer_ptr data_ptr, int num)
	{
		session_ptr->send(data_ptr);
	}
};


std::size_t g_recvd_total_packet_count = 0;
std::size_t g_recvd_total_bytes = 0;
std::size_t g_session_count = 0;


// head 1 byte <
// len  1 byte content len,not include head and tail,just include the content len
// ...  content
// tail 1 byte >

std::size_t pack_parser(asio2::buffer_ptr data_ptr)
{
	if (data_ptr->size() < 3)
		return asio2::need_more_data;

	uint8_t * data = data_ptr->data();
	if (data[0] == '<')
	{
		std::size_t total_len = data[1] + 3;
		if (data_ptr->size() < total_len)
			return asio2::need_more_data;
		if (data[total_len - 1] == '>')
			return total_len;
	}

	return asio2::invalid_data;
}


//http://blog.csdn.net/jiange_zh/article/details/50639178

//typedef std::map<std::string, std::string> header_t;
//typedef header_t::iterator header_iter_t;
//
//struct HttpRequest
//{
//	std::string http_method;
//	std::string http_url;
//	//std::string http_version;
//
//	header_t    http_headers;
//	std::string http_header_field; //field is waiting for value while parsing
//
//	std::string http_body;
//};
//
//struct HttpResponse
//{
//	//std::string http_version;
//	int         http_code;
//	std::string http_phrase;
//
//	header_t    http_headers;
//
//	std::string http_body;
//
//	std::string GetResponse();
//	void        ResetResponse();
//};
//
//std::string HttpResponse::GetResponse()
//{
//	std::ostringstream ostream;
//	ostream << "HTTP/1.1" << " " << http_code << " " << http_phrase << "\r\n"
//		<< "Connection: keep-alive" << "\r\n";
//
//	header_iter_t iter = http_headers.begin();
//
//	while (iter != http_headers.end())
//	{
//		ostream << iter->first << ": " << iter->second << "\r\n";
//		++iter;
//	}
//	ostream << "Content-Length: " << http_body.size() << "\r\n\r\n";
//	ostream << http_body;
//
//	return ostream.str();
//}
//
//void HttpResponse::ResetResponse()
//{
//	//http_version = "HTTP/1.1";
//	http_code = 200;
//	http_phrase = "OK";
//
//	http_body.clear();
//	http_headers.clear();
//}
//
//HttpRequest        *http_request_parser;    //解析时用
//HttpRequest        *http_request_process;   //处理请求时用
//HttpResponse        http_response;
//HttpParser          http_parser;
//
//class HttpParser
//{
//public:
//	void InitParser(Connection *con);
//	int  HttpParseRequest(const std::string &inbuf);
//
//	static int OnMessageBeginCallback(http_parser *parser);
//	static int OnUrlCallback(http_parser *parser, const char *at, size_t length);
//	static int OnHeaderFieldCallback(http_parser *parser, const char *at, size_t length);
//	static int OnHeaderValueCallback(http_parser *parser, const char *at, size_t length);
//	static int OnHeadersCompleteCallback(http_parser *parser);
//	static int OnBodyCallback(http_parser *parser, const char *at, size_t length);
//	static int OnMessageCompleteCallback(http_parser *parser);
//
//private:
//	http_parser          parser;
//	http_parser_settings settings;
//};
//
///*
//* 调用http_parser_execute
//* 在该函数执行期间，将调用一系列回调函数
//*/
//int HttpParser::HttpParseRequest(const std::string &inbuf)
//{
//	int nparsed = http_parser_execute(&parser, &settings, inbuf.c_str(), inbuf.size());
//
//	if (parser.http_errno != HPE_OK)
//	{
//		return -1;
//	}
//
//	return nparsed;
//}
//
///* 初始化http_request_parser */
//int HttpParser::OnMessageBeginCallback(http_parser *parser)
//{
//	Connection *con = (Connection*)parser->data;
//
//	con->http_request_parser = new HttpRequest();
//
//	return 0;
//}
//
///* 将解析好的url赋值给http_url */
//int HttpParser::OnUrlCallback(http_parser *parser, const char *at, size_t length)
//{
//	Connection *con = (Connection*)parser->data;
//
//	con->http_request_parser->http_url.assign(at, length);
//
//	return 0;
//}
//
///* 将解析到的header_field暂存在http_header_field中 */
//int HttpParser::OnHeaderFieldCallback(http_parser *parser, const char *at, size_t length)
//{
//	Connection *con = (Connection*)parser->data;
//
//	con->http_request_parser->http_header_field.assign(at, length);
//
//	return 0;
//}
//
///* 将解析到的header_value跟header_field一一对应 */
//int HttpParser::OnHeaderValueCallback(http_parser *parser, const char *at, size_t length)
//{
//	Connection      *con = (Connection*)parser->data;
//	HttpRequest *request = con->http_request_parser;
//
//	request->http_headers[request->http_header_field] = std::string(at, length);
//
//	return 0;
//}
//
///* 参照官方文档 */
//int HttpParser::OnHeadersCompleteCallback(http_parser *parser)
//{
//	Connection *con = (Connection*)parser->data;
//	HttpRequest *request = con->http_request_parser;
//	request->http_method = http_method_str((http_method)parser->method);
//	return 0;
//}
//
///* 本函数可能被调用不止一次，因此使用append */
//int HttpParser::OnBodyCallback(http_parser *parser, const char *at, size_t length)
//{
//	Connection *con = (Connection*)parser->data;
//
//	con->http_request_parser->http_body.append(at, length);
//
//	return 0;
//}
//
///* 将解析完毕的消息放到消息队列中 */
//int HttpParser::OnMessageCompleteCallback(http_parser *parser)
//{
//	Connection *con = (Connection*)parser->data;
//	HttpRequest *request = con->http_request_parser;
//
//	con->req_queue.push(request);
//	con->http_request_parser = NULL;
//	return 0;
//}
//
//http_parser_settings settings;
//settings.on_url = my_url_callback;
//settings.on_header_field = my_header_field_callback;
///* ... */
//
//http_parser *parser = malloc(sizeof(http_parser));
//http_parser_init(parser, HTTP_REQUEST);
//parser->data = my_socket;

//
//// 设置回调
//http_parser_settings settings;
//settings.on_url = my_url_callback;
//settings.on_header_field = my_header_field_callback;
///* ... */
//
//// 为结构体申请内存
//http_parser *parser = malloc(sizeof(http_parser));
//// 初始化解析器
//http_parser_init(parser, HTTP_REQUEST);
//// 设置保存调用者的数据，用于在callback内使用
//parser->data = my_socket;
//
//size_t len = 80 * 1024;   // 需要接受的数据大小80K
//size_t nparsed;         // 已经解析完成的数据大小
//char buf[len];          // 接收缓存
//ssize_t recved;         // 实际接收到的数据大小
//
//						// 接受数据
//recved = recv(fd, buf, len, 0);
//
//// 如果接收到的字节数小于0，说明从socket读取出错
//if (recved < 0) {
//	/* Handle error. */
//}
//
///* Start up / continue the parser.
//* Note we pass recved==0 to signal that EOF has been recieved.
//*/
//// 开始解析
//// @parser 解析器对象
//// @&settings 解析时的回调函数
//// @buf 要解析的数据
//// @receved 要解析的数据大小
//nparsed = http_parser_execute(parser, &settings, buf, recved);
//
//// 如果解析到websocket请求
//if (parser->upgrade) {
//	/* handle new protocol */
//	// 如果解析出错，即解析完成的数据大小不等于传递给http_parser_execute的大小
//}
//else if (nparsed != recved) {
//	/* Handle error. Usually just close the connection. */
//}
//

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

	//main_frame _main_frame;
	user_tcp_client_listener _user_tcp_client_listener;

	// in some situations, the asio2 stop function will block forever,so use a loop with start and stop to check the problem. 
	while (run_flag)
	{
		time_t aclock;
		time(&aclock);                 /* Get time in seconds */
		struct tm * newtime = localtime(&aclock);  /* Convert time to struct */
		char tmpbuf[128] = { 0 };
		strftime(tmpbuf, 128, "%Y-%m-%d %H:%M:%S\n", newtime);
		printf("%s", tmpbuf);

		int i = 0;
		const int client_count = 10;

		//GET / HTTP/1.1
		//Host: 127.0.0.1:8443
		//Connection: keep-alive
		//Cache-Control: max-age=0
		//User-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/61.0.3163.100 Safari/537.36
		//Upgrade-Insecure-Requests: 1
		//Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8
		//Accept-Encoding: gzip, deflate, br
		//Accept-Language: zh-CN,zh;q=0.8

		//GET /DataSvr/api/tag/ModTag?id=4506898531877019&name=tag004&peoplename=WangWu2&rfid=rfid001&department=depart001 HTTP/1.1
		//Host: localhost:8443
		//Connection: keep-alive
		//User-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/61.0.3163.100 Safari/537.36
		//Upgrade-Insecure-Requests: 1
		//Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8
		//Accept-Encoding: gzip, deflate, br
		//Accept-Language: zh-CN,zh;q=0.8

		//GET /DataSvr/api/anchor/AddAnchor?json=%7b%22id%22:4990560701320869680,%22name%22:%22anchor222%22,%22ip%22:%22192.168.0.101%22%7d HTTP/1.1
		//Host: localhost:8443
		//Connection: keep-alive
		//User-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/61.0.3163.100 Safari/537.36
		//Upgrade-Insecure-Requests: 1
		//Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8
		//Accept-Encoding: gzip, deflate, br
		//Accept-Language: zh-CN,zh;q=0.8

		const char * url = "http://localhost:8443/DataSvr/api/anchor/AddAnchor?json=%7b%22id%22:4990560701320869680,%22name%22:%22anchor222%22,%22ip%22:%22192.168.0.101%22%7d";
		asio2::http::http_parser_url u;
		asio2::http::http_parser_parse_url(url, std::strlen(url), 0, &u);

		asio2::server http_server("http://*:8443");
		http_server.bind_recv([&http_server](asio2::session_ptr session_ptr, asio2::buffer_ptr data_ptr)
		{
			char buf[1024] = { 0 };
			std::memcpy((void *)buf, (const void *)data_ptr->data(), data_ptr->size());
			buf[data_ptr->size()] = '\0';
			char * s = 0;
		});
		if(!http_server.start())
			std::printf("start http server failed : %d - %s.\n", asio2::get_last_error(), asio2::get_last_error_desc().c_str());
		else
			std::printf("start http server successed : %s - %u\n", http_server.get_listen_address().c_str(), http_server.get_listen_port());

		asio2::server tcps_server("tcps://*:9443");
		tcps_server.bind_recv([](asio2::session_ptr session_ptr, asio2::buffer_ptr data_ptr)
		{
		});
		if (!tcps_server.start())
			std::printf("start tcps server failed : %d - %s.\n", asio2::get_last_error(), asio2::get_last_error_desc().c_str());
		else
			std::printf("start tcps server successed : %s - %u\n", tcps_server.get_listen_address().c_str(), tcps_server.get_listen_port());

		asio2::client tcps_client("tcps://127.0.0.1:9443");
		tcps_client.bind_recv([] (asio2::buffer_ptr data_ptr)
		{
		});
		if(!tcps_client.start())
			std::printf("start tcps client failed : %d - %s.\n", asio2::get_last_error(), asio2::get_last_error_desc().c_str());
		else
			std::printf("start tcps client successed : %s - %u\n", tcps_client.get_remote_address().c_str(), tcps_client.get_remote_port());

		//-----------------------------------------------------------------------------------------
		std::shared_ptr<asio2::server> tcp_auto_server = std::make_shared<asio2::server>(" tcp://*:8088/auto?notify_mode=async&send_buffer_size=1024k & recv_buffer_size=1024K & pool_buffer_size=1024 & io_service_pool_size=3 ");
		tcp_auto_server->bind_recv([&tcp_auto_server](asio2::session_ptr session_ptr, asio2::buffer_ptr data_ptr)
		{
			session_ptr->send(data_ptr); 
		}).bind_accept([](asio2::session_ptr session_ptr)
		{
			session_ptr->set_user_data(session_ptr);
		}).bind_close([](asio2::session_ptr session_ptr, int error)
		{
			session_ptr->set_user_data(nullptr);
		});
		if(!tcp_auto_server->start())
			std::printf("start tcp server failed : %d - %s.\n", asio2::get_last_error(), asio2::get_last_error_desc().c_str());
		else
			std::printf("start tcp server successed : %s - %u\n", tcp_auto_server->get_listen_address().c_str(), tcp_auto_server->get_listen_port());

		std::shared_ptr<asio2::client> tcp_auto_client_ptr[client_count];
		for (i = 0; i < client_count; i++)
		{
			tcp_auto_client_ptr[i] = std::make_shared<asio2::client>("tcp://localhost:8088/auto");
			tcp_auto_client_ptr[i]->bind_recv([](asio2::buffer_ptr data_ptr)
			{
			});
			if (!tcp_auto_client_ptr[i]->start(false))
				std::printf("connect to tcp server failed : %d - %s. %d\n", asio2::get_last_error(), asio2::get_last_error_desc().c_str(), i);
			//else
			//	std::printf("connect to tcp server successed : %s - %u\n", tcp_auto_client[i].get_remote_address().c_str(), tcp_auto_client[i].get_remote_port());
		}


		//-----------------------------------------------------------------------------------------
		asio2::server tcp_pack_server(" tcp://*:8099/pack?send_buffer_size=1024k & recv_buffer_size=1024K & pool_buffer_size=1024 & io_service_pool_size=3");
		tcp_pack_server.bind_recv([&tcp_pack_server](asio2::session_ptr session_ptr, asio2::buffer_ptr data_ptr)
		{
			char * p = (char*)data_ptr->data();
			std::string s;
			s.resize(data_ptr->size());
			std::memcpy((void*)s.c_str(), (const void *)p, data_ptr->size());
			//std::printf("tcp_pack_server recv : %s\n", s.c_str());

			int send_len = std::rand() % ((int)data_ptr->size() / 2);
			int already_send_len = 0;
			while (true)
			{
				session_ptr->send((const uint8_t *)(p + already_send_len), (std::size_t)send_len);
				already_send_len += send_len;

				if ((std::size_t)already_send_len >= data_ptr->size())
					break;

				send_len = std::rand() % ((int)data_ptr->size() / 2);
				if (send_len + already_send_len > (int)data_ptr->size())
					send_len = (int)data_ptr->size() - already_send_len;
			}
			
		}).set_pack_parser(std::bind(pack_parser, std::placeholders::_1));
		if (!tcp_pack_server.start())
			std::printf("start tcp server failed : %d - %s.\n", asio2::get_last_error(), asio2::get_last_error_desc().c_str());
		else
			std::printf("start tcp server successed : %s - %u\n", tcp_pack_server.get_listen_address().c_str(), tcp_pack_server.get_listen_port());

		std::shared_ptr<asio2::client> tcp_pack_client[client_count];
		for (i = 0; i < client_count; i++)
		{
			tcp_pack_client[i] = std::make_shared<asio2::client>("tcp://localhost:8099/pack");
			tcp_pack_client[i]->bind_recv([](asio2::buffer_ptr data_ptr)
			{
				char * p = (char*)data_ptr->data();
				std::string s;
				s.resize(data_ptr->size());
				std::memcpy((void*)s.c_str(), (const void *)p, data_ptr->size());
				//std::printf("tcp_pack_client recv : %s\n", s.c_str());

			}).bind_send([&tcp_pack_client](asio2::buffer_ptr data_ptr,int error)
			{
				//char * p = (char*)data_ptr->data();
				//std::string s;
				//s.resize(data_ptr->size());
				//std::memcpy((void*)s.c_str(), (const void*)data_ptr->data(), data_ptr->size());
				//std::printf("tcp client send : %s\n", s.c_str());
			}).set_pack_parser(std::bind(pack_parser, std::placeholders::_1));
			if (!tcp_pack_client[i]->start(false))
				std::printf("connect to tcp server failed : %d - %s. %d\n", asio2::get_last_error(), asio2::get_last_error_desc().c_str(), i);
			//else
			//	std::printf("connect to tcp server successed : %s - %u\n", tcp_pack_client[i].get_remote_address().c_str(), tcp_pack_client[i].get_remote_port());
		}

		asio2::spin_lock lock;
		//-----------------------------------------------------------------------------------------
		asio2::server udp_server("udp://*:9530/?send_buffer_size=256m&recv_buffer_size=256m&pool_buffer_size=1024");
		//udp_server.bind_listener(std::make_shared<user_udp_server_listener>());
		//udp_server.bind_recv(std::bind(&main_frame::on_recv,&_main_frame,std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, 10));
		udp_server.bind_recv([&udp_server,&lock](asio2::session_ptr session_ptr, asio2::buffer_ptr data_ptr)
		{
			//char * p = (char*)data_ptr->data();
			//*(p + data_ptr->size() - 1) = 0;
			//std::printf("udp server recv : %d %s\n", data_ptr->size(), p);
			std::lock_guard<asio2::spin_lock> g(lock);
			{
				g_recvd_total_packet_count++;
				g_recvd_total_bytes += data_ptr->size();

				static std::clock_t s_start = std::clock();
				static std::clock_t c_start = std::clock();

				std::clock_t c_end = std::clock();
				if (c_end - c_start > 3000)
				{
					std::printf("%-5lld %6dms %7lldpackets %9lldbytes < %-5lldpackets/sec %5.2fMB bytes/sec >\n",  
						g_session_count,
						c_end - s_start, 
						g_recvd_total_packet_count, 
						g_recvd_total_bytes, 
						g_recvd_total_packet_count * 1000 / (c_end - s_start),
						(double)g_recvd_total_bytes * (double)1000 / (double)(c_end - s_start) / (double)1024 / (double)1024
						);

					c_start = std::clock();
				}
			}

			session_ptr->send(data_ptr);
			
			//session_ptr->send("0");
		}).bind_accept([](asio2::session_ptr session_ptr)
		{
			g_session_count++;
			//static int i = 1;
			//std::printf("udp session enter %2d: %s %d\n", i++, session_ptr->get_remote_address().c_str(), session_ptr->get_remote_port());
		}).bind_close([](asio2::session_ptr session_ptr, int error)
		{
			g_session_count--;
		});
		if(!udp_server.start())
			std::printf("start udp server failed : %d - %s.\n", asio2::get_last_error(), asio2::get_last_error_desc().c_str());
		else
			std::printf("start udp server successed : %s - %u\n", udp_server.get_listen_address().c_str(), udp_server.get_listen_port());

		std::shared_ptr<asio2::client> udp_client[client_count];
		for (i = 0; i < client_count; i++)
		{
			udp_client[i] = std::make_shared<asio2::client>("udp://localhost:9530");
			udp_client[i]->bind_recv([&udp_client](asio2::buffer_ptr data_ptr)
			{
				//char * p = (char*)data_ptr->data();
				//*(p + data_ptr->size() - 1) = 0;
				//std::printf("udp client recv : %s\n", p);
			}).bind_send([&udp_client](asio2::buffer_ptr data_ptr, int error)
			{
				//char * p = (char*)data_ptr->data();
				//*(p + data_ptr->size() - 1) = 0;
				//std::printf("udp client send :%lld %s\n", data_ptr->size(), p);
			});;
			if (!udp_client[i]->start(false))
			{
				if (asio2::get_last_error() != 24)
					std::printf("connect to udp server failed : %d - %s.\n %d", asio2::get_last_error(), asio2::get_last_error_desc().c_str(), i);
			}
			//else
			//	std::printf("connect to udp server successed : %s - %u\n", udp_client[i].get_remote_address().c_str(), udp_client[i].get_remote_port());
		}


		//-----------------------------------------------------------------------------------------
		std::thread([&]()
		{
			int count = 0,seconds = 30 + std::rand() % 6;
			//while (run_flag && count++ < seconds)
			while (run_flag )
			{
				static bool flag = false;
				//if (!flag)
				{
					//for (i = 0; i < client_count; i++)
					//	tcp_auto_client[i].send("<tcp abc 123>");
					for (i = 0; i < client_count; i++)
					{
						std::string s;
						for (int i = 0, len = 10 + std::rand() % 100; i < len; i++)
						{
							s += (char)(std::rand() % 26) + 'a';
						}
						tcp_auto_client_ptr[i]->send(s.c_str());
					}
					for (i = 0; i < client_count; i++)
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

						int packet_send_times = 0;
						int send_len = std::rand() % (len / 2);
						int already_send_len = 0;
						while (true)
						{
							packet_send_times++;
							tcp_pack_client[i]->send((const uint8_t *)(s.c_str() + already_send_len), (std::size_t)send_len);
							already_send_len += send_len;

							if (already_send_len >= len)
								break;

							send_len = std::rand() % (len / 2);
							if (send_len + already_send_len > len)
								send_len = len - already_send_len;
						}

						//std::printf("split packet count : %d\n", packet_send_times);
					}
					for (i = 0; i < client_count; i++)
						udp_client[i]->send("<udp abc 123 456>");
					for (i = 0; i < client_count; i++)
						udp_client[i]->send("<udp abc 123 456 789 xyz>");

					flag = true;
				}

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
