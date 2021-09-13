/*
 * COPYRIGHT (C) 2017-2021, zhllxt
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
#include <memory>
#include <functional>
#include <unordered_map>
#include <type_traits>

#include <asio2/base/selector.hpp>
#include <asio2/base/iopool.hpp>
#include <asio2/base/define.hpp>

#include <asio2/base/detail/allocator.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SESSION;
	ASIO2_CLASS_FORWARD_DECLARE_UDP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_UDP_SESSION;

	/**
	 * the session manager interface
	 */
	template<class session_t>
	class session_mgr_t
	{
		friend session_t; // C++11

		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION;
		ASIO2_CLASS_FRIEND_DECLARE_UDP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_UDP_SESSION;

	public:
		using self     = session_mgr_t<session_t>;
		using key_type = typename session_t::key_type;

		/**
		 * @constructor
		 */
		explicit session_mgr_t(io_t& acceptor_io, std::atomic<state_t>& server_state)
			: io_   (acceptor_io )
			, state_(server_state)
		{
			this->sessions_.reserve(64);
		}

		/**
		 * @destructor
		 */
		~session_mgr_t() = default;

		/**
		 * @function : emplace the session
		 * @callback : void(bool inserted);
		 */
		template<class Fun>
		inline void emplace(std::shared_ptr<session_t> session_ptr, Fun&& callback)
		{
			if (!session_ptr)
				return;

			if (!this->io_.strand().running_in_this_thread())
			{
				asio::post(this->io_.strand(), make_allocator(this->allocator_,
				[this, session_ptr = std::move(session_ptr), callback = std::forward<Fun>(callback)]
				() mutable
				{
					this->emplace(std::move(session_ptr), std::move(callback));
				}));
				return;
			}

			bool inserted = false;

			{
				asio2_unique_lock guard(this->mutex_);
				inserted = this->sessions_.try_emplace(session_ptr->hash_key(), session_ptr).second;
				session_ptr->in_sessions_ = inserted;
			}

			(callback)(inserted);
		}

		/**
		 * @function : erase the session
		 * @callback : void(bool erased);
		 */
		template<class Fun>
		inline void erase(std::shared_ptr<session_t> session_ptr, Fun&& callback)
		{
			if (!session_ptr)
				return;

			if (!this->io_.strand().running_in_this_thread())
			{
				asio::post(this->io_.strand(), make_allocator(this->allocator_,
				[this, session_ptr = std::move(session_ptr), callback = std::forward<Fun>(callback)]
				() mutable
				{
					this->erase(std::move(session_ptr), std::move(callback));
				}));
				return;
			}

			bool erased = false;

			{
				asio2_unique_lock guard(this->mutex_);
				if (session_ptr->in_sessions_)
					erased = (this->sessions_.erase(session_ptr->hash_key()) > 0);
			}

			(callback)(erased);
		}

		/**
		 * @function : Submits a completion token or function object for execution.
		 * @task : void();
		 */
		template<class Fun>
		inline void post(Fun&& task)
		{
			asio::post(this->io_.strand(), make_allocator(this->allocator_, std::forward<Fun>(task)));
		}

		/**
		 * @function : call user custom callback function for every session
		 * the custom callback function is like this :
		 * void on_callback(std::shared_ptr<tcp_session> & session_ptr)
		 */
		template<class Fun>
		inline void for_each(Fun&& fn)
		{
			asio2_shared_lock guard(this->mutex_);
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
			asio2_shared_lock guard(this->mutex_);
			auto iter = this->sessions_.find(key);
			return (iter == this->sessions_.end() ? std::shared_ptr<session_t>() : iter->second);
		}

		/**
		 * @function : find the session by user custom role
		 * bool on_callback(std::shared_ptr<tcp_session> & session_ptr)
		 */
		template<class Fun>
		inline std::shared_ptr<session_t> find_if(Fun&& fn)
		{
			asio2_shared_lock guard(this->mutex_);
			auto iter = std::find_if(this->sessions_.begin(), this->sessions_.end(),
			[this, &fn](auto &pair) mutable
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
		asio2_shared_mutex                        mutex_;

		/// the zero io_context refrence in the iopool
		io_t                                    & io_;

		/// The memory to use for handler-based custom memory allocation.
		handler_memory<size_op<>, std::true_type> allocator_;

		/// server state refrence
		std::atomic<state_t>                    & state_;
	};
}

#endif // !__ASIO2_SESSION_MGR_HPP__
