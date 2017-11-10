/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_SESSION_MGR_HPP__
#define __ASIO2_SESSION_MGR_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <deque>
#include <unordered_map>
#include <mutex>
#include <condition_variable>

#include <boost/asio.hpp>

#include <asio2/util/rwlock.hpp>
#include <asio2/util/thread_pool.hpp>

#include <asio2/base/session_impl.hpp>


namespace asio2
{
	/**
	 * the base session manager interface
	 */
	template<
		class _Kty,
		class _Session,
		class _Hasher = std::hash<_Kty>,
		class _Keyeq = std::equal_to<_Kty>
	>
	class session_mgr_t : public std::enable_shared_from_this<session_mgr_t<_Kty, _Session, _Hasher, _Keyeq>>
	{
	public:

		/**
		 * @construct
		 */
		explicit session_mgr_t()
		{
			// init session map ptr
			m_conn_sessions.reserve(64);

			m_thread_pool_ptr = std::make_shared<thread_pool<false>>(1);
		}

		/**
		 * @destruct
		 */
		virtual ~session_mgr_t()
		{
		}

		/**
		 * @function : put the connected session into the map
		 */
		virtual void put_session(std::shared_ptr<_Session> ptr)
		{
			if (ptr)
			{
				wlock_guard g(m_rwlock);
				m_conn_sessions.emplace(*(static_cast<_Kty *>(ptr->_get_key())), ptr.get());
			}
		}

		/**
		 * @function : get a session object,all params will be passed to session object constructor.
		 */
		template<typename...Args>
		std::shared_ptr<_Session> get_session(Args&&... args)
		{
			while (true)
			{
				if (m_idle_sessions.size() == 0)
				{
					wlock_guard g(m_rwlock);

					_Session * pointer = new _Session(std::forward<Args>(args)...);
					if (pointer)
					{
						m_pool_size++;
						m_idle_sessions.emplace_back(pointer);
					}
				}

				wlock_guard g(m_rwlock);
				if (m_idle_sessions.size() > 0)
				{
					_Session * session_pointer = m_idle_sessions.front();
					m_idle_sessions.pop_front();

					// [it's very important] why pass the shared_ptr<this> to the lambda function ? why not pass "this" pointer to the lambda function ?
					// because the shared_ptr custom deleter has used "this" object,when the session shared_ptr is destructed,it will call the custom 
					// deleter,but at this time the "this" object may be destructed already before the session shared_ptr destructed,so pass a 
					// shared_ptr<this> to the custom deleter,can make sure the "this" obejct is destructed after the the session shared_ptr destructed.

					// but : 
					// instead use shared_ptr of "this",we also can use "this" directly,then we must ensure that when "this" 
					// object is destroyed,all the allocated shared_ptr object must be released already,otherwise when the 
					// shared_ptr of object is disappeared,it will call the custom deleter,the custom deleter has referenced "this",
					// but this object is destroyed already,it is crash.

					// note : must use this->shared_from_this() ,if we use shared_from_this() ,it will occur [-fpermissive] error when compile on gcc
					auto this_ptr = this->shared_from_this();

					auto deleter = [this_ptr](_Session * pointer)
					{
						// # why we use a thread to recovery the session ? why don't recovery the session directly like this : this_ptr->_recovery(pointer); ?
						// the reason is : if we recovery the session directly,it may be cause dead lock.

						// in this situation ,it will appear dead lock : server::stop -> session_mgr::destroy -> session::stop,
						// inner session::stop,it make a this_ptr object,then post a event to force the session close,at this time,session::stop is called  
						// in thread 1,and the session close handler is called in thread 2,when the session close handler is completed in thread 2,it will   
						// cause all this_ptr disappeared,then code run to session::stop ,and this_ptr of session::stop will disappear,then code will run to 
						// this_ptr's deleter,then code will run to session_mgr::_recovery,and it will get the lock in session_mgr::_recovery,but the lock has 
						// obtained by session_mgr::destroy already,so cause dead lock. 

						this_ptr->m_thread_pool_ptr->put(&session_mgr_t::_recovery, this_ptr, pointer);
					};

					return std::shared_ptr<_Session>(session_pointer, deleter);
				}
			}

			return nullptr;
		}

		/**
		 * @function : call user custom define function for everyone element in the connected session map
		 * note : can't add or erase elems when call for_each_session,it will cause crash.
		 */
		template<typename H>
		void for_each_session(const H & _handler)
		{
			if (m_conn_sessions.size() > 0)
			{
				rlock_guard g(m_rwlock);
				for (auto & pair : m_conn_sessions)
				{
					// the map's second elem is the session raw pointer,and the session shared_ptr will pass to the 
					// _handler as a parameter
					// can't use std::shared_ptr<session>(pair.second),must use pair.second->shared_from_this().
					try
					{
						_handler(std::static_pointer_cast<_Session>(pair.second->shared_from_this()));
					}
					catch (std::bad_weak_ptr &) {}
				}
			}
		}

		/**
		 * @function : find the session by map key
		 * @return   : session shared_ptr
		 */
		std::shared_ptr<_Session> find_session(_Kty & key)
		{
			if (m_conn_sessions.size() > 0)
			{
				rlock_guard g(m_rwlock);
				auto iterator = m_conn_sessions.find(key);
				if (iterator != m_conn_sessions.end())
				{
					try
					{
						// may be the session shared_ptr just disappeared,then call shared_from_this will occur exception
						return std::static_pointer_cast<_Session>(iterator->second->shared_from_this());
					}
					catch (std::bad_weak_ptr &) {}
				}
			}
			return nullptr;
		}

		/**
		 * @function : get connected session count
		 */
		std::size_t get_connected_session_count()
		{
			return m_conn_sessions.size();
		}

		/**
		 * @function : get allocated session count
		 */
		std::size_t get_allocated_session_count()
		{
			return m_pool_size;
		}

		/**
		 * @function : destroy the "new" sessions,must call destroy function manaul begin application exit,otherwise will
		 *             cause memory leaks.
		 * because the "new" sessions shared_ptr hold a custom deleter,and the deleter hold the share_from_this ,so if we 
		 * don't delete the session pointer,the custom deleter will not destroyed,and the share_from_this will not disappear,
		 * this situation will cause ~session_mgr_t() never can be called,and the "new" sessions will never destroyed;
		 */
		virtual void destroy()
		{
			// if total session count greater than the idle session count,mean that there has some session is in using,we
			// should wait for until the session is idle.if total session count is equal the idle session count,we can't
			// wait,because the wait will never return
			{
				// Wait until all session closed completed
				std::unique_lock<std::mutex> lock(m_mtx);
				if (m_pool_size > m_idle_sessions.size())
				{
					if (std::cv_status::timeout == m_cv.wait_for(lock, std::chrono::seconds(60)))
					{
						assert(false);
					}
				}
			}

			{
				wlock_guard g(m_rwlock);

				for (auto & session_pointer : m_idle_sessions)
				{
					delete session_pointer;
					m_pool_size--;
				}
				m_idle_sessions.clear();

			}

			assert(m_pool_size == 0);
			assert(m_conn_sessions.size() == 0);
		}

	protected:
		// there has two possibles code will run to here:
		// 1 - user call session_mgr'stop function,cause the session socket closed,and the session shared_ptr disappered.in this situation,
		//     user close the socket manual,we don't care the socket close operation.
		// 2 - the client disconnect,and the session shared_ptr disappered.in this situation,the boost::asio::ip::xx::socket's destructor
		//     will call close aotumatic.
		// whatever which possible code run to here, we just only need erase the session pointer from the session map.
		virtual void _recovery(_Session * pointer)
		{
			wlock_guard g(m_rwlock);

			// remove the session from the connected session map
			if (pointer)
			{
				m_conn_sessions.erase(*(static_cast<_Kty *>(pointer->_get_key())));

				// place the session pointer into the idle sessions deque
				m_idle_sessions.emplace_back(pointer);
			}

			// if idle session count equal the total "new" session count,mean that may be user call stop_all function and waiting in the destroy 
			// function,so we should check and notify the waiter.
			if (m_idle_sessions.size() == m_pool_size)
			{
				// notify the wait thread,now all session is closed completed
				std::unique_lock<std::mutex> lock(m_mtx);
				m_cv.notify_one();
			}
		}

	protected:
		/// session unorder map,these session is already connected session 
		std::unordered_map<_Kty, _Session*, _Hasher, _Keyeq> m_conn_sessions;

		/// the idle sessions
		std::deque<_Session*> m_idle_sessions;

		std::size_t m_pool_size = 0;

		/// use rwlock to make this session map thread safe
		rwlock m_rwlock;

		/// used to recovery the shared_ptr object 
		std::shared_ptr<thread_pool<false>> m_thread_pool_ptr;

		/// use condition_variable to wait for all session closed completed
		std::mutex m_mtx;
		std::condition_variable m_cv;

	};
}

#endif // !__ASIO2_SESSION_MGR_HPP__
