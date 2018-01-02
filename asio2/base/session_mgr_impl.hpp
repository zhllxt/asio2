/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_SESSION_MGR_IMPL_HPP__
#define __ASIO2_SESSION_MGR_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/session_mgr.hpp>
#include <asio2/base/session_mgr_impl.hpp>

namespace asio2
{

	/**
	 * the session_impl manager interface
	 */
	template<
		class _key,
		class _hasher = std::hash<_key>,
		class _equaler = std::equal_to<_key>
	>
	class session_mgr_t : public session_mgr
	{
	public:
		/**
		 * @construct
		 */
		explicit session_mgr_t() :session_mgr()
		{
			m_sessions.reserve(64);
		}

		/**
		 * @destruct
		 */
		virtual ~session_mgr_t()
		{
		}

		/**
		 * @function : start and emplace session_impl
		 */
		virtual bool start(const std::shared_ptr<session_impl> & session_ptr) override
		{
			wlock_guard g(m_rwlock);
			if (m_stopped == false && session_ptr)
			{
				m_sessions.emplace(static_cast<_key>(session_ptr->_get_key()), session_ptr);
				return true;
			}
			return false;
		}

		/**
		 * @function : stop and erase session_impl
		 */
		virtual void stop(const std::shared_ptr<session_impl> & session_ptr) override
		{
			if (session_ptr)
			{
				wlock_guard g(m_rwlock);
				m_sessions.erase(static_cast<_key>(session_ptr->_get_key()));
			}
		}

		/**
		 * @function : stop all sessions
		 */
		virtual void stop_all() override
		{
			m_stopped = true;
			for_each_session([](std::shared_ptr<session_impl> & session_ptr)
			{
				session_ptr->stop();
			});
		}

		/**
		 * @function : call user custom function for every session_impl in the session_impl map
		 * the handler is like this :
		 * void on_callback(std::shared_ptr<session_impl> & session_ptr)
		 */
		virtual void for_each_session(const std::function<void(std::shared_ptr<session_impl> & session_ptr)> & handler) override
		{
			if (m_sessions.size() > 0)
			{
				rlock_guard g(m_rwlock);
				for (auto & pair : m_sessions)
				{
					handler(pair.second);
				}
			}
		}

		/**
		 * @function : find the session_impl by map key
		 * @return   : session_impl shared_ptr reference
		 */
		virtual std::shared_ptr<session_impl> & find_session(void * key) override
		{
			if (m_sessions.size() > 0)
			{
				rlock_guard g(m_rwlock);
				auto iter = m_sessions.find(static_cast<_key>(key));
				if (iter != m_sessions.end())
					return iter->second;
			}
			assert(!empty_session);
			return empty_session;
		}

		/**
		 * @function : get session_impl count
		 */
		virtual std::size_t get_session_count() override
		{
			return m_sessions.size();
		}

	protected:
		/// session_impl unorder map,these session_impl is already connected session_impl 
		std::unordered_map<_key, std::shared_ptr<session_impl>, _hasher, _equaler> m_sessions;

		/// use rwlock to make this session_impl map thread safe
		rwlock                        m_rwlock;

		volatile bool                 m_stopped = false;

		// just used for return a null shared_ptr object
		std::shared_ptr<session_impl> empty_session;
	};
}

#endif // !__ASIO2_SESSION_MGR_IMPL_HPP__
