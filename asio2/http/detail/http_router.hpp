/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_HTTP_ROUTER_HPP__
#define __ASIO2_HTTP_ROUTER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

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
#include <filesystem>

#include <asio2/base/selector.hpp>
#include <asio2/base/iopool.hpp>
#include <asio2/base/error.hpp>

#include <asio2/base/detail/function_traits.hpp>
#include <asio2/base/detail/util.hpp>

#include <asio2/http/detail/http_util.hpp>
#include <asio2/http/request.hpp>
#include <asio2/http/response.hpp>

#include <asio2/util/string.hpp>

namespace asio2::detail
{
	template<class> class http_router_t;
}

#ifdef BEAST_HEADER_ONLY
namespace beast::websocket
#else
namespace boost::beast::websocket
#endif
{
	template<class CallerT>
	class listener
	{
		template<class> friend class asio2::detail::http_router_t;

	public:
		using self = listener<CallerT>;

		listener() = default;
		listener(listener&&) = default;
		listener(listener const&) = default;
		listener& operator=(listener&&) = default;
		listener& operator=(listener const&) = default;

		template<class F>
		inline listener& on(std::string_view type, F&& f)
		{
			if      (beast::iequals(type, "message")) _bind(websocket::frame::message, std::forward<F>(f));
			else if (beast::iequals(type, "ping"   )) _bind(websocket::frame::ping   , std::forward<F>(f));
			else if (beast::iequals(type, "pong"   )) _bind(websocket::frame::pong   , std::forward<F>(f));
			else if (beast::iequals(type, "open"   )) _bind(websocket::frame::open   , std::forward<F>(f));
			else if (beast::iequals(type, "close"  )) _bind(websocket::frame::close  , std::forward<F>(f));
			return (*this);
		}
		template<class F, class C>
		inline listener& on(std::string_view type, F&& f, C* c)
		{
			auto mf = std::bind(std::forward<F>(f), c, std::placeholders::_1, std::placeholders::_2);
			if      (beast::iequals(type, "message")) _bind(websocket::frame::message, std::move(mf));
			else if (beast::iequals(type, "ping"   )) _bind(websocket::frame::ping   , std::move(mf));
			else if (beast::iequals(type, "pong"   )) _bind(websocket::frame::pong   , std::move(mf));
			else if (beast::iequals(type, "open"   )) _bind(websocket::frame::open   , std::move(mf));
			else if (beast::iequals(type, "close"  )) _bind(websocket::frame::close  , std::move(mf));
			return (*this);
		}
		template<class F, class C>
		inline listener& on(std::string_view type, F&& f, C& c)
		{
			return this->on(std::move(type), std::forward<F>(f), &c);
		}

	protected:
		inline void operator()(std::shared_ptr<CallerT>& caller, http::request& req, http::response&)
		{
			ASIO2_ASSERT(caller->is_websocket());

			auto& f = cbs_[asio2::detail::enum_to_int(req.ws_frame_type_)];

			if (f) f(caller, req.ws_frame_data_);
		}
		template<class F>
		inline void _bind(websocket::frame type, F&& f)
		{
			this->cbs_[asio2::detail::enum_to_int(type)] = std::bind(
				&self::template _proxy<F>, this, std::forward<F>(f),
				std::placeholders::_1, std::placeholders::_2);
		}
		template<class F>
		inline void _proxy(F& f, std::shared_ptr<CallerT>& caller, std::string_view data)
		{
			if constexpr (asio2::detail::is_template_callable_v<F, std::shared_ptr<CallerT>&, std::string_view>)
			{
				f(caller, data);
			}
			else
			{
				f(caller);
			}
		}

	protected:
		std::array<std::function<void(std::shared_ptr<CallerT>&, std::string_view)>,
			asio2::detail::enum_to_int(frame::close) + 1> cbs_;
	};
}

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SESSION;

	template<class CallerT>
	class http_router_t
	{
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

	public:
		using self = http_router_t<CallerT>;
		using optype = std::function<void(std::shared_ptr<CallerT>&, http::request&, http::response&)>;

		/**
		 * @constructor
		 */
		http_router_t()
		{
			this->not_found_router_ = std::make_shared<optype>(
				[](std::shared_ptr<CallerT>&, http::request& req, http::response& rep) mutable
			{
				std::string desc;
				desc.reserve(64);
				desc += "The resource for ";
				desc += req.method_string();
				desc += " \"";
				desc += req.target();
				desc += "\" was not found";

				rep.fill_page(http::status::not_found, std::move(desc), {}, req.version());
			});
		}

		/**
		 * @destructor
		 */
		~http_router_t() = default;

		/**
		 * @function : bind a function for http router
		 * @param    : name - uri name in string format
		 * @param    : fun - Function object
		 * @param    : caop - A pointer or reference to a class object, and aop object list.
		 * if fun is member function, the first caop param must the class object's pointer or refrence.
		 */
		template<http::verb... M, class F, class ...CAOP>
		inline self& bind(std::string name, F&& fun, CAOP&&... caop)
		{
			asio2::trim_both(name);

			static_assert(sizeof...(M), "The supported http method cannot be empty.");

			ASIO2_ASSERT(sizeof...(M));
			ASIO2_ASSERT(!name.empty());

			if ((sizeof...(M)) && !name.empty())
				this->_bind<M...>(std::move(name), std::forward<F>(fun), std::forward<CAOP>(caop)...);

			return (*this);
		}

		/**
		 * @function : bind a function for websocket router
		 * @param    : name - uri name in string format
		 * @param    : listener - callback listener
		 * @param    : aop - aop object list.
		 */
		template<class ...AOP>
		inline self& bind(std::string name, websocket::listener<CallerT> listener, AOP&&... aop)
		{
			asio2::trim_both(name);

			ASIO2_ASSERT(!name.empty());

			if (!name.empty())
				this->_bind(std::move(name), std::move(listener), std::forward<AOP>(aop)...);

			return (*this);
		}

		/**
		 * @function : set the 404 not found router function
		 */
		template<class F, class ...C>
		inline self& bind_not_found(F&& f, C&&... obj)
		{
			this->_bind_not_found(std::forward<F>(f), std::forward<C>(obj)...);
			return (*this);
		}

	public:
		/**
		 * @function : set the root directory where we load the files.
		 */
		inline self& root_directory(std::filesystem::path path)
		{
			this->root_directory_ = std::move(path);
			return (*this);
		}
		/**
		 * @function : get the root directory where we load the files.
		 */
		inline const std::filesystem::path& root_directory()
		{
			return this->root_directory_;
		}

		/**
		 * @function : set whether websocket is supported, default is true
		 */
		inline self& support_websocket(bool v)
		{
			this->support_websocket_ = v;
			return (*this);
		}
		/**
		 * @function : get whether websocket is supported, default is true
		 */
		inline bool support_websocket()
		{
			return this->support_websocket_;
		}

	protected:
		inline self& _router() { return (*this); }

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
		inline void _bind(std::string name, F f, AOP&&... aop)
		{
			using Tup = std::tuple<AOP...>;

			std::shared_ptr<optype> op = std::make_shared<optype>(
				std::bind(&self::template _proxy<F, Tup>, this,
					std::move(f), Tup{ std::forward<AOP>(aop)... },
					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

			this->_bind_uris(name, std::move(op), this->_make_uris<M...>(name));
		}

		template<http::verb... M, class F, class C, class... AOP>
		typename std::enable_if_t<std::is_same_v<C, typename function_traits<F>::class_type>, void>
		inline _bind(std::string name, F f, C* c, AOP&&... aop)
		{
			using Tup = std::tuple<AOP...>;

			std::shared_ptr<optype> op = std::make_shared<optype>(
				std::bind(&self::template _proxy<F, C, Tup>, this,
					std::move(f), c, Tup{ std::forward<AOP>(aop)... },
					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

			this->_bind_uris(name, std::move(op), this->_make_uris<M...>(name));
		}

		template<http::verb... M, class F, class C, class... AOP>
		typename std::enable_if_t<std::is_same_v<C, typename function_traits<F>::class_type>, void>
		inline _bind(std::string name, F f, C& c, AOP&&... aop)
		{
			this->_bind<M...>(std::move(name), std::move(f), &c, std::forward<AOP>(aop)...);
		}

		template<class... AOP>
		inline void _bind(std::string name, websocket::listener<CallerT> listener, AOP&&... aop)
		{
			using Tup = std::tuple<AOP...>;

			std::shared_ptr<optype> op = std::make_shared<optype>(
				std::bind(&self::template _proxy<Tup>, this,
					std::move(listener), Tup{ std::forward<AOP>(aop)... },
					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

			this->_bind_uris(name, std::move(op), std::array<std::string, 1>{std::string{ "Z" } +name});
		}

		template<class URIS>
		inline void _bind_uris(std::string& name, std::shared_ptr<optype> op, URIS uris)
		{
			for (auto& uri : uris)
			{
				if (uri.empty())
					continue;

				if (name.back() == '*')
					this->wildcard_routers_[std::move(uri)] = op;
				else
					this->strictly_routers_[std::move(uri)] = op;
			}
		}

		//template<http::verb... M, class C, class R, class...Ps, class... AOP>
		//inline void _bind(std::string name, R(C::*memfun)(Ps...), C* c, AOP&&... aop)
		//{
		//}

		template<class F, class Tup>
		inline void _proxy(F& f, Tup& aops, 
			std::shared_ptr<CallerT>& caller, http::request& req, http::response& rep)
		{
			using fun_traits_type = function_traits<F>;
			using arg0_type = typename std::remove_cv_t<std::remove_reference_t<
				typename fun_traits_type::template args<0>::type>>;

			if (!_call_aop_before(aops, caller, req, rep))
				return;

			if constexpr (std::is_same_v<std::shared_ptr<CallerT>, arg0_type>)
			{
				f(caller, req, rep);
			}
			else
			{
				f(req, rep);
			}

			_call_aop_after(aops, caller, req, rep);
		}

		template<class F, class C, class Tup>
		inline void _proxy(F& f, C* c, Tup& aops, 
			std::shared_ptr<CallerT>& caller, http::request& req, http::response& rep)
		{
			using fun_traits_type = function_traits<F>;
			using arg0_type = typename std::remove_cv_t<std::remove_reference_t<
				typename fun_traits_type::template args<0>::type>>;

			if (!_call_aop_before(aops, caller, req, rep))
				return;

			if constexpr (std::is_same_v<std::shared_ptr<CallerT>, arg0_type>)
			{
				if (c) (c->*f)(caller, req, rep);
			}
			else
			{
				if (c) (c->*f)(req, rep);
			}

			_call_aop_after(aops, caller, req, rep);
		}

		template<class Tup>
		inline void _proxy(websocket::listener<CallerT>& listener, Tup& aops,
			std::shared_ptr<CallerT>& caller, http::request& req, http::response& rep)
		{
			if (!_call_aop_before(aops, caller, req, rep))
				return;

			listener(caller, req, rep);

			_call_aop_after(aops, caller, req, rep);
		}

		template<class F>
		inline void _bind_not_found(F f)
		{
			this->not_found_router_ = std::make_shared<optype>(
				std::bind(&self::template _not_found_proxy<F>, this, std::move(f),
					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		}

		template<class F, class C>
		inline void _bind_not_found(F f, C* c)
		{
			this->not_found_router_ = std::make_shared<optype>(
				std::bind(&self::template _not_found_proxy<F, C>, this, std::move(f), c,
					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		}

		template<class F, class C>
		inline void _bind_not_found(F f, C& c)
		{
			this->_bind_not_found(std::move(f), &c);
		}

		template<class F>
		inline void _not_found_proxy(F& f, std::shared_ptr<CallerT>& caller,
			http::request& req, http::response& rep)
		{
			if constexpr (is_template_callable_v<F, std::shared_ptr<CallerT>&, http::request&, http::response&>)
			{
				f(caller, req, rep);
			}
			else
			{
				f(req, rep);
			}
		}

		template<class F, class C>
		inline void _not_found_proxy(F& f, C* c, std::shared_ptr<CallerT>& caller,
			http::request& req, http::response& rep)
		{
			if constexpr (is_template_callable_v<F, std::shared_ptr<CallerT>&,
				http::request&, http::response&>)
			{
				if (c) (c->*f)(caller, req, rep);
			}
			else
			{
				if (c) (c->*f)(req, rep);
			}
		}

		template <typename... Args, typename F, std::size_t... I>
		inline void _for_each_tuple(std::tuple<Args...>& t, const F& f, std::index_sequence<I...>)
		{
			(f(std::get<I>(t)), ...);
		}

		template<class Tup>
		inline bool _call_aop_before(Tup& aops, std::shared_ptr<CallerT>& caller,
			http::request& req, http::response& rep)
		{
			if constexpr (!std::tuple_size_v<Tup>)
			{
				return true;
			}
			else
			{
				std::ignore = true;
			}

			bool continued = true;
			_for_each_tuple(aops, [&continued, &caller, &req, &rep](auto& aop)
			{
				if (!continued)
					return;

				if constexpr (has_member_before<decltype(aop), bool,
					std::shared_ptr<CallerT>&, http::request&, http::response&>::value)
				{
					continued = aop.before(caller, req, rep);
				}
				else if constexpr (has_member_before<decltype(aop), bool,
					http::request&, http::response&>::value)
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
		inline bool _call_aop_after(Tup& aops, std::shared_ptr<CallerT>& caller,
			http::request& req, http::response& rep)
		{
			if constexpr (!std::tuple_size_v<Tup>)
			{
				return true;
			}
			else
			{
				std::ignore = true;
			}

			bool continued = true;
			_for_each_tuple(aops, [&continued, &caller, &req, &rep](auto& aop)
			{
				if (!continued)
					return;

				if constexpr (has_member_after<decltype(aop), bool,
					std::shared_ptr<CallerT>&, http::request&, http::response&>::value)
				{
					continued = aop.after(caller, req, rep);
				}
				else if constexpr (has_member_after<decltype(aop), bool,
					http::request&, http::response&>::value)
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

		inline std::string _make_uri(std::string_view root, std::string_view path)
		{
			while (path.size() > static_cast<std::string_view::size_type>(1) && path.back() == '/')
			{
				path.remove_suffix(1);
			}

			std::string uri;
			uri.reserve(root.size() + path.size());
			uri += root;
			uri += path;

			return uri;
		}

		template<bool IsHttp>
		inline std::shared_ptr<optype>& _find(http::request& req, http::response& rep)
		{
			detail::ignore_unused(rep);

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
				for (auto it = std::prev(this->wildcard_routers_.end()); it != this->wildcard_routers_.end(); --it)
				{
					auto& k = it->first;
					ASIO2_ASSERT(k.size() >= std::size_t(3));
					if (uri.front() == k.front() && uri.size() >= k.size() - 2 &&
						uri[k.size() - 3] == k[k.size() - 3] && http::url_match(k, uri))
					{
						return (it->second);
					}
				}
			}

			return this->dummy_router_;
		}

		inline bool _route(std::shared_ptr<CallerT>& caller, http::request& req, http::response& rep)
		{
			if (caller->websocket_router_)
			{
				(*(caller->websocket_router_))(caller, req, rep);
				return true;
			}

			std::shared_ptr<optype>& router_ptr = this->template _find<true>(req, rep);

			if (router_ptr)
			{
				(*router_ptr)(caller, req, rep);
				return true;
			}

			if (this->not_found_router_ && (*(this->not_found_router_)))
				(*(this->not_found_router_))(caller, req, rep);

			return false;
		}

		inline constexpr std::string_view _to_char(http::verb method)
		{
			using namespace std::literals;
			constexpr std::string_view chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"sv;
			return chars.substr(enum_to_int(method), 1);
		}

	protected:
		std::filesystem::path     root_directory_     = std::filesystem::current_path();

		bool                      support_websocket_  = true;

		std::unordered_map<std::string, std::shared_ptr<optype>> strictly_routers_;

		std::          map<std::string, std::shared_ptr<optype>> wildcard_routers_;

		std::shared_ptr<optype>                                  not_found_router_;

		std::shared_ptr<optype>                                  dummy_router_;
	};
}

#endif // !__ASIO2_HTTP_ROUTER_HPP__
