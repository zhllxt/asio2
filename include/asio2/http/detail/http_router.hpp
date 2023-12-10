/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_HTTP_ROUTER_HPP__
#define __ASIO2_HTTP_ROUTER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <cstdint>
#include <memory>
#include <chrono>
#include <functional>
#include <atomic>
#include <string>
#include <string_view>
#include <queue>
#include <any>
#include <future>
#include <tuple>
#include <map>
#include <unordered_map>
#include <type_traits>

#include <asio2/external/asio.hpp>
#include <asio2/external/beast.hpp>

#include <asio2/base/iopool.hpp>

#include <asio2/base/detail/filesystem.hpp>
#include <asio2/base/detail/function_traits.hpp>
#include <asio2/base/detail/util.hpp>

#include <asio2/http/detail/http_util.hpp>
#include <asio2/http/detail/http_cache.hpp>
#include <asio2/http/request.hpp>
#include <asio2/http/response.hpp>

#include <asio2/util/string.hpp>

namespace asio2::detail
{
	template<class, class> class http_router_t;
}

#ifdef ASIO2_HEADER_ONLY
namespace bho::beast::websocket
#else
namespace boost::beast::websocket
#endif
{
	namespace detail
	{
		struct listener_tag {};
	}

	template<class caller_t>
	class listener : public detail::listener_tag
	{
		template<class, class> friend class asio2::detail::http_router_t;

		// Cannot access the protected member of bho::beast::websocket::listener<caller_t>::operator ()
		template<typename, typename> friend struct asio2::detail::function_traits;

	public:
		using self = listener<caller_t>;

		listener() = default;
		listener(listener&&) noexcept = default;
		listener(listener const&) = default;
		listener& operator=(listener&&) noexcept = default;
		listener& operator=(listener const&) = default;

		template<class F>
		inline listener& on_message(F&& f)
		{
			this->_bind(websocket::frame::message, std::forward<F>(f));
			return (*this);
		}
		template<class F>
		inline listener& on_ping(F&& f)
		{
			this->_bind(websocket::frame::ping, std::forward<F>(f));
			return (*this);
		}
		template<class F>
		inline listener& on_pong(F&& f)
		{
			this->_bind(websocket::frame::pong, std::forward<F>(f));
			return (*this);
		}
		template<class F>
		inline listener& on_open(F&& f)
		{
			this->_bind(websocket::frame::open, std::forward<F>(f));
			return (*this);
		}
		template<class F>
		inline listener& on_close(F&& f)
		{
			this->_bind(websocket::frame::close, std::forward<F>(f));
			return (*this);
		}

		template<class F, class C>
		inline listener& on_message(F&& f, C* c)
		{
			auto mf = std::bind(std::forward<F>(f), c, std::placeholders::_1, std::placeholders::_2);
			this->_bind(websocket::frame::message, std::move(mf));
			return (*this);
		}
		template<class F, class C>
		inline listener& on_ping(F&& f, C* c)
		{
			auto mf = std::bind(std::forward<F>(f), c, std::placeholders::_1, std::placeholders::_2);
			this->_bind(websocket::frame::ping, std::move(mf));
			return (*this);
		}
		template<class F, class C>
		inline listener& on_pong(F&& f, C* c)
		{
			auto mf = std::bind(std::forward<F>(f), c, std::placeholders::_1, std::placeholders::_2);
			this->_bind(websocket::frame::pong, std::move(mf));
			return (*this);
		}
		template<class F, class C>
		inline listener& on_open(F&& f, C* c)
		{
			auto mf = std::bind(std::forward<F>(f), c, std::placeholders::_1, std::placeholders::_2);
			this->_bind(websocket::frame::open, std::move(mf));
			return (*this);
		}
		template<class F, class C>
		inline listener& on_close(F&& f, C* c)
		{
			auto mf = std::bind(std::forward<F>(f), c, std::placeholders::_1, std::placeholders::_2);
			this->_bind(websocket::frame::close, std::move(mf));
			return (*this);
		}

		template<class F, class C>
		inline listener& on_message(F&& f, C& c)
		{
			return this->on_message(std::forward<F>(f), std::addressof(c));
		}
		template<class F, class C>
		inline listener& on_ping(F&& f, C& c)
		{
			return this->on_ping(std::forward<F>(f), std::addressof(c));
		}
		template<class F, class C>
		inline listener& on_pong(F&& f, C& c)
		{
			return this->on_pong(std::forward<F>(f), std::addressof(c));
		}
		template<class F, class C>
		inline listener& on_open(F&& f, C& c)
		{
			return this->on_open(std::forward<F>(f), std::addressof(c));
		}
		template<class F, class C>
		inline listener& on_close(F&& f, C& c)
		{
			return this->on_close(std::forward<F>(f), std::addressof(c));
		}

		template<class F>
		inline listener& on(std::string_view type, F&& f)
		{
			if      (beast::iequals(type, "message")) this->_bind(websocket::frame::message, std::forward<F>(f));
			else if (beast::iequals(type, "ping"   )) this->_bind(websocket::frame::ping   , std::forward<F>(f));
			else if (beast::iequals(type, "pong"   )) this->_bind(websocket::frame::pong   , std::forward<F>(f));
			else if (beast::iequals(type, "open"   )) this->_bind(websocket::frame::open   , std::forward<F>(f));
			else if (beast::iequals(type, "close"  )) this->_bind(websocket::frame::close  , std::forward<F>(f));
			return (*this);
		}
		template<class F, class C>
		inline listener& on(std::string_view type, F&& f, C* c)
		{
			auto mf = std::bind(std::forward<F>(f), c, std::placeholders::_1, std::placeholders::_2);
			if      (beast::iequals(type, "message")) this->_bind(websocket::frame::message, std::move(mf));
			else if (beast::iequals(type, "ping"   )) this->_bind(websocket::frame::ping   , std::move(mf));
			else if (beast::iequals(type, "pong"   )) this->_bind(websocket::frame::pong   , std::move(mf));
			else if (beast::iequals(type, "open"   )) this->_bind(websocket::frame::open   , std::move(mf));
			else if (beast::iequals(type, "close"  )) this->_bind(websocket::frame::close  , std::move(mf));
			return (*this);
		}
		template<class F, class C>
		inline listener& on(std::string_view type, F&& f, C& c)
		{
			return this->on(std::move(type), std::forward<F>(f), std::addressof(c));
		}

	public:
		// under gcc 8.2.0, if this "operator()" is protected, it compiled error :
		// is protected within this context : function_traits<decltype(&Callable::operator())>{};
		inline void operator()(std::shared_ptr<caller_t>& caller, http::web_request& req, http::web_response&)
		{
			ASIO2_ASSERT(caller->is_websocket());

			auto& f = cbs_[asio2::detail::to_underlying(req.ws_frame_type_)];

			if (f) f(caller, req.ws_frame_data_);
		}

	protected:
		template<class F>
		inline void _bind(websocket::frame type, F&& f)
		{
			this->cbs_[asio2::detail::to_underlying(type)] = std::bind(
				&self::template _proxy<F>, this, std::forward<F>(f),
				std::placeholders::_1, std::placeholders::_2);
		}
		template<class F>
		inline void _proxy(F& f, std::shared_ptr<caller_t>& caller, std::string_view data)
		{
			asio2::detail::ignore_unused(data);

			if constexpr (asio2::detail::is_template_callable_v<F, std::shared_ptr<caller_t>&, std::string_view>)
			{
				f(caller, data);
			}
			else
			{
				f(caller);
			}
		}

	protected:
		std::array<std::function<void(std::shared_ptr<caller_t>&, std::string_view)>,
			asio2::detail::to_underlying(frame::close) + 1> cbs_;
	};
}

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SESSION;

	template<class caller_t, class args_t>
	class http_router_t
	{
		friend caller_t;

		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION;

		template<class T, class R, class... Args>
		struct has_member_before : std::false_type {};

		template<class T, class... Args>
		struct has_member_before<T, decltype(std::declval<std::decay_t<T>>().
			before((std::declval<Args>())...)), Args...> : std::true_type {};

		template<class T, class R, class... Args>
		struct has_member_after : std::false_type {};

		template<class T, class... Args>
		struct has_member_after<T, decltype(std::declval<std::decay_t<T>>().
			after((std::declval<Args>())...)), Args...> : std::true_type {};

		/////////////////////////////////////////////////////////////////////////////
		// cinatra-master/include/cinatra/utils.hpp
		template <typename T, typename Tuple>
		struct has_type;

		template <typename T, typename... Us>
		struct has_type<T, std::tuple<Us...>> : std::disjunction<std::is_same<T, Us>...> {};

		template< typename T>
		struct filter_helper
		{
			static constexpr auto func()
			{
				return std::tuple<>();
			}

			template< class... Args >
			static constexpr auto func(T&&, Args&&...args)
			{
				return filter_helper::func(std::forward<Args>(args)...);
			}

			template< class... Args >
			static constexpr auto func(const T&, Args&&...args)
			{
				return filter_helper::func(std::forward<Args>(args)...);
			}

			template< class X, class... Args >
			static constexpr auto func(X&& x, Args&&...args)
			{
				return std::tuple_cat(std::make_tuple(std::forward<X>(x)),
					filter_helper::func(std::forward<Args>(args)...));
			}
		};

		template<typename T, typename... Args>
		inline auto filter_cache(Args&&... args)
		{
			return filter_helper<T>::func(std::forward<Args>(args)...);
		}
		/////////////////////////////////////////////////////////////////////////////

	public:
		using self  = http_router_t<caller_t, args_t>;
		using opret = http::response<http::flex_body>*;
		using opfun = std::function<opret(std::shared_ptr<caller_t>&, http::web_request&, http::web_response&)>;

		/**
		 * @brief constructor
		 */
		http_router_t()
		{
			this->not_found_router_ = std::make_shared<opfun>(
			[](std::shared_ptr<caller_t>&, http::web_request& req, http::web_response& rep) mutable
			{
				std::string desc;
				desc.reserve(64);
				desc += "The resource for ";
				desc += req.method_string();
				desc += " \"";
				desc += http::url_decode(req.target());
				desc += "\" was not found";

				rep.fill_page(http::status::not_found, std::move(desc), {}, req.version());

				return std::addressof(rep.base());
			});

		#if defined(_DEBUG) || defined(DEBUG)
			// used to test ThreadSafetyAnalysis
			asio2::ignore_unused(this->http_cache_.empty());
			asio2::ignore_unused(this->http_cache_.get_cache_count());
		#endif
		}

		/**
		 * @brief destructor
		 */
		~http_router_t() = default;

		/**
		 * @brief bind a function for http router
		 * @param name - uri name in string format.
		 * @param fun - Function object.
		 * @param caop - A pointer or reference to a class object, and aop object list.
		 * if fun is member function, the first caop param must the class object's pointer or reference.
		 */
		template<http::verb... M, class F, class ...CAOP>
		typename std::enable_if_t<!std::is_base_of_v<websocket::detail::listener_tag,
			detail::remove_cvref_t<F>>, self&>
		inline bind(std::string name, F&& fun, CAOP&&... caop)
		{
			asio2::trim_both(name);

			if (name == "*")
				name = "/*";

			if (name.empty())
			{
				ASIO2_ASSERT(false);
				return (*this);
			}

			if constexpr (sizeof...(M) == std::size_t(0))
			{
				this->_bind<http::verb::get, http::verb::post>(
					std::move(name), std::forward<F>(fun), std::forward<CAOP>(caop)...);
			}
			else
			{
				this->_bind<M...>(
					std::move(name), std::forward<F>(fun), std::forward<CAOP>(caop)...);
			}

			return (*this);
		}

		/**
		 * @brief bind a function for websocket router
		 * @param name - uri name in string format.
		 * @param listener - callback listener.
		 * @param aop - aop object list.
		 */
		template<class ...AOP>
		inline self& bind(std::string name, websocket::listener<caller_t> listener, AOP&&... aop)
		{
			asio2::trim_both(name);

			ASIO2_ASSERT(!name.empty());

			if (!name.empty())
				this->_bind(std::move(name), std::move(listener), std::forward<AOP>(aop)...);

			return (*this);
		}

		/**
		 * @brief set the 404 not found router function
		 */
		template<class F, class ...C>
		inline self& bind_not_found(F&& f, C&&... obj)
		{
			this->_bind_not_found(std::forward<F>(f), std::forward<C>(obj)...);
			return (*this);
		}

		/**
		 * @brief set the 404 not found router function
		 */
		template<class F, class ...C>
		inline self& bind_404(F&& f, C&&... obj)
		{
			this->_bind_not_found(std::forward<F>(f), std::forward<C>(obj)...);
			return (*this);
		}

	public:
		/**
		 * @brief set the root directory where we load the files.
		 */
		inline self& set_root_directory(const std::filesystem::path& path)
		{
			std::error_code ec{};
			this->root_directory_ = std::filesystem::canonical(path, ec);
			assert(!ec);
			return (*this);
		}

		/**
		 * @brief get the root directory where we load the files.
		 */
		inline const std::filesystem::path& get_root_directory() noexcept
		{
			return this->root_directory_;
		}

		/**
		 * @brief set whether websocket is supported, default is true
		 */
		inline self& set_support_websocket(bool v) noexcept
		{
			this->support_websocket_ = v;
			return (*this);
		}

		/**
		 * @brief set whether websocket is supported, default is true, same as set_support_websocket
		 */
		inline self& support_websocket(bool v) noexcept
		{
			return this->set_support_websocket(v);
		}

		/**
		 * @brief get whether websocket is supported, default is true
		 */
		inline bool is_support_websocket() const noexcept
		{
			return this->support_websocket_;
		}

	protected:
		inline self& _router() noexcept { return (*this); }

		template<http::verb... M>
		inline decltype(auto) _make_uris(std::string name)
		{
			std::size_t index = 0;
			std::array<std::string, sizeof...(M)> uris;
			//while (!name.empty() && name.back() == '*')
			//	name.erase(std::prev(name.end()));
			while (name.size() > static_cast<std::string::size_type>(1) && name.back() == '/')
				name.erase(std::prev(name.end()));
			ASIO2_ASSERT(!name.empty());
			if (!name.empty())
			{
				((uris[index++] = std::string{ this->_to_char(M) } +name), ...);
			}
			return uris;
		}

		template<http::verb... M, class F, class... AOP>
		typename std::enable_if_t<!std::is_base_of_v<websocket::detail::listener_tag,
			detail::remove_cvref_t<F>>, void>
		inline _bind(std::string name, F f, AOP&&... aop)
		{
			constexpr bool CacheFlag = has_type<http::enable_cache_t, std::tuple<std::decay_t<AOP>...>>::value;

			auto tp = filter_cache<http::enable_cache_t>(std::forward<AOP>(aop)...);
			using Tup = decltype(tp);

		#if defined(ASIO2_ENABLE_LOG)
			static_assert(!has_type<http::enable_cache_t, Tup>::value);
		#endif

			std::shared_ptr<opfun> op = std::make_shared<opfun>(
				std::bind(&self::template _proxy<CacheFlag, F, Tup>, this,
					std::move(f), std::move(tp),
					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

			this->_bind_uris(name, std::move(op), this->_make_uris<M...>(name));
		}

		template<http::verb... M, class F, class C, class... AOP>
		typename std::enable_if_t<!std::is_base_of_v<websocket::detail::listener_tag,
			detail::remove_cvref_t<F>> && std::is_same_v<C, typename function_traits<F>::class_type>, void>
		inline _bind(std::string name, F f, C* c, AOP&&... aop)
		{
			constexpr bool CacheFlag = has_type<http::enable_cache_t, std::tuple<std::decay_t<AOP>...>>::value;

			auto tp = filter_cache<http::enable_cache_t>(std::forward<AOP>(aop)...);
			using Tup = decltype(tp);

		#if defined(ASIO2_ENABLE_LOG)
			static_assert(!has_type<http::enable_cache_t, Tup>::value);
		#endif

			std::shared_ptr<opfun> op = std::make_shared<opfun>(
				std::bind(&self::template _proxy<CacheFlag, F, C, Tup>, this,
					std::move(f), c, std::move(tp),
					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

			this->_bind_uris(name, std::move(op), this->_make_uris<M...>(name));
		}

		template<http::verb... M, class F, class C, class... AOP>
		typename std::enable_if_t<!std::is_base_of_v<websocket::detail::listener_tag,
			detail::remove_cvref_t<F>> && std::is_same_v<C, typename function_traits<F>::class_type>, void>
		inline _bind(std::string name, F f, C& c, AOP&&... aop)
		{
			this->_bind<M...>(std::move(name), std::move(f), std::addressof(c), std::forward<AOP>(aop)...);
		}

		template<class... AOP>
		inline void _bind(std::string name, websocket::listener<caller_t> listener, AOP&&... aop)
		{
			auto tp = filter_cache<http::enable_cache_t>(std::forward<AOP>(aop)...);
			using Tup = decltype(tp);

		#if defined(ASIO2_ENABLE_LOG)
			static_assert(!has_type<http::enable_cache_t, Tup>::value);
		#endif

			std::shared_ptr<opfun> op = std::make_shared<opfun>(
				std::bind(&self::template _proxy<Tup>, this,
					std::move(listener), std::move(tp),
					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

			// https://datatracker.ietf.org/doc/rfc6455/

			// Once a connection to the server has been established (including a
			// connection via a proxy or over a TLS-encrypted tunnel), the client
			// MUST send an opening handshake to the server.  The handshake consists
			// of an HTTP Upgrade request, along with a list of required and
			// optional header fields.  The requirements for this handshake are as
			// follows.

			// 2.   The method of the request MUST be GET, and the HTTP version MUST
			//      be at least 1.1.

			this->_bind_uris(name, std::move(op), std::array<std::string, 1>{std::string{ "Z" } +name});
		}

		template<class URIS>
		inline void _bind_uris(std::string& name, std::shared_ptr<opfun> op, URIS uris)
		{
			for (auto& uri : uris)
			{
				if (uri.empty())
					continue;

				if (name.back() == '*')
				{
					ASIO2_ASSERT(this->wildcard_routers_.find(uri) == this->wildcard_routers_.end());
					this->wildcard_routers_[std::move(uri)] = op;
				}
				else
				{
					ASIO2_ASSERT(this->strictly_routers_.find(uri) == this->strictly_routers_.end());
					this->strictly_routers_[std::move(uri)] = op;
				}
			}
		}

		//template<http::verb... M, class C, class R, class...Ps, class... AOP>
		//inline void _bind(std::string name, R(C::*memfun)(Ps...), C* c, AOP&&... aop)
		//{
		//}

		template<bool CacheFlag, class F, class Tup>
		typename std::enable_if_t<!CacheFlag, opret>
		inline _proxy(F& f, Tup& aops, 
			std::shared_ptr<caller_t>& caller, http::web_request& req, http::web_response& rep)
		{
			using fun_traits_type = function_traits<F>;
			using arg0_type = typename std::remove_cv_t<std::remove_reference_t<
				typename fun_traits_type::template args<0>::type>>;

			if (!_call_aop_before(aops, caller, req, rep))
				return std::addressof(rep.base());

			if constexpr (std::is_same_v<std::shared_ptr<caller_t>, arg0_type>)
			{
				f(caller, req, rep);
			}
			else
			{
				f(req, rep);
			}

			_call_aop_after(aops, caller, req, rep);

			return std::addressof(rep.base());
		}

		template<bool CacheFlag, class F, class C, class Tup>
		typename std::enable_if_t<!CacheFlag, opret>
		inline _proxy(F& f, C* c, Tup& aops,
			std::shared_ptr<caller_t>& caller, http::web_request& req, http::web_response& rep)
		{
			using fun_traits_type = function_traits<F>;
			using arg0_type = typename std::remove_cv_t<std::remove_reference_t<
				typename fun_traits_type::template args<0>::type>>;

			if (!_call_aop_before(aops, caller, req, rep))
				return std::addressof(rep.base());

			if constexpr (std::is_same_v<std::shared_ptr<caller_t>, arg0_type>)
			{
				if (c) (c->*f)(caller, req, rep);
			}
			else
			{
				if (c) (c->*f)(req, rep);
			}

			_call_aop_after(aops, caller, req, rep);

			return std::addressof(rep.base());
		}

		template<class Tup>
		inline opret _proxy(websocket::listener<caller_t>& listener, Tup& aops,
			std::shared_ptr<caller_t>& caller, http::web_request& req, http::web_response& rep)
		{
			ASIO2_ASSERT(req.ws_frame_type_ != websocket::frame::unknown);

			if (req.ws_frame_type_ == websocket::frame::open)
			{
				if (!_call_aop_before(aops, caller, req, rep))
					return nullptr;
			}

			if (req.ws_frame_type_ != websocket::frame::unknown)
			{
				listener(caller, req, rep);
			}

			if (req.ws_frame_type_ == websocket::frame::open)
			{
				if (!_call_aop_after(aops, caller, req, rep))
					return nullptr;
			}

			return std::addressof(rep.base());
		}

		template<bool CacheFlag, class F, class Tup>
		typename std::enable_if_t<CacheFlag, opret>
		inline _proxy(F& f, Tup& aops, 
			std::shared_ptr<caller_t>& caller, http::web_request& req, http::web_response& rep)
		{
			using fun_traits_type = function_traits<F>;
			using arg0_type = typename std::remove_cv_t<std::remove_reference_t<
				typename fun_traits_type::template args<0>::type>>;
			using pcache_node = decltype(this->http_cache_.find(""));

			if (http::is_cache_enabled(req.base()))
			{
				pcache_node pcn = this->http_cache_.find(req.target());
				if (!pcn)
				{
					if (!_call_aop_before(aops, caller, req, rep))
						return std::addressof(rep.base());

					if constexpr (std::is_same_v<std::shared_ptr<caller_t>, arg0_type>)
					{
						f(caller, req, rep);
					}
					else
					{
						f(req, rep);
					}

					if (_call_aop_after(aops, caller, req, rep) && rep.result() == http::status::ok)
					{
						http::web_response::super msg{ std::move(rep.base()) };
						if (msg.body().to_text())
						{
							this->http_cache_.shrink_to_fit();
							pcn = this->http_cache_.emplace(req.target(), std::move(msg));
							return std::addressof(pcn->msg);
						}
						else
						{
							rep.base() = std::move(msg);
						}
					}

					return std::addressof(rep.base());
				}
				else
				{
					if (!_call_aop_before(aops, caller, req, rep))
						return std::addressof(rep.base());

					if (!_call_aop_after(aops, caller, req, rep))
						return std::addressof(rep.base());

					ASIO2_ASSERT(!pcn->msg.body().is_file());
					pcn->update_alive_time();
					return std::addressof(pcn->msg);
				}
			}
			else
			{
				if (!_call_aop_before(aops, caller, req, rep))
					return std::addressof(rep.base());

				if constexpr (std::is_same_v<std::shared_ptr<caller_t>, arg0_type>)
				{
					f(caller, req, rep);
				}
				else
				{
					f(req, rep);
				}

				_call_aop_after(aops, caller, req, rep);

				return std::addressof(rep.base());
			}
		}

		template<bool CacheFlag, class F, class C, class Tup>
		typename std::enable_if_t<CacheFlag, opret>
		inline _proxy(F& f, C* c, Tup& aops,
			std::shared_ptr<caller_t>& caller, http::web_request& req, http::web_response& rep)
		{
			using fun_traits_type = function_traits<F>;
			using arg0_type = typename std::remove_cv_t<std::remove_reference_t<
				typename fun_traits_type::template args<0>::type>>;
			using pcache_node = decltype(this->http_cache_.find(""));

			if (http::is_cache_enabled(req.base()))
			{
				pcache_node pcn = this->http_cache_.find(req.target());
				if (!pcn)
				{
					if (!_call_aop_before(aops, caller, req, rep))
						return std::addressof(rep.base());

					if constexpr (std::is_same_v<std::shared_ptr<caller_t>, arg0_type>)
					{
						if (c) (c->*f)(caller, req, rep);
					}
					else
					{
						if (c) (c->*f)(req, rep);
					}

					if (_call_aop_after(aops, caller, req, rep) && rep.result() == http::status::ok)
					{
						http::web_response::super msg{ std::move(rep.base()) };
						if (msg.body().to_text())
						{
							this->http_cache_.shrink_to_fit();
							pcn = this->http_cache_.emplace(req.target(), std::move(msg));
							return std::addressof(pcn->msg);
						}
						else
						{
							rep.base() = std::move(msg);
						}
					}

					return std::addressof(rep.base());
				}
				else
				{
					if (!_call_aop_before(aops, caller, req, rep))
						return std::addressof(rep.base());

					if (!_call_aop_after(aops, caller, req, rep))
						return std::addressof(rep.base());

					ASIO2_ASSERT(!pcn->msg.body().is_file());
					pcn->update_alive_time();
					return std::addressof(pcn->msg);
				}
			}
			else
			{
				if (!_call_aop_before(aops, caller, req, rep))
					return std::addressof(rep.base());

				if constexpr (std::is_same_v<std::shared_ptr<caller_t>, arg0_type>)
				{
					if (c) (c->*f)(caller, req, rep);
				}
				else
				{
					if (c) (c->*f)(req, rep);
				}

				_call_aop_after(aops, caller, req, rep);

				return std::addressof(rep.base());
			}
		}

		template<class F>
		inline void _bind_not_found(F f)
		{
			this->not_found_router_ = std::make_shared<opfun>(
				std::bind(&self::template _not_found_proxy<F>, this, std::move(f),
					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		}

		template<class F, class C>
		inline void _bind_not_found(F f, C* c)
		{
			this->not_found_router_ = std::make_shared<opfun>(
				std::bind(&self::template _not_found_proxy<F, C>, this, std::move(f), c,
					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		}

		template<class F, class C>
		inline void _bind_not_found(F f, C& c)
		{
			this->_bind_not_found(std::move(f), std::addressof(c));
		}

		template<class F>
		inline opret _not_found_proxy(F& f, std::shared_ptr<caller_t>& caller,
			http::web_request& req, http::web_response& rep)
		{
			asio2::detail::ignore_unused(caller);

			if constexpr (detail::is_template_callable_v<F, std::shared_ptr<caller_t>&,
				http::web_request&, http::web_response&>)
			{
				f(caller, req, rep);
			}
			else
			{
				f(req, rep);
			}

			return std::addressof(rep.base());
		}

		template<class F, class C>
		inline opret _not_found_proxy(F& f, C* c, std::shared_ptr<caller_t>& caller,
			http::web_request& req, http::web_response& rep)
		{
			asio2::detail::ignore_unused(caller);

			if constexpr (detail::is_template_callable_v<F, std::shared_ptr<caller_t>&,
				http::web_request&, http::web_response&>)
			{
				if (c) (c->*f)(caller, req, rep);
			}
			else
			{
				if (c) (c->*f)(req, rep);
			}

			return std::addressof(rep.base());
		}

		template <typename... Args, typename F, std::size_t... I>
		inline void _for_each_tuple(std::tuple<Args...>& t, const F& f, std::index_sequence<I...>)
		{
			(f(std::get<I>(t)), ...);
		}

		template<class Tup>
		inline bool _do_call_aop_before(
			Tup& aops, std::shared_ptr<caller_t>& caller, http::web_request& req, http::web_response& rep)
		{
			asio2::detail::ignore_unused(aops, caller, req, rep);

			asio2::detail::get_current_object<std::shared_ptr<caller_t>>() = caller;

			bool continued = true;
			_for_each_tuple(aops, [&continued, &caller, &req, &rep](auto& aop)
			{
				asio2::detail::ignore_unused(caller, req, rep);

				if (!continued)
					return;

				if constexpr (has_member_before<decltype(aop), bool,
					std::shared_ptr<caller_t>&, http::web_request&, http::web_response&>::value)
				{
					continued = aop.before(caller, req, rep);
				}
				else if constexpr (has_member_before<decltype(aop), bool,
					http::web_request&, http::web_response&>::value)
				{
					continued = aop.before(req, rep);
				}
				else
				{
					std::ignore = true;
				}
			}, std::make_index_sequence<std::tuple_size_v<Tup>>{});

			return continued;
		}

		template<class Tup>
		inline bool _call_aop_before(
			Tup& aops, std::shared_ptr<caller_t>& caller, http::web_request& req, http::web_response& rep)
		{
			asio2::detail::ignore_unused(aops, caller, req, rep);

			if constexpr (!std::tuple_size_v<Tup>)
			{
				return true;
			}
			else
			{
				return this->_do_call_aop_before(aops, caller, req, rep);
			}
		}

		template<class Tup>
		inline bool _do_call_aop_after(
			Tup& aops, std::shared_ptr<caller_t>& caller, http::web_request& req, http::web_response& rep)
		{
			asio2::detail::ignore_unused(aops, caller, req, rep);

			asio2::detail::get_current_object<std::shared_ptr<caller_t>>() = caller;

			bool continued = true;
			_for_each_tuple(aops, [&continued, &caller, &req, &rep](auto& aop)
			{
				asio2::detail::ignore_unused(caller, req, rep);

				if (!continued)
					return;

				if constexpr (has_member_after<decltype(aop), bool,
					std::shared_ptr<caller_t>&, http::web_request&, http::web_response&>::value)
				{
					continued = aop.after(caller, req, rep);
				}
				else if constexpr (has_member_after<decltype(aop), bool,
					http::web_request&, http::web_response&>::value)
				{
					continued = aop.after(req, rep);
				}
				else
				{
					std::ignore = true;
				}
			}, std::make_index_sequence<std::tuple_size_v<Tup>>{});

			return continued;
		}

		template<class Tup>
		inline bool _call_aop_after(
			Tup& aops, std::shared_ptr<caller_t>& caller, http::web_request& req, http::web_response& rep)
		{
			asio2::detail::ignore_unused(aops, caller, req, rep);

			if constexpr (!std::tuple_size_v<Tup>)
			{
				return true;
			}
			else
			{
				if (rep.defer_guard_)
				{
					rep.defer_guard_->aop_after_cb_ = std::make_unique<std::function<void()>>(
					[this, &aops, caller, &req, &rep]() mutable
					{
						caller_t* p = caller.get();

						asio::dispatch(p->io_->context(), make_allocator(p->wallocator(),
						[this, &aops, caller = std::move(caller), &req, &rep]() mutable
						{
							this->_do_call_aop_after(aops, caller, req, rep);
						}));
					});
					return true;
				}
				else
				{
					return this->_do_call_aop_after(aops, caller, req, rep);
				}
			}
		}

		inline std::string _make_uri(std::string_view root, std::string_view path)
		{
			std::string uri;

			if (http::has_undecode_char(path, 1))
			{
				std::string decoded = http::url_decode(path);

				path = decoded;

				while (path.size() > static_cast<std::string_view::size_type>(1) && path.back() == '/')
				{
					path.remove_suffix(1);
				}

				uri.reserve(root.size() + path.size());
				uri += root;
				uri += path;
			}
			else
			{
				while (path.size() > static_cast<std::string_view::size_type>(1) && path.back() == '/')
				{
					path.remove_suffix(1);
				}

				uri.reserve(root.size() + path.size());
				uri += root;
				uri += path;
			}

			return uri;
		}

		template<bool IsHttp>
		inline std::shared_ptr<opfun>& _find(http::web_request& req, http::web_response& rep)
		{
			asio2::detail::ignore_unused(rep);

			std::string uri;

			if constexpr (IsHttp)
			{
				uri = this->_make_uri(this->_to_char(req.method()), req.path());
			}
			else
			{
				uri = this->_make_uri(std::string_view{ "Z" }, req.path());
			}

			{
				auto it = this->strictly_routers_.find(uri);
				if (it != this->strictly_routers_.end())
				{
					return (it->second);
				}
			}

			// Find the best match url from tail to head
			if (!wildcard_routers_.empty())
			{
				for (auto it = this->wildcard_routers_.rbegin(); it != this->wildcard_routers_.rend(); ++it)
				{
					auto& k = it->first;
					ASIO2_ASSERT(k.size() >= std::size_t(3));
					if (!uri.empty() && !k.empty() && uri.front() == k.front() && uri.size() >= (k.size() - 2)
						&& uri[k.size() - 3] == k[k.size() - 3] && http::url_match(k, uri))
					{
						return (it->second);
					}
				}
			}

			return this->dummy_router_;
		}

		inline opret _route(std::shared_ptr<caller_t>& caller, http::web_request& req, http::web_response& rep)
		{
			if (caller->websocket_router_)
			{
				return (*(caller->websocket_router_))(caller, req, rep);
			}

			std::shared_ptr<opfun>& router_ptr = this->template _find<true>(req, rep);

			if (router_ptr)
			{
				return (*router_ptr)(caller, req, rep);
			}

			if (this->not_found_router_ && (*(this->not_found_router_)))
				(*(this->not_found_router_))(caller, req, rep);

			return std::addressof(rep.base());
		}

		inline constexpr std::string_view _to_char(http::verb method) noexcept
		{
			using namespace std::literals;
			// Use a special name to avoid conflicts with global variable names
			constexpr std::string_view _hrmchars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"sv;
			return _hrmchars.substr(detail::to_underlying(method), 1);
		}

	protected:
		std::filesystem::path     root_directory_     = std::filesystem::current_path();

		bool                      support_websocket_  = true;

		std::unordered_map<std::string, std::shared_ptr<opfun>> strictly_routers_;

		std::          map<std::string, std::shared_ptr<opfun>> wildcard_routers_;

		std::shared_ptr<opfun>                                  not_found_router_;

		inline static std::shared_ptr<opfun>                    dummy_router_;

		detail::http_cache_t<caller_t, args_t>                  http_cache_;
	};
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_HTTP_ROUTER_HPP__
