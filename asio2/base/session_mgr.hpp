/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_SESSION_MGR_HPP__
#define __ASIO2_SESSION_MGR_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <algorithm>
#include <mutex>
#include <shared_mutex>
#include <memory>
#include <functional>
#include <unordered_map>
#include <type_traits>

#include <asio2/base/selector.hpp>
#include <asio2/base/iopool.hpp>

#include <asio2/base/detail/allocator.hpp>

namespace asio2::detail
{
	/**
	 * the session manager interface
	 */
	template<class session_t>
	class session_mgr_t
	{
	public:
		using self     = session_mgr_t<session_t>;
		using key_type = typename session_t::key_type;

		/**
		 * @constructor
		 */
		explicit session_mgr_t(io_t & acceptor_io) : io_(acceptor_io)
		{
			this->sessions_.reserve(64);
		}

		/**
		 * @destructor
		 */
		~session_mgr_t() = default;

		/**
		 * @function : emplace the session
		 */
		inline void emplace(std::shared_ptr<session_t> session_ptr, std::function<void(bool)> callback)
		{
			if (!session_ptr)
				return;

			if (!this->io_.strand().running_in_this_thread())
				return asio::post(this->io_.strand(), make_allocator(this->allocator_,
					std::bind(&self::emplace, this, std::move(session_ptr), std::move(callback))));

			bool inserted = false;

			{
				std::unique_lock<std::shared_mutex> guard(this->mutex_);
				inserted = this->sessions_.try_emplace(session_ptr->hash_key(), session_ptr).second;
				session_ptr->in_sessions_ = inserted;
			}

			(callback)(inserted);
		}

		/**
		 * @function : erase the session
		 */
		inline void erase(std::shared_ptr<session_t> session_ptr, std::function<void(bool)> callback)
		{
			if (!session_ptr)
				return;

			if (!this->io_.strand().running_in_this_thread())
				return asio::post(this->io_.strand(), make_allocator(this->allocator_,
					std::bind(&self::erase, this, std::move(session_ptr), std::move(callback))));

			bool erased = false;

			{
				std::unique_lock<std::shared_mutex> guard(this->mutex_);
				if (session_ptr->in_sessions_)
					erased = (this->sessions_.erase(session_ptr->hash_key()) > 0);
			}

			(callback)(erased);
		}

		/**
		 * @function : Submits a completion token or function object for execution.
		 */
		inline void post(std::function<void()> task)
		{
			asio::post(this->io_.strand(), make_allocator(this->allocator_, std::move(task)));
		}

		/**
		 * @function : call user custom callback function for every session
		 * the custom callback function is like this :
		 * void on_callback(std::shared_ptr<tcp_session> & session_ptr)
		 */
		inline void foreach(const std::function<void(std::shared_ptr<session_t> &)> & fn)
		{
			std::shared_lock<std::shared_mutex> guard(this->mutex_);
			for (auto &[k, session_ptr] : this->sessions_)
			{
				std::ignore = k;
				fn(session_ptr);
			}
		}

		/**
		 * @function : find the session by map key
		 */
		inline std::shared_ptr<session_t> find(const key_type & key)
		{
			std::shared_lock<std::shared_mutex> guard(this->mutex_);
			auto iter = this->sessions_.find(key);
			return (iter == this->sessions_.end() ? std::shared_ptr<session_t>() : iter->second);
		}

		/**
		 * @function : find the session by user custom role
		 */
		inline std::shared_ptr<session_t> find_if(const std::function<bool(std::shared_ptr<session_t> &)> & fn)
		{
			std::shared_lock<std::shared_mutex> guard(this->mutex_);
			auto iter = std::find_if(this->sessions_.begin(), this->sessions_.end(),
				[this, &fn](auto &pair)
			{
				return fn(pair.second);
			});
			return (iter == this->sessions_.end() ? std::shared_ptr<session_t>() : iter->second);
		}

		/**
		 * @function : get session count
		 */
		inline std::size_t size()
		{
			return this->sessions_.size();
		}

		/**
		 * @function : Checks if the session container has no elements
		 */
		inline bool empty()
		{
			return this->sessions_.empty();
		}

	protected:
		/// session unorder map,these session is already connected session 
		std::unordered_map<key_type, std::shared_ptr<session_t>> sessions_;

		/// use rwlock to make this session map thread safe
		std::shared_mutex mutex_;

		io_t & io_;

		/// The memory to use for handler-based custom memory allocation.
		handler_memory<size_op<>, std::true_type> allocator_;
	};
}

#endif // !__ASIO2_SESSION_MGR_HPP__
