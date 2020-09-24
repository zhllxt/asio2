// When compiling with vs under linux, you need to copy the "asio,beast,ceral,fmt" folders to
// the /usr/local/include directory first, and copy the "libcrypto.a,libssl.a" files to 
// /usr/local/lib directory first. "libcrypto.a,libssl.a" is in "asio2/lib/x64".

#ifndef ASIO2_USE_SSL
#define ASIO2_USE_SSL
#endif

#include <asio2/asio2.hpp>

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

int main()
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	std::string_view host = "127.0.0.1";
	std::string_view port = "8002";

	asio2::tcps_client client;

	client.connect_timeout(std::chrono::seconds(10));
	client.auto_reconnect(true, std::chrono::seconds(3));

	client.set_verify_mode(asio::ssl::verify_peer);
	client.set_cert_buffer(ca_crt, client_crt, client_key, "client");

	client.bind_recv([&](std::string_view sv)
	{
		printf("recv : %u %.*s\n", (unsigned)sv.size(), (int)sv.size(), sv.data());

		client.send(sv);

	}).bind_connect([&](asio::error_code ec)
    {
        printf("connect : %d %s\n", ec.value(), ec.message().c_str());

    }).bind_handshake([&](asio::error_code ec)
	{
		printf("handshake : %d %s\n", ec.value(), ec.message().c_str());

	});

	if (!client.start(host, port))
	{
		std::cout << "start failed : " << asio2::last_error_msg() << std::endl;
	}
	else
	{
		std::cout << "start success " << std::endl;
	}


	std::string s;
	s += '<';
	int len = 128 + std::rand() % (300);
	for (int i = 0; i < len; i++)
	{
		s += (char)((std::rand() % 26) + 'a');
	}
	s += '>';

	// send data, beacuse may be connect failed,
	// if connect failed, the data will sent failed to.
	client.send(std::move(s), []()
	{
		if (asio2::get_last_error())
			std::cout << "send failed : " << asio2::last_error_msg() << std::endl;
	});

	while (std::getchar() != '\n');

	return 0;
}
