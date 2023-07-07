/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_SESSION_MGR_HPP__
#define __ASIO2_SESSION_MGR_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <algorithm>
#include <memory>
#include <functional>
#include <unordered_map>
#include <type_traits>

#include <asio2/base/iopool.hpp>
#include <asio2/base/define.hpp>
#include <asio2/base/log.hpp>

#include <asio2/base/detail/allocator.hpp>
#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/shared_mutex.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SESSION;
	ASIO2_CLASS_FORWARD_DECLARE_UDP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_UDP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_UDP_SESSION;

	/**
	 * the session manager interface
	 */
	template<class session_t>
	class session_mgr_t
	{
		friend session_t; // C++11

		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION;
		ASIO2_CLASS_FRIEND_DECLARE_UDP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_UDP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_UDP_SESSION;

	protected:
	#if defined(_DEBUG) || defined(DEBUG)
		class [[maybe_unused]] deadlock_checker_value
		{
		public:
			[[maybe_unused]] static bool& get() noexcept
			{
				thread_local static bool b = false;

				return b;
			}
		};

		struct deadlock_checker_guard
		{
			deadlock_checker_guard(bool& b) : b_(b)
			{
				ASIO2_ASSERT(b_ == false);
				b_ = true;
			}
			~deadlock_checker_guard()
			{
				ASIO2_ASSERT(b_ == true);
				b_ = false;
			}
			bool& b_;
		};
	#endif

	public:
		using self     = session_mgr_t<session_t>;
		using args_type = typename session_t::args_type;
		using key_type  = typename session_t::key_type;

		/**
		 * @brief constructor
		 */
		explicit session_mgr_t(std::shared_ptr<io_t> acceptor_io, std::atomic<state_t>& server_state)
			: io_   (std::move(acceptor_io))
			, state_(server_state)
		{
			this->sessions_.reserve(64);
		}

		/**
		 * @brief destructor
		 */
		~session_mgr_t() = default;

		/**
		 * @brief emplace the session
		 * @callback : void(bool inserted);
		 */
		template<class Fun>
		inline void emplace(std::shared_ptr<session_t> session_ptr, Fun&& callback)
		{
			if (!session_ptr)
				return;

			asio::dispatch(this->io_->context(), make_allocator(this->allocator_,
			[this, session_ptr = std::move(session_ptr), callback = std::forward<Fun>(callback)]
			() mutable
			{
				bool inserted = false;

				// when run to here, the server state maybe started or stopping or stopped, 
				// if server state is not started, must can't push the session to the map
				// again, and we need disconnect the session directly, otherwise the server
				// maybe stopping, and the iopool's wait_iothreas is running in the "sleep"
				// this will cause the server.stop() never return;
				if (this->state_ == state_t::started)
				{
					// when code run to here, user maybe call server.stop() at other thread,
					// if user do this, at this time, the state_ is not started already( it
					// will be stopping ), but beacuase the server's sessions_.for_each -> 
					// session_ptr->stop(); is execute in the thread 0, and this code is 
					// executed in thread 0 too, so when code run to here, beacuse we have
					// " if (this->state_ == state_t::started) " judgment statement, so the
					// server's sessions_.for_each -> session_ptr->stop(); must not be
					// executed yet, so even if we put the session in the map here, it will
					// not have a problem, beacuse the server's sessions_.for_each -> 
					// session_ptr->stop(); will be called a later, and this session will be 
					// stopped at there.

					// we use a assert to check the server's sessions_.for_each -> 
					// session_ptr->stop(); must not be executed yet.
				#if defined(_DEBUG) || defined(DEBUG)
					ASIO2_ASSERT(is_all_session_stop_called_ == false);
					ASIO2_ASSERT(deadlock_checker_value::get() == false);
				#endif

					// this thread is same as the server's io thread, when code run to here,
					// the server's _post_stop must not be executed, so the server's sessions_.for_each
					// -> session_ptr->stop() must not be executed.
					asio2::unique_locker guard(this->mutex_);
					inserted = this->sessions_.try_emplace(session_ptr->hash_key(), session_ptr).second;

				#if defined(_DEBUG) || defined(DEBUG)
					ASIO2_ASSERT(is_all_session_stop_called_ == false);
				#endif
				}

				(callback)(inserted);
			}));
		}

		/**
		 * @brief erase the session
		 * @callback : void(bool erased);
		 */
		template<class Fun>
		inline void erase(std::shared_ptr<session_t> session_ptr, Fun&& callback)
		{
			if (!session_ptr)
				return;

			asio::dispatch(this->io_->context(), make_allocator(this->allocator_,
			[this, session_ptr = std::move(session_ptr), callback = std::forward<Fun>(callback)]
			() mutable
			{
				bool erased = false;

			#if defined(_DEBUG) || defined(DEBUG)
				ASIO2_ASSERT(deadlock_checker_value::get() == false);
			#endif

				{
					asio2::unique_locker guard(this->mutex_);
					erased = (this->sessions_.erase(session_ptr->hash_key()) > 0);
				}

				(callback)(erased);
			}));
		}

		/**
		 * @brief Submits a completion token or function object for execution.
		 * @task : void();
		 */
		template<class Fun>
		inline void post(Fun&& task)
		{
			asio::post(this->io_->context(), make_allocator(this->allocator_, std::forward<Fun>(task)));
		}

		/**
		 * @brief Submits a completion token or function object for execution.
		 * @task : void();
		 */
		template<class Fun>
		inline void dispatch(Fun&& task)
		{
			asio::dispatch(this->io_->context(), make_allocator(this->allocator_, std::forward<Fun>(task)));
		}

		/**
		 * @brief call user custom callback function for every session
		 * the custom callback function is like this :
		 * void on_callback(std::shared_ptr<tcp_session> & session_ptr)
		 */
		template<class Fun>
		inline void for_each(Fun&& fn)
		{
			// thred safety for each
			// 
			std::vector<std::shared_ptr<session_t>> sessions;

		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(deadlock_checker_value::get() == false);
		#endif

			{
				asio2::shared_locker guard(this->mutex_);

				sessions.reserve(this->sessions_.size());

				for (const auto& [k, session_ptr] : this->sessions_)
				{
					std::ignore = k;

					sessions.emplace_back(session_ptr);
				}
			}

			for (std::shared_ptr<session_t>& session_ptr : sessions)
			{
				fn(session_ptr);
			}


			// if the unique locker was called in the callback inner, then will cause deadlock.
			// and if the callback is a time-consuming operation, the new session will can't enter.
			// 
			//asio2::shared_locker guard(this->mutex_);

			//for (auto& [k, session_ptr] : this->sessions_)
			//{
			//	std::ignore = k;

			//	fn(session_ptr);
			//}
		}

		/**
		 * @brief call user custom callback function for every session
		 * the custom callback function is like this :
		 * void on_callback(std::shared_ptr<tcp_session> & session_ptr)
		 */
		template<class Fun>
		inline void quick_for_each(Fun&& fn)
		{
			// if the unique locker was called in the callback inner, then will cause deadlock.
			// and if the callback is a time-consuming operation, the new session will can't enter.

			asio2::shared_locker guard(this->mutex_);

		#if defined(_DEBUG) || defined(DEBUG)
			[[maybe_unused]] deadlock_checker_guard leg(deadlock_checker_value::get());
		#endif

			for (auto& [k, session_ptr] : this->sessions_)
			{
				std::ignore = k;

				fn(session_ptr);
			}
		}

		/**
		 * @brief find the session by map key
		 */
		inline std::shared_ptr<session_t> find(const key_type & key)
		{
		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(deadlock_checker_value::get() == false);
		#endif

			asio2::shared_locker guard(this->mutex_);
			auto iter = this->sessions_.find(key);
			return (iter == this->sessions_.end() ? std::shared_ptr<session_t>() : iter->second);
		}

		/**
		 * @brief find the session by user custom role
		 * bool on_callback(std::shared_ptr<tcp_session> & session_ptr)
		 */
		template<class Fun>
		inline std::shared_ptr<session_t> find_if(Fun&& fn)
		{
		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(deadlock_checker_value::get() == false);
		#endif

			// if the unique locker was called in the callback inner, then will cause deadlock.
			asio2::shared_locker guard(this->mutex_);
			auto iter = std::find_if(this->sessions_.begin(), this->sessions_.end(),
			[&fn](auto &pair) mutable
			{
				return fn(pair.second);
			});
			return (iter == this->sessions_.end() ? std::shared_ptr<session_t>() : iter->second);
		}

		/**
		 * @brief get session count
		 */
		inline std::size_t size() const noexcept
		{
		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(deadlock_checker_value::get() == false);
		#endif

			asio2::shared_locker guard(this->mutex_);

			// is std map.size() thread safety?
			// 
			// https://stackoverflow.com/questions/2170541/what-operations-are-thread-safe-on-stdmap
			// https://en.cppreference.com/w/cpp/container
			// https://timsong-cpp.github.io/cppwp/n3337/container.requirements.dataraces
			// https://stackoverflow.com/questions/15067160/stdmap-thread-safety
			// https://stackoverflow.com/questions/14127379/does-const-mean-thread-safe-in-c11

			// There is no one consensus:

			// The C++11 standard guarantees that const method access to containers is safe from
			// different threads (ie, both use const methods).

			// It should be thread-safe to call a const function from multiple threads simultaneously,
			// without calling a non-const function at the same time in another thread.

			// after my test on windows and linux:
			// multithread call const function without mutex, and multithread call no-const function 
			// with mutex at the same time, the result of the const function seems to be no problem.

			//#include <unordered_map>
			//#include <shared_mutex>
			//#include <thread>
			//
			//int main()
			//{
			//    std::shared_mutex mtx;
			//    std::unordered_map<int, int> map;
			//
			//    std::srand((unsigned int)std::time(nullptr));
			//
			//    for (std::size_t i = 0; i < std::thread::hardware_concurrency() * 2; i++)
			//    {
			//        std::thread([&]() mutable
			//        {
			//            for (;;)
			//            {
			//                int n = std::rand();
			//                std::unique_lock g(mtx);
			//                if (map.size() < 1000)
			//                    map.emplace(n, n);
			//                else
			//                    std::this_thread::sleep_for(std::chrono::milliseconds(0));
			//            }
			//        }).detach();
			//    }
			//
			//    for (std::size_t i = 0; i < std::thread::hardware_concurrency() * 2; i++)
			//    {
			//        std::thread([&]() mutable
			//        {
			//            for (;;)
			//            {
			//                std::unique_lock g(mtx);
			//                if (map.size() > 500)
			//                    map.erase(map.begin());
			//                else
			//                    std::this_thread::sleep_for(std::chrono::milliseconds(0));
			//            }
			//        }).detach();
			//    }
			//
			//    std::this_thread::sleep_for(std::chrono::seconds(5));
			//
			//    for (std::size_t i = 0; i < std::thread::hardware_concurrency() * 2; i++)
			//    {
			//        std::thread([&]() mutable
			//        {
			//            for (;;)
			//            {
			//                int n = int(map.size());
			//                if (n < 500 || n > 1000 || map.empty())
			//                {
			//                    printf("error %d\n", n);
			//                }
			//            }
			//        }).detach();
			//    }
			//
			//    while (std::getchar() != '\n');
			//
			//    return 0;
			//}

			return this->sessions_.size();
		}

		/**
		 * @brief Checks if the session container has no elements
		 */
		inline bool empty() const noexcept
		{
		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(deadlock_checker_value::get() == false);
		#endif

			asio2::shared_locker guard(this->mutex_);

			return this->sessions_.empty();
		}

		/**
		 * @brief get the io object reference
		 */
		inline io_t & io() noexcept
		{
			return (*this->io_);
		}

		/**
		 * @brief get the io object reference
		 */
		inline io_t const& io() const noexcept
		{
			return (*this->io_);
		}

	protected:
		/// use rwlock to make this session map thread safe
		mutable asio2::shared_mutexer                            mutex_;

		/// session unorder map,these session is already connected session 
		std::unordered_map<key_type, std::shared_ptr<session_t>> sessions_ ASIO2_GUARDED_BY(mutex_);

		/// the zero io_context reference in the iopool
		std::shared_ptr<io_t>                                    io_;

		/// The memory to use for handler-based custom memory allocation.
		handler_memory<std::false_type, assizer<args_type>>      allocator_;

		/// server state reference
		std::atomic<state_t>                                   & state_;

	#if defined(_DEBUG) || defined(DEBUG)
		bool                                                     is_all_session_stop_called_ = false;
	#endif
	};
}

#endif // !__ASIO2_SESSION_MGR_HPP__
