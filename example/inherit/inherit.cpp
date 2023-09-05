#include <asio2/asio2.hpp>
#include <asio2/tcp/tcp_server.hpp>
#include <asio2/tcp/tcp_client.hpp>
#include <iostream>

class my_tcp_session : public asio2::tcp_session_t<my_tcp_session>
{
public:
	//using asio2::tcp_session_t<my_tcp_session>::tcp_session_t;

	template<class... Args>
	my_tcp_session(Args&&... args) : asio2::tcp_session_t<my_tcp_session>(std::forward<Args>(args)...)
	{
		uuid = "custom string";
	}

	// ... user custom properties and functions
	std::string uuid;

	/**
	 * @brief Pre process the data before send it.
	 * You can overload this function in a derived class to implement additional
	 * processing of the data. eg: encrypt data with a custom encryption algorithm.
	 * @note this function must be public, or define "friend class asio2::detail::data_filter_before_helper;"
	 */
	template<class T>
	inline auto data_filter_before_send(T&& data)
	{
		std::string_view sv = asio2::to_string_view(asio::buffer(data));

		ASIO2_ASSERT(sv == "<0123456789abcdefghijklmnopqrstovwxyz>");

		for (const char& c : sv)
		{
			const_cast<char&>(c) ^= 'X';
		}

		return std::forward<T>(data);
	}

	/**
	 * @brief Pre process the data before recv callback was called.
	 * You can overload this function in a derived class to implement additional
	 * processing of the data. eg: decrypt data with a custom encryption algorithm.
	 * @note this function must be public, or define "friend class asio2::detail::data_filter_before_helper;"
	 */
	inline std::string_view data_filter_before_recv(std::string_view data)
	{
		for (const char& c : data)
		{
			const_cast<char&>(c) ^= 'X';
		}

		ASIO2_ASSERT(data == "<0123456789abcdefghijklmnopqrstovwxyz>");

		return data;
	}
};

using my_tcp_server1 = asio2::tcp_server_t<my_tcp_session>;

class my_tcp_server2 : public asio2::tcp_server_t<my_tcp_session>
{
public:
	using asio2::tcp_server_t<my_tcp_session>::tcp_server_t;
};

class my_tcp_client1 : public asio2::tcp_client_t<my_tcp_client1>
{
public:
	// ... user custom properties and functions
	std::string uuid;

	template<class T>
	inline auto data_filter_before_send(T&& data)
	{
		std::string_view sv = asio2::to_string_view(asio::buffer(data));

		ASIO2_ASSERT(sv == "<0123456789abcdefghijklmnopqrstovwxyz>");

		for (const char& c : sv)
		{
			const_cast<char&>(c) ^= 'X';
		}

		return std::forward<T>(data);
	}

	inline std::string_view data_filter_before_recv(std::string_view data)
	{
		for (const char& c : data)
		{
			const_cast<char&>(c) ^= 'X';
		}

		ASIO2_ASSERT(data == "<0123456789abcdefghijklmnopqrstovwxyz>");

		return data;
	}
};

class my_tcp_client2 : public asio2::tcp_client
{
public:
	// ... user custom properties and functions
	std::string uuid;
};

int main()
{
	my_tcp_server1 my_server1;

	my_server1.bind_connect([&](std::shared_ptr<my_tcp_session>& session_ptr)
	{
		session_ptr->uuid = std::to_string(session_ptr->hash_key());

	}).bind_recv([&](std::shared_ptr<my_tcp_session>& session_ptr, std::string_view data)
	{
		asio2::ignore_unused(session_ptr, data);

		ASIO2_ASSERT(session_ptr->uuid == std::to_string(session_ptr->hash_key()));
		ASIO2_ASSERT(data == "<0123456789abcdefghijklmnopqrstovwxyz>");

		printf("recv : %zu %.*s\n", data.size(), (int)data.size(), data.data());

		session_ptr->async_send(data);
	});

	my_server1.start("0.0.0.0", 9981);

	// --------------------------------------------------------------------------------

	my_tcp_client1 my_client1;

	my_client1.bind_connect([&]()
	{
		if (!asio2::get_last_error())
			my_client1.async_send("<0123456789abcdefghijklmnopqrstovwxyz>");

	}).bind_recv([&](std::string_view data)
	{
		//printf("recv : %zu %.*s\n", data.size(), (int)data.size(), data.data());
		ASIO2_ASSERT(data == "<0123456789abcdefghijklmnopqrstovwxyz>");

		my_client1.async_send(data);
	});

	my_client1.async_start("127.0.0.1", 9981);

	while (std::getchar() != '\n');

	return 0;
}
