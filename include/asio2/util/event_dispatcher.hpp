// eventpp library
// Copyright (C) 2018 Wang Qi (wqking)
// Github: https://github.com/wqking/eventpp
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//   http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*
 * This code is modified from eventpp
 * 
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 */

//{
//	std::cout << std::endl << "event_dispatcher tutorial 1, basic" << std::endl;
//
//	// The namespace is asio2
//	// The first template parameter int is the event type,
//	// the event type can be any type such as std::string, int, etc.
//	// The second is the prototype of the listener.
//	asio2::event_dispatcher<int, void ()> dispatcher;
//
//	// Add a listener. As the type of dispatcher,
//	// here 3 and 5 is the event type,
//	// []() {} is the listener.
//	// Lambda is not required, any function or std::function
//	// or whatever function object with the required prototype is fine.
//	dispatcher.append_listener(3, []() {
//		std::cout << "Got event 3." << std::endl;
//	});
//	dispatcher.append_listener(5, []() {
//		std::cout << "Got event 5." << std::endl;
//	});
//	dispatcher.append_listener(5, []() {
//		std::cout << "Got another event 5." << std::endl;
//	});
//
//	// Dispatch the events, the first argument is always the event type.
//	dispatcher.dispatch(3);
//	dispatcher.dispatch(5);
//}
//
//{
//	std::cout << std::endl << "event_dispatcher tutorial 2, listener with parameters" << std::endl;
//
//	// The listener has two parameters.
//	asio2::event_dispatcher<int, void (const std::string &, const bool)> dispatcher;
//
//	dispatcher.append_listener(3, [](const std::string & s, const bool b) {
//		std::cout << std::boolalpha << "Got event 3, s is " << s << " b is " << b << std::endl;
//	});
//	// The listener prototype doesn't need to be exactly same as the dispatcher.
//	// It would be find as long as the arguments is compatible with the dispatcher.
//	dispatcher.append_listener(5, [](std::string s, int b) {
//		std::cout << std::boolalpha << "Got event 5, s is " << s << " b is " << b << std::endl;
//	});
//	dispatcher.append_listener(5, [](const std::string & s, const bool b) {
//		std::cout << std::boolalpha << "Got another event 5, s is " << s << " b is " << b << std::endl;
//	});
//
//	// Dispatch the events, the first argument is always the event type.
//	dispatcher.dispatch(3, "Hello", true);
//	dispatcher.dispatch(5, "World", false);
//}
//
//{
//	std::cout << std::endl << "event_dispatcher tutorial 3, customized Event struct" << std::endl;
//
//	// Define an Event to hold all parameters.
//	struct MyEvent
//	{
//		int type;
//		std::string message;
//		int param;
//	};
//
//	// Define policies to let the dispatcher knows how to
//	// extract the event type.
//	struct MyEventPolicies
//	{
//		static int get_event(const MyEvent & e, bool /*b*/)
//		{
//			return e.type;
//		}
//
//		// single_thread, free lock
//		using thread_t = asio2::dispatcheres::single_thread;
//	};
//
//	// Pass MyEventPolicies as the third template argument of event_dispatcher.
//	// Note: the first template argument is the event type type int, not MyEvent.
//	asio2::event_dispatcher<
//		int,
//		void (const MyEvent &, bool),
//		MyEventPolicies
//	> dispatcher;
//
//	// Add a listener.
//	// Note: the first argument is the event type of type int, not MyEvent.
//	dispatcher.append_listener(3, [](const MyEvent & e, bool b) {
//		std::cout
//			<< std::boolalpha
//			<< "Got event 3" << std::endl
//			<< "Event::type is " << e.type << std::endl
//			<< "Event::message is " << e.message << std::endl
//			<< "Event::param is " << e.param << std::endl
//			<< "b is " << b << std::endl
//		;
//	});
//
//	// Dispatch the event.
//	// The first argument is Event.
//	dispatcher.dispatch(MyEvent { 3, "Hello world", 38 }, true);
//}
//
//struct Tutor4MyEvent {
//	Tutor4MyEvent() : type(0), canceled(false)
//	{
//	}
//	explicit Tutor4MyEvent(const int type)
//		: type(type), canceled(false)
//	{
//	}
//
//	int type;
//	mutable bool canceled;
//};
//
//struct Tutor4MyEventPolicies
//{
//	// E is Tutor4MyEvent and get_event doesn't need to be template.
//	// We make it template to show get_event can be templated member.
//	template <typename E>
//	static int get_event(const E & e)
//	{
//		return e.type;
//	}
//
//	// E is Tutor4MyEvent and can_continue_invoking doesn't need to be template.
//	// We make it template to show can_continue_invoking can be templated member.
//	template <typename E>
//	static bool can_continue_invoking(const E & e)
//	{
//		return ! e.canceled;
//	}
//};
//
//{
//	std::cout << std::endl << "event_dispatcher tutorial 4, event canceling" << std::endl;
//
//	asio2::event_dispatcher<int, void (const Tutor4MyEvent &), Tutor4MyEventPolicies> dispatcher;
//
//	dispatcher.append_listener(3, [](const Tutor4MyEvent & e) {
//		std::cout << "Got event 3" << std::endl;
//		e.canceled = true;
//	});
//	dispatcher.append_listener(3, [](const Tutor4MyEvent & /*e*/) {
//		std::cout << "Should not get this event 3" << std::endl;
//	});
//
//	dispatcher.dispatch(Tutor4MyEvent(3));
//}
//
//{
//	std::cout << std::endl << "event_dispatcher tutorial 5, event filter" << std::endl;
//
//	struct MyPolicies {
//		using mixins_t = asio2::dispatcheres::mixin_list<asio2::dispatcheres::mixin_filter>;
//	};
//	asio2::event_dispatcher<int, void (int e, int i, std::string), MyPolicies> dispatcher;
//
//	dispatcher.append_listener(3, [](const int /*e*/, const int i, const std::string & s) {
//		std::cout
//			<< "Got event 3, i was 1 but actural is " << i
//			<< " s was Hello but actural is " << s
//			<< std::endl
//		;
//	});
//	dispatcher.append_listener(5, [](const int /*e*/, const int /*i*/, const std::string & /*s*/) {
//		std::cout << "Shout not got event 5" << std::endl;
//	});
//
//	// Add three event filters.
//
//	// The first filter modifies the input arguments to other values, then the subsequence filters
//	// and listeners will see the modified values.
//	dispatcher.append_filter([](const int e, int & i, std::string & s) -> bool {
//		std::cout << "Filter 1, e is " << e << " passed in i is " << i << " s is " << s << std::endl;
//		i = 38;
//		s = "Hi";
//		std::cout << "Filter 1, changed i is " << i << " s is " << s << std::endl;
//		return true;
//	});
//
//	// The second filter filters out all event of 5. So no listeners on event 5 can be triggered.
//	// The third filter is not invoked on event 5 also.
//	dispatcher.append_filter([](const int e, int & i, std::string & s) -> bool {
//		std::cout << "Filter 2, e is " << e << " passed in i is " << i << " s is " << s << std::endl;
//		if(e == 5) {
//			return false;
//		}
//		return true;
//	});
//
//	// The third filter just prints the input arguments.
//	dispatcher.append_filter([](const int e, int & i, std::string & s) -> bool {
//		std::cout << "Filter 3, e is " << e << " passed in i is " << i << " s is " << s << std::endl;
//		return true;
//	});
//
//	// Dispatch the events, the first argument is always the event type.
//	dispatcher.dispatch(3, 1, "Hello");
//	dispatcher.dispatch(5, 2, "World");
//}

#ifndef __ASIO2_EVENT_DISPATCHER_HPP__
#define __ASIO2_EVENT_DISPATCHER_HPP__

#include <cassert>
#include <string>
#include <functional>
#include <type_traits>
#include <mutex>
#include <shared_mutex>
#include <memory>
#include <utility>
#include <tuple>
#include <atomic>
#include <condition_variable>
#include <map>
#include <unordered_map>
#include <list>
#include <thread>
#include <initializer_list>
#include <vector>
#include <optional>

// when compiled with "Visual Studio 2017 - Windows XP (v141_xp)"
// there is hasn't shared_mutex
#ifndef ASIO2_HAS_SHARED_MUTEX
	#if defined(_MSC_VER)
		#if defined(_HAS_SHARED_MUTEX)
			#if _HAS_SHARED_MUTEX
				#define ASIO2_HAS_SHARED_MUTEX 1
				#define asio2_shared_mutex std::shared_mutex
				#define asio2_shared_lock  std::shared_lock
				#define asio2_unique_lock  std::unique_lock
			#else
				#define ASIO2_HAS_SHARED_MUTEX 0
				#define asio2_shared_mutex std::mutex
				#define asio2_shared_lock  std::lock_guard
				#define asio2_unique_lock  std::lock_guard
			#endif
		#else
				#define ASIO2_HAS_SHARED_MUTEX 1
				#define asio2_shared_mutex std::shared_mutex
				#define asio2_shared_lock  std::shared_lock
				#define asio2_unique_lock  std::unique_lock
		#endif
	#else
				#define ASIO2_HAS_SHARED_MUTEX 1
				#define asio2_shared_mutex std::shared_mutex
				#define asio2_shared_lock  std::shared_lock
				#define asio2_unique_lock  std::unique_lock
	#endif
#endif


namespace asio2 {


namespace dispatcheres {

template <typename F, template <typename> class T>
struct transform_arguments;

template <template <typename> class T, typename RT, typename ...Args>
struct transform_arguments <RT (Args...), T>
{
	using type = RT (typename T<Args>::type...);
};

template <typename F, typename Replacement>
struct replace_return_type;

template <typename Replacement, typename RT, typename ...Args>
struct replace_return_type <RT (Args...), Replacement>
{
	using type = Replacement (Args...);
};

template <int N, int M>
struct int_to_constant_helper
{
	template <typename C, typename ...Args>
	static auto find(const int index, C && c, Args && ...args)
		-> decltype(std::declval<C>().template operator()<0>(std::declval<Args>()...))
	{
		if(N == index)
		{
			return c.template operator()<N>(std::forward<Args>(args)...);
		}
		else
		{
			return int_to_constant_helper<N + 1, M>::find(index, std::forward<C>(c), std::forward<Args>(args)...);
		}
	}
};

template <int M>
struct int_to_constant_helper <M, M>
{
	template <typename C, typename ...Args>
	static auto find(const int /*index*/, C && c, Args && ...args)
		-> decltype(std::declval<C>().template operator()<0>(std::declval<Args>()...))
	{
		return decltype(c.template operator()<0>(std::forward<Args>(args)...))();
	}
};

template <int M, typename C, typename ...Args>
auto int_to_constant(const int index, C && c, Args && ...args)
	-> decltype(std::declval<C>().template operator()<0>(std::declval<Args>()...))
{
	return int_to_constant_helper<0, M>::find(index, std::forward<C>(c), std::forward<Args>(args)...);
}

template <typename F, typename ...Args>
struct can_invoke
{
	template <typename U, typename ...X>
	static auto invoke(int) -> decltype(std::declval<U>()(std::declval<X>()...), std::true_type());

	template <typename U, typename ...X>
	static auto invoke(...) -> std::false_type;

	enum
	{
		value = !! decltype(invoke<F, Args...>(0))()
	};
};

template <typename T>
struct shift_tuple;

template <typename A, typename ...Args>
struct shift_tuple <std::tuple<A, Args...> >
{
	using type = std::tuple<Args...>;
};

template <>
struct shift_tuple <std::tuple<> >
{
	using type = std::tuple<>;
};

// for compile time debug
template<typename T>
void print_type_in_compile_time(T * = 0)
{
	static_assert(std::is_same<T, int>::value && ! std::is_same<T, int>::value, "The error shows the type name.");
}
template<int N>
void print_int_in_compile_time()
{
	int n = 0;
	switch(n)
	{
	case N:
	case N:
		break;
	};
}

} //namespace dispatcheres


//-------------------------------------------------------------------------------------------------


namespace dispatcheres {

struct tag_homo {};
struct tag_callback_list : public tag_homo {};
struct tag_event_dispatcher : public tag_homo {};
struct tag_event_queue : public tag_homo {};

template <
	typename MutexT = asio2_shared_mutex,
	template <typename > class SharedLockT = asio2_shared_lock,
	template <typename > class UniqueLockT = asio2_unique_lock,
	template <typename > class AtomicT = std::atomic,
	typename ConditionVariableT = std::condition_variable
>
struct general_thread
{
	using mutex = MutexT;

	template <typename T>
	using shared_lock = SharedLockT<T>;

	template <typename T>
	using unique_lock = UniqueLockT<T>;

	template <typename T>
	using atomic = AtomicT<T>;

	using condition_variable = ConditionVariableT;
};

struct multiple_thread
{
	using mutex = asio2_shared_mutex;

	template <typename T>
	using shared_lock = asio2_shared_lock<T>;

	template <typename T>
	using unique_lock = asio2_unique_lock<T>;

	template <typename T>
	using atomic = std::atomic<T>;

	using condition_variable = std::condition_variable;
};

struct single_thread
{
	struct mutex
	{
		inline void lock  () noexcept {}
		inline void unlock() noexcept {}
	};

	template <typename T>
	struct shared_lock
	{
		using mutex_type = T;

		inline shared_lock() noexcept = default;
		inline shared_lock(shared_lock&& other) noexcept = default;

		inline explicit shared_lock(mutex_type&) noexcept {}
		inline ~shared_lock() noexcept {};

		inline void lock  () noexcept {}
		inline void unlock() noexcept {}
	};

	template <typename T>
	struct unique_lock
	{
		using mutex_type = T;

		inline unique_lock() noexcept = default;
		inline unique_lock(unique_lock&& other) noexcept = default;

		inline explicit unique_lock(mutex_type&) noexcept {}
		inline ~unique_lock() noexcept {};

		inline void lock  () noexcept {}
		inline void unlock() noexcept {}
	};
	
	template <typename T>
	struct atomic
	{
		inline atomic() = default;

		constexpr atomic(T desired) noexcept
			: value(desired)
		{
		}

		inline void store(T desired, std::memory_order /*order*/ = std::memory_order_seq_cst) noexcept
		{
			value = desired;
		}
		
		inline T load(std::memory_order /*order*/ = std::memory_order_seq_cst) const noexcept
		{
			return value;
		}

		inline T exchange(T desired, std::memory_order /*order*/ = std::memory_order_seq_cst) noexcept
		{
			const T old = value;
			value = desired;
			return old;
		}
		
		inline T operator ++ () noexcept
		{
			return ++value;
		}

		inline T operator -- () noexcept
		{
			return --value;
		}

		T value{};
	};

	struct condition_variable
	{
		inline void notify_one() noexcept
		{
		}
		
		template <class Predicate>
		inline void wait(std::unique_lock<std::mutex> & /*lock*/, Predicate /*pred*/) noexcept
		{
		}
		
		template <class Rep, class Period, class Predicate>
		inline bool wait_for(std::unique_lock<std::mutex> & /*lock*/,
				const std::chrono::duration<Rep, Period> & /*rel_time*/,
				Predicate /*pred*/) noexcept
		{
			return true;
		}
	};
};

struct argument_passing_auto_detect
{
	enum
	{
		can_include_event_type = true,
		can_exclude_event_type = true
	};
};

struct argument_passing_include_event
{
	enum
	{
		can_include_event_type = true,
		can_exclude_event_type = false
	};
};

struct argument_passing_exclude_event
{
	enum
	{
		can_include_event_type = false,
		can_exclude_event_type = true
	};
};

struct default_policy
{
};

template <template <typename> class ...user_mixins>
struct mixin_list
{
};

} //namespace dispatcheres


//-------------------------------------------------------------------------------------------------


namespace dispatcheres {

template <typename T>
struct has_type_argument_passing_mode
{
	template <typename C> static std::true_type test(typename C::argument_passing_mode *) ;
	template <typename C> static std::false_type test(...);    

	enum { value = !! decltype(test<T>(0))() };
};
template <typename T, bool, typename Default>
struct select_argument_passing_mode { using type = typename T::argument_passing_mode; };

template <typename T, typename Default>
struct select_argument_passing_mode <T, false, Default> { using type = Default; };

// https://en.cppreference.com/w/cpp/types/void_t
// primary template handles types that have no nested ::type member:
template< class, class = void >
struct has_type_thread_t : std::false_type { };

// specialization recognizes types that do have a nested ::type member:
template< class T >
struct has_type_thread_t<T, std::void_t<typename T::thread_t>> : std::true_type { };

template <typename T, bool> struct select_thread { using type = typename T::thread_t; };
template <typename T> struct select_thread <T, false> { using type = multiple_thread; };

// primary template handles types that have no nested ::type member:
template< class, class = void >
struct has_type_callback_t : std::false_type { };

// specialization recognizes types that do have a nested ::type member:
template< class T >
struct has_type_callback_t<T, std::void_t<typename T::callback_t>> : std::true_type { };

template <typename T, bool, typename D> struct select_callback { using type = typename T::callback_t; };
template <typename T, typename D> struct select_callback<T, false, D> { using type = D; };

// primary template handles types that have no nested ::type member:
template< class, class = void >
struct has_type_user_data_t : std::false_type { };

// specialization recognizes types that do have a nested ::type member:
template< class T >
struct has_type_user_data_t<T, std::void_t<typename T::user_data_t>> : std::true_type { };

// primary template handles types that have no nested ::type member:
template< class, class = void >
struct has_type_listener_name_t : std::false_type { };

// specialization recognizes types that do have a nested ::type member:
template< class T >
struct has_type_listener_name_t<T, std::void_t<typename T::listener_name_t>> : std::true_type { };

template <typename T, bool, typename D> struct select_listener_name { using type = typename T::listener_name_t; };
template <typename T, typename D> struct select_listener_name<T, false, D> { using type = D; };

template <typename T, typename ...Args>
struct has_function_get_event
{
	template <typename C> static std::true_type test(decltype(C::get_event(std::declval<Args>()...)) *);
	template <typename C> static std::false_type test(...);
	
	enum { value = !! decltype(test<T>(0))() };
};
template <typename E>
struct default_get_event
{
	template <typename U, typename ...Args>
	static E get_event(U && e, Args && ...)
	{
		return e;
	}
};
template <typename T, typename Key, bool> struct select_get_event { using type = T; };
template <typename T, typename Key> struct select_get_event<T, Key, false> { using type = default_get_event<Key>; };

template <typename T, typename ...Args>
struct has_function_can_continue_invoking
{
	template <typename C> static std::true_type test(decltype(C::can_continue_invoking(std::declval<Args>()...)) *) ;
	template <typename C> static std::false_type test(...);    

	enum { value = !! decltype(test<T>(0))() };
};
struct default_can_continue_invoking
{
	template <typename ...Args>
	static bool can_continue_invoking(Args && ...)
	{
		return true;
	}
};
template <typename T, bool> struct select_can_continue_invoking { using type = T; };
template <typename T> struct select_can_continue_invoking<T, false> { using type = default_can_continue_invoking; };

template <typename T>
struct has_template_map_t
{
	template <typename C> static std::true_type test(typename C::template map_t<int, int> *) ;
	template <typename C> static std::false_type test(...);    

	enum { value = !! decltype(test<T>(0))() };
};
template <typename T>
class has_std_hash
{
	template <typename C> static std::true_type test(decltype(std::hash<C>()(std::declval<C>())) *) ;
	template <typename C> static std::false_type test(...);    

public:
	enum { value = !! decltype(test<T>(0))() };
};

template<class T, class R, class... Args>
struct has_member_reserve : std::false_type {};

template<class T, class... Args>
struct has_member_reserve<T, decltype(std::declval<std::decay_t<T>>().
	reserve((std::declval<Args>())...)), Args...> : std::true_type {};

template <typename Key, typename Value, typename T, bool>
struct select_map
{
	using type = typename T::template map_t<Key, Value>;
};
template <typename Key, typename Value, typename T>
struct select_map<Key, Value, T, false>
{
	using type = typename std::conditional<
		has_std_hash<Key>::value,
		std::unordered_map<Key, Value>,
		std::map<Key, Value>
	>::type;
};

template <typename ListenerNameType, typename EventType, typename Value, typename T, bool>
struct select_listener_name_map
{
	using type = typename T::template map_t<ListenerNameType, typename T::template map_t<EventType, Value>>;
};
template <typename ListenerNameType, typename EventType, typename Value, typename T>
struct select_listener_name_map<ListenerNameType, EventType, Value, T, false>
{
	using type = typename std::conditional<
		has_std_hash<ListenerNameType>::value,
		typename std::conditional_t<
		has_std_hash<EventType>::value,
		std::unordered_map<ListenerNameType, std::unordered_multimap<EventType, Value>>,
		std::unordered_map<ListenerNameType, std::multimap<EventType, Value>>
		>,
		typename std::conditional_t<
		has_std_hash<EventType>::value,
		std::map<ListenerNameType, std::unordered_multimap<EventType, Value>>,
		std::map<ListenerNameType, std::multimap<EventType, Value>>
		>
	>::type;
};

template <typename T>
struct has_template_queue_list_t
{
	template <typename C> static std::true_type test(typename C::template queue_list_t<int> *);
	template <typename C> static std::false_type test(...);

	enum { value = !! decltype(test<T>(0))() };
};
template <typename Value, typename T, bool>
struct select_queue_list
{
	using type = typename T::template queue_list_t<Value>;
};
template <typename Value, typename T>
struct select_queue_list<Value, T, false>
{
	using type = std::list<Value>;
};

template <typename T>
struct has_type_mixins_t
{
	template <typename C> static std::true_type test(typename C::mixins_t *) ;
	template <typename C> static std::false_type test(...);    

	enum { value = !! decltype(test<T>(0))() };
};
template <typename T, bool> struct select_mixins { using type = typename T::mixins_t; };
template <typename T> struct select_mixins <T, false> { using type = mixin_list<>; };


template <typename Root, typename TList>
struct inherit_mixins;

template <typename Root, template <typename> class T, template <typename> class ...Args>
struct inherit_mixins <Root, mixin_list<T, Args...> >
{
	using type = T <typename inherit_mixins<Root, mixin_list<Args...> >::type>;
};

template <typename Root>
struct inherit_mixins <Root, mixin_list<> >
{
	using type = Root;
};

template <typename Root, typename TList, typename Func>
struct for_each_mixins;

template <typename Func, typename Root, template <typename> class T, template <typename> class ...Args>
struct for_each_mixins <Root, mixin_list<T, Args...>, Func>
{
	using type = typename inherit_mixins<Root, mixin_list<T, Args...> >::type;

	template <typename ...A>
	static bool for_each(A && ...args)
	{
		if(Func::template for_each<type>(args...))
		{
			return for_each_mixins<Root, mixin_list<Args...>, Func>::for_each(std::forward<A>(args)...);
		}
		return false;
	}
};

template <typename Root, typename Func>
struct for_each_mixins <Root, mixin_list<>, Func>
{
	using type = Root;

	template <typename ...A>
	static bool for_each(A && .../*args*/)
	{
		return true;
	}
};

template <typename T, typename ...Args>
struct has_function_mixin_before_dispatch
{
	template <typename C> static std::true_type test(
		decltype(std::declval<C>().mixin_before_dispatch(std::declval<Args>()...)) *
	);
	template <typename C> static std::false_type test(...);    

	enum { value = !! decltype(test<T>(0))() };
};


} //namespace dispatcheres


//-------------------------------------------------------------------------------------------------


namespace dispatcheres {

template <
	typename EventTypeT,
	typename ProtoType,
	typename PolicyType
>
class callback_list_base;

template <
	typename EventTypeT,
	typename PrototypeT,
	typename PolicyT,
	typename MixinRootT
>
class event_dispatcher_base;

template <
	typename EventTypeT,
	typename PolicyType,
	bool     HasTypeUserData,
	typename ProtoType
>
class node_traits;

template <
	typename EventTypeT,
	typename PolicyType,
	typename ReturnType, typename ...Args
>
class node_traits<
	EventTypeT,
	PolicyType,
	true,
	ReturnType (Args...)
>
{
private:
template <
	typename,
	typename,
	typename
>
friend class callback_list_base;

template <
	typename,
	typename,
	typename,
	typename
>
friend class event_dispatcher_base;

public:
	using policy_type = PolicyType;
	using event_type  = EventTypeT;

	using callback_type = typename select_callback<
		policy_type,
		has_type_callback_t<policy_type>::value,
		std::function<ReturnType (Args...)>
	>::type;

	using listener_name_type = typename select_listener_name<
		policy_type,
		has_type_listener_name_t<policy_type>::value,
		std::string
	>::type;

	using user_data_type = typename policy_type::user_data_t;

	struct node;
	using node_ptr = std::shared_ptr<node>;

	struct node
	{
	private:
		template <
			typename,
			typename,
			typename
		>
		friend class callback_list_base;

		template <
			typename,
			typename,
			typename,
			typename
		>
		friend class event_dispatcher_base;

	public:
		using counter_type = unsigned int;

		node(const listener_name_type& nm, const event_type& e, callback_type cb, const counter_type count)
			: evt(e), callback(std::move(cb)), counter(count), name(nm)
		{
		}

		inline const event_type get_event_type() const noexcept { return evt; }
		inline const listener_name_type& get_listener_name() const noexcept { return name; }

		inline node& set_user_data(user_data_type data) noexcept { userdata = std::move(data); return (*this); }
		inline const user_data_type& get_user_data() const noexcept { return userdata; }

	protected:
		event_type         evt;
		callback_type      callback;
		counter_type       counter;
		listener_name_type name{};
		user_data_type     userdata{};
		node_ptr prev;
		node_ptr next;
	};
};

template <
	typename EventTypeT,
	typename PolicyType,
	typename ReturnType, typename ...Args
>
class node_traits<
	EventTypeT,
	PolicyType,
	false,
	ReturnType (Args...)
>
{
private:
template <
	typename,
	typename,
	typename
>
friend class callback_list_base;

template <
	typename,
	typename,
	typename,
	typename
>
friend class event_dispatcher_base;

public:
	using policy_type = PolicyType;
	using event_type  = EventTypeT;

	using callback_type = typename select_callback<
		policy_type,
		has_type_callback_t<policy_type>::value,
		std::function<ReturnType (Args...)>
	>::type;
	
	using listener_name_type = typename select_listener_name<
		policy_type,
		has_type_listener_name_t<policy_type>::value,
		std::string
	>::type;

	using user_data_type = void;

	struct node;
	using node_ptr = std::shared_ptr<node>;

	struct node
	{
	private:
		template <
			typename,
			typename,
			typename
		>
		friend class callback_list_base;

		template <
			typename,
			typename,
			typename,
			typename
		>
		friend class event_dispatcher_base;

	public:
		using counter_type = unsigned int;

		node(const listener_name_type& nm, const event_type& e, callback_type cb, const counter_type count)
			: evt(e), callback(std::move(cb)), counter(count), name(nm)
		{
		}

		inline const event_type get_event_type() const noexcept { return evt; }
		inline const listener_name_type& get_listener_name() const noexcept { return name; }

	protected:
		event_type         evt;
		callback_type      callback;
		counter_type       counter;
		listener_name_type name{};
		node_ptr prev;
		node_ptr next;
	};
};


template <
	typename EventTypeT,
	typename PolicyType,
	typename ReturnType, typename ...Args
>
class callback_list_base<
	EventTypeT,
	ReturnType (Args...),
	PolicyType
> : public node_traits<EventTypeT, PolicyType, has_type_user_data_t<PolicyType>::value, ReturnType(Args...)>
{
private:
template <
	typename,
	typename,
	typename,
	typename
>
friend class event_dispatcher_base;

public:
	using node_traits_type =
		node_traits<EventTypeT, PolicyType, has_type_user_data_t<PolicyType>::value, ReturnType(Args...)>;

	using this_type = callback_list_base<
		EventTypeT,
		ReturnType(Args...),
		PolicyType
	>;

	using policy_type = PolicyType;
	using event_type  = EventTypeT;

	using thread_type = typename select_thread<policy_type, has_type_thread_t<policy_type>::value>::type;

	using callback_type = typename node_traits_type::callback_type;
	using listener_name_type = typename node_traits_type::listener_name_type;

	using can_continue_invoking_type = typename select_can_continue_invoking<
		policy_type, has_function_can_continue_invoking<policy_type, Args...>::value
	>::type;

	using node     = typename node_traits_type::node;
	using node_ptr = typename node_traits_type::node_ptr;

	class handle_type : public std::weak_ptr<node>
	{
	private:
		using super = std::weak_ptr<node>;

	public:
		using super::super;

		inline operator bool () const noexcept
		{
			return ! this->expired();
		}
	};

	using counter_type = typename node::counter_type;

	static constexpr counter_type removed_counter = 0;

public:
	using node_wptr   = handle_type;
	using mutex_type = typename thread_type::mutex;

public:
	callback_list_base() noexcept
		: head()
		, tail()
		, list_mtx_()
		, current_counter_(0)
	{
	}

	callback_list_base(const callback_list_base & other)
		: callback_list_base()
	{
		size_ = other.size_;
		copy_from(other.head);
	}

	callback_list_base(callback_list_base && other) noexcept
		: callback_list_base()
	{
		swap(other);
	}

	// If we use pass by value idiom and omit the 'this' check,
	// when assigning to self there is a deep copy which is inefficient.
	callback_list_base & operator = (const callback_list_base & other)
	{
		if(this != std::addressof(other))
		{
			callback_list_base copied(other);
			swap(copied);
		}
		return *this;
	}

	callback_list_base & operator = (callback_list_base && other) noexcept
	{
		if(this != std::addressof(other))
		{
			do_free_all_nodes();

			head = std::move(other.head);
			tail = std::move(other.tail);
			current_counter_ = other.current_counter_.load();
			size_ = other.size_;
		}
		return *this;
	}

	~callback_list_base()
	{
		// Don't lock mutex here since it may throw exception

		do_free_all_nodes();
	}
	
	void swap(callback_list_base & other) noexcept
	{
		using std::swap;
		
		swap(head, other.head);
		swap(tail, other.tail);
		swap(size_, other.size_);

		const auto value = current_counter_.load();
		current_counter_.exchange(other.current_counter_.load());
		other.current_counter_.exchange(value);
	}

	inline bool empty() const noexcept
	{
		// Don't lock the mutex for performance reason.
		// !head still works even when the underlying raw pointer is garbled (for other thread is writting to head)
		// And empty() doesn't guarantee the list is still empty after the function returned.
		//std::lock_guard<mutex_type> guard(list_mtx_);

		return ! head;
	}

	inline operator bool() const noexcept
	{
		return ! empty();
	}

	node_wptr append(const listener_name_type& name, const event_type& e, callback_type cb)
	{
		node_ptr n(do_allocate_node(name, e, std::move(cb)));

		typename thread_type::template unique_lock<mutex_type> guard(list_mtx_);

		if(head)
		{
			n->prev = tail;
			tail->next = n;
			tail = n;
		}
		else
		{
			head = n;
			tail = n;
		}

		++size_;

		return node_wptr(n);
	}

	node_wptr prepend(const listener_name_type& name, const event_type& e, callback_type cb)
	{
		node_ptr n(do_allocate_node(name, e, std::move(cb)));

		typename thread_type::template unique_lock<mutex_type> guard(list_mtx_);

		if(head)
		{
			n->next = head;
			head->prev = n;
			head = n;
		}
		else
		{
			head = n;
			tail = n;
		}

		++size_;

		return node_wptr(n);
	}

	node_wptr insert(const listener_name_type& name, const event_type& e, callback_type cb, const node_wptr & before)
	{
		node_ptr before_node = before.lock();
		if(before_node)
		{
			node_ptr n(do_allocate_node(name, e, std::move(cb)));

			typename thread_type::template unique_lock<mutex_type> guard(list_mtx_);

			do_insert(n, before_node);

			++size_;

			return node_wptr(n);
		}

		return append(name, e, std::move(cb));
	}

	bool remove(const node_wptr & nwptr)
	{
		auto n = nwptr.lock();
		if (n)
		{
			typename thread_type::template unique_lock<mutex_type> guard(list_mtx_);

			do_free_node(n);
			--size_;
			return true;
		}

		return false;
	}

	inline std::size_t size() const noexcept
	{
		return size_;
	}

	inline void clear() noexcept
	{
		callback_list_base other{};
		swap(other);
	}

protected:
	inline void iterator_move_to_next(node_ptr& n) noexcept
	{
		typename thread_type::template shared_lock<mutex_type> guard(list_mtx_);
		n = n->next;
	}
	inline void iterator_move_to_prev(node_ptr& n) noexcept
	{
		typename thread_type::template shared_lock<mutex_type> guard(list_mtx_);
		n = n->prev;
	}

public:
	class iterator
	{
	public:
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = node_ptr;
		using difference_type = std::ptrdiff_t;
		using pointer = value_type&;
		using reference = value_type&;

		explicit iterator(this_type& m) : master_(m)
		{
		}

		explicit iterator(this_type& m, value_type v) : master_(m), value_(std::move(v))
		{
		}

		explicit iterator(const this_type& m) : master_(const_cast<this_type&>(m))
		{
		}

		explicit iterator(const this_type& m, value_type v) : master_(const_cast<this_type&>(m)), value_(std::move(v))
		{
		}

		[[nodiscard]] reference operator*() noexcept
		{
			return value_;
		}

		[[nodiscard]] pointer operator->() noexcept
		{
			return value_;
		}

		iterator& operator++() noexcept
		{
			if (value_)
			{
				master_.iterator_move_to_next(value_);
			}
			return (*this);
		}

		iterator& operator--() noexcept
		{
			if (value_)
			{
				master_.iterator_move_to_prev(value_);
			}
			return (*this);
		}

		[[nodiscard]] bool operator==(const iterator& right) const
		{
			return (value_.get() == right.value_.get());
		}

		[[nodiscard]] bool operator!=(const iterator& right) const
		{
			return (value_.get() != right.value_.get());
		}


	protected:
		this_type& master_;

		value_type value_{};
	};

	using const_iterator = iterator;

	/// Return a const iterator to the beginning of the field sequence.
	inline iterator begin() noexcept
	{
		typename thread_type::template shared_lock<mutex_type> guard(list_mtx_);
		return iterator(*this, head);
	}

	/// Return a const iterator to the end of the field sequence.
	inline iterator end() noexcept
	{
		return iterator(*this);
	}

	/// Return a const iterator to the beginning of the field sequence.
	inline const_iterator begin() const noexcept
	{
		typename thread_type::template shared_lock<mutex_type> guard(list_mtx_);
		return (const_iterator(*this, head));
	}

	/// Return a const iterator to the end of the field sequence.
	inline const_iterator end() const noexcept
	{
		return (const_iterator(*this));
	}

	/// Return a const iterator to the beginning of the field sequence.
	inline const_iterator cbegin() const noexcept
	{
		return (begin());
	}

	/// Return a const iterator to the end of the field sequence.
	inline const_iterator cend() const noexcept
	{
		return (end());
	}


	template <typename Func>
	void for_each(Func && func) const
	{
		do_for_each_if([&func, this](node_ptr & n) -> bool
		{
			do_for_each_invoke<void>(func, n);
			return true;
		});
	}

	template <typename Func>
	bool for_each_if(Func && func) const
	{
		return do_for_each_if([&func, this](node_ptr & n) -> bool
		{
			return do_for_each_invoke<bool>(func, n);
		});
	}

#if !defined(__GNUC__) || __GNUC__ >= 5
	void operator() (Args ...args) const
	{
		for_each_if([&args...](callback_type & cb) -> bool
		{
			// We can't use std::forward here, because if we use std::forward,
			// for arg that is passed by value, and the callback prototype accepts it by value,
			// std::forward will move it and may cause the original value invalid.
			// That happens on any value-to-value passing, no matter the callback moves it or not.

			cb(args...);
			return can_continue_invoking_type::can_continue_invoking(args...);
		});
	}
#else
	// This is a patch version for GCC 4. It inlines the unrolled do_for_each_if.
	// GCC 4.8.3 doesn't supporting parameter pack catpure in lambda, see,
	// https://github.com/wqking/eventpp/issues/19
	// This is a compromised patch for GCC 4, it may be not maintained or updated unless there are bugs.
	// We don't use the patch as main code because the patch generates longer code, and duplicated with do_for_each_if.
	void operator() (Args ...args) const
	{
		node_ptr n;

		{
			typename thread_type::template shared_lock<mutex_type> guard(list_mtx_);
			n = head;
		}

		const counter_type counter = current_counter_.load(std::memory_order_acquire);

		while(n)
		{
			if(n->counter != removed_counter && counter >= n->counter)
			{
				n->callback(args...);
				if(! can_continue_invoking_type::can_continue_invoking(args...))
				{
					break;
				}
			}

			{
				typename thread_type::template shared_lock<mutex_type> guard(list_mtx_);
				n = n->next;
			}
		}
	}
#endif

private:
	template <typename F>
	bool do_for_each_if(F && f) const
	{
		node_ptr n;

		{
			typename thread_type::template shared_lock<mutex_type> guard(list_mtx_);
			n = head;
		}

		const counter_type counter = current_counter_.load(std::memory_order_acquire);

		while(n)
		{
			if(n->counter != removed_counter && counter >= n->counter)
			{
				if(! f(n))
				{
					return false;
				}
			}

			{
				typename thread_type::template shared_lock<mutex_type> guard(list_mtx_);
				n = n->next;
			}
		}

		return true;
	}

	template <typename RT, typename Func>
	inline auto do_for_each_invoke(Func && func, node_ptr & n) const
		-> typename std::enable_if<can_invoke<Func, node_wptr, callback_type &>::value, RT>::type
	{
		return func(node_wptr(n), n->callback);
	}

	template <typename RT, typename Func>
	inline auto do_for_each_invoke(Func && func, node_ptr & n) const
		-> typename std::enable_if<can_invoke<Func, callback_type &>::value, RT>::type
	{
		return func(n->callback);
	}

	void do_insert(node_ptr & n, node_ptr & before_node)
	{
		n->prev = before_node->prev;
		n->next = before_node;
		if(before_node->prev)
		{
			before_node->prev->next = n;
		}
		before_node->prev = n;

		if(before_node == head)
		{
			head = n;
		}
	}
	
	inline node_ptr do_allocate_node(const listener_name_type& name, const event_type& e, callback_type cb)
	{
		return std::make_shared<node>(name, e, std::move(cb), get_next_counter());
	}
	
	void do_free_node(const node_ptr & cn)
	{
		node_ptr & n = const_cast<node_ptr &>(cn);

		if(n->next)
		{
			n->next->prev = n->prev;
		}
		if(n->prev)
		{
			n->prev->next = n->next;
		}

		// Mark it as deleted, this must be before the assignment of head and tail below,
		// because node can be a reference to head or tail, and after the assignment, node
		// can be null pointer.
		n->counter = removed_counter;

		if(head == n)
		{
			head = n->next;
		}
		if(tail == n)
		{
			tail = n->prev;
		}

		// don't modify n->prev or n->next
		// because node may be still used in a loop.
	}

	void do_free_all_nodes()
	{
		node_ptr n = head;
		head.reset();
		while(n)
		{
			node_ptr next = n->next;
			n->prev.reset();
			n->next.reset();
			n = next;
		}
		n.reset();
	}

	counter_type get_next_counter()
	{
		counter_type result = ++current_counter_;;
		if(result == 0)
		{
			// overflow, let's reset all nodes' counters.
			{
				typename thread_type::template unique_lock<mutex_type> guard(list_mtx_);
				node_ptr n = head;
				while(n)
				{
					n->counter = 1;
					n = n->next;
				}
			}
			result = ++current_counter_;
		}

		return result;
	}
	
	void copy_from(const node_ptr & from_head)
	{
		node_ptr from_node(from_head);
		node_ptr n;
		const counter_type counter = get_next_counter();
		while(from_node)
		{
			const node_ptr next_node(std::make_shared<node>(from_node->evt, from_node->callback, counter));

			next_node->prev = n;

			if(n)
			{
				n->next = next_node;
			}
			else
			{
				n = next_node;
				head = n;
			}
		
			n = next_node;
			from_node = from_node->next;
		}

		tail = n;
	}

private:
	node_ptr head;
	node_ptr tail;
	mutable mutex_type list_mtx_;
	typename thread_type::template atomic<counter_type> current_counter_;
	std::size_t size_ = 0;
};


} //namespace dispatcheres


namespace dispatcheres {

template <
	typename EventTypeT,
	typename PrototypeT,
	typename PolicyT = default_policy
>
class callback_list : public dispatcheres::callback_list_base<EventTypeT, PrototypeT, PolicyT>, public tag_callback_list
{
private:
	using super = dispatcheres::callback_list_base<EventTypeT, PrototypeT, PolicyT>;
	
public:
	using super::super;
	
	friend void swap(callback_list & first, callback_list & second) noexcept
	{
		first.swap(second);
	}
};

} //namespace dispatcheres


//-------------------------------------------------------------------------------------------------


namespace dispatcheres {

template <
	typename EventTypeT,
	typename PolicyT,
	typename MixinRootT,
	typename ReturnType, typename ...Args
>
class event_dispatcher_base <
	EventTypeT,
	ReturnType (Args...),
	PolicyT,
	MixinRootT
>
{
public:
	using this_type = event_dispatcher_base<
		EventTypeT,
		ReturnType (Args...),
		PolicyT,
		MixinRootT
	>;
	using mixin_root_type = typename std::conditional<
		std::is_same<MixinRootT, void>::value,
		this_type,
		MixinRootT
	>::type;
	using policy_type = PolicyT;

	using thread_type = typename select_thread<PolicyT, has_type_thread_t<PolicyT>::value>::type;

	using argument_passing_mode_type = typename select_argument_passing_mode<
		PolicyT,
		has_type_argument_passing_mode<PolicyT>::value,
		argument_passing_auto_detect
	>::type;

	using callback_type = typename select_callback<
		PolicyT,
		has_type_callback_t<PolicyT>::value,
		std::function<ReturnType (Args...)>
	>::type;
	using callback_list_type = callback_list<EventTypeT, ReturnType (Args...), PolicyT>;

	using listener_name_type = typename callback_list_type::listener_name_type;

	using proto_type = ReturnType (Args...);

	using map_type = typename select_map<
		EventTypeT,
		callback_list_type,
		PolicyT,
		has_template_map_t<PolicyT>::value
	>::type;

	using listener_name_map_type = typename select_listener_name_map<
		listener_name_type,
		EventTypeT,
		typename callback_list_type::node_wptr,
		PolicyT,
		has_template_map_t<PolicyT>::value
	>::type;

	using mixins_type = typename dispatcheres::select_mixins<
		PolicyT,
		dispatcheres::has_type_mixins_t<PolicyT>::value
	>::type;

public:
	using listener_wptr = typename callback_list_type::node_wptr;
	using listener_type = listener_wptr;
	using event_type = EventTypeT;
	using mutex_type = typename thread_type::mutex;

public:
	event_dispatcher_base()
		: listener_mtx_()
		, listener_map_()
		, listener_name_map_()
	{
	}

	event_dispatcher_base(const event_dispatcher_base & other)
		: listener_mtx_()
		, listener_map_(other.listener_map_)
		, listener_name_map_(other.listener_name_map_)
	{
	}

	event_dispatcher_base(event_dispatcher_base && other) noexcept
		: listener_mtx_()
		, listener_map_(std::move(other.listener_map_))
		, listener_name_map_(std::move(other.listener_name_map_))
	{
	}

	event_dispatcher_base & operator = (const event_dispatcher_base & other)
	{
		listener_map_ = other.listener_map_;
		listener_name_map_ = other.listener_name_map_;
		return *this;
	}

	event_dispatcher_base & operator = (event_dispatcher_base && other) noexcept
	{
		listener_map_ = std::move(other.listener_map_);
		listener_name_map_ = std::move(other.listener_name_map_);
		return *this;
	}

	void swap(event_dispatcher_base & other) noexcept
	{
		using std::swap;
		
		swap(listener_map_, other.listener_map_);
		swap(listener_name_map_, other.listener_name_map_);
	}

	listener_wptr append_listener(const event_type& e, callback_type cb)
	{
		return append_listener(listener_name_type{}, e, std::move(cb));
	}

	std::vector<listener_wptr> append_listener(const std::initializer_list<event_type>& es, const callback_type& cb)
	{
		return append_listener(listener_name_type{}, es, cb);
	}

	listener_wptr prepend_listener(const event_type& e, callback_type cb)
	{
		return prepend_listener(listener_name_type{}, e, std::move(cb));
	}

	std::vector<listener_wptr> prepend_listener(const std::initializer_list<event_type>& es, const callback_type& cb)
	{
		return prepend_listener(listener_name_type{}, es, cb);
	}

	listener_wptr insert_listener(const event_type& e, callback_type cb, const listener_wptr & before)
	{
		return insert_listener(listener_name_type{}, e, std::move(cb), before);
	}

	template<class Function>
	typename std::enable_if_t<can_invoke<Function, typename callback_list_type::node_ptr&>::value, listener_wptr>
	insert_listener(const event_type& e, callback_type cb, Function&& pred)
	{
		return insert_listener(listener_name_type{}, e, std::move(cb), std::forward<Function>(pred));
	}

	listener_wptr append_listener(const listener_name_type& name, const event_type& e, callback_type cb)
	{
		typename thread_type::template unique_lock<mutex_type> guard(listener_mtx_);

		listener_wptr ptr = listener_map_[e].append(name, e, std::move(cb));

		listener_name_map_[name].emplace(e, ptr);

		return ptr;
	}

	std::vector<listener_wptr> append_listener(
		const listener_name_type& name, const std::initializer_list<event_type>& es, const callback_type& cb)
	{
		typename thread_type::template unique_lock<mutex_type> guard(listener_mtx_);

		std::vector<listener_wptr> v;

		v.reserve(es.size());

		for (auto& e : es)
		{
			listener_wptr ptr = listener_map_[e].append(name, e, cb);

			listener_name_map_[name].emplace(e, ptr);

			v.emplace_back(std::move(ptr));
		}

		return v;
	}

	listener_wptr prepend_listener(const listener_name_type& name, const event_type & e, callback_type cb)
	{
		typename thread_type::template unique_lock<mutex_type> guard(listener_mtx_);

		listener_wptr ptr = listener_map_[e].prepend(name, e, std::move(cb));

		listener_name_map_[name].emplace(e, ptr);

		return ptr;
	}

	std::vector<listener_wptr> prepend_listener(
		const listener_name_type& name, const std::initializer_list<event_type>& es, const callback_type& cb)
	{
		typename thread_type::template unique_lock<mutex_type> guard(listener_mtx_);

		std::vector<listener_wptr> v;

		v.reserve(es.size());

		for (auto it = std::rbegin(es); it != std::rend(es); ++it)
		{
			listener_wptr ptr = listener_map_[*it].prepend(name, *it, cb);

			listener_name_map_[name].emplace(*it, ptr);

			v.emplace(v.begin(), std::move(ptr));
		}

		return v;
	}

	listener_wptr insert_listener(
		const listener_name_type& name, const event_type& e, callback_type cb, const listener_wptr & before)
	{
		auto n = before.lock();

		typename thread_type::template unique_lock<mutex_type> guard(listener_mtx_);

		if (n)
		{
			if (n->evt != e)
			{
			#if defined(_DEBUG) || defined(DEBUG)
				assert(false);
			#endif
				return listener_wptr();
			}
		}

		listener_wptr ptr = listener_map_[e].insert(name, e, std::move(cb), before);

		listener_name_map_[name].emplace(e, ptr);

		return ptr;
	}

	/**
	 * @param pred The check function, use to determin which listener of the new listener will be insert before
	 * the "pred" Function signature : bool(auto& listener_ptr)
	 * eg:
	 *	dispatcher.insert_listener("listener_a", 3,
	 *		[]()
	 *		{
	 *			// this is the listener callback function
	 *		},
	 *		[index](auto& listener_ptr) -> bool
	 *		{
	 *			// this is the check function
	 *
	 *			return (index < listener_ptr->get_user_data());
	 *
	 *			// if this function return true , the new listener will be insert before "listener_ptr"
	 *			// if this function return false, then will be check the next listener in the list
	 *			// if all checks return false, then the new listener will be append after the tail.
	 *		}
	 *	);
	 */
	template<class Function>
	typename std::enable_if_t<can_invoke<Function, typename callback_list_type::node_ptr&>::value, listener_wptr>
	insert_listener(const listener_name_type& name, const event_type & e, callback_type cb, Function&& pred)
	{
		typename thread_type::template unique_lock<mutex_type> guard(listener_mtx_);

		// Returns a reference to the value that is mapped to a key equivalent to key,
		// performing an insertion if such key does not already exist.

		callback_list_type& cblist = listener_map_[e];

		for (auto it = cblist.begin(); it != cblist.end(); ++it)
		{
			if (pred(*it))
			{
				listener_wptr ptr = cblist.insert(name, e, std::move(cb), *it);

				listener_name_map_[name].emplace(e, ptr);

				return ptr;
			}
		}

		listener_wptr ptr = cblist.append(name, e, std::move(cb));

		listener_name_map_[name].emplace(e, ptr);

		return ptr;
	}

	bool remove_listener(const listener_wptr& listener)
	{
		auto n = listener.lock();
		if (n)
		{
			typename thread_type::template unique_lock<mutex_type> guard(listener_mtx_);

			auto it1 = listener_name_map_.find(n->name);
			if (it1 != this->listener_name_map_.end())
			{
				auto& inner_map = it1->second;

				auto&& [beg, end] = inner_map.equal_range(n->evt);

				for (auto it = beg; it != end; ++it)
				{
					if (it->second.lock().get() == n.get())
					{
						inner_map.erase(it);
						break;
					}
				}

				if (inner_map.empty())
				{
					listener_name_map_.erase(n->name);
				}
			}

			auto it2 = this->listener_map_.find(n->evt);
			if (it2 != this->listener_map_.end())
			{
				callback_list_type& cblist = it2->second;

				bool r = cblist.remove(listener);

				if (cblist.empty())
				{
					this->listener_map_.erase(n->evt);
				}

				return r;
			}
		}

		return false;
	}

	bool remove_listener(const listener_name_type& name, const event_type & e)
	{
		typename thread_type::template unique_lock<mutex_type> guard(listener_mtx_);

		auto it1 = this->listener_name_map_.find(name);
		auto it2 = this->listener_map_.find(e);

		if (it1 != this->listener_name_map_.end() && it2 != this->listener_map_.end())
		{
			callback_list_type& cblist = it2->second;

			auto& inner_map = it1->second;

			auto&& [beg, end] = inner_map.equal_range(e);

			bool r = false;

			for (auto it = beg; it != end;)
			{
				r |= cblist.remove(it->second);

				it = inner_map.erase(it);
			}

			if (inner_map.empty())
			{
				this->listener_name_map_.erase(name);
			}

			if (cblist.empty())
			{
				this->listener_map_.erase(e);
			}

			return r;
		}

		return false;
	}

	bool remove_listener(const listener_name_type& name)
	{
		typename thread_type::template unique_lock<mutex_type> guard(listener_mtx_);

		auto it1 = listener_name_map_.find(name);
		if (it1 != this->listener_name_map_.end())
		{
			bool r = false;

			auto& inner_map = it1->second;

			for (auto it = inner_map.begin(); it != inner_map.end();)
			{
				auto n = it->second.lock();

				if (n)
				{
					auto it2 = this->listener_map_.find(n->evt);
					if (it2 != this->listener_map_.end())
					{
						callback_list_type& cblist = it2->second;

						r |= cblist.remove(it->second);

						if (cblist.empty())
						{
							this->listener_map_.erase(n->evt);
						}
					}
				}

				it = inner_map.erase(it);
			}

			if (inner_map.empty())
			{
				listener_name_map_.erase(name);
			}

			return r;
		}

		return false;
	}

	void clear_all_listeners() noexcept
	{
		typename thread_type::template unique_lock<mutex_type> guard(listener_mtx_);

		for (auto& [e, cblist] : listener_map_)
		{
			std::ignore = e;

			cblist.clear();
		}

		listener_map_.clear();
		listener_name_map_.clear();
	}

	void reserve(std::size_t new_cap)
	{
		typename thread_type::template unique_lock<mutex_type> guard(listener_mtx_);

		this->do_reserve(listener_map_, new_cap);
		this->do_reserve(listener_name_map_, new_cap);
	}

	std::size_t get_listener_count(const event_type& e)
	{
		const callback_list_type* callable_list = do_find_callable_list(e);
		if (callable_list)
		{
			return callable_list->size();
		}

		return 0;
	}

	/**
	 * Note : the "listener_name_type" maybe equal to the "event_type".
	 * so we can't use this function overloading, but we can use the std::optional<event_type>.
	 */
	//std::size_t get_listener_count_by_name(const listener_name_type& name) {...}

	std::size_t get_listener_count(const listener_name_type& name, std::optional<event_type> e = std::nullopt)
	{
		typename thread_type::template shared_lock<mutex_type> guard(listener_mtx_);

		if (listener_name_map_.empty())
			return 0;

		auto it = listener_name_map_.find(name);
		if (it != listener_name_map_.end())
		{
			if (e.has_value())
				return it->second.count(e.value());
			else
				return it->second.size();
		}

		return 0;
	}

	std::size_t get_listener_count()
	{
		std::size_t count = 0;

		typename thread_type::template shared_lock<mutex_type> guard(listener_mtx_);

		if (listener_map_.empty())
			return 0;

		for (auto& [e, cblist] : listener_map_)
		{
			std::ignore = e;

			count += cblist.size();
		}

	#if defined(_DEBUG) || defined(DEBUG)
		std::size_t count2 = 0;
		for (auto&[name, mulmap] : listener_name_map_)
		{
			std::ignore = name;

			count2 += mulmap.size();
		}
		assert(count == count2);
	#endif

		return count;
	}

	bool has_any_listener(const event_type & e) const
	{
		const callback_list_type * callable_list = do_find_callable_list(e);
		if(callable_list)
		{
			return ! callable_list->empty();
		}

		return false;
	}

	inline callback_list_type* find_listeners(const event_type & e)
	{
		return do_find_callable_list(e);
	}

	/**
	 * @param pred Find the listener by user defined rule
	 * the "pred" Function signature : bool(auto& listener_ptr)
	 */
	template<class Function>
	inline listener_wptr find_listener_if(const event_type & e, Function&& pred)
	{
		typename thread_type::template shared_lock<mutex_type> guard(listener_mtx_);

		auto it1 = this->listener_map_.find(e);
		if (it1 != this->listener_map_.end())
		{
			callback_list_type& cblist = it1->second;

			for (auto it2 = cblist.begin(); it2 != cblist.end(); ++it2)
			{
				if (pred(*it2))
				{
					return listener_wptr(*it2);
				}
			}
		}

		return listener_wptr();
	}

	template <typename Func>
	void for_each(Func&& func) const
	{
		typename thread_type::template shared_lock<mutex_type> guard(listener_mtx_);

		for (auto& [e, cblist] : this->listener_map_)
		{
			std::ignore = e;
			cblist.for_each(func);
		}
	}

	template <typename Func>
	void for_each(const event_type & e, Func && func) const
	{
		const callback_list_type * callable_list = do_find_callable_list(e);
		if(callable_list)
		{
			callable_list->for_each(std::forward<Func>(func));
		}
	}

	template <typename Func>
	bool for_each_if(const event_type & e, Func && func) const
	{
		const callback_list_type * callable_list = do_find_callable_list(e);
		if (callable_list)
		{
			return callable_list->for_each_if(std::forward<Func>(func));
		}

		return true;
	}

	void dispatch(Args ...args) const
	{
		static_assert(argument_passing_mode_type::can_include_event_type,
			"Dispatching arguments count doesn't match required (event type should be included).");

		using get_event_t = typename select_get_event<
			PolicyT, EventTypeT, has_function_get_event<PolicyT, Args...>::value>::type;

		const event_type e = get_event_t::get_event(args...);

		direct_dispatch(e, std::forward<Args>(args)...);
	}

	template <typename T>
	void dispatch(T && first, Args ...args) const
	{
		static_assert(argument_passing_mode_type::can_exclude_event_type,
			"Dispatching arguments count doesn't match required (event type should NOT be included).");

		using get_event_t = typename select_get_event<
			PolicyT, EventTypeT, has_function_get_event<PolicyT, T &&, Args...>::value>::type;

		const event_type e = get_event_t::get_event(std::forward<T>(first), args...);

		direct_dispatch(e, std::forward<Args>(args)...);
	}

	// Bypass any get_event policy. The first argument is the event type.
	// Most used for internal purpose.
	void direct_dispatch(const event_type & e, Args ...args) const
	{
		if(! dispatcheres::for_each_mixins<mixin_root_type, mixins_type, do_mixin_before_dispatch>::for_each(
			this, typename std::add_lvalue_reference<Args>::type(args)...))
		{
			return;
		}

		const callback_list_type * callable_list = do_find_callable_list(e);
		if(callable_list)
		{
			(*callable_list)(std::forward<Args>(args)...);
		}
	}

protected:
	inline const callback_list_type * do_find_callable_list(const event_type & e) const
	{
		return do_find_callable_list_helper(this, e);
	}

	inline callback_list_type * do_find_callable_list(const event_type & e)
	{
		return do_find_callable_list_helper(this, e);
	}

	template<class M>
	inline void do_reserve(M& map, std::size_t new_cap)
	{
		if constexpr (has_member_reserve<M, void, std::size_t>::value)
		{
			map.reserve(new_cap);
		}
		else
		{
			std::ignore = map;
			std::ignore = new_cap;
		}
	}

private:
	// template helper to avoid code duplication in do_find_callable_list
	template <typename T>
	static auto do_find_callable_list_helper(T * self, const event_type & e)
		-> typename std::conditional<std::is_const<T>::value, const callback_list_type *, callback_list_type *>::type
	{
		if (self->listener_map_.empty())
			return nullptr;

		typename thread_type::template shared_lock<mutex_type> guard(self->listener_mtx_);

		auto it = self->listener_map_.find(e);
		if(it != self->listener_map_.end())
		{
			return std::addressof(it->second);
		}
		else
		{
			return nullptr;
		}
	}

private:
	// Mixin related
	struct do_mixin_before_dispatch
	{
		template <typename T, typename Self, typename ...A>
		static auto for_each(const Self * self, A && ...args)
			-> typename std::enable_if<has_function_mixin_before_dispatch<T, A...>::value, bool>::type
		{
			return static_cast<const T *>(self)->mixin_before_dispatch(std::forward<A>(args)...);
		}

		template <typename T, typename Self, typename ...A>
		static auto for_each(const Self * /*self*/, A && ... /*args*/)
			-> typename std::enable_if<! has_function_mixin_before_dispatch<T, A...>::value, bool>::type
		{
			return true;
		}
	};

private:
	mutable mutex_type     listener_mtx_;
	map_type               listener_map_;
	listener_name_map_type listener_name_map_;
};


} //namespace dispatcheres


//-------------------------------------------------------------------------------------------------


namespace dispatcheres {

template <typename Base>
class mixin_filter : public Base
{
private:
	using super = Base;

	using bool_reference_prototype = typename dispatcheres::replace_return_type<
		typename dispatcheres::transform_arguments<
			typename super::proto_type,
			std::add_lvalue_reference
		>::type,
		bool
	>::type;

	using filter_func_type = std::function<bool_reference_prototype>;
	using filter_list_type = callback_list<char, bool_reference_prototype>;
	using listener_name_type = typename filter_list_type::listener_name_type;

public:
	using filter_wptr = typename filter_list_type::node_wptr;
	using filter_type = filter_wptr;

public:
	inline filter_wptr append_filter(const filter_func_type& filter_func)
	{
		return filter_list_.append(listener_name_type{}, '0', filter_func);
	}

	inline bool remove_filter(const filter_wptr & filter)
	{
		return filter_list_.remove(filter);
	}

	inline void clear_all_filters() noexcept
	{
		return filter_list_.clear();
	}

	inline std::size_t get_filter_count() const noexcept
	{
		return filter_list_.size();
	}

	template <typename ...Args>
	inline bool mixin_before_dispatch(Args && ...args) const
	{
		if(! filter_list_.empty())
		{
			if(! filter_list_.for_each_if([&args...](typename filter_list_type::callback_type & cb)
				{
					return cb(args...);
				})
			)
			{
				return false;
			}
		}

		return true;
	}

private:
	filter_list_type filter_list_;
};

} //namespace dispatcheres


//-------------------------------------------------------------------------------------------------


template <
	typename EventT,
	typename PrototypeT,
	typename PolicyT = dispatcheres::default_policy
>
class event_dispatcher : public dispatcheres::inherit_mixins<
		dispatcheres::event_dispatcher_base<EventT, PrototypeT, PolicyT, void>,
		typename dispatcheres::select_mixins<PolicyT, dispatcheres::has_type_mixins_t<PolicyT>::value >::type
	>::type, public dispatcheres::tag_event_dispatcher
{
private:
	using super = typename dispatcheres::inherit_mixins<
		dispatcheres::event_dispatcher_base<EventT, PrototypeT, PolicyT, void>,
		typename dispatcheres::select_mixins<PolicyT, dispatcheres::has_type_mixins_t<PolicyT>::value >::type
	>::type;

public:
	using super::super;
	
	friend void swap(event_dispatcher & first, event_dispatcher & second) noexcept
	{
		first.swap(second);
	}
};


} //namespace asio2

#endif
