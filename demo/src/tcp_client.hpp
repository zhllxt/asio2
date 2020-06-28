#pragma once

#include <asio2/asio2.hpp>

void on_recv(std::string_view sv)
{
	printf("1recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());
}

class listener
{
public:
	void on_recv(std::string_view sv)
	{
		printf("2recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());
	}
};

struct info
{
	int id;
	char name[20];
	int8_t age;
};

#ifdef ASIO_STANDALONE
namespace asio {
#else
namespace boost::asio {
#endif
	inline asio::const_buffer buffer(const info& u) noexcept
	{
		return asio::const_buffer(&u, sizeof(info));
	}
} // namespace asio

void run_tcp_client(std::string_view host, std::string_view port)
{
	int count = 1;
	std::unique_ptr<asio2::tcp_client[]> clients = std::make_unique<asio2::tcp_client[]>(count);
	//while (1) // use infinite loop and sleep 2 seconds to test start and stop
	//for (int i = 0; i < 1000; i++)
	{
		//listener lis;
		for (int i = 0; i < count; ++i)
		{
			auto & client = clients[i];
			//// == default reconnect option is "enable" ==
			//client.auto_reconnect(false); // disable auto reconnect
			//client.auto_reconnect(true); // enable auto reconnect and use the default delay
			client.auto_reconnect(true, std::chrono::milliseconds(1000)); // enable auto reconnect and use custom delay
			client.start_timer(1, std::chrono::seconds(1), []() {}); // test timer
			client.bind_connect([&](asio::error_code ec)
			{
				if (asio2::get_last_error())
					printf("connect failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
				else
					printf("connect success : %s %u\n", client.local_address().c_str(), client.local_port());

				std::string s;
				//s += '<';
				//int len = 128 + std::rand() % (300);
				//for (int i = 0; i < len; i++)
				//{
				//	s += (char)((std::rand() % 26) + 'a');
				//}
				//s += '>';

				s += '#';
				s += char(1);
				s += 'a';

				// ## All of the following ways of send operation are correct.
				// (send operation is running in the io.strand thread, the content will be sent directly)
				client.send(s);
				client.send(s, []() {});
				client.send((uint8_t*)"<abcdefghijklmnopqrstovuxyz0123456789>", 10);
				client.send((const uint8_t*)"<abcdefghijklmnopqrstovuxyz0123456789>", 10);
				client.send(s.data(), int(s.size()));
				client.send(s.data(), []() {});
				client.send(s.c_str(), size_t(s.size()));
				client.send(s, asio::use_future);
				client.send("<abcdefghijklmnopqrstovuxyz0123456789>", asio::use_future);
				client.send(s.data(), asio::use_future);
				client.send(s.c_str(), asio::use_future);
				int narys[2] = { 1,2 };
				client.send(narys);
				client.send(narys, []() {std::cout << asio2::last_error_msg() << std::endl; }); // callback with no params
				client.send(narys, [](std::size_t bytes) {}); // callback with param
				client.send(narys, asio::use_future);

				//asio::write(client.socket(), asio::buffer(s));
			}).bind_disconnect([&](asio::error_code ec)
			{
				printf("disconnect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
			}).bind_recv([&](std::string_view sv)
			{
				printf("recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());

				std::string s;
				s += '#';
				uint8_t len = uint8_t(100 + (std::rand() % 100));
				s += char(len);
				for (uint8_t i = 0; i < len; i++)
				{
					s += (char)((std::rand() % 26) + 'a');
				}
				client.send(std::move(s), []() {});

				//asio::write(client.socket(), asio::buffer(sv));
			})
				//.bind_recv(on_recv)//bind global function
				//.bind_recv(std::bind(&listener::on_recv, &lis, std::placeholders::_1))//bind member function
				//.bind_recv(&listener::on_recv, lis)//bind member function
				//.bind_recv(&listener::on_recv, &lis)//bind member function
				;
			//client.async_start(host, port);
			client.start(host, port);

			// ##Use this to check whether the send operation is running in current thread.
			//if (client.io().strand().running_in_this_thread())
			//{
			//}

			// ## All of the following ways of send operation are correct.
			// (beacuse these send operations is not running in the io.strand thread,
			// then the content will be sent asynchronous)
			std::string s;
			s += '#';
			s += char(1);
			s += 'a';
			if (client.is_started())
			{
				client.send(s);
				client.send(s, []() {});
				client.send((uint8_t*)"<abcdefghijklmnopqrstovuxyz0123456789>", 10);
				client.send((const uint8_t*)"<abcdefghijklmnopqrstovuxyz0123456789>", 10);
				client.send(s.data(), int(s.size()));
				client.send(s.data(), []() {});
				client.send(s.c_str(), size_t(s.size()));
				client.send(s, asio::use_future);
				client.send("<abcdefghijklmnopqrstovuxyz0123456789>", asio::use_future);
				client.send(s.data(), asio::use_future);
				client.send(s.c_str(), asio::use_future);
				int narys[2] = { 1,2 };
				client.send(narys);
				client.send(narys, []() {});
				client.send(narys, [](std::size_t bytes) {});
				client.send(narys, asio::use_future);

				//// ##Example how to send a struct directly:
				//info u;
				//client.send(u);

				// ##Thread-safe send operation example :
				//client.post([&client]()
				//{
				//	if (client.is_started())
				//		asio::write(client.stream(), asio::buffer(std::string("abcdefghijklmn")));
				//});
			}
		}

		while (std::getchar() != '\n');
		//std::this_thread::sleep_for(std::chrono::milliseconds(500));
		
		for (int i = 0; i < count; ++i)
		{
			auto & client = clients[i];
			client.stop();
		}
	}
}

void run_tcps_client(std::string_view host, std::string_view port)
{
#ifdef ASIO2_USE_SSL
	std::string_view client_key = R"(
-----BEGIN RSA PRIVATE KEY-----
Proc-Type: 4,ENCRYPTED
DEK-Info: DES-EDE3-CBC,78002969818515BD

Aj9fTQJNqbwYlB+0Sy/DToX1Ozc30cbSV2rEyPpZ+xI4yXpgyWP2QzWUNGdazc8A
Eu32ex2d3pw3E6sBxsvT0jR5pATMWMt2HHnRByvt0yzO10MY/0vLjjQ6LDmNEeoz
/VnwrJwgWrsfTxD9P7Crl7XKlajkEjIahZzS6cOlyh3BSjo34g1+WRIps2CyzVo8
+dSlRIJ9BsNFBTm9lqLBsSIEBt7Xx10PfpJms4tb2yDdk46A/zgj16jgGNopYKxm
zeHwRuI/lsX7Ko6t1p6L3j1QQLILR3hjbowxkh2NBE+3eoTxf0R/jBIL4XxCfxUX
uKdPwbMBfvDqb2crkiDdak/r8yS93QJGK1NbS4OVm1AXWswr+dQhSB690bFo8H5t
blG+wx3b3xYgTsrVDXNYTktPy+XwQkQHr72rcLkcJJl0+Og5vrvyEoxCVHbN+jAQ
GS9aJ+BKCh9WlcVqRclLiXGSDT95u7B/CvONHwgLlDSO+nHUELIhph9k7Gal15aW
vaIDTr5PKSXRJAALyqYLr1RsV2lJi/sgCDEssJKSfluIla6yk+chnaGb/J1Emkkd
ZVV/+N7uqn4QkEcQzs+rp/ssnTT+w6cvf2NUajNHGrJ9YVikp+vtQY2DkFxADrOV
RgDjj0z0YfNLueRHVNlUwlgf9b5HGPR+lnINhK8+kwEoHuPoWi2Ueq0x2aEPJDLv
g+wk9EPTPksQv0g5rJqjkiJ3iV/Esm7AGztyQgv3nFabZM2zhHUJ4UBpOxrg7LkE
vD1dm8qIDD7u6mZ2khvfD/8RPssJHjA2oHI16GmeMYist0Vd574gGA==
-----END RSA PRIVATE KEY-----
)";

	std::string_view client_crt = R"(
Certificate:
    Data:
        Version: 3 (0x2)
        Serial Number: 2 (0x2)
    Signature Algorithm: sha256WithRSAEncryption
        Issuer: C=CN, ST=HN, L=ZZ, O=LOC, OU=SOFT, CN=ZHLCA/emailAddress=37792738@QQ.COM
        Validity
            Not Before: Jun 24 14:14:42 2020 GMT
            Not After : Jun 24 14:14:42 2021 GMT
        Subject: C=CN, ST=HN, O=LOC, OU=SOFT, CN=ZHLC/emailAddress=37792738@QQ.COM
        Subject Public Key Info:
            Public Key Algorithm: rsaEncryption
                Public-Key: (1024 bit)
                Modulus:
                    00:95:0b:4b:84:fc:c8:93:7b:02:19:1f:da:fe:69:
                    f3:49:ff:2e:96:c5:0a:5c:0b:19:1f:bf:2a:12:f1:
                    c4:e3:77:3e:1d:b9:49:0d:e8:d0:48:b4:22:2a:d7:
                    bf:98:1b:84:1b:42:50:97:45:e6:54:1c:b4:87:5b:
                    33:77:18:0c:97:52:45:2d:74:13:ed:8b:34:b9:b6:
                    a6:5e:76:63:47:5b:43:c4:df:4a:83:63:28:7a:38:
                    b4:69:ca:6b:8f:77:a2:06:88:71:ed:fb:26:42:9e:
                    3b:25:04:df:d0:79:a2:be:6d:22:23:16:f1:ae:b3:
                    68:66:62:59:54:41:f1:b0:4d
                Exponent: 65537 (0x10001)
        X509v3 extensions:
            X509v3 Basic Constraints: 
                CA:FALSE
            Netscape Comment: 
                OpenSSL Generated Certificate
            X509v3 Subject Key Identifier: 
                C8:42:09:C2:D1:6D:1C:E6:B0:C0:48:48:37:9C:A7:08:26:D2:9E:71
            X509v3 Authority Key Identifier: 
                keyid:2E:15:D7:1B:99:D6:2D:BD:A2:06:4F:93:C3:10:2C:33:9F:0C:3D:3C

    Signature Algorithm: sha256WithRSAEncryption
         43:f2:5b:a4:61:d0:cd:e7:61:22:d0:79:d5:b3:37:3e:25:3e:
         38:84:93:f4:c8:a0:e3:cc:f2:63:0e:7a:08:0b:ed:18:2a:fa:
         67:ef:bf:3b:75:f0:27:de:42:66:e3:8c:ed:a3:49:81:5e:c1:
         9a:00:bb:ed:67:3e:e5:d0:97:62:5e:b4:1f:87:ab:55:26:c1:
         6c:81:13:20:f6:b7:d6:55:54:fb:82:31:20:15:7a:d2:d7:0b:
         fe:42:02:bf:9c:af:0d:ce:86:e7:3b:13:78:ca:bc:6e:83:3e:
         b5:29:07:0c:e3:1b:a5:71:ea:18:8d:90:b2:b1:1b:ab:a4:04:
         a1:bf:81:65:b6:9f:f9:22:43:35:db:96:a7:c1:d3:1e:8b:91:
         f4:2a:d3:e4:fd:27:9a:34:71:1d:a5:60:f6:26:d2:1c:55:ef:
         39:46:01:3c:86:14:3b:79:18:50:d8:8f:b6:21:93:d0:76:b7:
         34:23:cb:22:bf:00:b6:a6:b0:e5:63:59:37:b8:2a:9c:37:31:
         15:c6:02:52:12:b2:e4:31:b0:29:e7:80:66:ef:2e:d8:d4:e5:
         98:6f:69:98:dc:2f:df:7a:d5:36:2a:64:56:9c:19:3c:19:ff:
         2a:dc:dc:5d:ba:c7:4b:61:e1:ac:3a:a9:0e:77:78:d9:eb:27:
         bc:d6:97:31
-----BEGIN CERTIFICATE-----
MIIDTDCCAjSgAwIBAgIBAjANBgkqhkiG9w0BAQsFADB0MQswCQYDVQQGEwJDTjEL
MAkGA1UECAwCSE4xCzAJBgNVBAcMAlpaMQwwCgYDVQQKDANMT0MxDTALBgNVBAsM
BFNPRlQxDjAMBgNVBAMMBVpITENBMR4wHAYJKoZIhvcNAQkBFg8zNzc5MjczOEBR
US5DT00wHhcNMjAwNjI0MTQxNDQyWhcNMjEwNjI0MTQxNDQyWjBmMQswCQYDVQQG
EwJDTjELMAkGA1UECAwCSE4xDDAKBgNVBAoMA0xPQzENMAsGA1UECwwEU09GVDEN
MAsGA1UEAwwEWkhMQzEeMBwGCSqGSIb3DQEJARYPMzc3OTI3MzhAUVEuQ09NMIGf
MA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCVC0uE/MiTewIZH9r+afNJ/y6WxQpc
CxkfvyoS8cTjdz4duUkN6NBItCIq17+YG4QbQlCXReZUHLSHWzN3GAyXUkUtdBPt
izS5tqZedmNHW0PE30qDYyh6OLRpymuPd6IGiHHt+yZCnjslBN/QeaK+bSIjFvGu
s2hmYllUQfGwTQIDAQABo3sweTAJBgNVHRMEAjAAMCwGCWCGSAGG+EIBDQQfFh1P
cGVuU1NMIEdlbmVyYXRlZCBDZXJ0aWZpY2F0ZTAdBgNVHQ4EFgQUyEIJwtFtHOaw
wEhIN5ynCCbSnnEwHwYDVR0jBBgwFoAULhXXG5nWLb2iBk+TwxAsM58MPTwwDQYJ
KoZIhvcNAQELBQADggEBAEPyW6Rh0M3nYSLQedWzNz4lPjiEk/TIoOPM8mMOeggL
7Rgq+mfvvzt18CfeQmbjjO2jSYFewZoAu+1nPuXQl2JetB+Hq1UmwWyBEyD2t9ZV
VPuCMSAVetLXC/5CAr+crw3Ohuc7E3jKvG6DPrUpBwzjG6Vx6hiNkLKxG6ukBKG/
gWW2n/kiQzXblqfB0x6LkfQq0+T9J5o0cR2lYPYm0hxV7zlGATyGFDt5GFDYj7Yh
k9B2tzQjyyK/ALamsOVjWTe4Kpw3MRXGAlISsuQxsCnngGbvLtjU5ZhvaZjcL996
1TYqZFacGTwZ/yrc3F26x0th4aw6qQ53eNnrJ7zWlzE=
-----END CERTIFICATE-----
)";

	std::string_view ca_crt = R"(
-----BEGIN CERTIFICATE-----
MIIDvjCCAqagAwIBAgIJAP8BSVUfPfIkMA0GCSqGSIb3DQEBCwUAMHQxCzAJBgNV
BAYTAkNOMQswCQYDVQQIDAJITjELMAkGA1UEBwwCWloxDDAKBgNVBAoMA0xPQzEN
MAsGA1UECwwEU09GVDEOMAwGA1UEAwwFWkhMQ0ExHjAcBgkqhkiG9w0BCQEWDzM3
NzkyNzM4QFFRLkNPTTAeFw0yMDA2MjQxNDE0MDZaFw0zMDA2MjIxNDE0MDZaMHQx
CzAJBgNVBAYTAkNOMQswCQYDVQQIDAJITjELMAkGA1UEBwwCWloxDDAKBgNVBAoM
A0xPQzENMAsGA1UECwwEU09GVDEOMAwGA1UEAwwFWkhMQ0ExHjAcBgkqhkiG9w0B
CQEWDzM3NzkyNzM4QFFRLkNPTTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoC
ggEBAKgwxRQbjG1CJC0OjKHB4jZdyM/EStBj8udGQua4iQOyqCIKcDoM7vK3xVjJ
J6gC5mmp2HIZGitDdgvr6dd7VmmKKICNP541Cb/8LTcmIaMpCDXesZnZzFer6FeL
u88FnYKBrbseqqJmh9i2wd/Z2F1g4nnSLRZdD3HMHdE5+XjpEr3zGpSxk9RYc+r4
8GRMoZ7E/pQgNDDIYhi1OvUBWIjAJhpAOU62LNppzf9Pasvc4h3ai7PoZCajgpQt
gtSG04f6x0mNqRYiEjO8ZWZ9xsq9Q/5ExNtwtRFuttVUr3o6W0781lZuxa608pDw
FeZOsmm0Vl06DpAAsNhCFHNpp9UCAwEAAaNTMFEwHQYDVR0OBBYEFC4V1xuZ1i29
ogZPk8MQLDOfDD08MB8GA1UdIwQYMBaAFC4V1xuZ1i29ogZPk8MQLDOfDD08MA8G
A1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQELBQADggEBAHmOmvYw5/TU+++ZExMD
1phNa373Rc5L4b4yPiML7sohelr3OteZav8MAqj1c7dazdlzg6a4T13SkL9HK2Yv
4fzFcdHCRrPqlfXey2tqBICZ+6UN4+a9MtsLmsdnLiomdW+ntH/oHUGPLxm1KtkN
F1feqcUtpGxUCNYw8uuZDzgtS4NKWWxzDl8nlsGqrKGq7CaK4EoQIq27QhpNaYKp
bSAquaeXIiDX7tSBrirPwQmlc05kcx+C7OAGC87zIxvr2YQn0xQpCKy6P3UHeCci
7/64IViBv1GJx7Ct7UQ57u0XOKlIwgVwWPjxksbzJqpP7HfXO4pSC0hX+Ti+EhrM
a6Q=
-----END CERTIFICATE-----
)";
		
	//while (1)
	//for (int i = 0; i < 1000; i++)
	{
		size_t count = 1;
		std::unique_ptr<asio2::tcps_client[]> clients = std::make_unique<asio2::tcps_client[]>(count);
		for (size_t i = 0; i < count; i++)
		{
			auto & client = clients[i];
			client.connect_timeout(std::chrono::seconds(10));
			client.auto_reconnect(true, std::chrono::milliseconds(1000));
			//client.set_verify_mode(asio::ssl::verify_peer);
			client.set_cert_buffer(ca_crt, client_crt, client_key, "client");
			//client.set_cert_file("ca.crt", "client.crt", "client.key", "client");
			client.start_timer(1, std::chrono::seconds(1), []() {});
			client.bind_connect([&](asio::error_code ec)
			{
				printf("connect : %s %u %d %s\n", client.local_address().c_str(), client.local_port(),
					ec.value(), ec.message().c_str());

				std::string s;
				s += '<';
				int len = 128 + std::rand() % (300);
				for (int i = 0; i < len; i++)
				{
					s += (char)((std::rand() % 26) + 'a');
				}
				s += '>';

				client.send(std::move(s));

			}).bind_disconnect([](asio::error_code ec)
			{
				printf("disconnect : %d %s\n", ec.value(), ec.message().c_str());
			}).bind_recv([&](std::string_view sv)
			{
				printf("recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());

				client.send(sv);
			}).bind_handshake([&](asio::error_code ec)
			{
				printf("handshake : %d %s\n", ec.value(), ec.message().c_str());
			});
			if (client.start(host, port))
				client.send(std::string("<0123456789>"));
			//client.async_start(host, port);
			//client.async_start(host, port, '>');
			//client.async_start(host, port, "\r\n");
			//client.async_start(host, port, asio::transfer_at_least(1));
			//client.async_start(host, port, asio::transfer_exactly(100));
			//client.async_start(host, port, asio2::use_dgram);
		}
		while (std::getchar() != '\n');
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		for (size_t i = 0; i < count; i++)
		{
			auto & client = clients[i];
			client.stop();
		}
	}
#endif // ASIO2_USE_SSL
}
