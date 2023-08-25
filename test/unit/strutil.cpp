#include "unit_test.hpp"
#include <asio2/util/string.hpp>
#include <asio2/base/detail/util.hpp>
#include <asio2/external/fmt.hpp>

struct userinfo
{
	std::uint8_t age{ 0 };
	std::string name = "abc";
};

inline std::ostream& operator<<(std::ostream& os, userinfo v)
{
	std::string s;
	s += std::to_string(v.age);
	s += v.name;
	return os << std::move(s);
}

inline std::istream& operator>>(std::istream& is, userinfo& v)
{
	is >> v.age;
	v.age -= '0';
	is >> v.name;
	return is;
}

std::string make_string()
{
	std::string s(" a B c ");
	return s;
}

std::string_view make_string_view()
{
	std::string_view s(" a B c ");
	return s;
}

void strutil_test()
{
	{
		ASIO2_CHECK(fmt::kvformat("{}", 1) == "1");
		ASIO2_CHECK(fmt::kvformat("{}", 1, "{}", 2) == "12");
		ASIO2_CHECK(fmt::kvformat("{:02}", 1, "{:02}", 2, "{:.3f}", 3.56789) == "01023.568");
		ASIO2_CHECK(fmt::kvformat(L"{}", 1) == L"1");
		ASIO2_CHECK(fmt::kvformat(L"{}", 1, L"{}", 2) == L"12");
		ASIO2_CHECK(fmt::kvformat(L"{:02}", 1, L"{:02}", 2, L"{:.3f}", 3.56789) == L"01023.568");
	}

	{
		std::string str = "Text\n with\tsome \t  whitespaces\n\n";
		std::string_view sv = "Text\n with\tsome \t  whitespaces\n\n";

		ASIO2_CHECK(asio2::iequals("text\n With\tSome \t  whitespaces\n\n", "Text\n with\tsome \t  whitespaces\n\n"));
		ASIO2_CHECK(asio2::iequals(str, "Text\n with\tsome \t  whitespaces\n\n"));
		ASIO2_CHECK(asio2::iequals(sv, "Text\n with\tsome \t  whitespaces\n\n"));
		ASIO2_CHECK(asio2::iequals("Text\n With\tSome \t  whitespaces\n\n", str));
		ASIO2_CHECK(asio2::iequals("Text\n With\tSome \t  whitespaces\n\n", sv));
		ASIO2_CHECK(asio2::iequals(str, sv));
		ASIO2_CHECK(asio2::iequals(sv, str));
		ASIO2_CHECK(asio2::iequals(str, str));
		ASIO2_CHECK(asio2::iequals(sv, sv));

		str = "a";
		sv = "a";

		ASIO2_CHECK(asio2::iequals("a", 'a'));
		ASIO2_CHECK(asio2::iequals('a', "A"));
		ASIO2_CHECK(asio2::iequals(str, 'a'));
		ASIO2_CHECK(asio2::iequals(sv, 'a'));
		ASIO2_CHECK(asio2::iequals('a', str));
		ASIO2_CHECK(asio2::iequals('a', sv));
		ASIO2_CHECK(asio2::iequals(str, sv));
		ASIO2_CHECK(asio2::iequals(sv, str));
		ASIO2_CHECK(asio2::iequals(str, str));
		ASIO2_CHECK(asio2::iequals(sv, sv));
	}

	{
		std::wstring str = L"Text\n with\tsome \t  whitespaces\n\n";
		std::wstring_view sv = L"Text\n with\tsome \t  whitespaces\n\n";

		ASIO2_CHECK(asio2::iequals(L"text\n With\tSome \t  whitespaces\n\n", L"Text\n with\tsome \t  whitespaces\n\n"));
		ASIO2_CHECK(asio2::iequals(str, L"Text\n with\tsome \t  whitespaces\n\n"));
		ASIO2_CHECK(asio2::iequals(sv, L"Text\n with\tsome \t  whitespaces\n\n"));
		ASIO2_CHECK(asio2::iequals(L"Text\n With\tSome \t  whitespaces\n\n", str));
		ASIO2_CHECK(asio2::iequals(L"Text\n With\tSome \t  whitespaces\n\n", sv));
		ASIO2_CHECK(asio2::iequals(str, sv));
		ASIO2_CHECK(asio2::iequals(sv, str));
		ASIO2_CHECK(asio2::iequals(str, str));
		ASIO2_CHECK(asio2::iequals(sv, sv));

		str = L"a";
		sv = L"a";

		ASIO2_CHECK(asio2::iequals(L"a", L'a'));
		ASIO2_CHECK(asio2::iequals(L'a', L"A"));
		ASIO2_CHECK(asio2::iequals(str, L'a'));
		ASIO2_CHECK(asio2::iequals(sv, L'a'));
		ASIO2_CHECK(asio2::iequals(L'a', str));
		ASIO2_CHECK(asio2::iequals(L'a', sv));
		ASIO2_CHECK(asio2::iequals(str, sv));
		ASIO2_CHECK(asio2::iequals(sv, str));
		ASIO2_CHECK(asio2::iequals(str, str));
		ASIO2_CHECK(asio2::iequals(sv, sv));
	}

	{
		std::u16string str = u"z\u00df\u6c34\U0001f34c";
		std::u16string_view sv = u"z\u00df\u6c34\U0001f34c";

		ASIO2_CHECK(asio2::iequals(u"z\u00df\u6c34\U0001f34c", u"Z\u00df\u6c34\U0001f34c"));
		ASIO2_CHECK(asio2::iequals(str, u"z\u00df\u6c34\U0001f34c"));
		ASIO2_CHECK(asio2::iequals(sv, u"z\u00df\u6c34\U0001f34c"));
		ASIO2_CHECK(asio2::iequals(u"z\u00df\u6c34\U0001f34c", str));
		ASIO2_CHECK(asio2::iequals(u"z\u00df\u6c34\U0001f34c", sv));
		ASIO2_CHECK(asio2::iequals(str, sv));
		ASIO2_CHECK(asio2::iequals(sv, str));
		ASIO2_CHECK(asio2::iequals(str, str));
		ASIO2_CHECK(asio2::iequals(sv, sv));

		str = u"a";
		sv = u"a";

		ASIO2_CHECK(asio2::iequals(u"a", u'a'));
		ASIO2_CHECK(asio2::iequals(u'a', u"A"));
		ASIO2_CHECK(asio2::iequals(str, u'a'));
		ASIO2_CHECK(asio2::iequals(sv, u'a'));
		ASIO2_CHECK(asio2::iequals(u'a', str));
		ASIO2_CHECK(asio2::iequals(u'a', sv));
		ASIO2_CHECK(asio2::iequals(str, sv));
		ASIO2_CHECK(asio2::iequals(sv, str));
		ASIO2_CHECK(asio2::iequals(str, str));
		ASIO2_CHECK(asio2::iequals(sv, sv));
	}

	{
		std::string str = "Text\n with\tsome \t  whitespaces\n\n";
		std::string_view sv = "Text\n with\tsome \t  whitespaces\n\n";

		ASIO2_CHECK(!asio2::iequals("text\n With\tSome \t x whitespaces\n\n", "Text\n with\tsome \t  whitespaces\n\n"));
		ASIO2_CHECK(!asio2::iequals("text\n With\tSome \t  whitespaces\n\n", "Text\n With\tSome \t x whitespaces\n\n"));
		ASIO2_CHECK(!asio2::iequals(str, "Text\n with\tsome \t x whitespaces\n\n"));
		ASIO2_CHECK(!asio2::iequals(sv, "Text\n with\tsome \t x whitespaces\n\n"));
		ASIO2_CHECK(!asio2::iequals(" Text\n With\tSome \t  whitespaces\n\n", str));
		ASIO2_CHECK(!asio2::iequals(" Text\n With\tSome \t  whitespaces\n\n", sv));

		str = "a";
		sv = "a";

		ASIO2_CHECK(!asio2::iequals("a", 'B'));
		ASIO2_CHECK(!asio2::iequals('B', "A"));
		ASIO2_CHECK(!asio2::iequals(str, 'B'));
		ASIO2_CHECK(!asio2::iequals(sv, 'b'));
		ASIO2_CHECK(!asio2::iequals('b', str));
		ASIO2_CHECK(!asio2::iequals('B', sv));
	}

	{
		std::wstring str = L"Text\n with\tsome \t  whitespaces\n\n";
		std::wstring_view sv = L"Text\n with\tsome \t  whitespaces\n\n";

		ASIO2_CHECK(!asio2::iequals(L"text\n With\tSome \t x whitespaces\n\n", L"Text\n with\tsome \t  whitespaces\n\n"));
		ASIO2_CHECK(!asio2::iequals(L"text\n With\tSome \t  whitespaces\n\n", L"Text\n With\tSome \t x whitespaces\n\n"));
		ASIO2_CHECK(!asio2::iequals(str, L"Text\n with\tsome \t x whitespaces\n\n"));
		ASIO2_CHECK(!asio2::iequals(sv, L"Text\n with\tsome \t x whitespaces\n\n"));
		ASIO2_CHECK(!asio2::iequals(L" Text\n With\tSome \t  whitespaces\n\n", str));
		ASIO2_CHECK(!asio2::iequals(L" Text\n With\tSome \t  whitespaces\n\n", sv));

		str = L"a";
		sv = L"a";

		ASIO2_CHECK(!asio2::iequals(L"a", L'B'));
		ASIO2_CHECK(!asio2::iequals(L'b', L"A"));
		ASIO2_CHECK(!asio2::iequals(str, L'b'));
		ASIO2_CHECK(!asio2::iequals(sv, L'B'));
		ASIO2_CHECK(!asio2::iequals(L'B', str));
		ASIO2_CHECK(!asio2::iequals(L'b', sv));
	}

	{
		std::u16string str = u"z\u00df\u6c34\U0001f34c";
		std::u16string_view sv = u"z\u00df\u6c34\U0001f34c";

		ASIO2_CHECK(!asio2::iequals(u"z\u00df\u6c34\U0001f34c", u" Z\u00df\u6c34\U0001f34c"));
		ASIO2_CHECK(!asio2::iequals(u"z\u00df\u6c34\U0001f34c", u" Z\u00df\u6c34\U0001f34c"));
		ASIO2_CHECK(!asio2::iequals(str, u"z x\u00df\u6c34\U0001f34c"));
		ASIO2_CHECK(!asio2::iequals(sv, u"z x\u00df\u6c34\U0001f34c"));
		ASIO2_CHECK(!asio2::iequals(u"z x\u00df\u6c34\U0001f34c", str));
		ASIO2_CHECK(!asio2::iequals(u"z x\u00df\u6c34\U0001f34c", sv));

		str = u"a";
		sv = u"a";

		ASIO2_CHECK(!asio2::iequals(u"a", u'B'));
		ASIO2_CHECK(!asio2::iequals(u'B', u"A"));
		ASIO2_CHECK(!asio2::iequals(str, u'b'));
		ASIO2_CHECK(!asio2::iequals(sv, u'B'));
		ASIO2_CHECK(!asio2::iequals(u'b', str));
		ASIO2_CHECK(!asio2::iequals(u'B', sv));
	}

	{
		std::string str = "Text\n with\tsome \t  whitespaces\n\n";
		std::string_view sv = "Text\n with\tsome \t  whitespaces\n\n";

		ASIO2_CHECK(asio2::compare_ignore_case("text\n With\tSome \t  whitespaces\n\n", "Text\n with\tsome \t  whitespaces\n\n"));
		ASIO2_CHECK(asio2::compare_ignore_case(str, "Text\n with\tsome \t  whitespaces\n\n"));
		ASIO2_CHECK(asio2::compare_ignore_case(sv, "Text\n with\tsome \t  whitespaces\n\n"));
		ASIO2_CHECK(asio2::compare_ignore_case("Text\n With\tSome \t  whitespaces\n\n", str));
		ASIO2_CHECK(asio2::compare_ignore_case("Text\n With\tSome \t  whitespaces\n\n", sv));
		ASIO2_CHECK(asio2::compare_ignore_case(str, sv));
		ASIO2_CHECK(asio2::compare_ignore_case(sv, str));
		ASIO2_CHECK(asio2::compare_ignore_case(str, str));
		ASIO2_CHECK(asio2::compare_ignore_case(sv, sv));

		str = "a";
		sv = "a";

		ASIO2_CHECK(asio2::compare_ignore_case("a", 'a'));
		ASIO2_CHECK(asio2::compare_ignore_case('a', "A"));
		ASIO2_CHECK(asio2::compare_ignore_case(str, 'a'));
		ASIO2_CHECK(asio2::compare_ignore_case(sv, 'a'));
		ASIO2_CHECK(asio2::compare_ignore_case('a', str));
		ASIO2_CHECK(asio2::compare_ignore_case('a', sv));
		ASIO2_CHECK(asio2::compare_ignore_case(str, sv));
		ASIO2_CHECK(asio2::compare_ignore_case(sv, str));
		ASIO2_CHECK(asio2::compare_ignore_case(str, str));
		ASIO2_CHECK(asio2::compare_ignore_case(sv, sv));
	}

	{
		std::wstring str = L"Text\n with\tsome \t  whitespaces\n\n";
		std::wstring_view sv = L"Text\n with\tsome \t  whitespaces\n\n";

		ASIO2_CHECK(asio2::compare_ignore_case(L"text\n With\tSome \t  whitespaces\n\n", L"Text\n with\tsome \t  whitespaces\n\n"));
		ASIO2_CHECK(asio2::compare_ignore_case(str, L"Text\n with\tsome \t  whitespaces\n\n"));
		ASIO2_CHECK(asio2::compare_ignore_case(sv, L"Text\n with\tsome \t  whitespaces\n\n"));
		ASIO2_CHECK(asio2::compare_ignore_case(L"Text\n With\tSome \t  whitespaces\n\n", str));
		ASIO2_CHECK(asio2::compare_ignore_case(L"Text\n With\tSome \t  whitespaces\n\n", sv));
		ASIO2_CHECK(asio2::compare_ignore_case(str, sv));
		ASIO2_CHECK(asio2::compare_ignore_case(sv, str));
		ASIO2_CHECK(asio2::compare_ignore_case(str, str));
		ASIO2_CHECK(asio2::compare_ignore_case(sv, sv));

		str = L"a";
		sv = L"a";

		ASIO2_CHECK(asio2::compare_ignore_case(L"a", L'a'));
		ASIO2_CHECK(asio2::compare_ignore_case(L'a', L"A"));
		ASIO2_CHECK(asio2::compare_ignore_case(str, L'a'));
		ASIO2_CHECK(asio2::compare_ignore_case(sv, L'a'));
		ASIO2_CHECK(asio2::compare_ignore_case(L'a', str));
		ASIO2_CHECK(asio2::compare_ignore_case(L'a', sv));
		ASIO2_CHECK(asio2::compare_ignore_case(str, sv));
		ASIO2_CHECK(asio2::compare_ignore_case(sv, str));
		ASIO2_CHECK(asio2::compare_ignore_case(str, str));
		ASIO2_CHECK(asio2::compare_ignore_case(sv, sv));
	}

	{
		std::u16string str = u"z\u00df\u6c34\U0001f34c";
		std::u16string_view sv = u"z\u00df\u6c34\U0001f34c";

		ASIO2_CHECK(asio2::compare_ignore_case(u"z\u00df\u6c34\U0001f34c", u"Z\u00df\u6c34\U0001f34c"));
		ASIO2_CHECK(asio2::compare_ignore_case(str, u"z\u00df\u6c34\U0001f34c"));
		ASIO2_CHECK(asio2::compare_ignore_case(sv, u"z\u00df\u6c34\U0001f34c"));
		ASIO2_CHECK(asio2::compare_ignore_case(u"z\u00df\u6c34\U0001f34c", str));
		ASIO2_CHECK(asio2::compare_ignore_case(u"z\u00df\u6c34\U0001f34c", sv));
		ASIO2_CHECK(asio2::compare_ignore_case(str, sv));
		ASIO2_CHECK(asio2::compare_ignore_case(sv, str));
		ASIO2_CHECK(asio2::compare_ignore_case(str, str));
		ASIO2_CHECK(asio2::compare_ignore_case(sv, sv));

		str = u"a";
		sv = u"a";

		ASIO2_CHECK(asio2::compare_ignore_case(u"a", u'a'));
		ASIO2_CHECK(asio2::compare_ignore_case(u'a', u"A"));
		ASIO2_CHECK(asio2::compare_ignore_case(str, u'a'));
		ASIO2_CHECK(asio2::compare_ignore_case(sv, u'a'));
		ASIO2_CHECK(asio2::compare_ignore_case(u'a', str));
		ASIO2_CHECK(asio2::compare_ignore_case(u'a', sv));
		ASIO2_CHECK(asio2::compare_ignore_case(str, sv));
		ASIO2_CHECK(asio2::compare_ignore_case(sv, str));
		ASIO2_CHECK(asio2::compare_ignore_case(str, str));
		ASIO2_CHECK(asio2::compare_ignore_case(sv, sv));
	}

	{
		std::string str = "Text\n with\tsome \t  whitespaces\n\n";
		std::string_view sv = "Text\n with\tsome \t  whitespaces\n\n";

		ASIO2_CHECK(!asio2::compare_ignore_case("text\n With\tSome \t x whitespaces\n\n", "Text\n with\tsome \t  whitespaces\n\n"));
		ASIO2_CHECK(!asio2::compare_ignore_case("text\n With\tSome \t  whitespaces\n\n", "Text\n With\tSome \t x whitespaces\n\n"));
		ASIO2_CHECK(!asio2::compare_ignore_case(str, "Text\n with\tsome \t x whitespaces\n\n"));
		ASIO2_CHECK(!asio2::compare_ignore_case(sv, "Text\n with\tsome \t x whitespaces\n\n"));
		ASIO2_CHECK(!asio2::compare_ignore_case(" Text\n With\tSome \t  whitespaces\n\n", str));
		ASIO2_CHECK(!asio2::compare_ignore_case(" Text\n With\tSome \t  whitespaces\n\n", sv));

		str = "a";
		sv = "a";

		ASIO2_CHECK(!asio2::compare_ignore_case("a", 'B'));
		ASIO2_CHECK(!asio2::compare_ignore_case('B', "A"));
		ASIO2_CHECK(!asio2::compare_ignore_case(str, 'B'));
		ASIO2_CHECK(!asio2::compare_ignore_case(sv, 'b'));
		ASIO2_CHECK(!asio2::compare_ignore_case('b', str));
		ASIO2_CHECK(!asio2::compare_ignore_case('B', sv));
	}

	{
		std::wstring str = L"Text\n with\tsome \t  whitespaces\n\n";
		std::wstring_view sv = L"Text\n with\tsome \t  whitespaces\n\n";

		ASIO2_CHECK(!asio2::compare_ignore_case(L"text\n With\tSome \t x whitespaces\n\n", L"Text\n with\tsome \t  whitespaces\n\n"));
		ASIO2_CHECK(!asio2::compare_ignore_case(L"text\n With\tSome \t  whitespaces\n\n", L"Text\n With\tSome \t x whitespaces\n\n"));
		ASIO2_CHECK(!asio2::compare_ignore_case(str, L"Text\n with\tsome \t x whitespaces\n\n"));
		ASIO2_CHECK(!asio2::compare_ignore_case(sv, L"Text\n with\tsome \t x whitespaces\n\n"));
		ASIO2_CHECK(!asio2::compare_ignore_case(L" Text\n With\tSome \t  whitespaces\n\n", str));
		ASIO2_CHECK(!asio2::compare_ignore_case(L" Text\n With\tSome \t  whitespaces\n\n", sv));

		str = L"a";
		sv = L"a";

		ASIO2_CHECK(!asio2::compare_ignore_case(L"a", L'B'));
		ASIO2_CHECK(!asio2::compare_ignore_case(L'b', L"A"));
		ASIO2_CHECK(!asio2::compare_ignore_case(str, L'b'));
		ASIO2_CHECK(!asio2::compare_ignore_case(sv, L'B'));
		ASIO2_CHECK(!asio2::compare_ignore_case(L'B', str));
		ASIO2_CHECK(!asio2::compare_ignore_case(L'b', sv));
	}

	{
		std::u16string str = u"z\u00df\u6c34\U0001f34c";
		std::u16string_view sv = u"z\u00df\u6c34\U0001f34c";

		ASIO2_CHECK(!asio2::compare_ignore_case(u"z\u00df\u6c34\U0001f34c", u" Z\u00df\u6c34\U0001f34c"));
		ASIO2_CHECK(!asio2::compare_ignore_case(u"z\u00df\u6c34\U0001f34c", u" Z\u00df\u6c34\U0001f34c"));
		ASIO2_CHECK(!asio2::compare_ignore_case(str, u"z x\u00df\u6c34\U0001f34c"));
		ASIO2_CHECK(!asio2::compare_ignore_case(sv, u"z x\u00df\u6c34\U0001f34c"));
		ASIO2_CHECK(!asio2::compare_ignore_case(u"z x\u00df\u6c34\U0001f34c", str));
		ASIO2_CHECK(!asio2::compare_ignore_case(u"z x\u00df\u6c34\U0001f34c", sv));

		str = u"a";
		sv = u"a";

		ASIO2_CHECK(!asio2::compare_ignore_case(u"a", u'B'));
		ASIO2_CHECK(!asio2::compare_ignore_case(u'B', u"A"));
		ASIO2_CHECK(!asio2::compare_ignore_case(str, u'b'));
		ASIO2_CHECK(!asio2::compare_ignore_case(sv, u'B'));
		ASIO2_CHECK(!asio2::compare_ignore_case(u'b', str));
		ASIO2_CHECK(!asio2::compare_ignore_case(u'B', sv));
	}

	{
		std::string str = "Text\n with\tsome \t  whitespaces\n\n";
		std::string_view sv = "Text\n with\tsome \t  whitespaces\n\n";

		const char* p = "Text\n with\tsome \t  whitespaces\n\n";

		char buf[] = "Text\n with\tsome \t  whitespaces\n\n";

		ASIO2_CHECK(asio2::format("%d %s", 10, "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ") == "10 0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
		//ASIO2_CHECK(asio2::format(L"%d %s", 10, L"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ") == L"10 0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");

		ASIO2_CHECK(asio2::to_string(sv) == sv);
		ASIO2_CHECK(asio2::to_string(str) == str);
		ASIO2_CHECK(asio2::to_string(10) == "10");
		ASIO2_CHECK(asio2::to_string(999999) == "999999");
		ASIO2_CHECK(asio2::to_string(10.0).find("10") == 0);
		ASIO2_CHECK(asio2::to_string(999999.0).find("999999") == 0);
		ASIO2_CHECK(asio2::to_string(10.0f).find("10") == 0);
		ASIO2_CHECK(asio2::to_string(999999.0f).find("999999") == 0);
		ASIO2_CHECK(asio2::to_string(p) == str);
		ASIO2_CHECK(asio2::to_string("Text\n with\tsome \t  whitespaces\n\n") == str);
		ASIO2_CHECK(asio2::to_string(buf) == str);
		ASIO2_CHECK(asio2::to_string(userinfo{}) == "0abc");

		ASIO2_CHECK(asio2::to_string_view(sv) == sv);
		ASIO2_CHECK(asio2::to_string_view(str) == sv);
		ASIO2_CHECK(asio2::to_string_view('a') == "a");
		ASIO2_CHECK(asio2::to_string_view(p) == str);
		ASIO2_CHECK(asio2::to_string_view("Text\n with\tsome \t  whitespaces\n\n") == str);
		ASIO2_CHECK(asio2::to_string_view(buf) == str);

		ASIO2_CHECK(asio2::to_string_view(str.begin(), str.end()) == str);
		ASIO2_CHECK(asio2::to_string_view(sv.begin(), sv.end()) == str);
		ASIO2_CHECK(asio2::to_string_view(p, p + std::strlen(p)) == str);

		str = "20";
		sv = "30";

		const char* np = "40";
		char nb[] = "50";

		ASIO2_CHECK(asio2::to_numeric<int>("10") == 10);
		ASIO2_CHECK(asio2::to_numeric<int>(10) == 10);
		ASIO2_CHECK(asio2::to_numeric<int>(10.0) == 10);
		ASIO2_CHECK(asio2::to_numeric<int>("0x10") == 0x10);
		ASIO2_CHECK(asio2::to_numeric<int>("0X10") == 0x10);
		ASIO2_CHECK(asio2::to_numeric<int>(str) == 20);
		ASIO2_CHECK(asio2::to_numeric<int>(sv) == 30);
		ASIO2_CHECK(asio2::to_numeric<int>(np) == 40);
		ASIO2_CHECK(asio2::to_numeric<int>(nb) == 50);

		ASIO2_CHECK(asio2::string_to<std::string>("999999") == "999999");
		ASIO2_CHECK(asio2::string_to<int>("999999") == 999999);
		ASIO2_CHECK(asio2::string_to<double>("999999") == 999999.0);
		ASIO2_CHECK(asio2::string_to<float>("999999") == 999999.0f);
		ASIO2_CHECK(asio2::string_to<bool>("0") == false);
		ASIO2_CHECK(asio2::string_to<bool>("1") == true);
		userinfo u = asio2::string_to<userinfo>("0abc");
		ASIO2_CHECK(u.age == 0);
		ASIO2_CHECK(u.name == "abc");
	}

	{
		std::string str = "Text\n with\tsome \t  whitespaces\n\n";
		ASIO2_CHECK(asio2::to_lower(str) == "text\n with\tsome \t  whitespaces\n\n");
	}

	ASIO2_CHECK(asio2::to_lower(make_string()) == " a b c ");

	{
		std::string str = "Text\n with\tsome \t  whitespaces\n\n";
		ASIO2_CHECK(asio2::to_upper(str) == "TEXT\n WITH\tSOME \t  WHITESPACES\n\n");
	}

	ASIO2_CHECK(asio2::to_upper(make_string()) == " A B C ");

	{
		std::string str = "Textwithsomewhitespaces";
		ASIO2_CHECK(asio2::to_lower(str) == "textwithsomewhitespaces");
	}

	{
		std::string str = "Textwithsomewhitespaces";
		ASIO2_CHECK(asio2::to_upper(str) == "TEXTWITHSOMEWHITESPACES");
	}

	{
		std::string str = "text\n With\tSome \t  whitespaces\n\n";
		ASIO2_CHECK(asio2::capitalize(str) == "Text\n with\tsome \t  whitespaces\n\n");
	}

	ASIO2_CHECK(asio2::capitalize(make_string()) == " a b c ");

	{
		std::string str = "text\n With\tSome \t  whitespaces\n\n";
		ASIO2_CHECK(asio2::capitalize_first_char(str) == "Text\n With\tSome \t  whitespaces\n\n");
	}

	ASIO2_CHECK(asio2::capitalize_first_char(make_string()) == " a B c ");

	{
		std::string str = "Text\n with\tsome \t  whitespaces\n\n";
		std::string_view sv = "Text\n with\tsome \t  whitespaces\n\n";

		const char* p = "Text\n with\tsome \t  whitespaces\n\n";

		char buf[] = "Text\n with\tsome \t  whitespaces\n\n";

		ASIO2_CHECK(asio2::contains("text\n With\tSome \t  whitespaces\n\n", "Some"));
		ASIO2_CHECK(!asio2::contains("text\n With\tSome \t  whitespaces\n\n", "some"));
		ASIO2_CHECK(asio2::contains("text\n With\tSome \t  whitespaces\n\n", 'c'));
		ASIO2_CHECK(!asio2::contains("text\n With\tSome \t  whitespaces\n\n", 'C'));

		ASIO2_CHECK(asio2::contains(str, "some"));
		ASIO2_CHECK(asio2::contains(str, 'c'));
		ASIO2_CHECK(asio2::contains(sv, "some"));
		ASIO2_CHECK(asio2::contains(sv, 'c'));
		ASIO2_CHECK(asio2::contains(p, "some"));
		ASIO2_CHECK(asio2::contains(p, 'c'));
		ASIO2_CHECK(asio2::contains(buf, "some"));
		ASIO2_CHECK(asio2::contains(buf, 'c'));
	}

	{
		ASIO2_CHECK(asio2::compare_ignore_case("text\n With\tSome \t  whitespaces\n\n", "Text\n with\tsome \t  whitespaces\n\n"));
		ASIO2_CHECK(!asio2::compare_ignore_case("text\n With\tSome \t  whitespaces\n\n", "Text\n With\tSome \t x whitespaces\n\n"));
	}

	{
		std::string str = "Text\n with\tsome \t  whitespaces\n\n";
		ASIO2_CHECK(asio2::trim_all(str) == "Textwithsomewhitespaces");
	}

	ASIO2_CHECK(asio2::trim_all(make_string()) == "aBc");

	{
		std::string str = " \nText\n with\tsome \t  whitespaces\n\n";
		ASIO2_CHECK(asio2::trim_left(str) == "Text\n with\tsome \t  whitespaces\n\n");
	}

	ASIO2_CHECK(asio2::trim_left(make_string()) == "a B c ");

	{
		std::string str = " \nText\n with\tsome \t  whitespaces\n\n";
		ASIO2_CHECK(asio2::ltrim(str) == "Text\n with\tsome \t  whitespaces\n\n");
	}

	ASIO2_CHECK(asio2::ltrim(make_string()) == "a B c ");

	{
		std::string str = " \nText\n with\tsome \t  whitespaces \n\n ";
		ASIO2_CHECK(asio2::trim_right(str) == " \nText\n with\tsome \t  whitespaces");
	}

	ASIO2_CHECK(asio2::trim_right(make_string()) == " a B c");

	{
		std::string str = " \nText\n with\tsome \t  whitespaces \n\n ";
		ASIO2_CHECK(asio2::rtrim(str) == " \nText\n with\tsome \t  whitespaces");
	}

	ASIO2_CHECK(asio2::rtrim(make_string()) == " a B c");

	{
		std::string str = " \nText\n with\tsome \t  whitespaces \n\n ";
		ASIO2_CHECK(asio2::trim_both(str) == "Text\n with\tsome \t  whitespaces");
	}

	ASIO2_CHECK(asio2::trim_both(make_string()) == "a B c");

	{
		std::string str = " \nText\n with\tsome \t  whitespaces \n\n ";
		ASIO2_CHECK(asio2::trim(str) == "Text\n with\tsome \t  whitespaces");
	}

	ASIO2_CHECK(asio2::trim(make_string()) == "a B c");

	{
		std::string str = " \nText\n with\tsome \t  whitespaces \n\n ";
		ASIO2_CHECK(asio2::trim_left_copy(str) == "Text\n with\tsome \t  whitespaces \n\n ");
		ASIO2_CHECK(str == " \nText\n with\tsome \t  whitespaces \n\n ");
	}

	{
		std::string str = " \nText\n with\tsome \t  whitespaces \n\n ";
		ASIO2_CHECK(asio2::ltrim_copy(str) == "Text\n with\tsome \t  whitespaces \n\n ");
		ASIO2_CHECK(str == " \nText\n with\tsome \t  whitespaces \n\n ");
	}

	{
		std::string str = " \nText\n with\tsome \t  whitespaces \n\n ";
		ASIO2_CHECK(asio2::trim_right_copy(str) == " \nText\n with\tsome \t  whitespaces");
		ASIO2_CHECK(str == " \nText\n with\tsome \t  whitespaces \n\n ");
	}

	{
		std::string str = " \nText\n with\tsome \t  whitespaces \n\n ";
		ASIO2_CHECK(asio2::rtrim_copy(str) == " \nText\n with\tsome \t  whitespaces");
		ASIO2_CHECK(str == " \nText\n with\tsome \t  whitespaces \n\n ");
	}

	{
		std::string str = " \nText\n with\tsome \t  whitespaces \n\n ";
		ASIO2_CHECK(asio2::trim_copy(str) == "Text\n with\tsome \t  whitespaces");
		ASIO2_CHECK(str == " \nText\n with\tsome \t  whitespaces \n\n ");
	}

	{
		std::string_view str = " \nText\n with\tsome \t  whitespaces \n\n ";
		ASIO2_CHECK(asio2::trim_left(str) == "Text\n with\tsome \t  whitespaces \n\n ");
	}

	ASIO2_CHECK(asio2::trim_left(make_string_view()) == "a B c ");

	{
		std::string_view str = " \nText\n with\tsome \t  whitespaces \n\n ";
		ASIO2_CHECK(asio2::ltrim(str) == "Text\n with\tsome \t  whitespaces \n\n ");
	}

	ASIO2_CHECK(asio2::ltrim(make_string_view()) == "a B c ");

	{
		std::string_view str = " \nText\n with\tsome \t  whitespaces \n\n ";
		ASIO2_CHECK(asio2::trim_right(str) == " \nText\n with\tsome \t  whitespaces");
	}

	using namespace std::string_view_literals;

	ASIO2_CHECK(asio2::trim_right(make_string_view()) == " a B c");
	ASIO2_CHECK(asio2::trim_right(" a B c "sv) == " a B c");

	{
		std::string_view str = " \nText\n with\tsome \t  whitespaces \n\n ";
		ASIO2_CHECK(asio2::rtrim(str) == " \nText\n with\tsome \t  whitespaces");
	}

	ASIO2_CHECK(asio2::rtrim(make_string_view()) == " a B c");

	{
		std::string_view str = " \nText\n with\tsome \t  whitespaces \n\n ";
		ASIO2_CHECK(asio2::trim_both(str) == "Text\n with\tsome \t  whitespaces");
	}

	ASIO2_CHECK(asio2::trim_both(make_string_view()) == "a B c");

	{
		std::string_view str = " \nText\n with\tsome \t  whitespaces \n\n ";
		ASIO2_CHECK(asio2::trim(str) == "Text\n with\tsome \t  whitespaces");
	}

	ASIO2_CHECK(asio2::trim(make_string_view()) == "a B c");

	{
		std::string str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace_first(str, "some", "Any") == " \nText\n with\tAny \t some whitespaces \n\n ");
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace_first(str, "Some", "Any") == " \nText\n with\tsome \t some whitespaces \n\n ");
		std::string st = "some", sr = "Any";
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace_first(str, st, sr) == " \nText\n with\tAny \t some whitespaces \n\n ");
		std::string_view vt = "some", vr = "Any";
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace_first(str, vt, vr) == " \nText\n with\tAny \t some whitespaces \n\n ");
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace_first(str, st, vr) == " \nText\n with\tAny \t some whitespaces \n\n ");
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace_first(str, vt, sr) == " \nText\n with\tAny \t some whitespaces \n\n ");
		const char* pt = "some"; const char* pr = "Any";
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace_first(str, pt, pr) == " \nText\n with\tAny \t some whitespaces \n\n ");
		char bt[] = "some"; char br[] = "Any";
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace_first(str, bt, br) == " \nText\n with\tAny \t some whitespaces \n\n ");
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace_first(str, 'T', 't') == " \ntext\n with\tsome \t some whitespaces \n\n ");
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace_first(str, "T", 't') == " \ntext\n with\tsome \t some whitespaces \n\n ");
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace_first(str, 'T', "t") == " \ntext\n with\tsome \t some whitespaces \n\n ");
	}

	ASIO2_CHECK(asio2::replace_first(make_string(), " ", "") == "a B c ");

	{
		std::string str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace_last(str, "some", "Any") == " \nText\n with\tsome \t Any whitespaces \n\n ");
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace_last(str, "Some", "Any") == " \nText\n with\tsome \t some whitespaces \n\n ");
		std::string st = "some", sr = "Any";
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace_last(str, st, sr) == " \nText\n with\tsome \t Any whitespaces \n\n ");
		std::string_view vt = "some", vr = "Any";
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace_last(str, vt, vr) == " \nText\n with\tsome \t Any whitespaces \n\n ");
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace_last(str, st, vr) == " \nText\n with\tsome \t Any whitespaces \n\n ");
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace_last(str, vt, sr) == " \nText\n with\tsome \t Any whitespaces \n\n ");
		const char* pt = "some"; const char* pr = "Any";
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace_last(str, pt, pr) == " \nText\n with\tsome \t Any whitespaces \n\n ");
		char bt[] = "some"; char br[] = "Any";
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace_last(str, bt, br) == " \nText\n with\tsome \t Any whitespaces \n\n ");
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace_last(str, 't', 'x') == " \nText\n with\tsome \t some whixespaces \n\n ");
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace_last(str, "t", 'x') == " \nText\n with\tsome \t some whixespaces \n\n ");
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace_last(str, 't', "x") == " \nText\n with\tsome \t some whixespaces \n\n ");
	}

	ASIO2_CHECK(asio2::replace_last(make_string(), ' ', "") == " a B c");

	{
		std::string str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace_all(str, "some", "Any") == " \nText\n with\tAny \t Any whitespaces \n\n ");
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace_all(str, "Some", "Any") == " \nText\n with\tsome \t some whitespaces \n\n ");
		std::string st = "some", sr = "Any";
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace_all(str, st, sr) == " \nText\n with\tAny \t Any whitespaces \n\n ");
		std::string_view vt = "some", vr = "Any";
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace_all(str, vt, vr) == " \nText\n with\tAny \t Any whitespaces \n\n ");
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace_all(str, st, vr) == " \nText\n with\tAny \t Any whitespaces \n\n ");
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace_all(str, vt, sr) == " \nText\n with\tAny \t Any whitespaces \n\n ");
		const char* pt = "some"; const char* pr = "Any";
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace_all(str, pt, pr) == " \nText\n with\tAny \t Any whitespaces \n\n ");
		char bt[] = "some"; char br[] = "Any";
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace_all(str, bt, br) == " \nText\n with\tAny \t Any whitespaces \n\n ");
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace_all(str, 't', 'x') == " \nTexx\n wixh\tsome \t some whixespaces \n\n ");
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace_all(str, "t", 'x') == " \nTexx\n wixh\tsome \t some whixespaces \n\n ");
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace_all(str, 't', "x") == " \nTexx\n wixh\tsome \t some whixespaces \n\n ");
	}

	ASIO2_CHECK(asio2::replace_all(make_string(), ' ', "") == "aBc");

	{
		std::string str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace(str, "some", "Any") == " \nText\n with\tAny \t Any whitespaces \n\n ");
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace(str, "Some", "Any") == " \nText\n with\tsome \t some whitespaces \n\n ");
		std::string st = "some", sr = "Any";
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace(str, st, sr) == " \nText\n with\tAny \t Any whitespaces \n\n ");
		std::string_view vt = "some", vr = "Any";
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace(str, vt, vr) == " \nText\n with\tAny \t Any whitespaces \n\n ");
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace(str, st, vr) == " \nText\n with\tAny \t Any whitespaces \n\n ");
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace(str, vt, sr) == " \nText\n with\tAny \t Any whitespaces \n\n ");
		const char* pt = "some"; const char* pr = "Any";
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace(str, pt, pr) == " \nText\n with\tAny \t Any whitespaces \n\n ");
		char bt[] = "some"; char br[] = "Any";
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace(str, bt, br) == " \nText\n with\tAny \t Any whitespaces \n\n ");
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace(str, 't', 'x') == " \nTexx\n wixh\tsome \t some whixespaces \n\n ");
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace(str, "t", 'x') == " \nTexx\n wixh\tsome \t some whixespaces \n\n ");
		str = " \nText\n with\tsome \t some whitespaces \n\n ";
		ASIO2_CHECK(asio2::replace(str, 't', "x") == " \nTexx\n wixh\tsome \t some whixespaces \n\n ");
	}

	ASIO2_CHECK(asio2::replace(make_string(), ' ', "") == "aBc");

	{
		std::string str = " \nText\n with\tsome \t some whitespaces \n\n ";
		std::string_view sv = " \nText\n with\tsome \t some whitespaces \n\n ";

		const char* p = " \nText\n with\tsome \t some whitespaces \n\n ";
		char buf[] = " \nText\n with\tsome \t some whitespaces \n\n ";

		std::string sln = "\n";
		char cln = '\n';
		const char* pln = "\n";
		std::string_view vln = "\n";

		std::string sspace = " ";
		char cspace = ' ';
		const char* pspace = " ";
		std::string_view vspace = " ";

		ASIO2_CHECK(asio2::ends_with(str, " ") == true);
		ASIO2_CHECK(asio2::ends_with(str, ' ') == true);
		ASIO2_CHECK(asio2::ends_with(str, sspace) == true);
		ASIO2_CHECK(asio2::ends_with(str, cspace) == true);
		ASIO2_CHECK(asio2::ends_with(str, pspace) == true);
		ASIO2_CHECK(asio2::ends_with(str, vspace) == true);

		ASIO2_CHECK(asio2::ends_with(str, "\n") == false);
		ASIO2_CHECK(asio2::ends_with(str, '\n') == false);
		ASIO2_CHECK(asio2::ends_with(str, sln) == false);
		ASIO2_CHECK(asio2::ends_with(str, cln) == false);
		ASIO2_CHECK(asio2::ends_with(str, pln) == false);
		ASIO2_CHECK(asio2::ends_with(str, vln) == false);

		ASIO2_CHECK(asio2::ends_with(sv, " ") == true);
		ASIO2_CHECK(asio2::ends_with(sv, ' ') == true);
		ASIO2_CHECK(asio2::ends_with(sv, sspace) == true);
		ASIO2_CHECK(asio2::ends_with(sv, cspace) == true);
		ASIO2_CHECK(asio2::ends_with(sv, pspace) == true);
		ASIO2_CHECK(asio2::ends_with(sv, vspace) == true);

		ASIO2_CHECK(asio2::ends_with(sv, "\n") == false);
		ASIO2_CHECK(asio2::ends_with(sv, '\n') == false);
		ASIO2_CHECK(asio2::ends_with(sv, sln) == false);
		ASIO2_CHECK(asio2::ends_with(sv, cln) == false);
		ASIO2_CHECK(asio2::ends_with(sv, pln) == false);
		ASIO2_CHECK(asio2::ends_with(sv, vln) == false);

		ASIO2_CHECK(asio2::ends_with(p, " ") == true);
		ASIO2_CHECK(asio2::ends_with(p, ' ') == true);
		ASIO2_CHECK(asio2::ends_with(p, sspace) == true);
		ASIO2_CHECK(asio2::ends_with(p, cspace) == true);
		ASIO2_CHECK(asio2::ends_with(p, pspace) == true);
		ASIO2_CHECK(asio2::ends_with(p, vspace) == true);

		ASIO2_CHECK(asio2::ends_with(p, "\n") == false);
		ASIO2_CHECK(asio2::ends_with(p, '\n') == false);
		ASIO2_CHECK(asio2::ends_with(p, sln) == false);
		ASIO2_CHECK(asio2::ends_with(p, cln) == false);
		ASIO2_CHECK(asio2::ends_with(p, pln) == false);
		ASIO2_CHECK(asio2::ends_with(p, vln) == false);

		ASIO2_CHECK(asio2::ends_with(buf, " ") == true);
		ASIO2_CHECK(asio2::ends_with(buf, ' ') == true);
		ASIO2_CHECK(asio2::ends_with(buf, sspace) == true);
		ASIO2_CHECK(asio2::ends_with(buf, cspace) == true);
		ASIO2_CHECK(asio2::ends_with(buf, pspace) == true);
		ASIO2_CHECK(asio2::ends_with(buf, vspace) == true);

		ASIO2_CHECK(asio2::ends_with(buf, "\n") == false);
		ASIO2_CHECK(asio2::ends_with(buf, '\n') == false);
		ASIO2_CHECK(asio2::ends_with(buf, sln) == false);
		ASIO2_CHECK(asio2::ends_with(buf, cln) == false);
		ASIO2_CHECK(asio2::ends_with(buf, pln) == false);
		ASIO2_CHECK(asio2::ends_with(buf, vln) == false);

		ASIO2_CHECK(asio2::ends_with(" \nText\n with\tsome \t some whitespaces \n\n ", " ") == true);
		ASIO2_CHECK(asio2::ends_with(" \nText\n with\tsome \t some whitespaces \n\n ", ' ') == true);
		ASIO2_CHECK(asio2::ends_with(" \nText\n with\tsome \t some whitespaces \n\n ", sspace) == true);
		ASIO2_CHECK(asio2::ends_with(" \nText\n with\tsome \t some whitespaces \n\n ", cspace) == true);
		ASIO2_CHECK(asio2::ends_with(" \nText\n with\tsome \t some whitespaces \n\n ", pspace) == true);
		ASIO2_CHECK(asio2::ends_with(" \nText\n with\tsome \t some whitespaces \n\n ", vspace) == true);

		ASIO2_CHECK(asio2::ends_with(" \nText\n with\tsome \t some whitespaces \n\n ", "\n") == false);
		ASIO2_CHECK(asio2::ends_with(" \nText\n with\tsome \t some whitespaces \n\n ", '\n') == false);
		ASIO2_CHECK(asio2::ends_with(" \nText\n with\tsome \t some whitespaces \n\n ", sln) == false);
		ASIO2_CHECK(asio2::ends_with(" \nText\n with\tsome \t some whitespaces \n\n ", cln) == false);
		ASIO2_CHECK(asio2::ends_with(" \nText\n with\tsome \t some whitespaces \n\n ", pln) == false);
		ASIO2_CHECK(asio2::ends_with(" \nText\n with\tsome \t some whitespaces \n\n ", vln) == false);
	}

	{
		std::string str = " \nText\n with\tsome \t some whitespaces \n\n ";
		std::string_view sv = " \nText\n with\tsome \t some whitespaces \n\n ";

		const char* p = " \nText\n with\tsome \t some whitespaces \n\n ";
		char buf[] = " \nText\n with\tsome \t some whitespaces \n\n ";

		std::string sln = "\n";
		char cln = '\n';
		const char* pln = "\n";
		std::string_view vln = "\n";

		std::string sspace = " ";
		char cspace = ' ';
		const char* pspace = " ";
		std::string_view vspace = " ";

		ASIO2_CHECK(asio2::starts_with(str, " ") == true);
		ASIO2_CHECK(asio2::starts_with(str, ' ') == true);
		ASIO2_CHECK(asio2::starts_with(str, sspace) == true);
		ASIO2_CHECK(asio2::starts_with(str, cspace) == true);
		ASIO2_CHECK(asio2::starts_with(str, pspace) == true);
		ASIO2_CHECK(asio2::starts_with(str, vspace) == true);

		ASIO2_CHECK(asio2::starts_with(str, "\n") == false);
		ASIO2_CHECK(asio2::starts_with(str, '\n') == false);
		ASIO2_CHECK(asio2::starts_with(str, sln) == false);
		ASIO2_CHECK(asio2::starts_with(str, cln) == false);
		ASIO2_CHECK(asio2::starts_with(str, pln) == false);
		ASIO2_CHECK(asio2::starts_with(str, vln) == false);

		ASIO2_CHECK(asio2::starts_with(sv, " ") == true);
		ASIO2_CHECK(asio2::starts_with(sv, ' ') == true);
		ASIO2_CHECK(asio2::starts_with(sv, sspace) == true);
		ASIO2_CHECK(asio2::starts_with(sv, cspace) == true);
		ASIO2_CHECK(asio2::starts_with(sv, pspace) == true);
		ASIO2_CHECK(asio2::starts_with(sv, vspace) == true);

		ASIO2_CHECK(asio2::starts_with(sv, "\n") == false);
		ASIO2_CHECK(asio2::starts_with(sv, '\n') == false);
		ASIO2_CHECK(asio2::starts_with(sv, sln) == false);
		ASIO2_CHECK(asio2::starts_with(sv, cln) == false);
		ASIO2_CHECK(asio2::starts_with(sv, pln) == false);
		ASIO2_CHECK(asio2::starts_with(sv, vln) == false);

		ASIO2_CHECK(asio2::starts_with(p, " ") == true);
		ASIO2_CHECK(asio2::starts_with(p, ' ') == true);
		ASIO2_CHECK(asio2::starts_with(p, sspace) == true);
		ASIO2_CHECK(asio2::starts_with(p, cspace) == true);
		ASIO2_CHECK(asio2::starts_with(p, pspace) == true);
		ASIO2_CHECK(asio2::starts_with(p, vspace) == true);

		ASIO2_CHECK(asio2::starts_with(p, "\n") == false);
		ASIO2_CHECK(asio2::starts_with(p, '\n') == false);
		ASIO2_CHECK(asio2::starts_with(p, sln) == false);
		ASIO2_CHECK(asio2::starts_with(p, cln) == false);
		ASIO2_CHECK(asio2::starts_with(p, pln) == false);
		ASIO2_CHECK(asio2::starts_with(p, vln) == false);

		ASIO2_CHECK(asio2::starts_with(buf, " ") == true);
		ASIO2_CHECK(asio2::starts_with(buf, ' ') == true);
		ASIO2_CHECK(asio2::starts_with(buf, sspace) == true);
		ASIO2_CHECK(asio2::starts_with(buf, cspace) == true);
		ASIO2_CHECK(asio2::starts_with(buf, pspace) == true);
		ASIO2_CHECK(asio2::starts_with(buf, vspace) == true);

		ASIO2_CHECK(asio2::starts_with(buf, "\n") == false);
		ASIO2_CHECK(asio2::starts_with(buf, '\n') == false);
		ASIO2_CHECK(asio2::starts_with(buf, sln) == false);
		ASIO2_CHECK(asio2::starts_with(buf, cln) == false);
		ASIO2_CHECK(asio2::starts_with(buf, pln) == false);
		ASIO2_CHECK(asio2::starts_with(buf, vln) == false);

		ASIO2_CHECK(asio2::starts_with(" \nText\n with\tsome \t some whitespaces \n\n ", " ") == true);
		ASIO2_CHECK(asio2::starts_with(" \nText\n with\tsome \t some whitespaces \n\n ", ' ') == true);
		ASIO2_CHECK(asio2::starts_with(" \nText\n with\tsome \t some whitespaces \n\n ", sspace) == true);
		ASIO2_CHECK(asio2::starts_with(" \nText\n with\tsome \t some whitespaces \n\n ", cspace) == true);
		ASIO2_CHECK(asio2::starts_with(" \nText\n with\tsome \t some whitespaces \n\n ", pspace) == true);
		ASIO2_CHECK(asio2::starts_with(" \nText\n with\tsome \t some whitespaces \n\n ", vspace) == true);

		ASIO2_CHECK(asio2::starts_with(" \nText\n with\tsome \t some whitespaces \n\n ", "\n") == false);
		ASIO2_CHECK(asio2::starts_with(" \nText\n with\tsome \t some whitespaces \n\n ", '\n') == false);
		ASIO2_CHECK(asio2::starts_with(" \nText\n with\tsome \t some whitespaces \n\n ", sln) == false);
		ASIO2_CHECK(asio2::starts_with(" \nText\n with\tsome \t some whitespaces \n\n ", cln) == false);
		ASIO2_CHECK(asio2::starts_with(" \nText\n with\tsome \t some whitespaces \n\n ", pln) == false);
		ASIO2_CHECK(asio2::starts_with(" \nText\n with\tsome \t some whitespaces \n\n ", vln) == false);
	}

	{
		std::string str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		std::vector<std::string> result = asio2::split(str, '\n');
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		std::string str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		std::vector<std::string> result = asio2::split(str, "\n");
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		std::string str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		std::string d = "\n";
		std::vector<std::string> result = asio2::split(str, d);
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		std::string str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		std::string_view d = "\n";
		std::vector<std::string> result = asio2::split(str, d);
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		std::string str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		const char* d = "\n";
		std::vector<std::string> result = asio2::split(str, d);
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		std::string str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		char d[] = "\n";
		std::vector<std::string> result = asio2::split(str, d);
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		std::string str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		char d = '\n';
		std::vector<std::string> result = asio2::split(str, d);
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		std::string_view str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		std::vector<std::string_view> result = asio2::split(str, '\n');
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		std::string_view str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		std::vector<std::string_view> result = asio2::split(str, "\n");
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		std::string_view str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		std::string d = "\n";
		std::vector<std::string_view> result = asio2::split(str, d);
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		std::string_view str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		std::string_view d = "\n";
		std::vector<std::string_view> result = asio2::split(str, d);
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		std::string_view str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		const char* d = "\n";
		std::vector<std::string_view> result = asio2::split(str, d);
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		std::string_view str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		char d[] = "\n";
		std::vector<std::string_view> result = asio2::split(str, d);
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		std::string_view str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		char d = '\n';
		std::vector<std::string_view> result = asio2::split(str, d);
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	// ----------------------------------------------------------------------------------------


	{
		std::vector<std::string> result = asio2::split("\n \nText\n with\tsome \t some whitespaces \n\n \n", '\n');
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		std::vector<std::string> result = asio2::split("\n \nText\n with\tsome \t some whitespaces \n\n \n", "\n");
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		std::string d = "\n";
		std::vector<std::string> result = asio2::split("\n \nText\n with\tsome \t some whitespaces \n\n \n", d);
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		std::string_view d = "\n";
		std::vector<std::string> result = asio2::split("\n \nText\n with\tsome \t some whitespaces \n\n \n", d);
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		const char* d = "\n";
		std::vector<std::string> result = asio2::split("\n \nText\n with\tsome \t some whitespaces \n\n \n", d);
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		char d[] = "\n";
		std::vector<std::string> result = asio2::split("\n \nText\n with\tsome \t some whitespaces \n\n \n", d);
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		char d = '\n';
		std::vector<std::string> result = asio2::split("\n \nText\n with\tsome \t some whitespaces \n\n \n", d);
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	// ----------------------------------------------------------------------------------------

	{
		std::string str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		std::vector<std::string> result = asio2::split_any(str, '\n');
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		std::string str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		std::vector<std::string> result = asio2::split_any(str, "\n");
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		std::string str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		std::string d = "\n";
		std::vector<std::string> result = asio2::split_any(str, d);
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		std::string str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		std::string_view d = "\n";
		std::vector<std::string> result = asio2::split_any(str, d);
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		std::string str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		const char* d = "\n";
		std::vector<std::string> result = asio2::split_any(str, d);
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		std::string str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		char d[] = "\n";
		std::vector<std::string> result = asio2::split_any(str, d);
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		std::string str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		char d = '\n';
		std::vector<std::string> result = asio2::split_any(str, d);
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		std::string_view str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		std::vector<std::string_view> result = asio2::split_any(str, '\n');
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		std::string_view str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		std::vector<std::string_view> result = asio2::split_any(str, "\n");
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		std::string_view str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		std::string d = "\n";
		std::vector<std::string_view> result = asio2::split_any(str, d);
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		std::string_view str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		std::string_view d = "\n";
		std::vector<std::string_view> result = asio2::split_any(str, d);
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		std::string_view str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		const char* d = "\n";
		std::vector<std::string_view> result = asio2::split_any(str, d);
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		std::string_view str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		char d[] = "\n";
		std::vector<std::string_view> result = asio2::split_any(str, d);
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		std::string_view str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		char d = '\n';
		std::vector<std::string_view> result = asio2::split_any(str, d);
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	// ----------------------------------------------------------------------------------------

	{
		std::string str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		std::vector<std::string> result = asio2::split_any(str, "\t\n");
		ASIO2_CHECK(result.size() == 9);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with");
		ASIO2_CHECK(result[4] == "some ");
		ASIO2_CHECK(result[5] == " some whitespaces ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
		ASIO2_CHECK(result[7] == " ");
		ASIO2_CHECK(result[8] == "" && result[8].empty());
	}

	{
		std::string str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		std::string d = "\t\n";
		std::vector<std::string> result = asio2::split_any(str, d);
		ASIO2_CHECK(result.size() == 9);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with");
		ASIO2_CHECK(result[4] == "some ");
		ASIO2_CHECK(result[5] == " some whitespaces ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
		ASIO2_CHECK(result[7] == " ");
		ASIO2_CHECK(result[8] == "" && result[8].empty());
	}

	{
		std::string str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		std::string_view d = "\t\n";
		std::vector<std::string> result = asio2::split_any(str, d);
		ASIO2_CHECK(result.size() == 9);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with");
		ASIO2_CHECK(result[4] == "some ");
		ASIO2_CHECK(result[5] == " some whitespaces ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
		ASIO2_CHECK(result[7] == " ");
		ASIO2_CHECK(result[8] == "" && result[8].empty());
	}

	{
		std::string_view str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		std::vector<std::string_view> result = asio2::split_any(str, "\t\n");
		ASIO2_CHECK(result.size() == 9);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with");
		ASIO2_CHECK(result[4] == "some ");
		ASIO2_CHECK(result[5] == " some whitespaces ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
		ASIO2_CHECK(result[7] == " ");
		ASIO2_CHECK(result[8] == "" && result[8].empty());
	}

	{
		std::string_view str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		std::string d = "\t\n";
		std::vector<std::string_view> result = asio2::split_any(str, d);
		ASIO2_CHECK(result.size() == 9);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with");
		ASIO2_CHECK(result[4] == "some ");
		ASIO2_CHECK(result[5] == " some whitespaces ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
		ASIO2_CHECK(result[7] == " ");
		ASIO2_CHECK(result[8] == "" && result[8].empty());
	}

	{
		std::string_view str = "\n \nText\n with\tsome \t some whitespaces \n\n \n";
		std::string_view d = "\t\n";
		std::vector<std::string_view> result = asio2::split_any(str, d);
		ASIO2_CHECK(result.size() == 9);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with");
		ASIO2_CHECK(result[4] == "some ");
		ASIO2_CHECK(result[5] == " some whitespaces ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
		ASIO2_CHECK(result[7] == " ");
		ASIO2_CHECK(result[8] == "" && result[8].empty());
	}

	{
		std::vector<std::string> result = asio2::split_any("\n \nText\n with\tsome \t some whitespaces \n\n \n", '\n');
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		std::vector<std::string> result = asio2::split_any("\n \nText\n with\tsome \t some whitespaces \n\n \n", "\n");
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		std::string d = "\n";
		std::vector<std::string> result = asio2::split_any("\n \nText\n with\tsome \t some whitespaces \n\n \n", d);
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		std::string_view d = "\n";
		std::vector<std::string> result = asio2::split_any("\n \nText\n with\tsome \t some whitespaces \n\n \n", d);
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		const char* d = "\n";
		std::vector<std::string> result = asio2::split_any("\n \nText\n with\tsome \t some whitespaces \n\n \n", d);
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		char d[] = "\n";
		std::vector<std::string> result = asio2::split_any("\n \nText\n with\tsome \t some whitespaces \n\n \n", d);
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	{
		char d = '\n';
		std::vector<std::string> result = asio2::split_any("\n \nText\n with\tsome \t some whitespaces \n\n \n", d);
		ASIO2_CHECK(result.size() == 7);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with\tsome \t some whitespaces ");
		ASIO2_CHECK(result[4] == "" && result[4].empty());
		ASIO2_CHECK(result[5] == " ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
	}

	// ----------------------------------------------------------------------------------------

	{
		std::vector<std::string> result = asio2::split_any("\n \nText\n with\tsome \t some whitespaces \n\n \n", "\t\n");
		ASIO2_CHECK(result.size() == 9);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with");
		ASIO2_CHECK(result[4] == "some ");
		ASIO2_CHECK(result[5] == " some whitespaces ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
		ASIO2_CHECK(result[7] == " ");
		ASIO2_CHECK(result[8] == "" && result[8].empty());
	}

	{
		std::string d = "\t\n";
		std::vector<std::string> result = asio2::split_any("\n \nText\n with\tsome \t some whitespaces \n\n \n", d);
		ASIO2_CHECK(result.size() == 9);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with");
		ASIO2_CHECK(result[4] == "some ");
		ASIO2_CHECK(result[5] == " some whitespaces ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
		ASIO2_CHECK(result[7] == " ");
		ASIO2_CHECK(result[8] == "" && result[8].empty());
	}

	{
		std::string_view d = "\t\n";
		std::vector<std::string> result = asio2::split_any("\n \nText\n with\tsome \t some whitespaces \n\n \n", d);
		ASIO2_CHECK(result.size() == 9);
		ASIO2_CHECK(result[0] == "" && result[0].empty());
		ASIO2_CHECK(result[1] == " ");
		ASIO2_CHECK(result[2] == "Text");
		ASIO2_CHECK(result[3] == " with");
		ASIO2_CHECK(result[4] == "some ");
		ASIO2_CHECK(result[5] == " some whitespaces ");
		ASIO2_CHECK(result[6] == "" && result[6].empty());
		ASIO2_CHECK(result[7] == " ");
		ASIO2_CHECK(result[8] == "" && result[8].empty());
	}

	{
		std::vector<std::string> res;

		std::string str = "abc,abcd;abce.abcf?";

		// Basic usage
		res = asio2::regex_split(str, "[,;\\.\\?]+");

		ASIO2_CHECK(res.size() == 4);
		ASIO2_CHECK(res[0] == "abc");
		ASIO2_CHECK(res[1] == "abcd");
		ASIO2_CHECK(res[2] == "abce");
		ASIO2_CHECK(res[3] == "abcf");

		// Empty input => empty string
		ASIO2_CHECK(asio2::regex_split("", ",:")[0] == "");

		// No matches => original string
		res = asio2::regex_split("abc_123", ",; ");
		ASIO2_CHECK(res.size() == 1);
		ASIO2_CHECK(res[0] == "abc_123");

		// Empty delimiters => original string
		res = asio2::regex_split("abc;def", "");
		ASIO2_CHECK(res.size() == 8);
		// on macos, the all 8 elements are empty.
		//ASIO2_CHECK(res[0]== "");
		//ASIO2_CHECK(res[1]== "a");
		//ASIO2_CHECK(res[2]== "b");
		//ASIO2_CHECK(res[3]== "c");
		//ASIO2_CHECK(res[4]== ";");
		//ASIO2_CHECK(res[5]== "d");
		//ASIO2_CHECK(res[6]== "e");
		//ASIO2_CHECK(res[7]== "f");

		// Leading delimiters => leading empty string
		res = asio2::regex_split(";abc", ",; ");
		ASIO2_CHECK(res.size() == 1);
		ASIO2_CHECK(res[0] == ";abc");
	}

	{
		std::map<std::string, std::string> res = asio2::regex_split_map(
			"[abc] name = 123; [abd] name = 123;[abe] name = 123;  ", "\\[[^\\]]+\\]");
		std::map<std::string, std::string> ans = {
			{"[abc]", "name = 123;"}, {"[abd]", "name = 123;"}, {"[abe]", "name = 123;"}
		};
		for (auto each : res)
		{
			ASIO2_CHECK(ans.count(each.first) == 1);
			if (ans.count(each.first) == 1)
			{
				auto str = each.second;
				asio2::trim(str);
				ASIO2_CHECK(str == ans[each.first]);
			}
		}

		// TODO: More test is to be added.
	}

	{
		std::map<std::wstring, std::wstring> res = asio2::regex_split_map(
			L"[abc] name = 123; [abd] name = 123;[abe] name = 123;  ", L"\\[[^\\]]+\\]");
		std::map<std::wstring, std::wstring> ans = {
			{L"[abc]", L"name = 123;"}, {L"[abd]", L"name = 123;"}, {L"[abe]", L"name = 123;"}
		};
		for (auto each : res)
		{
			ASIO2_CHECK(ans.count(each.first) == 1);
			if (ans.count(each.first) == 1)
			{
				auto str = each.second;
				asio2::trim(str);
				ASIO2_CHECK(str == ans[each.first]);
			}
		}
	}

	{
		std::string str1 = "Col1;Col2;Col3";
		std::vector<std::string> tokens1 = { "Col1", "Col2", "Col3" };

		ASIO2_CHECK(str1 == asio2::join(tokens1, ";"));

		std::string str2 = "1|2|3";
		std::vector<unsigned> tokens2 = { 1, 2, 3 };

		ASIO2_CHECK(str2 == asio2::join(tokens2, "|"));
	}

	{
		std::vector<std::string> tokens = { "t1", "t2", "", "t4", "" };
		asio2::drop_empty(tokens);
		ASIO2_CHECK(tokens.size() == 3);
		ASIO2_CHECK(tokens[0] == "t1");
		ASIO2_CHECK(tokens[1] == "t2");
		ASIO2_CHECK(tokens[2] == "t4");
	}

	{
		std::vector<std::string> tokens = { "t1", "t2", "", "t4", "" };
		auto res = asio2::drop_empty_copy(tokens);
		ASIO2_CHECK(res.size() == 3);
		ASIO2_CHECK(res[0] == "t1");
		ASIO2_CHECK(res[1] == "t2");
		ASIO2_CHECK(res[2] == "t4");
	}

	{
		std::vector<std::string> str1 = { "t1", "t2", "", "t4", "", "t1" };
		asio2::drop_duplicate(str1);

		std::vector<std::string> str2 = { "", "t1", "t2", "t4" };

		ASIO2_CHECK(std::equal(str1.cbegin(), str1.cend(), str2.cbegin()) == true);
	}

	{
		std::vector<std::string> str1 = { "t1", "t2", "", "t4", "", "t1" };
		auto str3 = asio2::drop_duplicate_copy(str1);

		std::vector<std::string> str2 = { "", "t1", "t2", "t4" };
		ASIO2_CHECK(std::equal(str2.cbegin(), str2.cend(), str3.cbegin()) == true);
	}

	{
		ASIO2_CHECK(asio2::repeat("Go", 4) == "GoGoGoGo");
		ASIO2_CHECK(asio2::repeat('Z', 10) == "ZZZZZZZZZZ");
	}

	{
		const std::regex check_mail("^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\\.[a-zA-Z0-9-.]+$");

		ASIO2_CHECK(true == asio2::matches("jon.doe@somehost.com", check_mail));
		ASIO2_CHECK(false == asio2::matches("jon.doe@", check_mail));
	}

	{
		std::vector<std::string> str1 = { "ABC", "abc", "bcd", "", "-", "  ", "123", "-100" };
		asio2::sorting_ascending(str1);

		std::vector<std::string> str2 = { "", "  ", "-", "-100", "123", "ABC", "abc", "bcd" };
		ASIO2_CHECK(std::equal(str1.cbegin(), str1.cend(), str2.cbegin()) == true);
	}

	{
		std::vector<std::string> str1 = { "ABC", "abc", "bcd", "", "-", "  ", "123", "-100" };
		asio2::sorting_descending(str1);

		std::vector<std::string> str2 = { "bcd", "abc", "ABC", "123", "-100", "-", "  ", "" };
		ASIO2_CHECK(std::equal(str1.cbegin(), str1.cend(), str2.cbegin()) == true);
	}

	{
		std::vector<std::string> str1 = { "bcd", "abc", "ABC", "123", "-100", "-", "  ", "" };

		asio2::reverse_inplace(str1);

		std::vector<std::string> str2 = { "", "  ", "-", "-100", "123", "ABC", "abc", "bcd" };

		ASIO2_CHECK(std::equal(str1.cbegin(), str1.cend(), str2.cbegin()) == true);
	}

	{
		std::vector<std::string> str1 = { "bcd", "abc", "ABC", "123", "-100", "-", "  ", "" };
		std::vector<std::string> str3(str1.begin(), str1.end());

		auto str4 = asio2::reverse_copy(str1);

		std::vector<std::string> str2 = { "", "  ", "-", "-100", "123", "ABC", "abc", "bcd" };

		ASIO2_CHECK(std::equal(str1.cbegin(), str1.cend(), str3.cbegin())== true);
		ASIO2_CHECK(std::equal(str4.cbegin(), str4.cend(), str2.cbegin()) == true);
	}

	{
		std::string str = "Text\n with\tsome \t  whitespaces\n\n";
		std::string_view sv = "Text\n with\tsome \t  whitespaces\n\n";

		const char* p = "Text\n with\tsome \t  whitespaces\n\n";
		char buf[] = "Text\n with\tsome \t  whitespaces\n\n";;

		ASIO2_CHECK(asio2::ifind("text\n with\tSome \t  whitespaces\n\n", "With") == 6);
		ASIO2_CHECK(asio2::ifind(str, "With") == 6);
		ASIO2_CHECK(asio2::ifind(sv, "With") == 6);
		ASIO2_CHECK(asio2::ifind(p, "With") == 6);
		ASIO2_CHECK(asio2::ifind(buf, "With") == 6);

		ASIO2_CHECK(asio2::ifind("text\n with\tSome \t  whitespaces\n\n", "Soem") == std::string::npos);
		ASIO2_CHECK(asio2::ifind(str, "Soem") == std::string::npos);
		ASIO2_CHECK(asio2::ifind(sv, "Soem") == std::string::npos);
		ASIO2_CHECK(asio2::ifind(p, "Soem") == std::string::npos);
		ASIO2_CHECK(asio2::ifind(buf, "Soem") == std::string::npos);

		ASIO2_CHECK(asio2::ifind("text\n with\tSome \t  whitespaces\n\n", 'W') == 6);
		ASIO2_CHECK(asio2::ifind(str, 'W') == 6);
		ASIO2_CHECK(asio2::ifind(sv, 'W') == 6);
		ASIO2_CHECK(asio2::ifind(p, 'W') == 6);
		ASIO2_CHECK(asio2::ifind(buf, 'W') == 6);
	}

	//ASIO2_TEST_BEGIN_LOOP(test_loop_times);

	//ASIO2_TEST_END_LOOP;
}


ASIO2_TEST_SUITE
(
	"strutil",
	ASIO2_TEST_CASE(strutil_test)
)
