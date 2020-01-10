/*
g++ -c -x c++ /root/projects/client/demo/src/client.cpp -I /usr/local/include -I /root/projects/client/ -g2 -gdwarf-2 -o "/root/projects/client/obj/x64/Debug/client/client.o" -Wall -Wswitch -W"no-deprecated-declarations" -W"empty-body" -Wconversion -W"return-type" -Wparentheses -W"no-format" -Wuninitialized -W"unreachable-code" -W"unused-function" -W"unused-value" -W"unused-variable" -O0 -fno-strict-aliasing -fno-omit-frame-pointer -fthreadsafe-statics -fexceptions -frtti -std=c++17
g++ -o "/root/projects/client/bin/x64/Debug/client.out" -Wl,--no-undefined -Wl,-L/usr/local/lib -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -lpthread -lrt -ldl /root/projects/client/obj/x64/Debug/client/client.o -lboost_system -lssl -lcrypto
*/

#include <asio2/asio2.hpp>
#include <iostream>

#include "tcp_client.hpp"
//#include "tcp_client_character.hpp"
//#include "tcp_client_dgram.hpp"
//#include "tcp_client_match_role.hpp"
//#include "udp_client.hpp"
//#include "udp_client_kcp.hpp"
//#include "udp_cast.hpp"
//#include "http_client.hpp"
//#include "websocket_client.hpp"
//#include "rpc_client.hpp"
//#include "ping_test.hpp"
//#include "serial_port.hpp"

int main(int argc, char *argv[])
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system,linux has't these function
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	std::string_view host = "127.0.0.1", port = "8080";
	//std::string_view host = "192.168.1.146", port = "8080"; 
	//port = argv[1];

	run_tcp_client(host, port);
	//run_tcp_client_character(host, port);
	//run_tcp_client_dgram(host, port);
	//run_tcp_client_match_role(host, port);
	//run_udp_client(host, port);
	//run_udp_client_kcp(host, port);
	//run_http_client(host, port);
	//run_ws_client(host, port);
	//run_rpc_client(host, port);

	//run_tcps_client(host, port);
	//run_https_client(host, port);
	//run_wss_client(host, port);

	//run_ping_test();
	//run_serial_port("COM1", 9600); // for windows
	//run_serial_port("/dev/ttyS0", 9600); // for linux
	//run_udp_cast(host, port);

#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	//system("pause");
#endif

	return 0;
};
