#include <asio2/udp/udp_client.hpp>

int main()
{
	std::string_view host = "127.0.0.1";
	std::string_view port = "8035";

	asio2::udp_client client;

	client.bind_recv([&](std::string_view data)
	{
		printf("recv : %zu %.*s\n", data.size(), (int)data.size(), data.data());

		std::string s;
		s += '<';
		int len = 33 + std::rand() % (126 - 33);
		for (int i = 0; i < len; i++)
		{
			s += (char)((std::rand() % 26) + 'a');
		}
		s += '>';

		client.async_send(std::move(s));
	});

	client.start(host, port);

	client.async_send("<abcdefghijklmnopqrstovuxyz0123456789>");

	while (std::getchar() != '\n');

	return 0;
}
