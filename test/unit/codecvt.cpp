#include "unit_test.hpp"

#include <asio2/base/detail/push_options.hpp>
#include <asio2/external/predef.h>
#include <asio2/util/string.hpp>
#include <asio2/base/detail/util.hpp>
#include <sstream>
#include <fstream>
#include <asio2/base/detail/filesystem.hpp>

std::string ensure_char_pointer_valid(const char* p)
{
    if (p && *p)
        return p;
    return {};
}

// https://github.com/llvm/llvm-project/issues/56655
// clang maybe don't have the std::codecvt, github actions with macos-latest has this problem.

// after test, codecvt should't be used again, it has some issues under the gcc clang compiler.

#if defined(_MSC_VER) && (_MSC_VER > 1916)
#include <asio2/util/codecvt.hpp>
void codecvt_test()
{
#if defined(__cpp_lib_char8_t)
    {
        ASIO2_TEST_IOSTREAM << "LC_CTYPE: " << ensure_char_pointer_valid(std::getenv("LC_CTYPE")) << std::endl;
        ASIO2_TEST_IOSTREAM << "LC_ALL  : " << ensure_char_pointer_valid(std::getenv("LC_ALL")) << std::endl;
        ASIO2_TEST_IOSTREAM << "LANG    : " << ensure_char_pointer_valid(std::getenv("LANG")) << std::endl;
        ASIO2_TEST_IOSTREAM << "get_system_locale : " << asio2::get_system_locale() << std::endl;
        ASIO2_TEST_IOSTREAM << "get_codecvt_locale: " << asio2::get_codecvt_locale() << std::endl;
    }

    // wide char and ansi char convert
    // if the locale has gbk envirment
    if (asio2::get_codecvt_locale() == ".936")
    {
        auto* p = new asio2::codecvt_byname<wchar_t, char, std::mbstate_t>("chs");
        std::wstring str;
        str += 'H';
        str += 'i';
        str += ',';
        str += 20320;
        str += 22909;
        str += 65281;
        asio2::wstring_convert<asio2::codecvt<wchar_t, char, std::mbstate_t>> conv(p);
        std::string narrowStr = conv.to_bytes(str);
        std::wstring wideStr = conv.from_bytes(narrowStr);
        ASIO2_CHECK(wideStr == str);
    }

    // wcstombs mbstowcs
    {
        std::wstring str;
        str += 'H';
        str += 'i';
        str += ',';
        str += 20320;
        str += 22909;
        str += 65281;
        std::string mstr = asio2::wcstombs(str);
        std::wstring wstr = asio2::mbstowcs(mstr);
        ASIO2_CHECK(wstr == str);
    }

    // gbk_to_utf8 utf8_to_gbk
    if (asio2::get_codecvt_locale() == ".936")
    {
        std::u8string utf8 = u8"z\u6c34\u6c49";
        std::string g = asio2::utf8_to_gbk(utf8);
        std::string u = asio2::gbk_to_utf8(g);
        ASIO2_CHECK(u.size() == utf8.size());
        for (std::size_t i = 0; i < u.size(); ++i)
        {
            ASIO2_CHECK(
                static_cast<std::make_unsigned_t<char>>(u[i]) ==
                static_cast<std::make_unsigned_t<char8_t>>(utf8[i]));
        }
    }

    // https://en.cppreference.com/w/cpp/locale/codecvt_utf8
    // on visual studio 2017, when include char16_t, will cause compile error.
#if defined(_MSC_VER) && (_MSC_VER > 1916)
    {
        // UTF-8 data. The character U+1d10b, musical sign segno, does not fit in UCS-2
        std::u8string utf8 = u8"z\u6c34\U0001d10b";
        std::u16string u16 = u"z\u6c34\U0001d10b";

        // the UTF-8 / UTF-16 standard conversion facet
        asio2::wstring_convert<asio2::codecvt_utf8_utf16<char16_t, char8_t>, char16_t, char8_t> utf16conv;
        std::u16string utf16 = utf16conv.from_bytes(utf8);
        ASIO2_CHECK(u16 == utf16);

        // the UTF-8 / UCS-2 standard conversion facet
        asio2::wstring_convert<asio2::codecvt_utf8<char16_t, char8_t>, char16_t, char8_t> ucs2conv;
        try
        {
            std::u16string ucs2 = ucs2conv.from_bytes(utf8);
            ASIO2_CHECK(ucs2.size() == 3);
            ASIO2_CHECK(ucs2[0] == 'z');
            ASIO2_CHECK(ucs2[1] == 0x6c34);
            ASIO2_CHECK(ucs2[2] == 0xd10b);
        }
        catch (const std::range_error&)
        {
            //std::u16string ucs2 = ucs2conv.from_bytes(utf8.substr(0, ucs2conv.converted()));
        }
    }
#endif

    // https://en.cppreference.com/w/cpp/locale/codecvt_utf16
    {
        // UTF-16le data (if host system is little-endian)
        char16_t utf16le[4] = { 0x007a,          // latin small letter 'z' U+007a
                               0x6c34,          // CJK ideograph "water"  U+6c34
                               0xd834, 0xdd0b }; // musical sign segno U+1d10b    

        // store in a file
        std::ofstream fout("text.txt");
        fout.write(reinterpret_cast<char*>(utf16le), sizeof utf16le);
        fout.close();

        // open as a byte stream
        std::wifstream fin("text.txt", std::ios::binary);
        // apply facet
        fin.imbue(std::locale(fin.getloc(),
            new asio2::codecvt_utf16<wchar_t, char, 0x10ffff, asio2::little_endian>));

        std::wstring str;

        wchar_t c = 0;
        for (; fin.get(c); str += c);

        ASIO2_CHECK(str.size() == 3);
        ASIO2_CHECK(str[0] == 0x7a);
        ASIO2_CHECK(str[1] == 0x6c34);
        if constexpr (sizeof(wchar_t) == 2)
            ASIO2_CHECK(str[2] == 0xd10b); // chinese windows, why ?
        else // sizeof(wchar_t) == 4
            ASIO2_CHECK(str[2] == 0x1d10b); // wsl ubuntu
    }

    // https://en.cppreference.com/w/cpp/locale/codecvt_utf8_utf16
#if defined(_MSC_VER) && (_MSC_VER > 1916)
    {
        std::u8string u8 = u8"z\u00df\u6c34\U0001f34c";
        std::u16string u16 = u"z\u00df\u6c34\U0001f34c";

        // UTF-8 to UTF-16/char16_t
        std::u16string u16_conv = asio2::wstring_convert<
            asio2::codecvt_utf8_utf16<char16_t, char8_t>, char16_t, char8_t>{}.from_bytes(u8);
        ASIO2_CHECK(u16 == u16_conv);

        // UTF-16/char16_t to UTF-8
        std::u8string u8_conv = asio2::wstring_convert<
            asio2::codecvt_utf8_utf16<char16_t, char8_t>, char16_t, char8_t>{}.to_bytes(u16);
        ASIO2_CHECK(u8 == u8_conv);
    }
#endif

    // https://en.cppreference.com/w/cpp/locale/wbuffer_convert/wbuffer_convert
    if (asio2::get_codecvt_locale() == ".936")
    {
        const char* filepathutf8 = "../../test/codecvt_utf8.json";
        const char* filepathgbk = "../../test/codecvt_gbk.json";
        std::fstream fileutf8(filepathutf8, std::ios::in | std::ios::binary);
        std::fstream filegbk(filepathgbk, std::ios::in | std::ios::binary);
        if (fileutf8 && filegbk)
        {
            std::string contentutf8, contentgbk;
            contentutf8.resize(std::filesystem::file_size(filepathutf8));
            contentgbk.resize(std::filesystem::file_size(filepathgbk));
            fileutf8.read(contentutf8.data(), contentutf8.size());
            filegbk.read(contentgbk.data(), contentgbk.size());

            // wrap a UTF-8 string stream in a UCS4 wbuffer_convert
            std::stringbuf utf8buf(contentutf8);  // or 
            // or "\x7a\xc3\x9f\xe6\xb0\xb4\xf0\x9f\x8d\x8c";
            asio2::wbuffer_convert<asio2::codecvt_utf8<wchar_t>> conv_in(&utf8buf);
            std::wistream ucsbuf(&conv_in);
            std::ostringstream ucsout;
            for (wchar_t c; ucsbuf.get(c); )
                ucsout << std::hex << std::showbase << int(c) << '\n';

            // wrap a UTF-8 aware std::cout in a UCS4 wbuffer_convert to output UCS4
            asio2::wbuffer_convert<asio2::codecvt_utf8<wchar_t>> conv_out(ucsout.rdbuf());
            std::wostream out(&conv_out);
            out << L"z\u00df\u6c34\U0001f34c\n";
        }
    }

    if (asio2::get_codecvt_locale() == ".936")
    {
        const char* filepathutf8 = "../../test/codecvt_utf8.json";
        const char* filepathgbk = "../../test/codecvt_gbk.json";
        std::fstream fileutf8(filepathutf8, std::ios::in | std::ios::binary);
        std::fstream filegbk(filepathgbk, std::ios::in | std::ios::binary);
        if (fileutf8 && filegbk)
        {
            std::string contentutf8, contentgbk;
            contentutf8.resize(std::filesystem::file_size(filepathutf8));
            contentgbk.resize(std::filesystem::file_size(filepathgbk));
            fileutf8.read(contentutf8.data(), contentutf8.size());
            filegbk.read(contentgbk.data(), contentgbk.size());
            std::string g = asio2::utf8_to_gbk(contentutf8);
            ASIO2_CHECK(g == contentgbk);
            std::string u = asio2::gbk_to_utf8(contentgbk);
            ASIO2_CHECK(u == contentutf8);
        }
    }

    if (asio2::get_codecvt_locale() == ".936")
    {
        const char* filepathutf8 = "../../test/codecvt_utf8.json";
        const char* filepathgbk = "../../test/codecvt_gbk.json";
        std::fstream fileutf8(filepathutf8, std::ios::in | std::ios::binary);
        std::fstream filegbk(filepathgbk, std::ios::in | std::ios::binary);
        if (fileutf8 && filegbk)
        {
            std::string contentutf8, contentgbk;
            contentutf8.resize(std::filesystem::file_size(filepathutf8));
            contentgbk.resize(std::filesystem::file_size(filepathgbk));
            fileutf8.read(contentutf8.data(), contentutf8.size());
            filegbk.read(contentgbk.data(), contentgbk.size());
            std::string g = asio2::utf8_to_locale(contentutf8);
            ASIO2_CHECK(g == contentgbk);
            std::string u = asio2::locale_to_utf8(contentgbk);
            ASIO2_CHECK(u == contentutf8);
        }
    }
#else
	{
		ASIO2_TEST_IOSTREAM << "LC_CTYPE: " << ensure_char_pointer_valid(std::getenv("LC_CTYPE")) << std::endl;
		ASIO2_TEST_IOSTREAM << "LC_ALL  : " << ensure_char_pointer_valid(std::getenv("LC_ALL")) << std::endl;
		ASIO2_TEST_IOSTREAM << "LANG    : " << ensure_char_pointer_valid(std::getenv("LANG")) << std::endl;
		ASIO2_TEST_IOSTREAM << "get_system_locale : " << asio2::get_system_locale() << std::endl;
		ASIO2_TEST_IOSTREAM << "get_codecvt_locale: " << asio2::get_codecvt_locale() << std::endl;
	}

    // wide char and ansi char convert
    // if the locale has gbk envirment
    if (asio2::get_codecvt_locale() == ".936")
	{
		auto* p = new asio2::codecvt_byname<wchar_t, char, std::mbstate_t>("chs");
        std::wstring str;
        str += 'H';
        str += 'i';
        str += ',';
        str += 20320;
        str += 22909;
        str += 65281;
		asio2::wstring_convert<asio2::codecvt<wchar_t, char, std::mbstate_t>> conv(p);
		std::string narrowStr = conv.to_bytes(str);
		std::wstring wideStr = conv.from_bytes(narrowStr);
		ASIO2_CHECK(wideStr == str);
	}

    // wcstombs mbstowcs
	{
        std::wstring str;
        str += 'H';
        str += 'i';
        str += ',';
        str += 20320;
        str += 22909;
        str += 65281;
        std::string mstr = asio2::wcstombs(str);
        std::wstring wstr = asio2::mbstowcs(mstr);
		ASIO2_CHECK(wstr == str);
	}

    // gbk_to_utf8 utf8_to_gbk
    if (asio2::get_codecvt_locale() == ".936")
	{
        std::string utf8 = u8"z\u6c34\u6c49";
        std::string g = asio2::utf8_to_gbk(utf8);
        std::string u = asio2::gbk_to_utf8(g);
		ASIO2_CHECK(u == utf8);
	}

    // https://en.cppreference.com/w/cpp/locale/codecvt_utf8
#if defined(_MSC_VER) && (_MSC_VER > 1916)
    {
        // UTF-8 data. The character U+1d10b, musical sign segno, does not fit in UCS-2
        std::string utf8 = u8"z\u6c34\U0001d10b";
        std::u16string u16 = u"z\u6c34\U0001d10b";

        // the UTF-8 / UTF-16 standard conversion facet
        asio2::wstring_convert<asio2::codecvt_utf8_utf16<char16_t>, char16_t> utf16conv;
        std::u16string utf16 = utf16conv.from_bytes(utf8);
        ASIO2_CHECK(u16 == utf16);

        // the UTF-8 / UCS-2 standard conversion facet
        asio2::wstring_convert<asio2::codecvt_utf8<char16_t>, char16_t> ucs2conv;
        try
        {
            std::u16string ucs2 = ucs2conv.from_bytes(utf8);
            ASIO2_CHECK(ucs2.size() == 3);
            ASIO2_CHECK(ucs2[0] == 'z');
            ASIO2_CHECK(ucs2[1] == 0x6c34);
            ASIO2_CHECK(ucs2[2] == 0xd10b);
        }
        catch (const std::range_error&)
        {
            //std::u16string ucs2 = ucs2conv.from_bytes(utf8.substr(0, ucs2conv.converted()));
        }
    }
#endif

    // https://en.cppreference.com/w/cpp/locale/codecvt_utf16
	{
		// UTF-16le data (if host system is little-endian)
		char16_t utf16le[4] = { 0x007a,          // latin small letter 'z' U+007a
							   0x6c34,          // CJK ideograph "water"  U+6c34
							   0xd834, 0xdd0b }; // musical sign segno U+1d10b    

		// store in a file
		std::ofstream fout("text.txt");
		fout.write(reinterpret_cast<char*>(utf16le), sizeof utf16le);
		fout.close();

		// open as a byte stream
		std::wifstream fin("text.txt", std::ios::binary);
		// apply facet
		fin.imbue(std::locale(fin.getloc(),
			new asio2::codecvt_utf16<wchar_t, char, 0x10ffff, asio2::little_endian>));

		std::wstring str;

		wchar_t c = 0;
		for (; fin.get(c); str += c);

		ASIO2_CHECK(str.size() == 3);
		ASIO2_CHECK(str[0] == 0x7a);
		ASIO2_CHECK(str[1] == 0x6c34);
		if constexpr (sizeof(wchar_t) == 2)
			ASIO2_CHECK_VALUE(str[2], str[2] == 0xd10b); // chinese windows, why ?
		else // sizeof(wchar_t) == 4
			ASIO2_CHECK_VALUE(str[2], str[2] == 0x1d10b); // wsl ubuntu
	}

    // https://en.cppreference.com/w/cpp/locale/codecvt_utf8_utf16
#if defined(_MSC_VER) && (_MSC_VER > 1916)
    {
        std::string u8 = u8"z\u00df\u6c34\U0001f34c";
        std::u16string u16 = u"z\u00df\u6c34\U0001f34c";

        // UTF-8 to UTF-16/char16_t
        std::u16string u16_conv = asio2::wstring_convert<
            asio2::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(u8);
        ASIO2_CHECK(u16 == u16_conv);

        // UTF-16/char16_t to UTF-8
        std::string u8_conv = asio2::wstring_convert<
            asio2::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(u16);
        ASIO2_CHECK(u8 == u8_conv);
    }
#endif

    // https://en.cppreference.com/w/cpp/locale/wbuffer_convert/wbuffer_convert
    {
        // wrap a UTF-8 string stream in a UCS4 wbuffer_convert
        std::stringbuf utf8buf(u8"z\u00df\u6c34\U0001f34c");  // or 
        // or "\x7a\xc3\x9f\xe6\xb0\xb4\xf0\x9f\x8d\x8c";
        asio2::wbuffer_convert<asio2::codecvt_utf8<wchar_t>> conv_in(&utf8buf);
        std::wistream ucsbuf(&conv_in);
        for (wchar_t c; ucsbuf.get(c); )
            std::cout << std::hex << std::showbase << c << '\n';

        // wrap a UTF-8 aware std::cout in a UCS4 wbuffer_convert to output UCS4
        asio2::wbuffer_convert<asio2::codecvt_utf8<wchar_t>> conv_out(std::cout.rdbuf());
        std::wostream out(&conv_out);
        out << L"z\u00df\u6c34\U0001f34c\n";
    }

    if (asio2::get_codecvt_locale() == ".936")
    {
        const char* filepathutf8 = "../../test/codecvt_utf8.json";
        const char* filepathgbk = "../../test/codecvt_gbk.json";
        std::fstream fileutf8(filepathutf8, std::ios::in | std::ios::binary);
        std::fstream filegbk(filepathgbk, std::ios::in | std::ios::binary);
        if (fileutf8 && filegbk)
        {
            std::string contentutf8, contentgbk;
            contentutf8.resize(std::filesystem::file_size(filepathutf8));
            contentgbk.resize(std::filesystem::file_size(filepathgbk));
            fileutf8.read(contentutf8.data(), contentutf8.size());
            filegbk.read(contentgbk.data(), contentgbk.size());
            std::string g = asio2::utf8_to_gbk(contentutf8);
            ASIO2_CHECK(g == contentgbk);
            std::string u = asio2::gbk_to_utf8(contentgbk);
            ASIO2_CHECK(u == contentutf8);
        }
    }

    if (asio2::get_codecvt_locale() == ".936")
    {
        const char* filepathutf8 = "../../test/codecvt_utf8.json";
        const char* filepathgbk = "../../test/codecvt_gbk.json";
        std::fstream fileutf8(filepathutf8, std::ios::in | std::ios::binary);
        std::fstream filegbk(filepathgbk, std::ios::in | std::ios::binary);
        if (fileutf8 && filegbk)
        {
            std::string contentutf8, contentgbk;
            contentutf8.resize(std::filesystem::file_size(filepathutf8));
            contentgbk.resize(std::filesystem::file_size(filepathgbk));
            fileutf8.read(contentutf8.data(), contentutf8.size());
            filegbk.read(contentgbk.data(), contentgbk.size());
            std::string g = asio2::utf8_to_locale(contentutf8);
            ASIO2_CHECK(g == contentgbk);
            std::string u = asio2::locale_to_utf8(contentgbk);
            ASIO2_CHECK(u == contentutf8);
        }
    }
#endif

	//ASIO2_TEST_BEGIN_LOOP(test_loop_times);

	//ASIO2_TEST_END_LOOP;
}
#else
void codecvt_test()
{
}
#endif

ASIO2_TEST_SUITE
(
	"codecvt",
	ASIO2_TEST_CASE(codecvt_test)
)
