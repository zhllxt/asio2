#include "unit_test.hpp"
#include <fmt/format.h>
#include <asio2/tcp/tcp_server.hpp>
#include <asio2/tcp/tcp_client.hpp>

static std::string_view chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

// the byte 1    head   (1 bytes) : #
// the byte 2    length (1 bytes) : the body length
// the byte 3... body   (n bytes) : the body content
class match_role
{
public:
	explicit match_role(char c) : c_(c) {}

	template <typename Iterator>
	std::pair<Iterator, bool> operator()(Iterator begin, Iterator end) const
	{
		Iterator p = begin;
		while (p != end)
		{
			// how to convert the Iterator to char* 
			[[maybe_unused]] const char * buf = &(*p);

			ASIO2_CHECK(remote_addr_ == session_ptr_->get_remote_address());
			ASIO2_CHECK(remote_port_ == session_ptr_->get_remote_port());

			// eg : How to close illegal clients
			if (*p != c_)
			{
				if (std::rand() % 2)
				{
					// method 1:
					// call the session stop function directly.
					session_ptr_->stop();
					break;
				}
				else
				{
					// method 2:
					// return the matching success here and then determine the number of bytes received
					// in the on_recv callback function, if it is 0, we close the connection in on_recv.
					return std::pair(begin, true); // head character is not #, return and kill the client
				}
			}

			p++;
			if (p == end) break;

			int length = std::uint8_t(*p); // get content length

			p++;
			if (p == end) break;

			if (end - p >= length)
				return std::pair(p + length, true);

			break;
		}
		return std::pair(begin, false);
	}

	// the asio2 framework will call this function immediately after the session is created, 
	// you can save the session pointer into a member variable, or do something else.
	void init(std::shared_ptr<asio2::tcp_session>& session_ptr)
	{
		ASIO2_CHECK(remote_addr_.empty());
		ASIO2_CHECK(remote_port_ == 0);
		ASIO2_CHECK(session_ptr_ == nullptr);

		session_ptr_ = session_ptr;

		remote_addr_ = session_ptr_->get_remote_address();
		remote_port_ = session_ptr_->get_remote_port();
	}

private:
	char c_;

	std::string   remote_addr_;
	std::uint16_t remote_port_ = 0;

	// note : use a shared_ptr to save the session does not cause circular reference.
	std::shared_ptr<asio2::tcp_session> session_ptr_;
};

using buffer_iterator = asio::buffers_iterator<asio::streambuf::const_buffers_type>;
std::pair<buffer_iterator, bool> match_role_func(buffer_iterator begin, buffer_iterator end)
{
	buffer_iterator p = begin;
	while (p != end)
	{
		// how to convert the Iterator to char* 
		[[maybe_unused]] const char* buf = &(*p);

		if (*p != '#')
			return std::pair(begin, true); // head character is not #, return and kill the client

		p++;
		if (p == end) break;

		int length = std::uint8_t(*p); // get content length

		p++;
		if (p == end) break;

		if (end - p >= length)
			return std::pair(p + length, true);

		break;
	}
	return std::pair(begin, false);
}

#ifdef ASIO_STANDALONE
namespace asio
#else
namespace boost::asio
#endif
{
	template <> struct is_match_condition<match_role> : public std::true_type {};
}

void tcp_custom_test()
{
	ASIO2_TEST_BEGIN_LOOP(test_loop_times);

	// 
	{
		asio2::tcp_server server;
		std::atomic<int> server_recv_counter = 0;
		server.bind_recv([&](std::shared_ptr<asio2::tcp_session> & session_ptr, std::string_view data)
		{
			if (data.size() == 0)
			{
				session_ptr->stop();
				return;
			}

			if (server.iopool().size() > 1)
			{
				ASIO2_CHECK(std::addressof(session_ptr->io()) != std::addressof(server.io()));
			}

			server_recv_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(session_ptr->is_started());

			session_ptr->async_send(data);

			ASIO2_CHECK(session_ptr->io().running_in_this_thread());
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
				ASIO2_CHECK(server.get_listen_port() == 18026);
				ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_port() == 18026);
				ASIO2_CHECK(server.io().running_in_this_thread());
				ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());

				//// You can close the connection directly here.
				//if (session_ptr->remote_address() == "192.168.0.254")
				//	session_ptr->stop();
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
			ASIO2_CHECK(session_ptr->local_port() == 18026);
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
			ASIO2_CHECK(session_ptr->socket().is_open());
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18026);
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
			ASIO2_CHECK(server.get_listen_port() == 18026);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18026);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});

		bool server_start_ret = server.start("127.0.0.1", 18026, match_role('#'));

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		std::vector<std::shared_ptr<asio2::tcp_client>> clients;
		std::atomic<int> client_init_counter = 0;
		std::atomic<int> client_connect_counter = 0;
		std::atomic<int> client_disconnect_counter = 0;
		std::atomic<int> client_recv_counter = 0;
		for (int i = 0; i < test_client_count; i++)
		{
			auto iter = clients.emplace_back(std::make_shared<asio2::tcp_client>());

			asio2::tcp_client& client = *iter;

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
				ASIO2_CHECK(client.get_remote_port() == 18026);

				client_connect_counter++;
			});
			client.bind_disconnect([&]()
			{
				client_disconnect_counter++;

				ASIO2_CHECK(asio2::get_last_error());
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
			});
			client.bind_recv([&]([[maybe_unused]] std::string_view data)
			{
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!data.empty());
				ASIO2_CHECK(client.is_started());
				//client.async_send(data);

				client_recv_counter++;

				std::vector<std::uint8_t> msg;

				for (int n = 20 + std::rand() % 100; n > 0; n--)
					msg.emplace_back(chars.at(std::rand() % chars.size()));

				msg.insert(msg.begin(), std::uint8_t(msg.size()));
				msg.insert(msg.begin(), '#');

				if (client_recv_counter > test_client_count * 20)
					msg.insert(msg.begin(), '0');

				client.async_send(std::move(msg));
			});

			bool client_start_ret = client.async_start("127.0.0.1", 18026, match_role_func);

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

		ASIO2_CHECK_VALUE(client_init_counter.load(), client_init_counter == test_client_count);
		ASIO2_CHECK_VALUE(client_connect_counter.load(), client_connect_counter == test_client_count);

		ASIO2_CHECK_VALUE(server_accept_counter.load(), server_accept_counter == test_client_count);
		ASIO2_CHECK_VALUE(server_connect_counter.load(), server_connect_counter == test_client_count);

		std::vector<std::shared_ptr<asio2::tcp_client>> illegal_clients;
		std::atomic<int> illegal_client_init_counter = 0;
		std::atomic<int> illegal_client_connect_counter = 0;
		std::atomic<int> illegal_client_disconnect_counter = 0;
		for (int i = 0; i < test_client_count; i++)
		{
			auto iter = illegal_clients.emplace_back(std::make_shared<asio2::tcp_client>());

			asio2::tcp_client& illegal_client = *iter;

			// disable auto reconnect, default reconnect option is "enable"
			illegal_client.set_auto_reconnect(true, std::chrono::milliseconds(100));

			illegal_client.bind_init([&]()
			{
				illegal_client_init_counter++;

				illegal_client.set_no_delay(true);

				ASIO2_CHECK(illegal_client.io().running_in_this_thread());
				ASIO2_CHECK(illegal_client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(illegal_client.is_keep_alive());
				ASIO2_CHECK(illegal_client.is_reuse_address());
				ASIO2_CHECK(illegal_client.is_no_delay());
			});
			illegal_client.bind_connect([&]()
			{
				ASIO2_CHECK(illegal_client.io().running_in_this_thread());
				ASIO2_CHECK(illegal_client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(illegal_client.get_local_address() == "127.0.0.1");
				ASIO2_CHECK(illegal_client.get_remote_address() == "127.0.0.1");
				ASIO2_CHECK(illegal_client.get_remote_port() == 18026);

				illegal_client_connect_counter++;

				std::vector<std::uint8_t> msg;

				for (int n = 20 + std::rand() % 100; n > 0; n--)
					msg.emplace_back(chars.at(std::rand() % chars.size()));

				illegal_client.async_send(std::move(msg));
			});
			illegal_client.bind_disconnect([&]()
			{
				illegal_client_disconnect_counter++;

				ASIO2_CHECK(asio2::get_last_error());
				ASIO2_CHECK(illegal_client.io().running_in_this_thread());
				ASIO2_CHECK(illegal_client.iopool().get(0)->running_in_this_thread());
			});
			illegal_client.bind_recv([&]([[maybe_unused]] std::string_view data)
			{
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(illegal_client.io().running_in_this_thread());
				ASIO2_CHECK(illegal_client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!data.empty());
				ASIO2_CHECK(illegal_client.is_started());
				//illegal_client.async_send(data);
			});

			bool illegal_client_start_ret = illegal_client.async_start("127.0.0.1", 18026);

			ASIO2_CHECK(illegal_client_start_ret);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
		}

		for (int i = 0; i < test_client_count; i++)
		{
			auto& clt = clients[i];

			std::vector<std::uint8_t> msg;

			for (int n = 20 + std::rand() % 100; n > 0; n--)
				msg.emplace_back(chars.at(std::rand() % chars.size()));

			msg.insert(msg.begin(), std::uint8_t(msg.size()));
			msg.insert(msg.begin(), '#');

			clt->async_send(std::move(msg));
		}

		while (client_disconnect_counter != test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		//// use this to ensure the ASIO2_CHECK(session_ptr->is_started());
		//while (server_disconnect_counter != test_client_count)
		//{
		//	ASIO2_TEST_WAIT_CHECK();
		//}

		//ASIO2_CHECK_VALUE(server_disconnect_counter.load(), server_disconnect_counter == test_client_count);
		//ASIO2_CHECK_VALUE(server_recv_counter.load(), server_recv_counter == test_client_count * 20);
		ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->stop();
			ASIO2_CHECK(clients[i]->is_stopped());
		}

		for (int i = 0; i < test_client_count; i++)
		{
			illegal_clients[i]->stop();
			ASIO2_CHECK(illegal_clients[i]->is_stopped());
		}

		server.stop();
		ASIO2_CHECK(server.is_stopped());

		ASIO2_CHECK_VALUE(server_stop_counter      .load(), server_stop_counter       == 1);

		//-----------------------------------------------------------------------------------------

		//ASIO2_CHECK_VALUE(server.get_session_count(), server.get_session_count() == std::size_t(0));

		//server_init_counter = 0;
		//server_start_counter = 0;
		//server_disconnect_counter = 0;
		//server_stop_counter = 0;
		//server_accept_counter = 0;
		//server_connect_counter = 0;

		//server_start_ret = server.start("127.0.0.1", 18026, match_role('#'));

		//ASIO2_CHECK(server_start_ret);
		//ASIO2_CHECK(server.is_started());

		//ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		//ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		//client_init_counter = 0;
		//client_connect_counter = 0;
		//client_disconnect_counter = 0;

		//for (int i = 0; i < test_client_count; i++)
		//{
		//	bool client_start_ret = clients[i]->async_start("127.0.0.1", 18026, match_role_func);
		//	ASIO2_CHECK(client_start_ret);
		//	ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
		//}

		//while (server.get_session_count() < std::size_t(test_client_count))
		//{
		//	ASIO2_TEST_WAIT_CHECK();
		//}

		//ASIO2_CHECK_VALUE(server.get_session_count(), server.get_session_count() == std::size_t(test_client_count));

		//while (client_connect_counter < test_client_count)
		//{
		//	ASIO2_TEST_WAIT_CHECK();
		//}

		//ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == test_client_count);
		//ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == test_client_count);

		//ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == test_client_count);
		//ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		//for (int i = 0; i < test_client_count; i++)
		//{
		//	auto& clt = clients[i];
		//	clients[i]->post([&clt]()
		//	{
		//		std::vector<std::uint8_t> msg;

		//		int len = 200 + (std::rand() % 200);

		//		msg.resize(len);

		//		for (int i = 0; i < len; i++)
		//		{
		//			msg[i] = std::uint8_t(std::rand() % 0xff);
		//		}

		//		msg[0] = std::uint8_t(254);
		//		if (asio2::detail::is_little_endian())
		//		{
		//			msg[1] = std::uint8_t(253);
		//			msg[2] = std::uint8_t(0);
		//		}
		//		else
		//		{
		//			msg[1] = std::uint8_t(0);
		//			msg[2] = std::uint8_t(253);
		//		}
		//		msg[3] = std::uint8_t(0);
		//		msg[4] = std::uint8_t(0);
		//		msg[5] = std::uint8_t(0);
		//		msg[6] = std::uint8_t(0);
		//		msg[7] = std::uint8_t(0);
		//		msg[8] = std::uint8_t(0);

		//		asio::write(clt->socket(), asio::buffer(msg));
		//	});
		//}

		//while (client_disconnect_counter != test_client_count)
		//{
		//	ASIO2_TEST_WAIT_CHECK();
		//}

		//// use this to ensure the ASIO2_CHECK(session_ptr->is_started());
		//while (server_disconnect_counter != test_client_count)
		//{
		//	ASIO2_TEST_WAIT_CHECK();
		//}

		//ASIO2_CHECK_VALUE(server_disconnect_counter.load(), server_disconnect_counter == test_client_count);
		//ASIO2_CHECK_VALUE(server_recv_counter.load(), server_recv_counter == 0);
		//ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == test_client_count);

		//for (int i = 0; i < test_client_count; i++)
		//{
		//	clients[i]->stop();
		//	ASIO2_CHECK(clients[i]->is_stopped());
		//}

		//server.stop();
		//ASIO2_CHECK(server.is_stopped());

		//ASIO2_CHECK_VALUE(server_stop_counter      .load(), server_stop_counter       == 1);
	}

	// test match role init function with ecs
	{
		asio2::tcp_server server;
		std::atomic<int> server_recv_counter = 0;
		server.bind_recv([&](std::shared_ptr<asio2::tcp_session> & session_ptr, std::string_view data)
		{
			if (data.size() == 0)
			{
				session_ptr->stop();
				return;
			}
			if (server.iopool().size() > 1)
			{
				ASIO2_CHECK(std::addressof(session_ptr->io()) != std::addressof(server.io()));
			}
			server_recv_counter++;

			ASIO2_CHECK(!asio2::get_last_error());
			ASIO2_CHECK(session_ptr->is_started());

			session_ptr->async_send(data);

			ASIO2_CHECK(session_ptr->io().running_in_this_thread());
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
				ASIO2_CHECK(server.get_listen_port() == 18026);
				ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_address() == "127.0.0.1");
				ASIO2_CHECK(session_ptr->local_port() == 18026);
				ASIO2_CHECK(server.io().running_in_this_thread());
				ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());

				//// You can close the connection directly here.
				//if (session_ptr->remote_address() == "192.168.0.254")
				//	session_ptr->stop();
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
			ASIO2_CHECK(session_ptr->local_port() == 18026);
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
			ASIO2_CHECK(session_ptr->socket().is_open());
			ASIO2_CHECK(session_ptr->remote_address() == "127.0.0.1");
			ASIO2_CHECK(session_ptr->local_port() == 18026);
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
			ASIO2_CHECK(server.get_listen_port() == 18026);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});
		std::atomic<int> server_stop_counter = 0;
		server.bind_stop([&]()
		{
			server_stop_counter++;

			ASIO2_CHECK(asio2::get_last_error());
			ASIO2_CHECK(server.get_listen_address() == "127.0.0.1");
			ASIO2_CHECK(server.get_listen_port() == 18026);
			ASIO2_CHECK(server.io().running_in_this_thread());
			ASIO2_CHECK(server.iopool().get(0)->running_in_this_thread());
		});

		asio2::rdc::option rdc_option
		{
			[](std::string_view)
			{
				return 0;
			}
		};

		bool server_start_ret = server.start("127.0.0.1", 18026, match_role('#'), rdc_option);

		ASIO2_CHECK(server_start_ret);
		ASIO2_CHECK(server.is_started());

		ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		std::vector<std::shared_ptr<asio2::tcp_client>> clients;
		std::atomic<int> client_init_counter = 0;
		std::atomic<int> client_connect_counter = 0;
		std::atomic<int> client_disconnect_counter = 0;
		std::atomic<int> client_recv_counter = 0;
		for (int i = 0; i < test_client_count; i++)
		{
			auto iter = clients.emplace_back(std::make_shared<asio2::tcp_client>());

			asio2::tcp_client& client = *iter;

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
				ASIO2_CHECK(client.get_remote_port() == 18026);

				client_connect_counter++;
			});
			client.bind_disconnect([&]()
			{
				client_disconnect_counter++;

				ASIO2_CHECK(asio2::get_last_error());
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
			});
			client.bind_recv([&]([[maybe_unused]] std::string_view data)
			{
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(client.io().running_in_this_thread());
				ASIO2_CHECK(client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!data.empty());
				ASIO2_CHECK(client.is_started());
				//client.async_send(data);

				client_recv_counter++;

				std::vector<std::uint8_t> msg;

				for (int n = 20 + std::rand() % 100; n > 0; n--)
					msg.emplace_back(chars.at(std::rand() % chars.size()));

				msg.insert(msg.begin(), std::uint8_t(msg.size()));
				msg.insert(msg.begin(), '#');

				if (client_recv_counter > test_client_count * 20)
					msg.insert(msg.begin(), '0');

				client.async_send(std::move(msg));
			});

			bool client_start_ret = client.async_start("127.0.0.1", 18026, match_role_func);

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

		ASIO2_CHECK_VALUE(client_init_counter.load(), client_init_counter == test_client_count);
		ASIO2_CHECK_VALUE(client_connect_counter.load(), client_connect_counter == test_client_count);

		ASIO2_CHECK_VALUE(server_accept_counter.load(), server_accept_counter == test_client_count);
		ASIO2_CHECK_VALUE(server_connect_counter.load(), server_connect_counter == test_client_count);

		std::vector<std::shared_ptr<asio2::tcp_client>> illegal_clients;
		std::atomic<int> illegal_client_init_counter = 0;
		std::atomic<int> illegal_client_connect_counter = 0;
		std::atomic<int> illegal_client_disconnect_counter = 0;
		for (int i = 0; i < test_client_count; i++)
		{
			auto iter = illegal_clients.emplace_back(std::make_shared<asio2::tcp_client>());

			asio2::tcp_client& illegal_client = *iter;

			// disable auto reconnect, default reconnect option is "enable"
			illegal_client.set_auto_reconnect(true, std::chrono::milliseconds(100));

			illegal_client.bind_init([&]()
			{
				illegal_client_init_counter++;

				illegal_client.set_no_delay(true);

				ASIO2_CHECK(illegal_client.io().running_in_this_thread());
				ASIO2_CHECK(illegal_client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(illegal_client.is_keep_alive());
				ASIO2_CHECK(illegal_client.is_reuse_address());
				ASIO2_CHECK(illegal_client.is_no_delay());
			});
			illegal_client.bind_connect([&]()
			{
				ASIO2_CHECK(illegal_client.io().running_in_this_thread());
				ASIO2_CHECK(illegal_client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(illegal_client.get_local_address() == "127.0.0.1");
				ASIO2_CHECK(illegal_client.get_remote_address() == "127.0.0.1");
				ASIO2_CHECK(illegal_client.get_remote_port() == 18026);

				illegal_client_connect_counter++;

				std::vector<std::uint8_t> msg;

				for (int n = 20 + std::rand() % 100; n > 0; n--)
					msg.emplace_back(chars.at(std::rand() % chars.size()));

				illegal_client.async_send(std::move(msg));
			});
			illegal_client.bind_disconnect([&]()
			{
				illegal_client_disconnect_counter++;

				ASIO2_CHECK(asio2::get_last_error());
				ASIO2_CHECK(illegal_client.io().running_in_this_thread());
				ASIO2_CHECK(illegal_client.iopool().get(0)->running_in_this_thread());
			});
			illegal_client.bind_recv([&]([[maybe_unused]] std::string_view data)
			{
				ASIO2_CHECK(!asio2::get_last_error());
				ASIO2_CHECK(illegal_client.io().running_in_this_thread());
				ASIO2_CHECK(illegal_client.iopool().get(0)->running_in_this_thread());
				ASIO2_CHECK(!data.empty());
				ASIO2_CHECK(illegal_client.is_started());
				//illegal_client.async_send(data);
			});

			bool illegal_client_start_ret = illegal_client.async_start("127.0.0.1", 18026);

			ASIO2_CHECK(illegal_client_start_ret);
			ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
		}

		for (int i = 0; i < test_client_count; i++)
		{
			auto& clt = clients[i];

			std::vector<std::uint8_t> msg;

			for (int n = 20 + std::rand() % 100; n > 0; n--)
				msg.emplace_back(chars.at(std::rand() % chars.size()));

			msg.insert(msg.begin(), std::uint8_t(msg.size()));
			msg.insert(msg.begin(), '#');

			clt->async_send(std::move(msg));
		}

		while (client_disconnect_counter != test_client_count)
		{
			ASIO2_TEST_WAIT_CHECK();
		}

		//// use this to ensure the ASIO2_CHECK(session_ptr->is_started());
		//while (server_disconnect_counter != test_client_count)
		//{
		//	ASIO2_TEST_WAIT_CHECK();
		//}

		//ASIO2_CHECK_VALUE(server_disconnect_counter.load(), server_disconnect_counter == test_client_count);
		//ASIO2_CHECK_VALUE(server_recv_counter.load(), server_recv_counter == test_client_count * 20);
		ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == test_client_count);

		for (int i = 0; i < test_client_count; i++)
		{
			clients[i]->stop();
			ASIO2_CHECK(clients[i]->is_stopped());
		}

		for (int i = 0; i < test_client_count; i++)
		{
			illegal_clients[i]->stop();
			ASIO2_CHECK(illegal_clients[i]->is_stopped());
		}

		server.stop();
		ASIO2_CHECK(server.is_stopped());

		ASIO2_CHECK_VALUE(server_stop_counter      .load(), server_stop_counter       == 1);

		//-----------------------------------------------------------------------------------------

		//ASIO2_CHECK_VALUE(server.get_session_count(), server.get_session_count() == std::size_t(0));

		//server_init_counter = 0;
		//server_start_counter = 0;
		//server_disconnect_counter = 0;
		//server_stop_counter = 0;
		//server_accept_counter = 0;
		//server_connect_counter = 0;

		//server_start_ret = server.start("127.0.0.1", 18026, match_role('#'));

		//ASIO2_CHECK(server_start_ret);
		//ASIO2_CHECK(server.is_started());

		//ASIO2_CHECK_VALUE(server_init_counter      .load(), server_init_counter       == 1);
		//ASIO2_CHECK_VALUE(server_start_counter     .load(), server_start_counter      == 1);

		//client_init_counter = 0;
		//client_connect_counter = 0;
		//client_disconnect_counter = 0;

		//for (int i = 0; i < test_client_count; i++)
		//{
		//	bool client_start_ret = clients[i]->async_start("127.0.0.1", 18026, match_role_func);
		//	ASIO2_CHECK(client_start_ret);
		//	ASIO2_CHECK(asio2::get_last_error() == asio::error::in_progress);
		//}

		//while (server.get_session_count() < std::size_t(test_client_count))
		//{
		//	ASIO2_TEST_WAIT_CHECK();
		//}

		//ASIO2_CHECK_VALUE(server.get_session_count(), server.get_session_count() == std::size_t(test_client_count));

		//while (client_connect_counter < test_client_count)
		//{
		//	ASIO2_TEST_WAIT_CHECK();
		//}

		//ASIO2_CHECK_VALUE(client_init_counter      .load(), client_init_counter    == test_client_count);
		//ASIO2_CHECK_VALUE(client_connect_counter   .load(), client_connect_counter == test_client_count);

		//ASIO2_CHECK_VALUE(server_accept_counter    .load(), server_accept_counter     == test_client_count);
		//ASIO2_CHECK_VALUE(server_connect_counter   .load(), server_connect_counter    == test_client_count);

		//for (int i = 0; i < test_client_count; i++)
		//{
		//	auto& clt = clients[i];
		//	clients[i]->post([&clt]()
		//	{
		//		std::vector<std::uint8_t> msg;

		//		int len = 200 + (std::rand() % 200);

		//		msg.resize(len);

		//		for (int i = 0; i < len; i++)
		//		{
		//			msg[i] = std::uint8_t(std::rand() % 0xff);
		//		}

		//		msg[0] = std::uint8_t(254);
		//		if (asio2::detail::is_little_endian())
		//		{
		//			msg[1] = std::uint8_t(253);
		//			msg[2] = std::uint8_t(0);
		//		}
		//		else
		//		{
		//			msg[1] = std::uint8_t(0);
		//			msg[2] = std::uint8_t(253);
		//		}
		//		msg[3] = std::uint8_t(0);
		//		msg[4] = std::uint8_t(0);
		//		msg[5] = std::uint8_t(0);
		//		msg[6] = std::uint8_t(0);
		//		msg[7] = std::uint8_t(0);
		//		msg[8] = std::uint8_t(0);

		//		asio::write(clt->socket(), asio::buffer(msg));
		//	});
		//}

		//while (client_disconnect_counter != test_client_count)
		//{
		//	ASIO2_TEST_WAIT_CHECK();
		//}

		//// use this to ensure the ASIO2_CHECK(session_ptr->is_started());
		//while (server_disconnect_counter != test_client_count)
		//{
		//	ASIO2_TEST_WAIT_CHECK();
		//}

		//ASIO2_CHECK_VALUE(server_disconnect_counter.load(), server_disconnect_counter == test_client_count);
		//ASIO2_CHECK_VALUE(server_recv_counter.load(), server_recv_counter == 0);
		//ASIO2_CHECK_VALUE(client_disconnect_counter.load(), client_disconnect_counter == test_client_count);

		//for (int i = 0; i < test_client_count; i++)
		//{
		//	clients[i]->stop();
		//	ASIO2_CHECK(clients[i]->is_stopped());
		//}

		//server.stop();
		//ASIO2_CHECK(server.is_stopped());

		//ASIO2_CHECK_VALUE(server_stop_counter      .load(), server_stop_counter       == 1);
	}

	ASIO2_TEST_END_LOOP;
}


ASIO2_TEST_SUITE
(
	"tcp_custom",
	ASIO2_TEST_CASE(tcp_custom_test)
)
