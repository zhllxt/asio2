// When compiling with vs under linux, you need to copy the "asio,beast,ceral,fmt" folders to
// the /usr/local/include directory first, and copy the "libcrypto.a,libssl.a" files to 
// /usr/local/lib directory first. "libcrypto.a,libssl.a" is in "asio2/lib/x64".

#ifndef ASIO2_USE_SSL
#define ASIO2_USE_SSL
#endif

#include <asio2/asio2.hpp>

std::string_view server_key = R"(
-----BEGIN RSA PRIVATE KEY-----
Proc-Type: 4,ENCRYPTED
DEK-Info: DES-EDE3-CBC,799FC130093662D3

pDxF0FWVcQgHs27NpqdrkeJ2amTgIJyn8zSywe48b/B/Dm1tEOS1ickWwDapod9J
vXah1ZrK5PefCCptA0cIndm2fOLQ9bVvTz8AmTcYpxSalzmY9DgKUVDY4lFoa9eC
I6K0O3UqkhrC4quEFLV6lDGNhzK9ZKnnDfA4QrKrDAchm8JPjxO5+JGw/TeIyms7
zgosbdcrpv6LM/KoWQg1p4V07GajrCIWc/bPXkhA4g8X53vbFToEzfdjnuOLU2U/
qApqCndX+h758YW6LJzxy/Yf2Ve5RvXdH5F44831Fkexk1Gt5eOQd1HhdaDR72kH
9qbiao+KtroVqd36Of++DxFXoV8IElGs863Czst+pTG2PoCHTBsDS+926YJzTels
DaSAbym1hoNNz+wrBOMdfhBbN00rx9teBxQwJxmppVl/0zwYj5CdDjKOMVQzPsFd
lmfHmeLpuXJkFWMDDMzP2S2UpKlRFkWtOCACOHN5FKa96zpild7yyr2l+BpesvNs
mFyEREn41IKa4lugeN5RgJh7gNkuBggPoTfPBUzodKJiJD8btUAkejhehOpfSFZh
rtBZ+xFas8GxMblUH/erLBjVtauAiBjQeQcbzCZOtFP03zDDDTgf+2q06mX0HlaY
o6b9WMMB9RdQ0YrwVPkEZ78qcxjkgw8P580CG0WEQnQ5+Rc3a2lCMJ8QdfGFnsZT
bdWMAcBN/48BuH9pdAOTskjEyL1m1SAGL7kFk044RaMRbz2T+KWW2xVyF5Dr58RT
rP9ImHAhTyBwURM8HumcDrX1suVfTL9ye4GUPuSEupFCPl6f4c7IWg==
-----END RSA PRIVATE KEY-----
)";

std::string_view server_crt = R"(
Certificate:
    Data:
        Version: 3 (0x2)
        Serial Number: 1 (0x1)
    Signature Algorithm: sha256WithRSAEncryption
        Issuer: C=CN, ST=HN, L=ZZ, O=LOC, OU=SOFT, CN=ZHLCA/emailAddress=37792738@QQ.COM
        Validity
            Not Before: Jun 24 14:14:18 2020 GMT
            Not After : Jun 24 14:14:18 2021 GMT
        Subject: C=CN, ST=HN, O=LOC, OU=SOFT, CN=ZHLS/emailAddress=37792738@QQ.COM
        Subject Public Key Info:
            Public Key Algorithm: rsaEncryption
                Public-Key: (1024 bit)
                Modulus:
                    00:cd:09:bb:de:2c:ee:59:51:ae:9d:7e:81:b9:07:
                    43:45:f6:3b:91:44:5c:3b:e7:40:fe:17:f9:71:b7:
                    76:83:8d:0b:45:e8:56:90:e3:7b:95:23:69:ef:98:
                    08:0f:8a:d4:7b:47:8d:2f:bb:15:ae:37:ed:56:14:
                    7c:a3:1c:4c:68:fd:16:74:b1:4d:7f:d8:cb:77:8f:
                    be:76:b0:57:b3:96:7f:73:54:80:e8:5c:5f:42:7b:
                    06:6e:a3:5a:53:0c:b0:fd:b9:46:14:e2:5c:a3:22:
                    2e:01:4b:21:10:b0:27:8b:73:87:fe:5c:73:26:4e:
                    5a:dd:c2:02:67:ea:13:bc:2d
                Exponent: 65537 (0x10001)
        X509v3 extensions:
            X509v3 Basic Constraints: 
                CA:FALSE
            Netscape Comment: 
                OpenSSL Generated Certificate
            X509v3 Subject Key Identifier: 
                6E:C1:73:0D:7A:C9:CF:50:FE:87:8D:C9:E2:DD:82:40:0C:E7:8F:2B
            X509v3 Authority Key Identifier: 
                keyid:2E:15:D7:1B:99:D6:2D:BD:A2:06:4F:93:C3:10:2C:33:9F:0C:3D:3C

    Signature Algorithm: sha256WithRSAEncryption
         94:94:5c:61:0a:00:ff:e4:be:8f:bc:f0:0a:72:e8:c7:97:5c:
         10:00:2d:e9:1d:7b:a2:22:04:44:95:17:6a:26:ec:7c:58:08:
         23:6a:99:29:56:6f:52:66:fd:ea:db:67:86:45:a9:3f:95:48:
         e5:3d:17:24:b1:04:73:64:f0:d6:c6:e5:4e:a3:4b:cf:fe:94:
         71:9c:f4:f4:9f:b2:91:f5:26:93:5e:de:5e:5b:d0:d3:c3:94:
         35:d7:25:a8:cc:2d:22:52:05:46:82:0a:1e:b4:6a:a5:71:0e:
         bc:cb:9f:09:51:92:fa:e1:23:57:b2:d4:b5:1a:ca:c0:2e:af:
         48:95:07:9e:45:1b:c3:fa:a9:f7:b8:ef:06:34:15:8f:6f:48:
         f7:8a:6a:09:b1:43:42:22:a8:63:96:24:f2:8f:38:4e:0f:16:
         dd:cb:65:08:fd:51:d5:57:62:42:80:c2:a3:90:6d:93:f0:b3:
         ab:02:fe:34:6e:15:24:fd:be:f0:56:f9:69:c7:48:57:5f:cc:
         d5:7b:ba:11:fb:6c:f1:59:19:09:59:2e:fc:d8:6c:3c:c3:0f:
         6a:68:d3:e7:5a:94:4a:ce:de:7f:2a:78:99:7a:10:02:95:28:
         73:5c:2c:90:d0:70:1c:c6:68:60:b1:3b:76:ff:e4:ed:e1:4b:
         be:70:61:63
-----BEGIN CERTIFICATE-----
MIIDTDCCAjSgAwIBAgIBATANBgkqhkiG9w0BAQsFADB0MQswCQYDVQQGEwJDTjEL
MAkGA1UECAwCSE4xCzAJBgNVBAcMAlpaMQwwCgYDVQQKDANMT0MxDTALBgNVBAsM
BFNPRlQxDjAMBgNVBAMMBVpITENBMR4wHAYJKoZIhvcNAQkBFg8zNzc5MjczOEBR
US5DT00wHhcNMjAwNjI0MTQxNDE4WhcNMjEwNjI0MTQxNDE4WjBmMQswCQYDVQQG
EwJDTjELMAkGA1UECAwCSE4xDDAKBgNVBAoMA0xPQzENMAsGA1UECwwEU09GVDEN
MAsGA1UEAwwEWkhMUzEeMBwGCSqGSIb3DQEJARYPMzc3OTI3MzhAUVEuQ09NMIGf
MA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDNCbveLO5ZUa6dfoG5B0NF9juRRFw7
50D+F/lxt3aDjQtF6FaQ43uVI2nvmAgPitR7R40vuxWuN+1WFHyjHExo/RZ0sU1/
2Mt3j752sFezln9zVIDoXF9CewZuo1pTDLD9uUYU4lyjIi4BSyEQsCeLc4f+XHMm
TlrdwgJn6hO8LQIDAQABo3sweTAJBgNVHRMEAjAAMCwGCWCGSAGG+EIBDQQfFh1P
cGVuU1NMIEdlbmVyYXRlZCBDZXJ0aWZpY2F0ZTAdBgNVHQ4EFgQUbsFzDXrJz1D+
h43J4t2CQAznjyswHwYDVR0jBBgwFoAULhXXG5nWLb2iBk+TwxAsM58MPTwwDQYJ
KoZIhvcNAQELBQADggEBAJSUXGEKAP/kvo+88Apy6MeXXBAALekde6IiBESVF2om
7HxYCCNqmSlWb1Jm/erbZ4ZFqT+VSOU9FySxBHNk8NbG5U6jS8/+lHGc9PSfspH1
JpNe3l5b0NPDlDXXJajMLSJSBUaCCh60aqVxDrzLnwlRkvrhI1ey1LUaysAur0iV
B55FG8P6qfe47wY0FY9vSPeKagmxQ0IiqGOWJPKPOE4PFt3LZQj9UdVXYkKAwqOQ
bZPws6sC/jRuFST9vvBW+WnHSFdfzNV7uhH7bPFZGQlZLvzYbDzDD2po0+dalErO
3n8qeJl6EAKVKHNcLJDQcBzGaGCxO3b/5O3hS75wYWM=
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

std::string_view dh = R"(
-----BEGIN DH PARAMETERS-----
MIGHAoGBAJ40f9kA5j5I3pbgoUR5ts/TbHFbja4STDgYLtAnly4NuyGUqJk+czTE
Ng3pWcySN5N0zq+eX3iWX92151a/Lb0omA3TA44Qky+qUrjV5scDhahkSlDibsbt
XLDS3egcHnTzMj+7PQBvTiH/fXX71oAT1MLvBtjZ7wLqx4OWW7STAgEC
-----END DH PARAMETERS-----
)";

int main()
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	std::string_view host = "0.0.0.0";
	std::string_view port = "8002";

	bool all_stopped = false;

	asio2::tcps_server server;

	server.set_verify_mode(asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert);

	server.set_cert_buffer(ca_crt, server_crt, server_key, "server"); // use memory string for cert
	server.set_dh_buffer(dh);

	server.start_timer(0x0f, std::chrono::seconds(3), []()
	{
		printf("0x0f timer is running...\n");
	});

	server.bind_recv([&](auto & session_ptr, std::string_view s)
	{
		printf("recv : %u %.*s\n", (unsigned)s.size(), (int)s.size(), s.data());

		session_ptr->send(s, []() {});
	}).bind_accept([&](auto& session_ptr)
	{
		printf("client accept : %s %u %s %u\n", session_ptr->remote_address().c_str(), session_ptr->remote_port(),
			session_ptr->local_address().c_str(), session_ptr->local_port());
	}).bind_connect([&](auto & session_ptr)
	{
		printf("client enter : %s %u %s %u\n", session_ptr->remote_address().c_str(), session_ptr->remote_port(),
			session_ptr->local_address().c_str(), session_ptr->local_port());
	}).bind_disconnect([&](auto & session_ptr)
	{
		// Used to test that all sessions must be closed before entering the on_stop(bind_stop) function.
		if (all_stopped)
		{
			ASIO2_ASSERT(false);
		}
		printf("client leave : %s %u %s\n", session_ptr->remote_address().c_str(),
			session_ptr->remote_port(), asio2::last_error_msg().c_str());
	}).bind_handshake([&](auto & session_ptr, asio::error_code ec)
	{
		if (asio2::get_last_error())
			printf("handshake failure : %d %s\n", ec.value(), ec.message().c_str());
		else
			printf("handshake success : %s %u\n", session_ptr->remote_address().c_str(), session_ptr->remote_port());
	}).bind_start([&](asio::error_code ec)
	{
		if (asio2::get_last_error())
			printf("start tcps server failure : %d %s\n", ec.value(), ec.message().c_str());
		else
			printf("start tcps server success : %s %u\n", server.listen_address().c_str(), server.listen_port());
		//server.stop();
	}).bind_stop([&](asio::error_code ec)
	{
		all_stopped = true;
		printf("stop : %d %s\n", ec.value(), ec.message().c_str());
	});

	server.start(host, port);

	while (std::getchar() != '\n');

	server.stop();

	return 0;
}
