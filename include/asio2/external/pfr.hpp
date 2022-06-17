/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_PFR_FUNCTION_HPP__
#define __ASIO2_PFR_FUNCTION_HPP__

#include <asio2/config.hpp>
#include <asio2/bho/config.hpp>
#include <asio2/bho/core/type_name.hpp>

#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include <type_traits>

#if !defined(ASIO2_DISABLE_BOOST) && __has_include(<boost/pfr.hpp>)
#include <boost/pfr.hpp>
namespace pfr = ::boost::pfr;
#else
#include <asio2/bho/pfr.hpp>
namespace pfr = ::bho::pfr;
#endif


#if !defined(ASIO2_DISABLE_BOOST) && __has_include(<boost/pfr.hpp>)
namespace boost::pfr
#else
namespace bho::pfr
#endif
{
	namespace detail
	{
		template<int N> struct refl_encode_counter : refl_encode_counter<N - 1> {};
		template<>      struct refl_encode_counter<0> {};

		struct __refl_members_iterator_dummy__
		{
			template<typename ThisType, typename Func>
			static inline void for_each_field(ThisType&, const Func&)
			{
			}
			template<typename Func>
			static inline void for_each_field_name(const Func&)
			{
			}
		};
	}

	//---------------------------------------------------------------------------------------------
	// this code is refrenced from : https://zhuanlan.zhihu.com/p/320061875
	// this code has some unresolved problems.
	// eg:
	// struct actor : public dynamic_creator<actor, std::string, const char*>
	// create<actor>("abc", "xyz");
	// the create function will failed, beacuse the "abc" type is not equal to std::string, and the
	// "xyz" type is not equal to const char*
	// you must call the create function like this:
	// create<actor>(std::string("abc"), (const char*)"xyz");
	// This is terrible.
	// Is it possible to use serialization to solve this problem?
	//---------------------------------------------------------------------------------------------
	template<typename BaseT, typename... Args>
	class class_factory
	{
	private:
		 class_factory() {}
		~class_factory() {}

	public:
		static class_factory<BaseT, Args...>& instance()
		{
			static class_factory<BaseT, Args...> inst{};
			return inst;
		}

		bool regist(std::string name, std::function<BaseT*(Args&&... args)> fn)
		{
			if (!fn)
				return(false);

			return create_functions_.emplace(std::move(name), std::move(fn)).second;
		}

		BaseT* create(const std::string& name, Args&&... args)
		{
			auto iter = create_functions_.find(name);
			if (iter == create_functions_.end())
			{
				return (nullptr);
			}
			else
			{
				return ((iter->second)(std::forward<Args>(args)...));
			}
		}

	private:
		std::unordered_map<std::string, std::function<BaseT*(Args&&...)>> create_functions_;
	};

	template<typename BaseT, typename T, typename ...Args>
	class dynamic_creator
	{
	public:
		struct registor
		{
			registor()
			{
				std::string name = bho::core::type_name<T>();
				class_factory<BaseT, Args...>::instance().regist(std::move(name), create);
			}
			inline void makesure_construct() const noexcept { };
		};

		explicit dynamic_creator() noexcept
		{
			registor_.makesure_construct();
		}
		virtual ~dynamic_creator() noexcept
		{
			registor_.makesure_construct();
		};

	private:
		static BaseT* create(Args&&... args)
		{
			return (new T(std::forward<Args>(args)...));
		}

	private:
		static inline registor registor_{};
	};

	template<typename BaseT>
	class create_helper
	{
	public:
		// dont use Args&&... args
		// it will cause issue like this:
		// int i = 2;
		// create("dog", i);
		// the i will be parsed as int&
		template<typename ...Args>
		static inline BaseT* create(const std::string& name, Args... args)
		{
			return (class_factory<BaseT, Args...>::instance().create(
				name, std::move(args)...));
		}
	};

	template<typename BaseT>
	class create_helper<BaseT*>
	{
	public:
		template<typename ...Args>
		static inline BaseT* create(const std::string& name, Args... args)
		{
			return (class_factory<BaseT, Args...>::instance().create(
				name, std::move(args)...));
		}
	};

	template<typename BaseT>
	class create_helper<std::shared_ptr<BaseT>>
	{
	public:
		template<typename ...Args>
		static inline std::shared_ptr<BaseT> create(const std::string& name, Args... args)
		{
			return std::shared_ptr<BaseT>(class_factory<BaseT, Args...>::instance().create(
				name, std::move(args)...));
		}
	};

	template<typename BaseT>
	class create_helper<std::unique_ptr<BaseT>>
	{
	public:
		template<typename ...Args>
		static inline std::unique_ptr<BaseT> create(const std::string& name, Args... args)
		{
			return std::unique_ptr<BaseT>(class_factory<BaseT, Args...>::instance().create(
				name, std::move(args)...));
		}
	};
}


// this code is refrenced from : 
// https://github.com/yuanzhubi/reflect_struct
// boost/typeof/dmc/typeof_impl.hpp

#ifndef MAX_REFLECT_COUNT
#define MAX_REFLECT_COUNT 256
#endif

#define BHO_REFLECT_INDEX(counter)                                                                 \
	((sizeof(*counter((pfr::detail::refl_encode_counter<MAX_REFLECT_COUNT>*)0)) -                  \
		sizeof(*counter((void*)0))) / sizeof(char))                                                \

#define BHO_REFLECT_INCREASE(counter, tag)                                                         \
    static constexpr std::size_t tag = BHO_REFLECT_INDEX(counter);                                 \
    static char (*counter(pfr::detail::refl_encode_counter<                                        \
		 sizeof(*counter((void*)0)) / sizeof(char) + tag + 1>*))                                   \
		[sizeof(*counter((void*)0)) / sizeof(char) + tag + 1];                                     \

#define BHO_REFLECT_F_TAG(name) BHO_JOIN(__reflect_tag_, name)

#define F_BEGIN(Class)                                                                             \
private:                                                                                           \
    static char (*__refl_field_counter__(...))[1];                                                 \
    template<typename T, int N = -1>   struct __refl_members_iterator__ :                          \
		public pfr::detail::__refl_members_iterator_dummy__{};                                     \
public:                                                                                            \
    template<typename Func>                                                                        \
    void for_each_field(const Func& func) noexcept                                                 \
	{                                                                                              \
        __refl_members_iterator__<int, 0>::for_each_field(*this, func);                            \
    }                                                                                              \
    template<typename Func>                                                                        \
    static void for_each_field_name(const Func& func) noexcept                                     \
	{                                                                                              \
        __refl_members_iterator__<int, 0>::for_each_field_name(func);                              \
    }                                                                                              \
    template<typename T, int N> friend struct __refl_members_iterator__                            \

#define F(type, name)                                                                              \
public:                                                                                            \
    type name{};                                                                                   \
private:                                                                                           \
    BHO_REFLECT_INCREASE(__refl_field_counter__, BHO_REFLECT_F_TAG(name))                          \
    template <typename T>                                                                          \
    struct __refl_members_iterator__<T, BHO_REFLECT_F_TAG(name)>                                   \
	{                                                                                              \
        typedef __refl_members_iterator__<T, BHO_REFLECT_F_TAG(name) + 1> next_type;               \
        template<typename ThisType, typename Func>                                                 \
        static void for_each_field(ThisType& This, const Func& func) noexcept                      \
		{                                                                                          \
            func(BHO_STRINGIZE(name), This.name);                                                  \
            next_type::for_each_field(This, func);                                                 \
        }                                                                                          \
        template<typename Func>                                                                    \
        static void for_each_field_name(const Func& func) noexcept                                 \
		{                                                                                          \
            func(BHO_STRINGIZE(name));                                                             \
            next_type::for_each_field_name(func);                                                  \
        }                                                                                          \
    }                                                                                              \


#define F_END()                                                                                    \
public:                                                                                            \
	constexpr std::size_t get_field_count() const noexcept                                         \
	{                                                                                              \
		constexpr std::size_t field_count = BHO_REFLECT_INDEX(__refl_field_counter__);             \
		return field_count;                                                                        \
	}                                                                                              \


#endif
