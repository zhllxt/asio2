# asio2
A open source cross-platform c++ library for network programming based on boost::asio,support for tcp,udp,http,ssl and so on.

```c++
asio2::server tcp_pack_server(" tcp://*:8099/pack?send_buffer_size=1024k & recv_buffer_size=1024K & pool_buffer_size=1024 & io_service_pool_size=5");
		tcp_pack_server.bind_recv([&tcp_pack_server](asio2::session_ptr session_ptr, asio2::buffer_ptr data_ptr)
		{
			std::printf("recv : %.*s\n", (int)data_ptr->size(), (const char*)data_ptr->data());

			int send_len = std::rand() % ((int)data_ptr->size() / 2);
			int already_send_len = 0;
			while (true)
			{
				session_ptr->send((const uint8_t *)(data_ptr->data() + already_send_len), (std::size_t)send_len);
				already_send_len += send_len;

				if ((std::size_t)already_send_len >= data_ptr->size())
					break;

				send_len = std::rand() % ((int)data_ptr->size() / 2);
				if (send_len + already_send_len > (int)data_ptr->size())
					send_len = (int)data_ptr->size() - already_send_len;

				// send for several packets,and sleep for a moment after each send is completed
				// the client will recv a full packet
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}

		}).set_pack_parser(std::bind(pack_parser, std::placeholders::_1));

		if (!tcp_pack_server.start())
			std::printf("start tcp server failed : %d - %s.\n", asio2::get_last_error(), asio2::get_last_error_desc().c_str());
		else
			std::printf("start tcp server successed : %s - %u\n", tcp_pack_server.get_listen_address().c_str(), tcp_pack_server.get_listen_port());
