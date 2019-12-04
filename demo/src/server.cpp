/*
> Additional dependency Libraries: (stdc++fs; is used for gcc std::filesystem)
boost_system;ssl;crypto;stdc++fs;
> Compile:
g++ -c -x c++ /root/projects/server/demo/src/server.cpp -I /usr/local/include -I /root/projects/server -g2 -gdwarf-2 -o "/root/projects/server/obj/x64/Debug/server/server.o" -Wall -Wswitch -W"no-deprecated-declarations" -W"empty-body" -Wconversion -W"return-type" -Wparentheses -W"no-format" -Wuninitialized -W"unreachable-code" -W"unused-function" -W"unused-value" -W"unused-variable" -O0 -fno-strict-aliasing -fno-omit-frame-pointer -fthreadsafe-statics -fexceptions -frtti -std=c++17
> Link:
g++ -o "/root/projects/server/bin/x64/Debug/server.out" -Wl,--no-undefined -Wl,-L/usr/local/lib -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -lpthread -lrt -ldl /root/projects/server/obj/x64/Debug/server/server.o -lboost_system -lssl -lcrypto
*/

#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

#include <asio2/asio2.hpp>
#include <iostream>

//#include "tcp_server.hpp"
//#include "tcp_server_character.hpp"
//#include "tcp_server_dgram.hpp"
//#include "tcp_server_match_role.hpp"
//#include "udp_server.hpp"
//#include "udp_server_kcp.hpp"
//#include "http_server.hpp"
//#include "websocket_server.hpp"
//#include "httpws_server.hpp"
#include "rpc_server.hpp"


int main(int argc, char *argv[])
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system,linux has't these function
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif


	asio2::timer timer; // test timer
	timer.start_timer(1, std::chrono::seconds(1), [&]()
	{
		printf("timer 1\n");
		timer.stop_timer(1);
	});

	std::string_view host = "0.0.0.0", port = "8080";
	//port = argv[1];

	//run_tcp_server(host, port);
	//run_tcp_server_character(host, port);
	//run_tcp_server_dgram(host, port);
	//run_tcp_server_match_role(host, port);
	//run_udp_server(host, port);
	//run_udp_server_kcp(host, port);
	//run_http_server(host, port);
	//run_ws_server(host, port);
	//run_httpws_server(host, port);
	run_rpc_server(host, port);

	//run_tcps_server(host, port);
	//run_https_server(host, port);
	//run_wss_server(host, port);
	//run_httpwss_server(host, port);

#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	//system("pause");
#endif

	return 0;
};
