#include "unit_test.hpp"
#include <asio2/util/event_dispatcher.hpp>
#include <iostream>

void event_dispatcher_test_basic()
{
	std::cout << std::endl << "event_dispatcher tutorial 1, basic" << std::endl;

	// The namespace is asio2
	// The first template parameter int is the event type,
	// the event type can be any type such as std::string, int, etc.
	// The second is the prototype of the listener.
	asio2::event_dispatcher<int, void ()> dispatcher;

	std::vector<asio2::event_dispatcher<int, void()>::listener_type> listeners;
	ASIO2_CHECK(dispatcher.get_listener_count() == 0);

	// Add a listener. As the type of dispatcher,
	// here 3 and 5 is the event type,
	// []() {} is the listener.
	// Lambda is not required, any function or std::function
	// or whatever function object with the required prototype is fine.
	int f31 = -1;
	auto listener = dispatcher.append_listener(3, [&]()
	{
		f31 = 1;
	});

	dispatcher.for_each(3, [](const auto& listener_wptr, const auto& callback)
	{
		auto listener_ptr = listener_wptr.lock();
		std::ignore = listener_ptr;
		std::ignore = callback;
	});

	ASIO2_CHECK(dispatcher.get_listener_count() == 1);
	listeners.emplace_back(listener);
	ASIO2_CHECK(dispatcher.has_any_listener(3));
	dispatcher.remove_listener(listeners.front());
	ASIO2_CHECK(dispatcher.get_listener_count() == 0);
	ASIO2_CHECK(!dispatcher.has_any_listener(3));

	// listener has been removed, so the listener will be append at the tail
	listener = dispatcher.insert_listener(3, []() {}, listener);
	ASIO2_CHECK(listener);
	ASIO2_CHECK(dispatcher.get_listener_count(3) == 1);

	listener = dispatcher.find_listener_if(3, [](auto& listener_ptr) { return (listener_ptr->get_event_type() == 30); });
	ASIO2_CHECK(!listener);

	// the listener is empty, so remove listener will be failed.
	ASIO2_CHECK(!dispatcher.remove_listener(listener));

	listener = dispatcher.find_listener_if(3, [](auto& listener_ptr) { return (listener_ptr->get_event_type() == 3); });
	ASIO2_CHECK(listener);

	dispatcher.remove_listener(listener);
	ASIO2_CHECK(!listener);
	ASIO2_CHECK(dispatcher.get_listener_count(3) == 0);

	listener = dispatcher.find_listener_if(3, [](auto& listener_ptr) { return (listener_ptr->get_event_type() == 3); });
	ASIO2_CHECK(!listener);

	int f32 = -1;
	listener = dispatcher.append_listener(3, [&]()
	{
		f32 = 1;
	});
	ASIO2_CHECK(dispatcher.has_any_listener(3));
	ASIO2_CHECK(dispatcher.get_listener_count() == 1);
	ASIO2_CHECK(dispatcher.get_listener_count(3) == 1);

	// the event_type 30 is not equal to the "listener->evt", so this insert will failed.
#if !defined(DEBUG) && !defined(_DEBUG)
	listener = dispatcher.insert_listener(30, []() {}, listener);
	ASIO2_CHECK(!listener);
#endif

	int f51 = -1;
	dispatcher.append_listener(5, [&]()
	{
		f51 = 1;
	});
	ASIO2_CHECK(dispatcher.get_listener_count() == 2);
	ASIO2_CHECK(dispatcher.get_listener_count(5) == 1);

	dispatcher.for_each([](const auto& listener_wptr, const auto& callback)
	{
		auto listener_ptr = listener_wptr.lock();
		std::ignore = listener_ptr;
		std::ignore = callback;
	});

	int f52 = -1;
	dispatcher.append_listener(5, [&]()
	{
		f52 = 1;
	});
	ASIO2_CHECK(dispatcher.get_listener_count() == 3);
	ASIO2_CHECK(dispatcher.get_listener_count(5) == 2);

	auto liss1 = dispatcher.append_listener({ 6,7 }, [&]()
	{

	});
	ASIO2_CHECK(dispatcher.get_listener_count(6) == 1);
	ASIO2_CHECK(dispatcher.get_listener_count(7) == 1);

	listeners.insert(listeners.end(), liss1.begin(), liss1.end());
	ASIO2_CHECK(dispatcher.get_listener_count() == 5);

	auto liss2 = dispatcher.prepend_listener({ 8,9 }, [&]()
	{

	});
	ASIO2_CHECK(dispatcher.get_listener_count(8) == 1);
	ASIO2_CHECK(dispatcher.get_listener_count(9) == 1);

	listeners.insert(listeners.end(), liss2.begin(), liss2.end());
	ASIO2_CHECK(dispatcher.get_listener_count() == 7);

	dispatcher.insert_listener(3, []() {}, listener);
	ASIO2_CHECK(dispatcher.get_listener_count(3) == 2);
	ASIO2_CHECK(dispatcher.get_listener_count() == 8);

	auto* listener_list = dispatcher.find_listeners(5);
	ASIO2_CHECK(listener_list != nullptr);
	ASIO2_CHECK(listener_list->size() == 2);
	for (auto it = listener_list->begin(); it != listener_list->end(); ++it)
	{
		ASIO2_CHECK(it->get_event_type() == 5);
	}
	for (auto it = listener_list->cbegin(); it != listener_list->cend(); ++it)
	{
		ASIO2_CHECK(it->get_event_type() == 5);
	}

	ASIO2_CHECK(dispatcher.get_listener_count() == 8);
	// Dispatch the events, the first argument is always the event type.
	dispatcher.dispatch(3);
	dispatcher.dispatch(5);

	ASIO2_CHECK(f31 == -1);
	ASIO2_CHECK(f32 == 1);
	ASIO2_CHECK(f51 == 1);
	ASIO2_CHECK(f52 == 1);

	dispatcher.clear_all_listeners();
	ASIO2_CHECK(dispatcher.get_listener_count() == 0);
}

void event_dispatcher_test_listener_with_parameters()
{
	std::cout << std::endl << "event_dispatcher tutorial 2, listener with parameters" << std::endl;

	// The listener has two parameters.
	asio2::event_dispatcher<int, void (const std::string &, const bool)> dispatcher;

	int f31 = -1;
	dispatcher.append_listener(3, [&](const std::string & s, const bool b)
	{
		f31 = 1;
		ASIO2_CHECK(s == "Hello" && b == true);
	});
	// The listener prototype doesn't need to be exactly same as the dispatcher.
	// It would be find as long as the arguments is compatible with the dispatcher.
	int f51 = -1;
	dispatcher.append_listener(5, [&](std::string s, int b)
	{
		f51 = 1;
		ASIO2_CHECK(s == "World" && b == false);
	});
	int f52 = -1;
	dispatcher.append_listener(5, [&](const std::string & s, const bool b)
	{
		f52 = 1;
		ASIO2_CHECK(s == "World" && b == false);
	});

	// Dispatch the events, the first argument is always the event type.
	dispatcher.dispatch(3, "Hello", true);
	dispatcher.dispatch(5, "World", false);

	ASIO2_CHECK(f31 == 1);
	ASIO2_CHECK(f51 == 1);
	ASIO2_CHECK(f52 == 1);
}

void event_dispatcher_test_customized_event_struct()
{
	std::cout << std::endl << "event_dispatcher tutorial 3, customized event struct" << std::endl;

	// Define an Event to hold all parameters.
	struct my_event
	{
		int type;
		std::string message;
		int param;
	};

	// Define policies to let the dispatcher knows how to
	// extract the event type.
	struct my_event_policy
	{
		static int get_event(const my_event & e, bool /*b*/)
		{
			return e.type;
		}
	};

	// Pass my_event_policy as the third template argument of event_dispatcher.
	// Note: the first template argument is the event type type int, not my_event.
	asio2::event_dispatcher<int, void(const my_event&, bool), my_event_policy> dispatcher;

	// Add a listener.
	// Note: the first argument is the event type of type int, not my_event.
	int f = -1;
	dispatcher.append_listener(3, [&](const my_event & e, bool b)
	{
		f = 1;
		ASIO2_CHECK(e.type == 3);
		ASIO2_CHECK(e.message == "Hello world");
		ASIO2_CHECK(e.param == 38);
		ASIO2_CHECK(b == true);
	});

	// Dispatch the event.
	// The first argument is Event.
	dispatcher.dispatch(my_event { 3, "Hello world", 38 }, true);

	ASIO2_CHECK(f == 1);
}

struct Tutor4MyEvent
{
	Tutor4MyEvent() : type(0), canceled(false)
	{
	}
	explicit Tutor4MyEvent(const int type)
		: type(type), canceled(false)
	{
	}

	int type;
	mutable bool canceled;
};

struct Tutor4MyEventPolicy
{
	// E is Tutor4MyEvent and get_event doesn't need to be template.
	// We make it template to show get_event can be templated member.
	template <typename E>
	static int get_event(const E & e)
	{
		return e.type;
	}

	// E is Tutor4MyEvent and can_continue_invoking doesn't need to be template.
	// We make it template to show can_continue_invoking can be templated member.
	template <typename E>
	static bool can_continue_invoking(const E & e)
	{
		return ! e.canceled;
	}

	using thread_t = asio2::dispatcheres::single_thread;
	using user_data_t = std::size_t;
};

void event_dispatcher_test_event_canceling()
{
	std::cout << std::endl << "event_dispatcher tutorial 4, event canceling" << std::endl;

	asio2::event_dispatcher<int, void (const Tutor4MyEvent &), Tutor4MyEventPolicy> dispatcher;

	int f31 = -1;
	dispatcher.append_listener(3, [&](const Tutor4MyEvent & e)
	{
		f31 = 1;
		ASIO2_CHECK(e.type == 3);
		e.canceled = true;
	});
	int f32 = -1;
	auto listener = dispatcher.append_listener(3, [&](const Tutor4MyEvent & /*e*/)
	{
		f32 = 1;
	});

	listener.lock()->set_user_data(200);

	ASIO2_CHECK(listener.lock()->get_user_data() == 200);

	int f33 = -1;
	dispatcher.insert_listener(3,
	[&](const Tutor4MyEvent & /*e*/)
	{
		f33 = 1;
	},
	[](auto& listener_ptr)
	{
		return listener_ptr->get_user_data() == 0;
	});
	ASIO2_CHECK(dispatcher.get_listener_count(3) == 3);
	ASIO2_CHECK(dispatcher.get_listener_count() == 3);

	dispatcher.dispatch(Tutor4MyEvent(3));

	ASIO2_CHECK(f31 == 1);
	ASIO2_CHECK(f32 == -1);
	ASIO2_CHECK(f33 == 1);
}

void event_dispatcher_test_event_filter()
{
	std::cout << std::endl << "event_dispatcher tutorial 5, event filter" << std::endl;

	struct MyPolicy
	{
		using mixins_t = asio2::dispatcheres::mixin_list<asio2::dispatcheres::mixin_filter>;
	};

	asio2::event_dispatcher<int, void (int e, int i, std::string), MyPolicy> dispatcher;

	dispatcher.reserve(100);

	std::vector<int> v;

	int f3 = -1;
	dispatcher.append_listener(3, [&](const int e, const int i, const std::string & s)
	{
		f3 = 1;
		v.emplace_back(304);
		ASIO2_CHECK(e == 3 && i == 38 && s == "Hi");
	});
	int f5 = -1;
	dispatcher.append_listener(5, [&](const int /*e*/, const int /*i*/, const std::string & /*s*/)
	{
		f5 = 1;
		v.emplace_back(-2);
	});

	ASIO2_CHECK(dispatcher.get_listener_count() == 2);
	ASIO2_CHECK(dispatcher.get_filter_count() == 0);

	// Add three event filters.

	// The first filter modifies the input arguments to other values, then the subsequence filters
	// and listeners will see the modified values.
	dispatcher.append_filter([&](const int e, int & i, std::string & s) -> bool
	{
		ASIO2_CHECK(e == 3 || e == 5);
		if (e == 3)
		{
			v.emplace_back(301);
			ASIO2_CHECK(e == 3 && i == 1 && s == "Hello");
		}
		else
		{
			v.emplace_back(501);
			ASIO2_CHECK(e == 5 && i == 2 && s == "World");
		}
			
		i = 38;
		s = "Hi";
		ASIO2_CHECK(i == 38 && s == "Hi");
		return true;
	});
	ASIO2_CHECK(dispatcher.get_filter_count() == 1);

	std::vector<asio2::event_dispatcher<int, void(int e, int i, std::string), MyPolicy>::filter_type> filters;
	// The second filter filters out all event of 5. So no listeners on event 5 can be triggered.
	// The third filter is not invoked on event 5 also.
	auto filter = dispatcher.append_filter([&](const int , int & e, std::string & ) -> bool
	{
		// can't run to here
		v.emplace_back(-1);
		if(e == 5)
		{
			return false;
		}
		return true;
	});
	ASIO2_CHECK(dispatcher.get_filter_count() == 2);
	filters.emplace_back(filter);
	dispatcher.remove_filter(filter);
	ASIO2_CHECK(dispatcher.get_filter_count() == 1);
	dispatcher.remove_filter(filter);
	ASIO2_CHECK(dispatcher.get_filter_count() == 1);

	dispatcher.append_filter([&](const int e, int & i, std::string & s) -> bool
	{
		ASIO2_CHECK(e == 3 || e == 5);
		if (e == 3)
		{
			v.emplace_back(302);
			ASIO2_CHECK(e == 3 && i == 38 && s == "Hi");
		}
		else
		{
			v.emplace_back(502);
			ASIO2_CHECK(e == 5 && i == 38 && s == "Hi");
		}
			
		if(e == 5)
		{
			return false;
		}
		return true;
	});

	// The third filter just prints the input arguments.
	dispatcher.append_filter([&](const int e, int & i, std::string & s) -> bool
	{
		ASIO2_CHECK(e == 3);
		v.emplace_back(303);
		ASIO2_CHECK(e == 3 && i == 38 && s == "Hi");
		
		return true;
	});
	ASIO2_CHECK(dispatcher.get_filter_count() == 3);

	// Dispatch the events, the first argument is always the event type.
	dispatcher.dispatch(3, 1, "Hello");
	dispatcher.dispatch(5, 2, "World");

	ASIO2_CHECK(f3 == 1);
	ASIO2_CHECK(f5 == -1);
	ASIO2_CHECK(v.size() == 6);
	ASIO2_CHECK(v[0] == 301);
	ASIO2_CHECK(v[1] == 302);
	ASIO2_CHECK(v[2] == 303);
	ASIO2_CHECK(v[3] == 304);
	ASIO2_CHECK(v[4] == 501);
	ASIO2_CHECK(v[5] == 502);

	dispatcher.clear_all_filters();
	ASIO2_CHECK(dispatcher.get_filter_count() == 0);
}

void event_dispatcher_test_name()
{
	std::cout << std::endl << "event_dispatcher tutorial 6, listener name" << std::endl;

	asio2::event_dispatcher<int, void ()> dispatcher;

	std::vector<asio2::event_dispatcher<int, void()>::listener_type> listeners;
	ASIO2_CHECK(dispatcher.get_listener_count() == 0);

	int f31 = -1;
	auto listener = dispatcher.append_listener("e31", 3, [&]()
	{
		f31 = 1;
	});

	ASIO2_CHECK(dispatcher.get_listener_count("e31") == 1);
	ASIO2_CHECK(dispatcher.get_listener_count("e31", 3) == 1);
	ASIO2_CHECK(dispatcher.get_listener_count() == 1);
	listeners.emplace_back(listener);
	ASIO2_CHECK(dispatcher.has_any_listener(3));
	dispatcher.remove_listener(listeners.front());
	ASIO2_CHECK(dispatcher.get_listener_count("e31") == 0);
	ASIO2_CHECK(dispatcher.get_listener_count("e31", 3) == 0);
	ASIO2_CHECK(dispatcher.get_listener_count() == 0);
	ASIO2_CHECK(!dispatcher.has_any_listener(3));

	// listener has been removed, so the listener will be append at the tail
	listener = dispatcher.insert_listener("e32", 3, []() {}, listener);
	ASIO2_CHECK(listener);
	ASIO2_CHECK(dispatcher.get_listener_count(3) == 1);
	ASIO2_CHECK(dispatcher.get_listener_count("e31") == 0);
	ASIO2_CHECK(dispatcher.get_listener_count("e31", 3) == 0);
	ASIO2_CHECK(dispatcher.get_listener_count("e32") == 1);
	ASIO2_CHECK(dispatcher.get_listener_count("e32", 3) == 1);
	
	dispatcher.remove_listener(listener);
	ASIO2_CHECK(!listener);
	ASIO2_CHECK(dispatcher.get_listener_count(3) == 0);
	ASIO2_CHECK(dispatcher.get_listener_count("e31") == 0);
	ASIO2_CHECK(dispatcher.get_listener_count("e31", 3) == 0);
	ASIO2_CHECK(dispatcher.get_listener_count("e32") == 0);
	ASIO2_CHECK(dispatcher.get_listener_count("e32", 3) == 0);

	int f32 = -1;
	listener = dispatcher.append_listener("e32", 3, [&]()
	{
		f32 = 1;
	});
	ASIO2_CHECK(dispatcher.has_any_listener(3));
	ASIO2_CHECK(dispatcher.get_listener_count() == 1);
	ASIO2_CHECK(dispatcher.get_listener_count(3) == 1);
	ASIO2_CHECK(dispatcher.get_listener_count("e32") == 1);
	ASIO2_CHECK(dispatcher.get_listener_count("e32", 3) == 1);

	dispatcher.append_listener("e32", 5, [&]()
	{
	});
	ASIO2_CHECK(dispatcher.get_listener_count("e32") == 2);
	ASIO2_CHECK(dispatcher.get_listener_count("e32", 3) == 1);
	ASIO2_CHECK(dispatcher.get_listener_count("e32", 5) == 1);

	dispatcher.remove_listener("e32", 5);
	ASIO2_CHECK(dispatcher.get_listener_count("e32") == 1);
	ASIO2_CHECK(dispatcher.get_listener_count("e32", 3) == 1);
	ASIO2_CHECK(dispatcher.get_listener_count("e32", 5) == 0);

	int f51 = -1;
	dispatcher.append_listener("e51", 5, [&]()
	{
		f51 = 1;
	});
	ASIO2_CHECK(dispatcher.get_listener_count() == 2);
	ASIO2_CHECK(dispatcher.get_listener_count(5) == 1);

	int f52 = -1;
	dispatcher.append_listener("e52", 5, [&]()
	{
		f52 = 1;
	});
	ASIO2_CHECK(dispatcher.get_listener_count() == 3);
	ASIO2_CHECK(dispatcher.get_listener_count(5) == 2);

	auto liss1 = dispatcher.append_listener("e67", { 6,7 }, [&]()
	{

	});
	ASIO2_CHECK(dispatcher.get_listener_count(6) == 1);
	ASIO2_CHECK(dispatcher.get_listener_count(7) == 1);

	listeners.insert(listeners.end(), liss1.begin(), liss1.end());
	ASIO2_CHECK(dispatcher.get_listener_count() == 5);

	auto liss2 = dispatcher.prepend_listener("e89", { 8,9 }, [&]()
	{

	});
	ASIO2_CHECK(dispatcher.get_listener_count(8) == 1);
	ASIO2_CHECK(dispatcher.get_listener_count(9) == 1);

	listeners.insert(listeners.end(), liss2.begin(), liss2.end());
	ASIO2_CHECK(dispatcher.get_listener_count() == 7);

	dispatcher.insert_listener(3, []() {}, listener);
	ASIO2_CHECK(dispatcher.get_listener_count(3) == 2);
	ASIO2_CHECK(dispatcher.get_listener_count() == 8);

	auto* listener_list = dispatcher.find_listeners(5);
	ASIO2_CHECK(listener_list != nullptr);
	ASIO2_CHECK(listener_list->size() == 2);
	for (auto it = listener_list->begin(); it != listener_list->end(); ++it)
	{
		ASIO2_CHECK(it->get_event_type() == 5);
	}
	for (auto it = listener_list->cbegin(); it != listener_list->cend(); ++it)
	{
		ASIO2_CHECK(it->get_event_type() == 5);
	}

	ASIO2_CHECK(dispatcher.get_listener_count("e51") == 1);
	ASIO2_CHECK(dispatcher.get_listener_count("e51", 3) == 0);
	ASIO2_CHECK(dispatcher.get_listener_count("e51", 5) == 1);

	ASIO2_CHECK(dispatcher.get_listener_count("e52") == 1);
	ASIO2_CHECK(dispatcher.get_listener_count("e52", 3) == 0);
	ASIO2_CHECK(dispatcher.get_listener_count("e52", 5) == 1);

	ASIO2_CHECK(dispatcher.get_listener_count("e67") == 2);
	ASIO2_CHECK(dispatcher.get_listener_count("e67", 3) == 0);
	ASIO2_CHECK(dispatcher.get_listener_count("e67", 5) == 0);
	ASIO2_CHECK(dispatcher.get_listener_count("e67", 6) == 1);
	ASIO2_CHECK(dispatcher.get_listener_count("e67", 7) == 1);

	ASIO2_CHECK(dispatcher.get_listener_count("e89") == 2);
	ASIO2_CHECK(dispatcher.get_listener_count("e89", 3) == 0);
	ASIO2_CHECK(dispatcher.get_listener_count("e89", 5) == 0);
	ASIO2_CHECK(dispatcher.get_listener_count("e89", 6) == 0);
	ASIO2_CHECK(dispatcher.get_listener_count("e89", 7) == 0);
	ASIO2_CHECK(dispatcher.get_listener_count("e89", 8) == 1);
	ASIO2_CHECK(dispatcher.get_listener_count("e89", 9) == 1);

	ASIO2_CHECK(dispatcher.get_listener_count() == 8);

	ASIO2_CHECK(dispatcher.remove_listener("e89"));
	ASIO2_CHECK(dispatcher.get_listener_count("e89") == 0);
	ASIO2_CHECK(dispatcher.get_listener_count("e89", 3) == 0);
	ASIO2_CHECK(dispatcher.get_listener_count("e89", 5) == 0);

	// Dispatch the events, the first argument is always the event type.
	dispatcher.dispatch(3);
	dispatcher.dispatch(5);

	ASIO2_CHECK(f31 == -1);
	ASIO2_CHECK(f32 == 1);
	ASIO2_CHECK(f51 == 1);
	ASIO2_CHECK(f52 == 1);

	dispatcher.clear_all_listeners();
	ASIO2_CHECK(dispatcher.get_listener_count() == 0);
}

class ipacket
{
public:
	ipacket()
	{
	}
	virtual ~ipacket()
	{
	}

	virtual const char * get_packet_type() = 0;
};

class mem_packet : public ipacket
{
public:
	mem_packet(std::string m) : message(std::move(m))
	{
	}
	virtual ~mem_packet()
	{
	}

	virtual const char * get_packet_type() { return "mp"; }

	std::string message;
};

void event_dispatcher_test_move()
{
	std::cout << std::endl << "event_dispatcher tutorial 6, move" << std::endl;

	struct my_policy
	{
		static std::string_view get_event(const std::shared_ptr<ipacket> & e)
		{
			return e->get_packet_type();
		}
	};

	asio2::event_dispatcher<std::string_view, void(const std::shared_ptr<ipacket>&), my_policy> dispatcher;

	int f = -1;
	dispatcher.append_listener("mp", [&](const std::shared_ptr<ipacket>& e)
	{
		f = 1;
		ASIO2_CHECK(std::string_view{ "mp" } == e->get_packet_type());
		mem_packet* p = dynamic_cast<mem_packet*>(e.get());
		ASIO2_CHECK(p->message == "Hello world");
	});

	std::shared_ptr<mem_packet> e = std::make_shared<mem_packet>("Hello world");

	dispatcher.dispatch(std::move(e));

	ASIO2_CHECK(f == 1);
}

#include <asio2/util/uuid.hpp>
void event_dispatcher_test_bench()
{
	struct policy
	{
		//using thread_t = asio2::dispatcheres::single_thread;
	};

	asio2::event_dispatcher<std::string_view, void(std::string_view), policy> dispatcher;

	std::vector<std::string> vs;

	std::size_t sum = 0;
	int count = 100;
	int loop = 10000000;

#if defined(DEBUG) || defined(_DEBUG)
	loop /= 100;
#endif

	vs.reserve(count);

	asio2::uuid u1;

	for (int i = 0; i < count; i++)
	{
		std::string s = u1.short_uuid(8);

		vs.emplace_back(s);
	}

	for (auto& s : vs)
	{
		dispatcher.append_listener(s, [&sum](std::string_view sr) { sum += std::size_t(sr.data()); });
	}

	{
		sum = 0;
		auto t1 = std::chrono::high_resolution_clock::now();

		for (int i = 0; i < loop; i++)
		{
			std::string& s = vs[i % vs.size()];

			dispatcher.dispatch(s, s);
		}
		auto t2 = std::chrono::high_resolution_clock::now();
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
		std::cout << sum << " dispatcher : " << ms << std::endl;
	}

	{
		sum = 0;
		auto t1 = std::chrono::high_resolution_clock::now();

		for (int i = 0; i < loop; i++)
		{
			std::string& s = vs[i % vs.size()];

			for (auto& sr : vs)
			{
				if (std::strcmp(s.data(), sr.data()) == 0)
				{
					sum += std::size_t(sr.data());
					break;
				}
			}
		}
		auto t2 = std::chrono::high_resolution_clock::now();
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
		std::cout << sum << " strcmp     : " << ms << std::endl;
	}
}

ASIO2_TEST_SUITE
(
	"event_dispatcher",
	ASIO2_TEST_CASE(event_dispatcher_test_basic)
	ASIO2_TEST_CASE(event_dispatcher_test_listener_with_parameters)
	ASIO2_TEST_CASE(event_dispatcher_test_customized_event_struct)
	ASIO2_TEST_CASE(event_dispatcher_test_event_canceling)
	ASIO2_TEST_CASE(event_dispatcher_test_event_filter)
	ASIO2_TEST_CASE(event_dispatcher_test_name)
	ASIO2_TEST_CASE(event_dispatcher_test_move)
	ASIO2_TEST_CASE(event_dispatcher_test_bench)
)
