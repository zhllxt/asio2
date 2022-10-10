//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2021 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <asio2/base/detail/push_options.hpp>

#define ASIO_STANDALONE
#include "asio.hpp"
#include <algorithm>
#include <iostream>
#include <list>
#include "handler_allocator.hpp"

class session
{
public:
  session(asio::io_context& ioc, size_t block_size)
    : io_context_(ioc),
      strand_(ioc.get_executor()),
      socket_(ioc),
      block_size_(block_size),
      read_data_(new char[block_size]),
      read_data_length_(0),
      write_data_(new char[block_size]),
      unsent_count_(0),
      op_count_(0)
  {
  }

  ~session()
  {
    delete[] read_data_;
    delete[] write_data_;
  }

  asio::ip::tcp::socket& socket()
  {
    return socket_;
  }

  void start()
  {
    asio::error_code set_option_err;
    asio::ip::tcp::no_delay no_delay(true);
    socket_.set_option(no_delay, set_option_err);
    if (!set_option_err)
    {
      ++op_count_;
      socket_.async_read_some(asio::buffer(read_data_, block_size_),
          asio::bind_executor(strand_,
            make_custom_alloc_handler(read_allocator_,
              std::bind(&session::handle_read, this,
				  std::placeholders::_1,
				  std::placeholders::_2))));
    }
    else
    {
      asio::post(io_context_, std::bind(&session::destroy, this));
    }
  }

  void handle_read(const asio::error_code& err, size_t length)
  {
    --op_count_;

    if (!err)
    {
		if (first)
		{
			first = false;
			time1 = std::chrono::steady_clock::now();
			time2 = std::chrono::steady_clock::now();
		}

		recvd_bytes += length;

		decltype(std::chrono::steady_clock::now()) time3 = std::chrono::steady_clock::now();
		auto ms = std::chrono::duration_cast<std::chrono::seconds>(time3 - time2).count();
		if (ms > 1)
		{
			time2 = time3;
			ms = std::chrono::duration_cast<std::chrono::seconds>(time3 - time1).count();
			double speed = (double)recvd_bytes / (double)ms / double(1024) / double(1024);
			printf("%.1lf MByte/Sec\n", speed);
		}

      read_data_length_ = length;
      ++unsent_count_;
      if (unsent_count_ == 1)
      {
        op_count_ += 2;
        std::swap(read_data_, write_data_);
        async_write(socket_, asio::buffer(write_data_, read_data_length_),
            asio::bind_executor(strand_,
              make_custom_alloc_handler(write_allocator_,
                std::bind(&session::handle_write, this,
					std::placeholders::_1))));
        socket_.async_read_some(asio::buffer(read_data_, block_size_),
            asio::bind_executor(strand_,
              make_custom_alloc_handler(read_allocator_,
                std::bind(&session::handle_read, this,
					std::placeholders::_1,
					std::placeholders::_2))));
      }
    }

    if (op_count_ == 0)
      asio::post(io_context_, std::bind(&session::destroy, this));
  }

  void handle_write(const asio::error_code& err)
  {
    --op_count_;

    if (!err)
    {
      --unsent_count_;
      if (unsent_count_ == 1)
      {
        op_count_ += 2;
        std::swap(read_data_, write_data_);
        async_write(socket_, asio::buffer(write_data_, read_data_length_),
            asio::bind_executor(strand_,
              make_custom_alloc_handler(write_allocator_,
                std::bind(&session::handle_write, this,
					std::placeholders::_1))));
        socket_.async_read_some(asio::buffer(read_data_, block_size_),
            asio::bind_executor(strand_,
              make_custom_alloc_handler(read_allocator_,
                std::bind(&session::handle_read, this,
					std::placeholders::_1,
					std::placeholders::_2))));
      }
    }

    if (op_count_ == 0)
      asio::post(io_context_, std::bind(&session::destroy, this));
  }

  static void destroy(session* s)
  {
    delete s;
  }

private:
  asio::io_context& io_context_;
  asio::strand<asio::io_context::executor_type> strand_;
  asio::ip::tcp::socket socket_;
  size_t block_size_;
  char* read_data_;
  size_t read_data_length_;
  char* write_data_;
  int unsent_count_;
  int op_count_;
  handler_allocator read_allocator_;
  handler_allocator write_allocator_;

  decltype(std::chrono::steady_clock::now()) time1 = std::chrono::steady_clock::now();
  decltype(std::chrono::steady_clock::now()) time2 = std::chrono::steady_clock::now();
  std::size_t recvd_bytes = 0;
  bool first = true;
};

class server
{
public:
  server(asio::io_context& ioc, const asio::ip::tcp::endpoint& endpoint,
      size_t block_size)
    : io_context_(ioc),
      acceptor_(ioc),
      block_size_(block_size)
  {
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(1));
    acceptor_.bind(endpoint);
    acceptor_.listen();

    start_accept();
  }

  void start_accept()
  {
    session* new_session = new session(io_context_, block_size_);
    acceptor_.async_accept(new_session->socket(),
        std::bind(&server::handle_accept, this, new_session,
			std::placeholders::_1));
  }

  void handle_accept(session* new_session, const asio::error_code& err)
  {
    if (!err)
    {
      new_session->start();
    }
    else
    {
      delete new_session;
    }

    start_accept();
  }

private:
  asio::io_context& io_context_;
  asio::ip::tcp::acceptor acceptor_;
  size_t block_size_;
};

int main(int argc, char* argv[])
{
  (void)argc;
  (void)argv;
  try
  {

    using namespace std; // For atoi.
    asio::ip::address address = asio::ip::make_address("0.0.0.0");
    short port = short(atoi("8080"));
    int thread_count = 1;
    size_t block_size = 1024;

    asio::io_context ioc;

    server s(ioc, asio::ip::tcp::endpoint(address, port), block_size);

    // Threads not currently supported in this test.
    std::list<asio::thread*> threads;
    while (--thread_count > 0)
    {
		asio::thread* new_thread = new asio::thread([&ioc]()
		{
			ioc.run();
	  }
          /*std::bind(&asio::io_context::run, &ioc)*/);
      threads.push_back(new_thread);
    }

    ioc.run();

    while (!threads.empty())
    {
      threads.front()->join();
      delete threads.front();
      threads.pop_front();
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}

#include <asio2/base/detail/pop_options.hpp>
