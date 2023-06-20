#include "unit_test.hpp"
#include <iostream>
#include <asio2/external/pfr.hpp>

class actor
{
public:
	actor(int index) : m_index(index)
	{
	}
	virtual ~actor()
	{
	}

	int m_index = 0;
};

class dog
	: public actor
	, public pfr::base_dynamic_creator<actor, dog, int, std::string, const char*>
{
public:
	dog(int index, std::string name, const char* type)
		: actor(index), m_name(std::move(name)), m_type(type)
	{
	}
	virtual ~dog()
	{
	}

	std::string  m_name;
	const char * m_type;
};

struct userinfo
{
	F_BEGIN(userinfo);

	F_FIELD(int, age);
	F_FIELD(std::string, name);

	F_END();
};

class imodule
{
public:
	imodule() {}
	virtual ~imodule() {}
	virtual int status() = 0;
};

class config_module : public imodule, public pfr::base_dynamic_creator<imodule, config_module>
{
public:
	config_module() : imodule() {}
	virtual ~config_module() {}
	virtual int status() override
	{
		return 12;
	}
};

class config_module_ex : public config_module, public pfr::base_dynamic_creator<imodule, config_module_ex>
{
public:
	config_module_ex() : config_module() {}
	virtual ~config_module_ex() {}
	virtual int status() override
	{
		return 13;
	}
};

void reflection_test()
{
	int N = 0;
	pfr::class_factory<imodule>& factory = pfr::class_factory<imodule>::instance();
	factory.for_each([&N](const auto& name, const auto& func)
	{
		N++;
		ASIO2_CHECK(name == "config_module_ex" || name == "config_module");
		std::shared_ptr<imodule> p(func());
		ASIO2_CHECK(12 == p->status() || 13 == p->status());
	});
	ASIO2_CHECK(N == 2);
	ASIO2_CHECK(factory.size() == 2);
	ASIO2_CHECK(factory.empty() == false);
	ASIO2_CHECK(factory.find("config_module_ex") != nullptr);
	ASIO2_CHECK(factory.find("config_module_ex_2") == nullptr);

	int i1 = 1;
	actor* p1 = pfr::create_helper<actor>::create(
		"dog", i1, std::string("dog1"), (const char*)"d1");
	std::string dog2 = "dog2";
	std::shared_ptr<actor> p2 = pfr::create_helper<std::shared_ptr<actor>>::create(
		"dog", 2, dog2, (const char*)"d2");
	const char* type3 = "d3";
	std::unique_ptr<actor> p3 = pfr::create_helper<std::unique_ptr<actor>>::create(
		"dog", 3, std::string("dog3"), type3);
	std::string class_name = "dog";
	actor* p4 = pfr::create_helper<actor*>::create(
		class_name, 4, std::string("dog4"), (const char*)"d4");

	ASIO2_CHECK(p1->m_index == 1);
	ASIO2_CHECK(p2->m_index == 2);
	ASIO2_CHECK(p3->m_index == 3);
	ASIO2_CHECK(p4->m_index == 4);

	dog* d1 = (dog*)p1;
	dog* d2 = (dog*)p2.get();
	dog* d3 = (dog*)p3.get();
	dog* d4 = (dog*)p4;

	ASIO2_CHECK(d1->m_name == "dog1" && std::string_view{ "d1" } == d1->m_type);
	ASIO2_CHECK(d2->m_name == "dog2" && std::string_view{ "d2" } == d2->m_type);
	ASIO2_CHECK(d3->m_name == "dog3" && std::string_view{ "d3" } == d3->m_type);
	ASIO2_CHECK(d4->m_name == "dog4" && std::string_view{ "d4" } == d4->m_type);

	userinfo u;

	u.age = 101;
	u.name = "hanmeimei";

	ASIO2_CHECK(2 == u.get_field_count());

	ASIO2_CHECK(u.get_class_name() == "userinfo");

	int userage = -1;
	std::string username;
	int c = 0;
	u.for_each_field([&](const char* name, auto& value) mutable
	{
		std::ignore = true;
		if constexpr (std::is_integral_v<std::remove_reference_t<decltype(value)>>)
		{
			ASIO2_CHECK(std::string_view{ "age" } == name);
			userage = value;
			ASIO2_CHECK(value == 101);
			ASIO2_CHECK(c == 0);
		}
		else
		{
			ASIO2_CHECK(std::string_view{ "name" } == name);
			username = value;
			ASIO2_CHECK(value == "hanmeimei");
			ASIO2_CHECK(c == 1);
		}
		c++;
	});

    ASIO2_CHECK(c == 2);

	ASIO2_CHECK(userage  == u.age);
	ASIO2_CHECK(username == u.name);

	c = 0;
	userinfo::for_each_field_name([&c](const char* name) mutable
	{
		if (c == 0)
		{
			ASIO2_CHECK(std::string_view{ "age" } == name);
		}
		else
		{
			ASIO2_CHECK(std::string_view{ "name" } == name);
		}
		c++;
	});

    ASIO2_CHECK(c == 2);

	delete p1;
	delete p4;
}


ASIO2_TEST_SUITE
(
	"reflection",
	ASIO2_TEST_CASE(reflection_test)
)
