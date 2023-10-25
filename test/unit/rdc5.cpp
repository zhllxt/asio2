#ifndef ASIO2_USE_SSL
#define ASIO2_USE_SSL
#endif

#include "unit_test.hpp"
#include <asio2/asio2.hpp>
#include <asio2/external/fmt.hpp>

static std::string_view chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";


std::string_view server_key = R"(
-----BEGIN RSA PRIVATE KEY-----
Proc-Type: 4,ENCRYPTED
DEK-Info: DES-EDE3-CBC,32DBC28981A12AF1

Ss9aGjqLB5ax69XsLHS7zurjRtGTfUCBOGNVYsFkVnfwjogIwU6cviRoK+T/pB07
lw2Kn/Q98vdoghZilKeH8qE4FTqfeYo0tXx/F8fXzff5UbSKGTVavVFCbphF2Ww4
5iiCoDGi6eY1MRz2eplCWZnHdsGqm9Y6EZ4Pmv/Pg27ZQbFWXPXbvGuC2Ct+rChU
7LnTZzI3UTo4y5RrhtxaBPx/Gx2juOTjIexlzHMA9Bk7q/LdnnxtZQO0cQH+izOC
h7UK01I5hIC5WERobtas2MIhHE2i3YQpm/QjPL5FtckIX2z0NMudHUM1VsYVek8E
Vv1VVIiPApzBpU4S2ov7jtVIu85ayZrgi6m2uAa+R6QJ66uIh1ECD/D4ZCTbDfYO
c+EORUKIU9uQ3VwG028WT8ZyWPxLQAeckd7TYDT4MiEwhnTsnGjpx9caieFcyqtt
SfPAbmzuVLUV+tjVs5msRmZy3s/fUMpA/iFm6zmKz7OoXulHKMG2oVzDkr8jPcSi
6Hgt+xUcsUDj6LKy0xMI3YDC0P/UHly9BL1g/o7wwFBIxnNUYpRIGwMLyUskBROs
hp9gegeOSP/O0LYIoHNi1PFfbiVhV9KaXf+qvLCdgWn9g9APJl/eJZKsP+i124RR
Vk3Cqn8pjDIVh+40yFFuBnECi3NY6000EUKO1PLjjBONOiR5+QTX4qv5gE8pwzlM
SHrfwS58jNw4l9A9rmTLGps4pwhsd4Jlr+OJHj0GeA7q3gC7uJCEjsCE+Cxo/erw
139esKJvgOyJxnMilPfON6u6OIbbKO9IzfR3iIW7oSFr9/7Yh5XOAA==
-----END RSA PRIVATE KEY-----
)";

std::string_view server_crt = R"(
Certificate:
    Data:
        Version: 3 (0x2)
        Serial Number: 1 (0x1)
    Signature Algorithm: sha256WithRSAEncryption
        Issuer: C=AU, ST=HENAN, L=ZHENGZHOU, O=WJ, OU=WJSOFT, CN=ZHL/emailAddress=37792738@qq.com
        Validity
            Not Before: Nov 26 08:21:41 2021 GMT
            Not After : Nov 24 08:21:41 2031 GMT
        Subject: C=AU, ST=HENAN, O=WJ, OU=WJSOFT, CN=ZHL/emailAddress=37792738@qq.com
        Subject Public Key Info:
            Public Key Algorithm: rsaEncryption
                Public-Key: (1024 bit)
                Modulus:
                    00:c4:ba:4e:5f:22:45:ac:74:8f:5a:c3:06:4b:b4:
                    a6:22:be:68:7b:99:bf:44:02:66:69:09:ec:2c:7a:
                    68:c9:a9:0a:b2:f4:ed:69:6b:ad:29:59:b7:a6:ff:
                    69:df:f6:e5:45:44:d7:70:a7:40:84:d6:19:dd:c4:
                    36:27:86:1d:6d:79:e0:91:e5:77:79:49:28:4f:06:
                    7f:31:70:8b:ec:c2:58:9c:f4:14:1d:29:bb:2c:5a:
                    82:c2:b5:ca:de:eb:cb:a8:34:fc:7b:eb:48:76:44:
                    ed:29:a1:7d:99:3c:ad:a9:3d:8c:8d:ef:12:ef:d5:
                    ad:bf:40:34:b4:fd:e4:f2:a9
                Exponent: 65537 (0x10001)
        X509v3 extensions:
            X509v3 Basic Constraints: 
                CA:FALSE
            Netscape Comment: 
                OpenSSL Generated Certificate
            X509v3 Subject Key Identifier: 
                9B:D5:B6:0E:47:C3:A7:B6:DA:84:3B:F0:CE:D1:50:D3:8F:4F:0A:8A
            X509v3 Authority Key Identifier: 
                keyid:61:74:1F:7E:B1:0E:0D:F9:46:DD:6A:97:85:72:DE:1A:7D:A2:34:65

    Signature Algorithm: sha256WithRSAEncryption
         b6:1e:bb:f7:fa:c5:9f:07:6e:36:9d:2e:7d:39:8e:a1:ed:f1:
         65:a0:0c:e4:bb:6d:bc:eb:58:d5:1d:c2:03:57:8a:41:0a:f1:
         81:0f:87:38:c4:56:83:c3:9d:dc:f3:47:88:c8:a7:ba:69:f9:
         bb:45:1f:73:48:96:f9:d7:fc:da:73:f9:17:5f:2f:94:19:83:
         27:4b:b0:3e:19:29:71:a2:fc:db:d2:5f:6e:4f:e5:f1:d8:35:
         55:f8:d9:db:75:dc:fe:11:e0:9f:70:6e:a8:26:2a:ca:7e:25:
         08:e1:d5:d8:e3:0b:10:48:c6:ae:c5:b4:7b:15:20:87:97:20:
         31:ee:e1:6f:d7:be:41:5d:2a:22:b0:36:16:1d:7a:70:bc:1b:
         d3:89:94:ae:33:66:0c:cd:39:95:9e:69:30:37:05:bb:62:cd:
         3f:dd:2b:bb:72:16:48:75:91:33:33:ae:b7:d7:2d:bd:ce:66:
         f3:6b:69:81:fa:0d:aa:0e:5a:09:9d:24:54:ac:21:9b:14:43:
         44:12:56:8b:cc:13:b5:3b:5a:ba:4e:7b:81:42:1e:38:61:ff:
         a0:a7:01:2f:0b:67:77:90:48:bb:8a:52:62:69:76:3c:a8:a1:
         d6:13:1e:27:f6:02:58:ae:91:4b:9d:37:4e:31:55:73:18:4e:
         d0:61:54:3b
-----BEGIN CERTIFICATE-----
MIIDWDCCAkCgAwIBAgIBATANBgkqhkiG9w0BAQsFADB9MQswCQYDVQQGEwJBVTEO
MAwGA1UECAwFSEVOQU4xEjAQBgNVBAcMCVpIRU5HWkhPVTELMAkGA1UECgwCV0ox
DzANBgNVBAsMBldKU09GVDEMMAoGA1UEAwwDWkhMMR4wHAYJKoZIhvcNAQkBFg8z
Nzc5MjczOEBxcS5jb20wHhcNMjExMTI2MDgyMTQxWhcNMzExMTI0MDgyMTQxWjBp
MQswCQYDVQQGEwJBVTEOMAwGA1UECAwFSEVOQU4xCzAJBgNVBAoMAldKMQ8wDQYD
VQQLDAZXSlNPRlQxDDAKBgNVBAMMA1pITDEeMBwGCSqGSIb3DQEJARYPMzc3OTI3
MzhAcXEuY29tMIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDEuk5fIkWsdI9a
wwZLtKYivmh7mb9EAmZpCewsemjJqQqy9O1pa60pWbem/2nf9uVFRNdwp0CE1hnd
xDYnhh1teeCR5Xd5SShPBn8xcIvswlic9BQdKbssWoLCtcre68uoNPx760h2RO0p
oX2ZPK2pPYyN7xLv1a2/QDS0/eTyqQIDAQABo3sweTAJBgNVHRMEAjAAMCwGCWCG
SAGG+EIBDQQfFh1PcGVuU1NMIEdlbmVyYXRlZCBDZXJ0aWZpY2F0ZTAdBgNVHQ4E
FgQUm9W2DkfDp7bahDvwztFQ049PCoowHwYDVR0jBBgwFoAUYXQffrEODflG3WqX
hXLeGn2iNGUwDQYJKoZIhvcNAQELBQADggEBALYeu/f6xZ8HbjadLn05jqHt8WWg
DOS7bbzrWNUdwgNXikEK8YEPhzjEVoPDndzzR4jIp7pp+btFH3NIlvnX/Npz+Rdf
L5QZgydLsD4ZKXGi/NvSX25P5fHYNVX42dt13P4R4J9wbqgmKsp+JQjh1djjCxBI
xq7FtHsVIIeXIDHu4W/XvkFdKiKwNhYdenC8G9OJlK4zZgzNOZWeaTA3BbtizT/d
K7tyFkh1kTMzrrfXLb3OZvNraYH6DaoOWgmdJFSsIZsUQ0QSVovME7U7WrpOe4FC
Hjhh/6CnAS8LZ3eQSLuKUmJpdjyoodYTHif2AliukUudN04xVXMYTtBhVDs=
-----END CERTIFICATE-----
)";

std::string_view ca_crt = R"(
-----BEGIN CERTIFICATE-----
MIID0DCCArigAwIBAgIJAKb3L14tWHhjMA0GCSqGSIb3DQEBCwUAMH0xCzAJBgNV
BAYTAkFVMQ4wDAYDVQQIDAVIRU5BTjESMBAGA1UEBwwJWkhFTkdaSE9VMQswCQYD
VQQKDAJXSjEPMA0GA1UECwwGV0pTT0ZUMQwwCgYDVQQDDANaSEwxHjAcBgkqhkiG
9w0BCQEWDzM3NzkyNzM4QHFxLmNvbTAeFw0yMTExMjYwODIxMDlaFw0zMTExMjQw
ODIxMDlaMH0xCzAJBgNVBAYTAkFVMQ4wDAYDVQQIDAVIRU5BTjESMBAGA1UEBwwJ
WkhFTkdaSE9VMQswCQYDVQQKDAJXSjEPMA0GA1UECwwGV0pTT0ZUMQwwCgYDVQQD
DANaSEwxHjAcBgkqhkiG9w0BCQEWDzM3NzkyNzM4QHFxLmNvbTCCASIwDQYJKoZI
hvcNAQEBBQADggEPADCCAQoCggEBANVbanX0OZCM3X+QbtOpBEBXCmY2IfrTeMNZ
LYr25gPoxFYilPa4CqCZ2hHX2KjvqB04XOYou49g9x6CUsEFxGzr3W1YziL13HQQ
5E2903ntpb2QHChJ1huUoRnsR4cBqn50r1w+a49xffSCbA1QMd1HzdIgwZmH5PQR
epQD3JPKKomSPfS9vLmh7XddfaTX4FsuJXLXr/OON24KYihJ5e18skN4e4h2bViE
0qjWtHeJDomcUyGZmQ3jF4EzBcng8w8XzD5J7lfiOOVMdFt/2Snoe3ylc0vJ6uRv
fLxQquzwVleqZg3v3t4GwKISzQ7wiZi/Gu7wjdzTvQz2mOwRdf0CAwEAAaNTMFEw
HQYDVR0OBBYEFGF0H36xDg35Rt1ql4Vy3hp9ojRlMB8GA1UdIwQYMBaAFGF0H36x
Dg35Rt1ql4Vy3hp9ojRlMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQELBQAD
ggEBAGekedydDeqnWkqh3buU45drHl4I6wOr7AibfNDlNffl8fhG4CjMKKLi1JW/
Z5Nnma/ypeFInKK4LybIsQ8GlLQ144PhGTKsV+9uDKhtiHGzNg5ZGGHqG5NXEjDr
KvqpOlWmaxqIeXlKmGR7a5agaYkd4abHq+Ye/qbZBoiMwJdrpT5LViYLp81pnFt9
62dTNb3vqRWbCgLz4enXsVGHRrWCHT2K9caQeilvVxlpIrvXobKsTfKPKQ5AgzXh
iB3cY4DnXG/RT6LMOU2c5f7SRw2edAMCZMinnkootR/EMlDJgUjSamQ3ssoXqSX3
NcJfvucspIZ1QdOFX5wESIpAMlQ=
-----END CERTIFICATE-----
)";

std::string_view dh = R"(
-----BEGIN DH PARAMETERS-----
MIGHAoGBAPr5rFoiBSxOovqiT+2R06yBd2WcQmVr580kSv/PI2HYdJNNg9JB9/xb
glSjeuKkZPMipdpZygqTk86rrlGWNKcOtybuGSJdcTBMUxcofDTZxIWkwr09JD7c
J5fipXRE8kFry0Nk9lL96seMYoER32zw6y2tXgUeksVrjOkGuheTAgEC
-----END DH PARAMETERS-----
)";


std::string_view client_key = R"(
-----BEGIN RSA PRIVATE KEY-----
Proc-Type: 4,ENCRYPTED
DEK-Info: DES-EDE3-CBC,967AA0B1DF77C592

/hmvltrtAZ26yCD28FRJWPBvmVwnW3MCt5e56jUcyarCpcRWlB1RRkBd51VQWQ74
9rAtn2HH1nsUMdorSCszcDUkTqLg9DAIiz8qMH0chChNLV173yJTLHJHGjqDnwUY
bQMVJMRjuHBmNfnyoeBbdiYjftpSJVn/G3CZopLdT7Pr+T15Ij8VIuOIrbtEcKBO
VbHMQYgqZeh/7sP2E5RRkddEkFycistiJ+laTP+t3Ro90NyJNy6/JA0tewknFOTK
LpsJ1RoUzTTJWY0QyNDFB5nOahPBIeTrPfzufWxzk6QqXRG4u9NWrCSiJfm/VEet
qQcVVlV7hwsq0NbT5Bqk+Fu632NgN4NPSh2A6noUFUlQm/srebrOFL/2Fsnegc4H
jOJUpbEkTJq9vkdBnLJYXaUS1es07ZzDCL9rqmlLLnkUm+9WuGG2EFwOEz0abUv7
QSyCjqGz1cCe6uvyTZZLKRQhClGNvGk1BZ4pG8PZPqCwi9b1UhTw9FtqWUJKrN3u
w5D81b6d6pZfpSw1Zyvo24GVBZxp9ISO/PN/l32GM1Nv+PRqUHb1Ew83uHuTLFmQ
H+3BBom0gk6k19bESDu0hV4QGmfFASaAb/T31i9Ssd/av4QUJryv+HXp1tiG2W89
eHSVdDVkIf0g80Ryp+VYZhQw9dgywVGxQqs0CLlMMh/BbHnNqc8dLKaVnAiwMEeb
eN7dTsdJVzsOOAtA04IYavQJQAkMbuIGalPg11Fgeb1geB/208xJX8Bf6farT9El
iN1CWBFC/q8QHLbI4DcTV2YCSk19ZV2U+EoDbz1bI9KP61+Rvgb9Rw==
-----END RSA PRIVATE KEY-----
)";

std::string_view client_crt = R"(
Certificate:
    Data:
        Version: 3 (0x2)
        Serial Number: 2 (0x2)
    Signature Algorithm: sha256WithRSAEncryption
        Issuer: C=AU, ST=HENAN, L=ZHENGZHOU, O=WJ, OU=WJSOFT, CN=ZHL/emailAddress=37792738@qq.com
        Validity
            Not Before: Nov 26 09:04:13 2021 GMT
            Not After : Nov 24 09:04:13 2031 GMT
        Subject: C=AU, ST=HENAN, O=WJ, OU=WJSOFT, CN=ZHL/emailAddress=37792738@qq.com
        Subject Public Key Info:
            Public Key Algorithm: rsaEncryption
                Public-Key: (1024 bit)
                Modulus:
                    00:be:94:6f:5b:ae:20:a8:73:25:3f:a8:4d:92:5a:
                    5b:8b:64:4d:7b:53:2c:fb:10:e9:ad:e5:06:41:5a:
                    f6:eb:58:9a:22:6b:5c:ac:04:03:c5:09:2a:3d:84:
                    d9:34:25:42:76:f8:e6:c7:64:cd:5d:ce:ee:03:54:
                    da:af:dd:da:f2:b4:93:72:3f:26:d6:57:ea:18:ec:
                    9c:c8:20:bc:1a:a1:f9:e0:f5:64:67:9d:61:b8:f6:
                    87:a4:d3:36:01:24:b4:e7:00:c3:54:82:bd:7f:22:
                    48:40:df:43:8c:26:83:aa:b3:68:5d:e9:a1:fe:7c:
                    6f:a6:5d:a4:bd:f4:1f:e3:25
                Exponent: 65537 (0x10001)
        X509v3 extensions:
            X509v3 Basic Constraints: 
                CA:FALSE
            Netscape Comment: 
                OpenSSL Generated Certificate
            X509v3 Subject Key Identifier: 
                50:C7:26:F5:62:F6:B7:24:3C:1C:5C:58:96:08:59:94:A5:7A:A1:22
            X509v3 Authority Key Identifier: 
                keyid:61:74:1F:7E:B1:0E:0D:F9:46:DD:6A:97:85:72:DE:1A:7D:A2:34:65

    Signature Algorithm: sha256WithRSAEncryption
         39:c1:66:e0:1a:68:2a:bc:6e:56:a0:a4:18:53:ac:2e:49:03:
         d3:df:e0:7d:4e:51:4c:0b:fb:f9:1d:ae:a5:b8:16:b1:b6:23:
         db:62:33:25:72:14:99:e1:3a:1a:52:d2:f4:51:74:ef:df:04:
         91:34:4f:e6:57:c2:d6:41:60:05:d5:65:09:6c:44:14:e6:29:
         b8:51:03:e0:bd:33:c3:9d:da:99:24:eb:a8:76:18:88:14:84:
         83:ee:33:a3:5c:05:9a:3c:e3:f0:35:e3:52:a2:4e:aa:39:07:
         41:43:10:3f:cb:03:a2:48:a9:90:ef:5a:53:3b:5b:c7:4d:bb:
         76:05:70:eb:a7:15:22:ad:b5:ed:3b:74:58:71:c0:53:90:44:
         92:81:8b:62:62:2f:3a:96:66:ee:18:53:d4:4c:5e:d6:29:f4:
         4c:18:2c:7a:79:96:38:d7:cf:51:e4:3b:72:9f:9c:be:7f:2e:
         05:8a:06:18:61:62:d5:9c:cc:31:a7:4c:d9:94:bd:d3:b4:22:
         b8:42:d2:6f:99:7a:72:43:b3:a9:03:e2:36:6d:6b:28:4f:f8:
         c5:b5:1b:2b:1d:e9:34:8f:66:0a:13:58:d5:28:38:03:22:bc:
         37:27:ed:c7:b3:c7:80:63:25:d7:fc:38:ad:ac:f9:aa:5b:07:
         15:df:56:17
-----BEGIN CERTIFICATE-----
MIIDWDCCAkCgAwIBAgIBAjANBgkqhkiG9w0BAQsFADB9MQswCQYDVQQGEwJBVTEO
MAwGA1UECAwFSEVOQU4xEjAQBgNVBAcMCVpIRU5HWkhPVTELMAkGA1UECgwCV0ox
DzANBgNVBAsMBldKU09GVDEMMAoGA1UEAwwDWkhMMR4wHAYJKoZIhvcNAQkBFg8z
Nzc5MjczOEBxcS5jb20wHhcNMjExMTI2MDkwNDEzWhcNMzExMTI0MDkwNDEzWjBp
MQswCQYDVQQGEwJBVTEOMAwGA1UECAwFSEVOQU4xCzAJBgNVBAoMAldKMQ8wDQYD
VQQLDAZXSlNPRlQxDDAKBgNVBAMMA1pITDEeMBwGCSqGSIb3DQEJARYPMzc3OTI3
MzhAcXEuY29tMIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC+lG9briCocyU/
qE2SWluLZE17Uyz7EOmt5QZBWvbrWJoia1ysBAPFCSo9hNk0JUJ2+ObHZM1dzu4D
VNqv3drytJNyPybWV+oY7JzIILwaofng9WRnnWG49oek0zYBJLTnAMNUgr1/IkhA
30OMJoOqs2hd6aH+fG+mXaS99B/jJQIDAQABo3sweTAJBgNVHRMEAjAAMCwGCWCG
SAGG+EIBDQQfFh1PcGVuU1NMIEdlbmVyYXRlZCBDZXJ0aWZpY2F0ZTAdBgNVHQ4E
FgQUUMcm9WL2tyQ8HFxYlghZlKV6oSIwHwYDVR0jBBgwFoAUYXQffrEODflG3WqX
hXLeGn2iNGUwDQYJKoZIhvcNAQELBQADggEBADnBZuAaaCq8blagpBhTrC5JA9Pf
4H1OUUwL+/kdrqW4FrG2I9tiMyVyFJnhOhpS0vRRdO/fBJE0T+ZXwtZBYAXVZQls
RBTmKbhRA+C9M8Od2pkk66h2GIgUhIPuM6NcBZo84/A141KiTqo5B0FDED/LA6JI
qZDvWlM7W8dNu3YFcOunFSKtte07dFhxwFOQRJKBi2JiLzqWZu4YU9RMXtYp9EwY
LHp5ljjXz1HkO3KfnL5/LgWKBhhhYtWczDGnTNmUvdO0IrhC0m+ZenJDs6kD4jZt
ayhP+MW1Gysd6TSPZgoTWNUoOAMivDcn7cezx4BjJdf8OK2s+apbBxXfVhc=
-----END CERTIFICATE-----
)";


void rdc5_test()
{
	ASIO2_TEST_BEGIN_LOOP(test_loop_times);

	// test http rdc
	{
		asio2::rdc::option client_rdc_option
		{
			[](http::web_request&)
			{
				return 0;
			},
			[](http::web_response&)
			{
				return 0;
			}
		};

		asio2::http_server server;
		std::atomic<int> server_recv_counter = 0;
		server.bind_recv(
		[&](std::shared_ptr<asio2::http_session> & session_ptr, http::web_request& req, http::web_response& rep)
		{
			server_recv_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(session_ptr->is_started());
			ASIO2_CHECK(session_ptr->io().running_in_this_thread());

			asio2::ignore_unused(req, rep);
		});
		std::atomic<int> server_accept_counter = 0;
		server.bind_accept([&](auto & session_ptr)
		{
			if (!asio2::get_last_error())
			{
				session_ptr->no_delay(true);

				server_accept_counter++;

				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
				ASIO2_CHECK(server.get_listen_port() == 18037);
				ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_port() == 18037);
				ASIO2_CHECK(server.io().running_in_this_thread());
				ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
			}
		});
		std::atomic<int> server_connect_counter = 0;
		server.bind_connect([&](auto & session_ptr)
		{
			server_connect_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(!server.find_session(session_ptr->hash_key()));
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18037);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
			ASIO2_CHECK(session_ptr->is_keep_alive());
			ASIO2_CHECK(session_ptr->is_no_delay());
		});
		std::atomic<int> server_disconnect_counter = 0;
		server.bind_disconnect([&](auto & session_ptr)
		{
			server_disconnect_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(!server.find_session(session_ptr->hash_key()));
			//ASIO2_CHECK(session_ptr->socket().is_open());
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			//ASIO2_CHECK(session_ptr->local_port() == 18037);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_init_counter = 0;
		server.bind_init([&]()
		{
			server_init_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());

			asio::socket_base::reuse_address option;
			server.acceptor().get_option(option);
			ASIO2_CHECK(option.value());
		});
		std::atomic<int> server_start_counter = 0;
		server.bind_start([&]()
		{
			server_start_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18037);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18037);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		server.bind("*", [&](http::web_request& req, http::web_response& rep)
		{
			ASIO2_CHECK(!asio2::get_last_error());

			asio2::ignore_unused(req, rep);

			std::string target(req.target());

			ASIO2_CHECK(target == req.body());

			target += ":response";

			rep.fill_text(target);
		});

		bool server_start_ret = server.start("127.0.0.1", 18037);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		std::vector<std::shared_ptr<asio2::http_client>> clients;
		std::atomic<int> client_init_counter = 0;
		std::atomic<int> client_connect_counter = 0;
		std::atomic<int> client_disconnect_counter = 0;
		for (int i = 0; i < test_client_count; i++)
		{
			auto iter = clients.emplace_back(std::make_shared<asio2::http_client>());

			asio2::http_client& client = *iter;

			// disable auto reconnect, default reconnect option is "enable"
			client.set_auto_reconnect(false);

			client.bind_init([&]()
			{
				client_init_counter++;

				client.set_no_delay(true);

				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.is_keep_alive());
				ASIO2_CHECK(client.is_reuse_address());
				ASIO2_CHECK(client.is_no_delay());
			});
			client.bind_connect([&]()
			{
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.get_local_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_address() == "127.0.0.1");
				ASIO2_CHECK(client.get_remote_port() == 18037);

				client_connect_counter++;
			});
			client.bind_disconnect([&]()
			{
				client_disconnect_counter++;

				ASIO2_CHECK(asio2::get_last_error());
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
			});
			client.bind_recv([&](http::web_request& req, http::web_response& rep)
			{
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(client.is_started());

				asio2::ignore_unused(req, rep);

				client.stop();
			});

			bool client_start_ret = client.async_start("127.0.0.1", 18037, client_rdc_option);

			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
		}

		while (server.get_session_count() < std::size_t(test_client_count))
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		auto session_count = server.get_session_count();
		ASIO2_CHECK_VALUE(session_count, session_count == std::size_t(test_client_count));

		while (client_connect_counter < test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == test_client_count);
		ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == test_client_count);

		ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == test_client_count);
		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			std::string str = std::to_string(std::rand());

			str.insert(0, "/");

			http::web_request req;
			req.version(11);
			req.method(http::verb::get);
			req.target(str);
			req.body() = str;
			req.prepare_payload();

			clients[i]->async_call(req, [str](http::web_response& rep) mutable
			{
				str += ":response";
				ASIO2_CHECK(rep.body().text() == str);
			});
		}

		while (client_disconnect_counter != test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		// use this to ensure the ASIO2_CHECK(session_ptr->is_started());
		while (server_disconnect_counter != test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(server_disconnect_counter.load(), server_disconnect_counter == test_client_count);
		ASIO2_CHECK_VALUE(server_recv_counter.load(), server_recv_counter == 1 * test_client_count);
		ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->stop();
			ASIO2_CHECK(clients[i]->is_stopped());
		}

		server.stop();
		ASIO2_CHECK(server.is_stopped());

		ASIO2_CHECK_VALUE(server_stop_counter      .load(), server_stop_counter       == 1);

		//-----------------------------------------------------------------------------------------

		ASIO2_CHECK_VALUE(server.get_session_count(), server.get_session_count() == std::size_t(0));

		server_init_counter = 0;
		server_start_counter = 0;
		server_disconnect_counter = 0;
		server_stop_counter = 0;
		server_accept_counter = 0;
		server_connect_counter = 0;
		server_recv_counter = 0;

		server_start_ret = server.start("127.0.0.1", 18037);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		client_init_counter = 0;
		client_connect_counter = 0;
		client_disconnect_counter = 0;

		for (int i = 0; i < test_client_count; i++)
		{
			bool client_start_ret = clients[i]->async_start("127.0.0.1", 18037);
			ASIO2_CHECK(client_start_ret);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
		}

		while (server.get_session_count() < std::size_t(test_client_count))
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(server.get_session_count(), server.get_session_count() == std::size_t(test_client_count));

		while (client_connect_counter < test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == test_client_count);
		ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == test_client_count);

		ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == test_client_count);
		ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			std::string str = std::to_string(std::rand());

			str.insert(0, "/");

			http::web_request req;
			req.version(11);
			req.method(http::verb::get);
			req.target(str);
			req.body() = str;
			req.prepare_payload();

			str += ":response";

			http::web_response rep = clients[i]->call<http::web_response>(req);
			ASIO2_CHECK(rep.body().text() == str);
		}

		while (client_disconnect_counter != test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		// use this to ensure the ASIO2_CHECK(session_ptr->is_started());
		while (server_disconnect_counter != test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		ASIO2_CHECK_VALUE(server_disconnect_counter.load(), server_disconnect_counter == test_client_count);
		ASIO2_CHECK_VALUE(server_recv_counter.load(), server_recv_counter == 1 * test_client_count);
		ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->stop();
			ASIO2_CHECK(clients[i]->is_stopped());
		}

		server.stop();
		ASIO2_CHECK(server.is_stopped());

		ASIO2_CHECK_VALUE(server_stop_counter      .load(), server_stop_counter       == 1);
	}

	ASIO2_TEST_END_LOOP;
}


ASIO2_TEST_SUITE
(
	"rdc5",
	ASIO2_TEST_CASE(rdc5_test)
)
