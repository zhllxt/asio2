#include <asio2/proxy/socks5_server.hpp>
#include <iostream>

int main()
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
	// Detected memory leaks on windows system
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	std::string_view host = "0.0.0.0";
	std::string_view port = "20808";

	asio2::socks5_server server;

	std::map<std::string, std::string> auths =
	{
		{"user1", "password1"},
		{"user2", "password2"},
		{"user3", "password3"},
	};

	server.bind_accept([&](std::shared_ptr<asio2::socks5_session>& session_ptr)
	{
		socks5::options opts;
		//opts.set_methods(socks5::method::anonymous);
		opts.set_methods(socks5::method::password);
		// set a default username and password for all clients.
		// if the default username or password is empty, the auth callback will be called.
		opts.set_username("admin");
		opts.set_password("123456");
		// if the default username and password is auth failed, then this auth callback will be called.
		opts.set_auth_callback([&](const std::string& username, const std::string& password)
		{
			if (auto it = auths.find(username); it != auths.end())
			{
				return it->second == password;
			}
			return false;
		});
		session_ptr->set_socks5_options(std::move(opts));
	}).bind_connect([](auto & session_ptr)
	{
		printf("client enter : %s %u %s %u\n",
			session_ptr->remote_address().c_str(), session_ptr->remote_port(),
			session_ptr->local_address().c_str(), session_ptr->local_port());

	}).bind_disconnect([](auto & session_ptr)
	{
		asio2::ignore_unused(session_ptr);
		printf("client leave : %s\n", asio2::last_error_msg().c_str());

	}).bind_socks5_handshake([](auto & session_ptr)
	{
		printf("socks5 handshake : %s %u %d %s\n",
			session_ptr->remote_address().c_str(), session_ptr->remote_port(),
			asio2::last_error_val(), asio2::last_error_msg().c_str());
	}).bind_start([&]()
	{
		if (asio2::get_last_error())
			printf("start socks5 server failure : %s %u %d %s\n",
				server.listen_address().c_str(), server.listen_port(),
				asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
			printf("start socks5 server success : %s %u\n",
				server.listen_address().c_str(), server.listen_port());
	}).bind_stop([&]()
	{
		printf("stop socks5 server : %s %u %d %s\n",
			server.listen_address().c_str(), server.listen_port(),
			asio2::last_error_val(), asio2::last_error_msg().c_str());
	});

	server.start(host, port);

	while (std::getchar() != '\n');

	server.stop();

	return 0;
}
