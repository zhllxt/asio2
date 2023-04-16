/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * /Microsoft Visual Studio/2022/Enterprise/VC/Tools/MSVC/14.33.31629/include/codecvt
 * 
 */

#ifndef __ASIO2_CODECVT_HPP__
#define __ASIO2_CODECVT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <cstdlib>

#include <locale>
#include <codecvt>

#include <asio2/external/predef.h>
#include <asio2/external/asio.hpp>

#include <asio2/util/string.hpp>

#if defined(ASIO2_OS_WINDOWS) || defined(__CYGWIN__)
#  if __has_include(<Windows.h>)
#    include <Windows.h>
#    ifndef ASIO2_LOCALE_USE_WIN32_API
#    define ASIO2_LOCALE_USE_WIN32_API
#    endif
#  endif
#endif

namespace asio2
{
using codecvt_base = std::codecvt_base;

template <class _Elem, class _Byte, class _Statype>
using codecvt = std::codecvt<_Elem, _Byte, _Statype>;

template <class _Elem, class _Byte, class _Statype>
using codecvt_byname = std::codecvt_byname<_Elem, _Byte, _Statype>;

namespace detail
{
inline constexpr int _Codecvt_Little_first = 1;
inline constexpr int _Codecvt_Big_first    = 2;

template <class _CvtTy, class _Byte, class _Statype>
[[nodiscard]] int _Codecvt_do_length(
    const _CvtTy& _Cvt, _Statype& _State, const _Byte* _First1, const _Byte* _Last1, std::size_t _Count) {
    // return p - _First1, for the largest value p in [_First1, _Last1] such that _Cvt will successfully convert
    // [_First1, p) to at most _Count wide characters

    using _Elem = typename _CvtTy::intern_type;

    const auto _Old_first1 = _First1;

    while (_Count > 0 && _First1 != _Last1) { // convert another wide character
        const _Byte* _Mid1;
        _Elem* _Mid2;
        _Elem _Ch;

        // test result of single widechar conversion

        const auto _Result = _Cvt._CvtTy::do_in(_State, _First1, _Last1, _Mid1, &_Ch, &_Ch + 1, _Mid2);

        if (_Result != std::codecvt_base::ok) {
            if (_Result == std::codecvt_base::noconv) {
                _First1 += (std::min)(static_cast<std::size_t>(_Last1 - _First1), _Count);
            }

            break; // error, noconv, or partial
        }

        if (_Mid2 == &_Ch + 1) {
            --_Count; // do_in converted an output character
        }

        _First1 = _Mid1;
    }

    return static_cast<int>((std::min)(_First1 - _Old_first1, std::ptrdiff_t{(std::numeric_limits<int>::max)()}));
}
}

enum codecvt_mode { consume_header = 4, generate_header = 2, little_endian = 1 };

template <
    class _Elem,
    class CharT = char,
    unsigned long _Mymax = 0x10ffff,
    asio2::codecvt_mode _Mymode = asio2::codecvt_mode{}>
class codecvt_utf8 : public std::codecvt<_Elem, CharT, std::mbstate_t> {
    // facet for converting between _Elem and UTF-8 byte sequences
public:
    using _Mybase     = std::codecvt<_Elem, CharT, std::mbstate_t>;
    using result      = typename _Mybase::result;
    using _Byte       = CharT;
    using intern_type = _Elem;
    using extern_type = _Byte;
    using state_type  = std::mbstate_t;

    explicit codecvt_utf8(std::size_t _Refs = 0) : _Mybase(_Refs) {}

    ~codecvt_utf8() noexcept override {}

protected:
    result do_in(std::mbstate_t& _State, const _Byte* _First1, const _Byte* _Last1, const _Byte*& _Mid1,
        _Elem* _First2, _Elem* _Last2, _Elem*& _Mid2) const override {
        // convert bytes [_First1, _Last1) to [_First2, _Last2)
        std::int8_t* _Pstate = reinterpret_cast<std::int8_t*>(&_State);
        _Mid1         = _First1;
        _Mid2         = _First2;

        while (_Mid1 != _Last1 && _Mid2 != _Last2) { // convert a multibyte sequence
            unsigned long _By = static_cast<std::make_unsigned_t<CharT>>(*_Mid1);
            unsigned long _Ch;
            int _Nextra;

            if (_By < 0x80u) {
                _Ch     = _By;
                _Nextra = 0;
            } else if (_By < 0xc0u) { // 0x80-0xbf not first byte
                ++_Mid1;
                return _Mybase::error;
            } else if (_By < 0xe0u) {
                _Ch     = _By & 0x1f;
                _Nextra = 1;
            } else if (_By < 0xf0u) {
                _Ch     = _By & 0x0f;
                _Nextra = 2;
            } else if (_By < 0xf8u) {
                _Ch     = _By & 0x07;
                _Nextra = 3;
            } else {
                _Ch     = _By & 0x03;
                _Nextra = _By < 0xfc ? 4 : 5;
            }

            if (_Nextra == 0) {
                ++_Mid1;
            } else if (_Last1 - _Mid1 < _Nextra + 1) {
                break; // not enough input
            } else {
                for (++_Mid1; 0 < _Nextra; --_Nextra, ++_Mid1) {
                    if ((_By = static_cast<std::make_unsigned_t<CharT>>(*_Mid1)) < 0x80u || 0xc0u <= _By) {
                        return _Mybase::error; // not continuation byte
                    } else {
                        _Ch = _Ch << 6 | (_By & 0x3f);
                    }
                }
            }

            if (*_Pstate == 0) { // first time, maybe look for and consume header
                *_Pstate = 1;

                constexpr bool _Consuming = (_Mymode & consume_header) != 0;
                if constexpr (_Consuming) {
                    if (_Ch == 0xfeff) { // drop header and retry
                        const result _Ans = do_in(_State, _Mid1, _Last1, _Mid1, _First2, _Last2, _Mid2);

                        if (_Ans == _Mybase::partial) { // roll back header determination
                            *_Pstate = 0;
                            _Mid1    = _First1;
                        }

                        return _Ans;
                    }
                }
            }

            if (_Mymax < _Ch) {
                return _Mybase::error; // code too large
            }

            *_Mid2++ = static_cast<_Elem>(_Ch);
        }

        return _First1 == _Mid1 ? _Mybase::partial : _Mybase::ok;
    }

    result do_out(std::mbstate_t& _State, const _Elem* _First1, const _Elem* _Last1, const _Elem*& _Mid1,
        _Byte* _First2, _Byte* _Last2, _Byte*& _Mid2) const override {
        // convert [_First1, _Last1) to bytes [_First2, _Last2)
        std::int8_t* _Pstate = reinterpret_cast<std::int8_t*>(&_State);
        _Mid1         = _First1;
        _Mid2         = _First2;

        while (_Mid1 != _Last1 && _Mid2 != _Last2) { // convert and put a widechar
            _Byte _By;
            int _Nextra;
            unsigned long _Ch = static_cast<unsigned long>(*_Mid1);

            if (_Mymax < _Ch) {
                return _Mybase::error;
            }

            if (_Ch < 0x0080u) {
                _By     = static_cast<_Byte>(_Ch);
                _Nextra = 0;
            } else if (_Ch < 0x0800u) {
                _By     = static_cast<_Byte>(0xc0 | _Ch >> 6);
                _Nextra = 1;
            } else if (_Ch < 0x00010000u) {
                _By     = static_cast<_Byte>(0xe0 | _Ch >> 12);
                _Nextra = 2;
            } else if (_Ch < 0x00200000u) {
                _By     = static_cast<_Byte>(0xf0 | _Ch >> 18);
                _Nextra = 3;
            } else if (_Ch < 0x04000000u) {
                _By     = static_cast<_Byte>(0xf8 | _Ch >> 24);
                _Nextra = 4;
            } else {
                _By     = static_cast<_Byte>(0xfc | (_Ch >> 30 & 0x03));
                _Nextra = 5;
            }

            if (*_Pstate == 0) { // first time, maybe generate header
                *_Pstate                   = 1;
                constexpr bool _Generating = (_Mymode & generate_header) != 0;
                if constexpr (_Generating) {
                    if (_Last2 - _Mid2 < 3 + 1 + _Nextra) {
                        return _Mybase::partial; // not enough room for both
                    }

                    // prepend header
                    *_Mid2++ = '\xef';
                    *_Mid2++ = '\xbb';
                    *_Mid2++ = '\xbf';
                }
            }

            if (_Last2 - _Mid2 < 1 + _Nextra) {
                break; // not enough room for output
            }

            ++_Mid1;
            for (*_Mid2++ = _By; 0 < _Nextra;) {
                *_Mid2++ = static_cast<_Byte>((_Ch >> 6 * --_Nextra & 0x3f) | 0x80);
            }
        }

        return _First1 == _Mid1 ? _Mybase::partial : _Mybase::ok;
    }

    result do_unshift(std::mbstate_t&, _Byte* _First2, _Byte*, _Byte*& _Mid2) const override {
        // generate bytes to return to default shift state
        _Mid2 = _First2;
        return _Mybase::noconv;
    }

    friend int detail::_Codecvt_do_length<>(const codecvt_utf8&, std::mbstate_t&, const _Byte*, const _Byte*, std::size_t);

    int do_length(
        std::mbstate_t& _State, const _Byte* _First1, const _Byte* _Last1, std::size_t _Count) const noexcept override {
        return detail::_Codecvt_do_length(*this, _State, _First1, _Last1, _Count);
    }

    bool do_always_noconv() const noexcept override {
        // return true if conversions never change input
        return false;
    }

    int do_max_length() const noexcept override {
        // return maximum length required for a conversion
        if constexpr ((_Mymode & (consume_header | generate_header)) != 0) {
            return 9;
        } else {
            return 6;
        }
    }

    int do_encoding() const noexcept override {
        // return length of code sequence (from codecvt)
        if constexpr ((_Mymode & (consume_header | generate_header)) != 0) {
            return -1; // -1 => state dependent
        } else {
            return 0; // 0 => varying length
        }
    }
};

template <
    class _Elem,
    class CharT = char,
    unsigned long _Mymax = 0x10ffff,
    asio2::codecvt_mode _Mymode = asio2::codecvt_mode{}>
class codecvt_utf16 : public std::codecvt<_Elem, CharT, std::mbstate_t> {
    // facet for converting between _Elem and UTF-16 multibyte sequences
private:
    enum { _Bytes_per_word = 2 };

public:
    using _Mybase     = std::codecvt<_Elem, CharT, std::mbstate_t>;
    using result      = typename _Mybase::result;
    using _Byte       = CharT;
    using intern_type = _Elem;
    using extern_type = _Byte;
    using state_type  = std::mbstate_t;

    explicit codecvt_utf16(std::size_t _Refs = 0) : _Mybase(_Refs) {}

    ~codecvt_utf16() noexcept override {}

protected:
    result do_in(std::mbstate_t& _State, const _Byte* _First1, const _Byte* _Last1, const _Byte*& _Mid1,
        _Elem* _First2, _Elem* _Last2, _Elem*& _Mid2) const override {
        // convert bytes [_First1, _Last1) to [_First2, _Last2)
        std::int8_t* _Pstate = reinterpret_cast<std::int8_t*>(&_State);
        _Mid1         = _First1;
        _Mid2         = _First2;

        while (_Bytes_per_word <= _Last1 - _Mid1 && _Mid2 != _Last2) { // convert a multibyte sequence
            const auto _Ptr = reinterpret_cast<const std::make_unsigned_t<CharT>*>(_Mid1);
            unsigned long _Ch;
            unsigned short _Ch0;
            unsigned short _Ch1;

            if (*_Pstate == detail::_Codecvt_Little_first) {
                _Ch0 = static_cast<unsigned short>(_Ptr[1] << 8 | _Ptr[0]);
            } else if (*_Pstate == detail::_Codecvt_Big_first) {
                _Ch0 = static_cast<unsigned short>(_Ptr[0] << 8 | _Ptr[1]);
            } else { // no header seen yet, try preferred mode
                constexpr bool _Prefer_LE      = (_Mymode & little_endian) != 0;
                constexpr std::int8_t _Default_endian = _Prefer_LE ? detail::_Codecvt_Little_first : detail::_Codecvt_Big_first;

                if constexpr (_Prefer_LE) {
                    _Ch0 = static_cast<unsigned short>(_Ptr[1] << 8 | _Ptr[0]);
                } else {
                    _Ch0 = static_cast<unsigned short>(_Ptr[0] << 8 | _Ptr[1]);
                }

                *_Pstate                  = _Default_endian;
                constexpr bool _Consuming = (_Mymode & consume_header) != 0;
                if constexpr (_Consuming) {
                    if (_Ch0 == 0xfffeu) {
                        *_Pstate = 3 - _Default_endian;
                    }

                    if (_Ch0 == 0xfffeu || _Ch0 == 0xfeffu) { // consume header, fixate on endianness, and retry
                        _Mid1 += _Bytes_per_word;
                        result _Ans = do_in(_State, _Mid1, _Last1, _Mid1, _First2, _Last2, _Mid2);

                        if (_Ans == _Mybase::partial) { // not enough bytes, roll back header
                            *_Pstate = 0;
                            _Mid1    = _First1;
                        }

                        return _Ans;
                    }
                }
            }

            if (_Ch0 < 0xd800u || 0xdc00u <= _Ch0) { // one word, consume bytes
                _Mid1 += _Bytes_per_word;
                _Ch = _Ch0;
            } else if (_Last1 - _Mid1 < 2 * _Bytes_per_word) {
                break;
            } else { // get second word
                if (*_Pstate == detail::_Codecvt_Little_first) {
                    _Ch1 = static_cast<unsigned short>(_Ptr[3] << 8 | _Ptr[2]);
                } else {
                    _Ch1 = static_cast<unsigned short>(_Ptr[2] << 8 | _Ptr[3]);
                }

                if (_Ch1 < 0xdc00u || 0xe000u <= _Ch1) {
                    return _Mybase::error;
                }

                _Mid1 += 2 * _Bytes_per_word;
                _Ch = static_cast<unsigned long>(_Ch0 - 0xd800 + 0x0040) << 10 | (_Ch1 - 0xdc00);
            }

            if (_Mymax < _Ch) {
                return _Mybase::error; // code too large
            }

            *_Mid2++ = static_cast<_Elem>(_Ch);
        }

        return _First1 == _Mid1 ? _Mybase::partial : _Mybase::ok;
    }

    result do_out(std::mbstate_t& _State, const _Elem* _First1, const _Elem* _Last1, const _Elem*& _Mid1,
        _Byte* _First2, _Byte* _Last2, _Byte*& _Mid2) const override {
        // convert [_First1, _Last1) to bytes [_First2, _Last2)
        std::int8_t* _Pstate = reinterpret_cast<std::int8_t*>(&_State);
        _Mid1         = _First1;
        _Mid2         = _First2;

        if (*_Pstate == 0) { // determine endianness once, maybe generate header
            if constexpr ((_Mymode & little_endian) != 0) {
                *_Pstate = detail::_Codecvt_Little_first;
            } else {
                *_Pstate = detail::_Codecvt_Big_first;
            }

            constexpr bool _Generating = (_Mymode & generate_header) != 0;
            if constexpr (_Generating) {
                if (_Last2 - _Mid2 < 3 * _Bytes_per_word) {
                    return _Mybase::partial; // not enough room for all
                }

                if (*_Pstate == detail::_Codecvt_Little_first) { // put header LS byte first
                    *_Mid2++ = '\xff';
                    *_Mid2++ = '\xfe';
                } else { // put header MS byte first
                    *_Mid2++ = '\xfe';
                    *_Mid2++ = '\xff';
                }
            }
        }

        while (_Mid1 != _Last1 && _Bytes_per_word <= _Last2 - _Mid2) { // convert and put a widechar
            bool _Extra       = false;
            unsigned long _Ch = static_cast<unsigned long>(*_Mid1++);

            if ((_Mymax < 0x10ffffu ? _Mymax : 0x10ffffu) < _Ch) {
                return _Mybase::error; // value too large
            }

            if (_Ch <= 0xffffu) { // one word, can't be code for first of two
                if (0xd800u <= _Ch && _Ch < 0xdc00u) {
                    return _Mybase::error;
                }
            } else if (_Last2 - _Mid2 < 2 * _Bytes_per_word) { // not enough room for two-word output, back up
                --_Mid1;
                return _Mybase::partial;
            } else {
                _Extra = true;
            }

            if (*_Pstate == detail::_Codecvt_Little_first) {
                if (_Extra) { // put a pair of words LS byte first
                    unsigned short _Ch0 =
                        static_cast<unsigned short>(0xd800 | (static_cast<unsigned short>(_Ch >> 10) - 0x0040));
                    *_Mid2++ = static_cast<_Byte>(_Ch0);
                    *_Mid2++ = static_cast<_Byte>(_Ch0 >> 8);

                    _Ch0     = static_cast<unsigned short>(0xdc00 | (static_cast<unsigned short>(_Ch) & 0x03ff));
                    *_Mid2++ = static_cast<_Byte>(_Ch0);
                    *_Mid2++ = static_cast<_Byte>(_Ch0 >> 8);
                } else { // put a single word LS byte first
                    *_Mid2++ = static_cast<_Byte>(_Ch);
                    *_Mid2++ = static_cast<_Byte>(_Ch >> 8);
                }
            } else {
                if (_Extra) { // put a pair of words MS byte first
                    unsigned short _Ch0 =
                        static_cast<unsigned short>(0xd800 | (static_cast<unsigned short>(_Ch >> 10) - 0x0040));
                    *_Mid2++ = static_cast<_Byte>(_Ch0 >> 8);
                    *_Mid2++ = static_cast<_Byte>(_Ch0);

                    _Ch0     = static_cast<unsigned short>(0xdc00 | (static_cast<unsigned short>(_Ch) & 0x03ff));
                    *_Mid2++ = static_cast<_Byte>(_Ch0 >> 8);
                    *_Mid2++ = static_cast<_Byte>(_Ch0);
                } else { // put a single word MS byte first
                    *_Mid2++ = static_cast<_Byte>(_Ch >> 8);
                    *_Mid2++ = static_cast<_Byte>(_Ch);
                }
            }
        }

        return _First1 == _Mid1 ? _Mybase::partial : _Mybase::ok;
    }

    result do_unshift(std::mbstate_t&, _Byte* _First2, _Byte*, _Byte*& _Mid2) const override {
        // generate bytes to return to default shift state
        _Mid2 = _First2;
        return _Mybase::noconv;
    }

    friend int detail::_Codecvt_do_length<>(const codecvt_utf16&, std::mbstate_t&, const _Byte*, const _Byte*, std::size_t);

    int do_length(
        std::mbstate_t& _State, const _Byte* _First1, const _Byte* _Last1, std::size_t _Count) const noexcept override {
        return detail::_Codecvt_do_length(*this, _State, _First1, _Last1, _Count);
    }

    bool do_always_noconv() const noexcept override {
        // return true if conversions never change input
        return false;
    }

    int do_max_length() const noexcept override {
        // return maximum length required for a conversion
        if constexpr ((_Mymode & (consume_header | generate_header)) != 0) {
            return 3 * _Bytes_per_word;
        } else {
            return 6 * _Bytes_per_word;
        }
    }

    int do_encoding() const noexcept override {
        // return length of code sequence (from codecvt)
        if constexpr ((_Mymode & (consume_header | generate_header)) != 0) {
            return -1; // -1 => state dependent
        } else {
            return 0; // 0 => varying length
        }
    }
};

template <
    class _Elem,
    class CharT = char,
    unsigned long _Mymax = 0x10ffff,
    asio2::codecvt_mode _Mymode = asio2::codecvt_mode{}>
class codecvt_utf8_utf16
    : public std::codecvt<_Elem, CharT, std::mbstate_t> { // facet for converting between UTF-16 _Elem and UTF-8 byte sequences
public:
    using _Mybase     = std::codecvt<_Elem, CharT, std::mbstate_t>;
    using result      = typename _Mybase::result;
    using _Byte       = CharT;
    using intern_type = _Elem;
    using extern_type = _Byte;
    using state_type  = std::mbstate_t;

    static_assert(sizeof(unsigned short) <= sizeof(state_type), "state_type too small");

    explicit codecvt_utf8_utf16(std::size_t _Refs = 0) : _Mybase(_Refs) {}

    ~codecvt_utf8_utf16() noexcept override {}

protected:
    result do_in(std::mbstate_t& _State, const _Byte* _First1, const _Byte* _Last1, const _Byte*& _Mid1,
        _Elem* _First2, _Elem* _Last2, _Elem*& _Mid2) const override {
        // convert bytes [_First1, _Last1) to [_First2, _Last2)
        unsigned short* _Pstate = reinterpret_cast<unsigned short*>(&_State);
        _Mid1                   = _First1;
        _Mid2                   = _First2;

        while (_Mid1 != _Last1 && _Mid2 != _Last2) { // convert a multibyte sequence
            unsigned long _By = static_cast<std::make_unsigned_t<CharT>>(*_Mid1);
            unsigned long _Ch;
            int _Nextra;
            int _Nskip;

            if (*_Pstate > 1u) {
                if (_By < 0x80u || 0xc0u <= _By) {
                    return _Mybase::error; // not continuation byte
                }

                // deliver second half of two-word value
                ++_Mid1;
                *_Mid2++ = static_cast<_Elem>(*_Pstate | (_By & 0x3f));
                *_Pstate = 1;
                continue;
            }

            if (_By < 0x80u) {
                _Ch     = _By;
                _Nextra = 0;
            } else if (_By < 0xc0u) { // 0x80-0xbf not first byte
                ++_Mid1;
                return _Mybase::error;
            } else if (_By < 0xe0u) {
                _Ch     = _By & 0x1f;
                _Nextra = 1;
            } else if (_By < 0xf0u) {
                _Ch     = _By & 0x0f;
                _Nextra = 2;
            } else if (_By < 0xf8u) {
                _Ch     = _By & 0x07;
                _Nextra = 3;
            } else {
                _Ch     = _By & 0x03;
                _Nextra = _By < 0xfc ? 4 : 5;
            }

            _Nskip  = _Nextra < 3 ? 0 : 1; // leave a byte for 2nd word
            _First1 = _Mid1; // roll back point

            if (_Nextra == 0) {
                ++_Mid1;
            } else if (_Last1 - _Mid1 < _Nextra + 1 - _Nskip) {
                break; // not enough input
            } else {
                for (++_Mid1; _Nskip < _Nextra; --_Nextra, ++_Mid1) {
                    if ((_By = static_cast<std::make_unsigned_t<CharT>>(*_Mid1)) < 0x80u || 0xc0u <= _By) {
                        return _Mybase::error; // not continuation byte
                    }

                    _Ch = _Ch << 6 | (_By & 0x3f);
                }
            }

            if (0 < _Nskip) {
                _Ch <<= 6; // get last byte on next call
            }

            if ((_Mymax < 0x10ffffu ? _Mymax : 0x10ffffu) < _Ch) {
                return _Mybase::error; // value too large
            }

            if (0xffffu < _Ch) { // deliver first half of two-word value, save second word
                unsigned short _Ch0 = static_cast<unsigned short>(0xd800 | ((_Ch >> 10) - 0x0040));

                *_Mid2++ = static_cast<_Elem>(_Ch0);
                *_Pstate = static_cast<unsigned short>(0xdc00 | (_Ch & 0x03ff));
                continue;
            }

            if (_Nskip != 0) {
                if (_Mid1 == _Last1) { // not enough bytes, noncanonical value
                    _Mid1 = _First1;
                    break;
                }

                if ((_By = static_cast<std::make_unsigned_t<CharT>>(*_Mid1++)) < 0x80u || 0xc0u <= _By) {
                    return _Mybase::error; // not continuation byte
                }

                _Ch |= _By & 0x3f; // complete noncanonical value
            }

            if (*_Pstate == 0u) { // first time, maybe look for and consume header
                *_Pstate = 1;

                constexpr bool _Consuming = (_Mymode & consume_header) != 0;
                if constexpr (_Consuming) {
                    if (_Ch == 0xfeffu) { // drop header and retry
                        result _Ans = do_in(_State, _Mid1, _Last1, _Mid1, _First2, _Last2, _Mid2);

                        if (_Ans == _Mybase::partial) { // roll back header determination
                            *_Pstate = 0;
                            _Mid1    = _First1;
                        }

                        return _Ans;
                    }
                }
            }

            *_Mid2++ = static_cast<_Elem>(_Ch);
        }

        return _First1 == _Mid1 ? _Mybase::partial : _Mybase::ok;
    }

    result do_out(std::mbstate_t& _State, const _Elem* _First1, const _Elem* _Last1, const _Elem*& _Mid1,
        _Byte* _First2, _Byte* _Last2, _Byte*& _Mid2) const override {
        // convert [_First1, _Last1) to bytes [_First2, _Last2)
        unsigned short* _Pstate = reinterpret_cast<unsigned short*>(&_State);
        _Mid1                   = _First1;
        _Mid2                   = _First2;

        while (_Mid1 != _Last1 && _Mid2 != _Last2) { // convert and put a widechar
            unsigned long _Ch;
            unsigned short _Ch1 = static_cast<unsigned short>(*_Mid1);
            bool _Save          = false;

            if (1u < *_Pstate) { // get saved MS 11 bits from *_Pstate
                if (_Ch1 < 0xdc00u || 0xe000u <= _Ch1) {
                    return _Mybase::error; // bad second word
                }

                _Ch = static_cast<unsigned long>((*_Pstate << 10) | (_Ch1 - 0xdc00));
            } else if (0xd800u <= _Ch1 && _Ch1 < 0xdc00u) { // get new first word
                _Ch   = static_cast<unsigned long>((_Ch1 - 0xd800 + 0x0040) << 10);
                _Save = true; // put only first byte, rest with second word
            } else {
                _Ch = _Ch1; // not first word, just put it
            }

            _Byte _By;
            int _Nextra;

            if (_Ch < 0x0080u) {
                _By     = static_cast<_Byte>(_Ch);
                _Nextra = 0;
            } else if (_Ch < 0x0800u) {
                _By     = static_cast<_Byte>(0xc0 | _Ch >> 6);
                _Nextra = 1;
            } else if (_Ch < 0x10000u) {
                _By     = static_cast<_Byte>(0xe0 | _Ch >> 12);
                _Nextra = 2;
            } else {
                _By     = static_cast<_Byte>(0xf0 | _Ch >> 18);
                _Nextra = 3;
            }

            int _Nput = _Nextra < 3 ? _Nextra + 1 : _Save ? 1 : 3;

            if (_Last2 - _Mid2 < _Nput) {
                break; // not enough room, even without header
            }

            if constexpr ((_Mymode & generate_header) != 0) { // header to put
                if (*_Pstate == 0u) {
                    if (_Last2 - _Mid2 < 3 + _Nput) {
                        break; // not enough room for header + output
                    }

                    // prepend header
                    *_Mid2++ = '\xef';
                    *_Mid2++ = '\xbb';
                    *_Mid2++ = '\xbf';
                }
            }

            ++_Mid1;
            if (_Save || _Nextra < 3) { // put first byte of sequence, if not already put
                *_Mid2++ = _By;
                --_Nput;
            }

            for (; 0 < _Nput; --_Nput) {
                *_Mid2++ = static_cast<_Byte>((_Ch >> 6 * --_Nextra & 0x3f) | 0x80);
            }

            *_Pstate = static_cast<unsigned short>(_Save ? _Ch >> 10 : 1);
        }

        return _First1 == _Mid1 ? _Mybase::partial : _Mybase::ok;
    }

    result do_unshift(std::mbstate_t&, _Byte* _First2, _Byte*, _Byte*& _Mid2) const override {
        // generate bytes to return to default shift state
        _Mid2 = _First2;
        return _Mybase::noconv;
    }

    friend int detail::_Codecvt_do_length<>(const codecvt_utf8_utf16&, std::mbstate_t&, const _Byte*, const _Byte*, std::size_t);

    int do_length(
        std::mbstate_t& _State, const _Byte* _First1, const _Byte* _Last1, std::size_t _Count) const noexcept override {
        return detail::_Codecvt_do_length(*this, _State, _First1, _Last1, _Count);
    }

    bool do_always_noconv() const noexcept override {
        // return true if conversions never change input
        return false;
    }

    int do_max_length() const noexcept override {
        // return maximum length required for a conversion
        if constexpr ((_Mymode & consume_header) != 0) {
            return 9; // header + max input
        } else if constexpr ((_Mymode & generate_header) != 0) {
            return 7; // header + max output
        } else {
            return 6; // 6-byte max input sequence, no 3-byte header
        }
    }

    int do_encoding() const noexcept override {
        // return length of code sequence (from codecvt)
        return 0; // 0 => varying length
    }
};


template <
    class _Codecvt,
    class _Elem = wchar_t,
    class CharT = char,
    class _Traits = std::char_traits<_Elem>>
class wbuffer_convert
    : public std::basic_streambuf<_Elem, _Traits> { // stream buffer associated with a codecvt facet
private:
    enum _Mode { _Unused, _Wrote, _Need, _Got, _Eof };
    enum { _STRING_INC = 8 };

public:
    using _Mysb        = std::basic_streambuf<CharT>;
    using _Byte_traits = std::char_traits<CharT>;

    using int_type   = typename _Traits::int_type;
    using pos_type   = typename _Traits::pos_type;
    using off_type   = typename _Traits::off_type;
    using state_type = typename _Codecvt::state_type;

    wbuffer_convert() : _State(), _Pcvt(new _Codecvt), _Mystrbuf(nullptr), _Status(_Unused), _Nback(0) {
        // construct without buffer pointer
        _Loc = std::locale(_Loc, const_cast<_Codecvt*>(_Pcvt));
    }

    explicit wbuffer_convert(_Mysb* _Strbuf)
        : _State(), _Pcvt(new _Codecvt), _Mystrbuf(_Strbuf), _Status(_Unused), _Nback(0) {
        // construct with byte stream buffer pointer
        _Loc = std::locale(_Loc, const_cast<_Codecvt*>(_Pcvt));
    }

    wbuffer_convert(_Mysb* _Strbuf, const _Codecvt* _Pcvt_arg)
        : _State(), _Pcvt(_Pcvt_arg), _Mystrbuf(_Strbuf), _Status(_Unused), _Nback(0) {
        // construct with byte stream buffer pointer and codecvt
        _Loc = std::locale(_Loc, const_cast<_Codecvt*>(_Pcvt));
    }

    wbuffer_convert(_Mysb* _Strbuf, const _Codecvt* _Pcvt_arg, state_type _State_arg)
        : _State(_State_arg), _Pcvt(_Pcvt_arg), _Mystrbuf(_Strbuf), _Status(_Unused), _Nback(0) {
        // construct with byte stream buffer pointer, codecvt, and state
        _Loc = std::locale(_Loc, const_cast<_Codecvt*>(_Pcvt));
    }

    ~wbuffer_convert() noexcept override {
        while (_Status == _Wrote) { // put any trailing homing shift
            if (_Str.size() < _STRING_INC) {
                _Str.assign(_STRING_INC, '\0');
            }

            CharT* _Buf = &_Str[0];
            CharT* _Dest;
            switch (_Pcvt->unshift(_State, _Buf, _Buf + _Str.size(), _Dest)) { // test result of homing conversion
            case _Codecvt::ok:
                _Status = _Unused; // homed successfully

            case _Codecvt::partial: // fall through
                { // put any generated bytes
                    ptrdiff_t _Count = _Dest - _Buf;
                    if (0 < _Count
                        && _Byte_traits::eq_int_type(
                            _Byte_traits::eof(), static_cast<typename _Byte_traits::int_type>(_Mystrbuf->sputn(_Buf, _Count)))) {
                        return; // write failed
                    }

                    if (_Status == _Wrote && _Count == 0) {
                        _Str.append(_STRING_INC, '\0'); // try with more space
                    }

                    break;
                }

            case _Codecvt::noconv:
                return; // nothing to do

            default:
                return; // conversion failed
            }
        }
    }

    [[nodiscard]] _Mysb* rdbuf() const {
        return _Mystrbuf;
    }

    _Mysb* rdbuf(_Mysb* _Strbuf) { // set byte stream buffer pointer
        _Mysb* _Oldstrbuf = _Mystrbuf;
        _Mystrbuf         = _Strbuf;
        return _Oldstrbuf;
    }

    [[nodiscard]] state_type state() const {
        return _State;
    }

    wbuffer_convert(const wbuffer_convert&) = delete;
    wbuffer_convert& operator=(const wbuffer_convert&) = delete;

protected:
    int_type overflow(int_type _Meta = _Traits::eof()) override { // put an element to stream
        if (_Traits::eq_int_type(_Traits::eof(), _Meta)) {
            return _Traits::not_eof(_Meta); // EOF, return success code
        } else if (!_Mystrbuf || 0 < _Nback || (_Status != _Unused && _Status != _Wrote)) {
            return _Traits::eof(); // no buffer or reading, fail
        } else { // put using codecvt facet
            const _Elem _Ch = _Traits::to_char_type(_Meta);

            if (_Str.size() < _STRING_INC) {
                _Str.assign(_STRING_INC, '\0');
            }

            for (_Status = _Wrote;;) {
                CharT* _Buf = &_Str[0];
                const _Elem* _Src;
                CharT* _Dest;

                // test result of converting one element
                switch (_Pcvt->out(_State, &_Ch, &_Ch + 1, _Src, _Buf, _Buf + _Str.size(), _Dest)) {
                case _Codecvt::partial:
                case _Codecvt::ok:
                    { // converted something, try to put it out
                        ptrdiff_t _Count = _Dest - _Buf;
                        if (0 < _Count
                            && _Byte_traits::eq_int_type(_Byte_traits::eof(),
                                static_cast<typename _Byte_traits::int_type>(_Mystrbuf->sputn(_Buf, _Count)))) {
                            return _Traits::eof(); // write failed
                        }

                        if (_Src != &_Ch) {
                            return _Meta; // converted whole element
                        }

                        if (0 >= _Count) {
                            if (_Str.size() >= 4 * _STRING_INC) {
                                return _Traits::eof(); // conversion failed
                            }

                            _Str.append(_STRING_INC, '\0'); // try with more space
                        }

                        break;
                    }

                case _Codecvt::noconv:
                    if (_Traits::eq_int_type(
                            _Traits::eof(), static_cast<int_type>(_Mystrbuf->sputn(reinterpret_cast<const char*>(&_Ch),
                                                static_cast<std::streamsize>(sizeof(_Elem)))))) {
                        return _Traits::eof();
                    }

                    return _Meta; // put native byte order

                default:
                    return _Traits::eof(); // conversion failed
                }
            }
        }
    }

    int_type pbackfail(int_type _Meta = _Traits::eof()) override { // put an element back to stream
        if (sizeof(_Myback) / sizeof(_Myback[0]) <= _Nback || _Status == _Wrote) {
            return _Traits::eof(); // nowhere to put back
        } else { // enough room, put it back
            if (!_Traits::eq_int_type(_Traits::eof(), _Meta)) {
                _Myback[_Nback] = _Traits::to_char_type(_Meta);
            }

            ++_Nback;
            if (_Status == _Unused) {
                _Status = _Got;
            }

            return _Meta;
        }
    }

    int_type underflow() override { // get an element from stream, but don't point past it
        int_type _Meta;

        if (0 >= _Nback) {
            if (_Traits::eq_int_type(_Traits::eof(), _Meta = _Get_elem())) {
                return _Meta; // _Get_elem failed, return EOF
            }

            _Myback[_Nback++] = _Traits::to_char_type(_Meta);
        }

        return _Traits::to_int_type(_Myback[_Nback - 1]);
    }

    int_type uflow() override { // get an element from stream, point past it
        int_type _Meta;

        if (0 >= _Nback) {
            if (_Traits::eq_int_type(_Traits::eof(), _Meta = _Get_elem())) {
                return _Meta; // _Get_elem failed, return EOF
            }

            _Myback[_Nback++] = _Traits::to_char_type(_Meta);
        }

        return _Traits::to_int_type(_Myback[--_Nback]);
    }

    pos_type seekoff(off_type, std::ios_base::seekdir,
        std::ios_base::openmode = static_cast<std::ios_base::openmode>(std::ios_base::in | std::ios_base::out)) override {
        return pos_type(-1); // always fail
    }

    pos_type seekpos(
        pos_type, std::ios_base::openmode = static_cast<std::ios_base::openmode>(std::ios_base::in | std::ios_base::out)) override {
        return pos_type(-1); // always fail
    }

private:
    int_type _Get_elem() { // compose an element from byte stream buffer
        if (_Mystrbuf && _Status != _Wrote) { // got buffer, haven't written, try to compose an element
            if (_Status != _Eof) {
                if (_Str.empty()) {
                    _Status = _Need;
                } else {
                    _Status = _Got;
                }
            }

            while (_Status != _Eof) { // get using codecvt facet
                CharT* _Buf = &_Str[0];
                _Elem _Ch;
                _Elem* _Dest;
                const CharT* _Src;
                int _Meta;

                if (_Status == _Need) {
                    if (_Byte_traits::eq_int_type(_Byte_traits::eof(), _Meta = _Mystrbuf->sbumpc())) {
                        _Status = _Eof;
                    } else {
                        _Str.push_back(_Byte_traits::to_char_type(_Meta));
                    }
                }

                // test result of converting one element
                switch (_Pcvt->in(_State, _Buf, _Buf + _Str.size(), _Src, &_Ch, &_Ch + 1, _Dest)) {
                case _Codecvt::partial:
                case _Codecvt::ok:
                    _Str.erase(0, static_cast<std::size_t>(_Src - _Buf)); // discard any used input
                    if (_Dest != &_Ch) {
                        return _Traits::to_int_type(_Ch);
                    }

                    break;

                case _Codecvt::noconv:
                    if (_Str.size() < sizeof(_Elem)) {
                        break; // no conversion, but need more chars
                    }

                    std::memcpy(&_Ch, _Buf, sizeof(_Elem)); // copy raw bytes to element
                    _Str.erase(0, sizeof(_Elem));
                    return _Traits::to_int_type(_Ch); // return result

                default:
                    _Status = _Eof; // conversion failed
                    break;
                }
            }
        }

        return _Traits::eof();
    }

    state_type _State; // code conversion state
    const _Codecvt* _Pcvt; // the codecvt facet
    _Mysb* _Mystrbuf; // pointer to stream buffer
    _Mode _Status; // buffer read/write status
    std::size_t _Nback; // number of elements in putback buffer
    _Elem _Myback[8]; // putback buffer
    std::basic_string<CharT> _Str; // unconsumed input bytes
    std::locale _Loc; // manages reference to codecvt facet
};

template <
    class _Codecvt,
    class _Elem = wchar_t,
    class CharT = char,
    class _Walloc = std::allocator<_Elem>,
    class _Balloc = std::allocator<CharT>>
class wstring_convert { // converts between _Elem (wide) and char (byte) strings
private:
    enum { _BUF_INC = 8, _BUF_MAX = 16 };
    void _Init(const _Codecvt* _Pcvt_arg = new _Codecvt) { // initialize the object
        _State = state_type{};
        _Pcvt  = _Pcvt_arg;
        _Loc   = std::locale(_Loc, const_cast<_Codecvt*>(_Pcvt));
        _Nconv = 0;
    }

public:
    using byte_string = std::basic_string<CharT, std::char_traits<CharT>, _Balloc>;
    using wide_string = std::basic_string<_Elem, std::char_traits<_Elem>, _Walloc>;
    using state_type  = typename _Codecvt::state_type;
    using int_type    = typename wide_string::traits_type::int_type;

    wstring_convert() : _Has_state(false), _Has_berr(false), _Has_werr(false) { // construct with no error strings
        _Init();
    }

    explicit wstring_convert(const _Codecvt* _Pcvt_arg)
        : _Has_state(false), _Has_berr(false), _Has_werr(false) { // construct with no error strings and codecvt
        _Init(_Pcvt_arg);
    }

    wstring_convert(const _Codecvt* _Pcvt_arg, state_type _State_arg)
        : _Has_state(true), _Has_berr(false), _Has_werr(false) { // construct with no error strings, codecvt, and state
        _Init(_Pcvt_arg);
        _State = _State_arg;
    }

    explicit wstring_convert(const byte_string& _Berr_arg)
        : _Berr(_Berr_arg), _Has_state(false), _Has_berr(true), _Has_werr(false) { // construct with byte error string
        _Init();
    }

    wstring_convert(const byte_string& _Berr_arg, const wide_string& _Werr_arg)
        : _Berr(_Berr_arg), _Werr(_Werr_arg), _Has_state(false), _Has_berr(true),
          _Has_werr(true) { // construct with byte and wide error strings
        _Init();
    }

    virtual ~wstring_convert() noexcept {}

    [[nodiscard]] std::size_t converted() const noexcept { // get conversion count
        return _Nconv;
    }

    [[nodiscard]] state_type state() const {
        return _State;
    }

    [[nodiscard]] wide_string from_bytes(CharT _Byte) { // convert a byte to a wide string
        return from_bytes(&_Byte, &_Byte + 1);
    }

    [[nodiscard]] wide_string from_bytes(const CharT* _Ptr) { // convert a NTBS to a wide string
        return from_bytes(_Ptr, _Ptr + std::strlen(_Ptr));
    }

    [[nodiscard]] wide_string from_bytes(const byte_string& _Bstr) { // convert a byte string to a wide string
        const CharT* _Ptr = _Bstr.c_str();
        return from_bytes(_Ptr, _Ptr + _Bstr.size());
    }

    [[nodiscard]] wide_string from_bytes(
        const CharT* _First, const CharT* _Last) { // convert byte sequence [_First, _Last) to a wide string
        wide_string _Wbuf;
        wide_string _Wstr;
        const CharT* _First_sav = _First;

        if (!_Has_state) {
            _State = state_type{}; // reset state if not remembered
        }

        _Wbuf.append(_BUF_INC, _Elem{});
        for (_Nconv = 0; _First != _Last; _Nconv = static_cast<std::size_t>(_First - _First_sav)) {
            // convert one or more bytes
            _Elem* _Dest = &_Wbuf[0];
            _Elem* _Dnext;

            // test result of converting one or more bytes
            switch (_Pcvt->in(_State, _First, _Last, _First, _Dest, _Dest + _Wbuf.size(), _Dnext)) {
            case _Codecvt::partial:
            case _Codecvt::ok:
                if (_Dest < _Dnext) {
                    _Wstr.append(_Dest, static_cast<std::size_t>(_Dnext - _Dest));
                } else if (_Wbuf.size() < _BUF_MAX) {
                    _Wbuf.append(_BUF_INC, _Elem{});
                } else if (_Has_werr) {
                    return _Werr;
                } else {
                    throw std::range_error("bad conversion");
                }

                break;

            case _Codecvt::noconv:
                for (; _First != _Last; ++_First) {
                    _Wstr.push_back(static_cast<_Elem>(static_cast<std::make_unsigned_t<CharT>>(*_First)));
                }

                break; // no conversion, just copy code values

            default:
                if (_Has_werr) {
                    return _Werr;
                } else {
                    throw std::range_error("bad conversion");
                }
            }
        }
        return _Wstr;
    }

    [[nodiscard]] byte_string to_bytes(_Elem _Char) { // convert a widechar to a byte string
        return to_bytes(&_Char, &_Char + 1);
    }

    [[nodiscard]] byte_string to_bytes(const _Elem* _Wptr) { // convert a NTWCS to a byte string
        const _Elem* _Next = _Wptr;
        while (*_Next != 0) {
            ++_Next;
        }

        return to_bytes(_Wptr, _Next);
    }

    [[nodiscard]] byte_string to_bytes(const wide_string& _Wstr) { // convert a wide string to a byte string
        const _Elem* _Wptr = _Wstr.c_str();
        return to_bytes(_Wptr, _Wptr + _Wstr.size());
    }

    [[nodiscard]] byte_string to_bytes(
        const _Elem* _First, const _Elem* _Last) { // convert wide sequence [_First, _Last) to a byte string
        byte_string _Bbuf;
        byte_string _Bstr;
        const _Elem* _First_sav = _First;

        if (!_Has_state) {
            _State = state_type{}; // reset state if not remembered
        }

        _Bbuf.append(_BUF_INC, '\0');
        for (_Nconv = 0; _First != _Last; _Nconv = static_cast<std::size_t>(_First - _First_sav)) {
            // convert one or more wide chars
            CharT* _Dest = &_Bbuf[0];
            CharT* _Dnext;

            // test result of converting one or more wide chars
            switch (_Pcvt->out(_State, _First, _Last, _First, _Dest, _Dest + _Bbuf.size(), _Dnext)) {
            case _Codecvt::partial:
            case _Codecvt::ok:
                if (_Dest < _Dnext) {
                    _Bstr.append(_Dest, static_cast<std::size_t>(_Dnext - _Dest));
                } else if (_Bbuf.size() < _BUF_MAX) {
                    _Bbuf.append(_BUF_INC, '\0');
                } else if (_Has_berr) {
                    return _Berr;
                } else {
                    throw std::range_error("bad conversion");
                }

                break;

            case _Codecvt::noconv:
                for (; _First != _Last; ++_First) {
                    _Bstr.push_back(static_cast<CharT>(static_cast<int_type>(*_First)));
                }

                break; // no conversion, just copy code values

            default:
                if (_Has_berr) {
                    return _Berr;
                } else {
                    throw std::range_error("bad conversion");
                }
            }
        }
        return _Bstr;
    }

    wstring_convert(const wstring_convert&) = delete;
    wstring_convert& operator=(const wstring_convert&) = delete;

private:
    const _Codecvt* _Pcvt; // the codecvt facet
    std::locale _Loc; // manages reference to codecvt facet
    byte_string _Berr;
    wide_string _Werr;
    state_type _State; // the remembered state
    bool _Has_state;
    bool _Has_berr;
    bool _Has_werr;
    std::size_t _Nconv;
};

}

namespace asio2
{
    /**
     * @brief Return default system locale name in POSIX format.
     *
     * This function tries to detect the locale using, LC_CTYPE, LC_ALL and LANG environment
     * variables in this order and if all of them unset, in POSIX platforms it returns "C"
     *
     * On Windows additionally to check the above environment variables, this function
     * tries to creates locale name from ISO-339 and ISO-3199 country codes defined
     * for user default locale.
     * If use_utf8_on_windows is true it sets the encoding to UTF-8, otherwise, if system
     * locale supports ANSI code-page it defines the ANSI encoding like windows-1252, otherwise it fall-backs
     * to UTF-8 encoding if ANSI code-page is not available.
     *
     * /boost/libs/locale/src/boost/locale/util/default_locale.cpp
     */ 
    inline std::string get_system_locale(bool use_utf8_on_windows = false)
    {
        char const *lang = 0;
        if(!lang || !*lang)
            lang = std::getenv("LC_CTYPE");
        if(!lang || !*lang)
            lang = std::getenv("LC_ALL");
        if(!lang || !*lang)
            lang = std::getenv("LANG");
    #if !defined(BOOST_LOCALE_USE_WIN32_API) && !defined(BHO_LOCALE_USE_WIN32_API) && !defined(ASIO2_LOCALE_USE_WIN32_API)
        (void)use_utf8_on_windows; // not relevant for non-windows
        if(!lang || !*lang)
            lang = "C";
        return lang;
    #else
        if(lang && *lang) {
            return lang;
        }
        char buf[10] = { 0 };
        if(GetLocaleInfoA(LOCALE_USER_DEFAULT,LOCALE_SISO639LANGNAME,buf,sizeof(buf))==0)
            return "C";
        std::string lc_name = buf;
        if(GetLocaleInfoA(LOCALE_USER_DEFAULT,LOCALE_SISO3166CTRYNAME,buf,sizeof(buf))!=0) {
            lc_name += "_";
            lc_name += buf;
        }
        if(!use_utf8_on_windows) {
            if(GetLocaleInfoA(LOCALE_USER_DEFAULT,LOCALE_IDEFAULTANSICODEPAGE,buf,sizeof(buf))!=0) {
                if(std::atoi(buf)==0)
                    lc_name+=".UTF-8";
                else {
                    lc_name +=".windows-";
                    lc_name +=buf;
                }
            }
            else {
                lc_name += "UTF-8";
            }
        }
        else {
            lc_name += ".UTF-8";
        }
        return lc_name;
    #endif
    }

    /**
     * @brief Return default system locale name that can be used in codecvt.
     *
     */ 
    inline std::string get_codecvt_locale(bool use_utf8_on_windows = false)
    {
        char const *lang = 0;
        if(!lang || !*lang)
            lang = std::getenv("LC_CTYPE");
        if(!lang || !*lang)
            lang = std::getenv("LC_ALL");
        if(!lang || !*lang)
            lang = std::getenv("LANG");
    #if !defined(BOOST_LOCALE_USE_WIN32_API) && !defined(BHO_LOCALE_USE_WIN32_API) && !defined(ASIO2_LOCALE_USE_WIN32_API)
        (void)use_utf8_on_windows; // not relevant for non-windows
        if(!lang || !*lang)
            lang = "C";
        return lang;
    #else
        if(lang && *lang) {
            return lang;
        }
		char buf[10] = { 0 };
        std::string lc_name;
        if(!use_utf8_on_windows) {
            if(GetLocaleInfoA(LOCALE_USER_DEFAULT,LOCALE_IDEFAULTANSICODEPAGE,buf,sizeof(buf))!=0) {
                if(std::atoi(buf)==0)
                    lc_name+=".UTF-8";
                else {
                    lc_name +=".";
                    lc_name +=buf;
                }
            }
            else {
                lc_name += "UTF-8";
            }
        }
        else {
            lc_name += ".UTF-8";
        }
        return lc_name;
    #endif
    }

	/**
	 * @brief Converts gbk characters to utf8 characters.
	 * @param str - gbk characters
	 * @return Converted value as std::string.
	 */
	template<class StringT>
	inline auto gbk_to_utf8(const StringT& str, const std::string& locale_name = "chs") noexcept
	{
		using CharT = typename detail::char_type<StringT>::type;

		clear_last_error();

		std::wstring w;

		std::codecvt_byname<wchar_t, CharT, std::mbstate_t>* c = nullptr;
		try
		{
			c = new std::codecvt_byname<wchar_t, CharT, std::mbstate_t>(locale_name);
		}
		catch (const std::exception&)
		{
			set_last_error(std::errc::invalid_argument);

			return std::basic_string<CharT>{};
		}

        // gbk to widechar
        {
            auto sv = asio2::to_basic_string_view(str);

            asio2::wstring_convert<std::codecvt_byname<wchar_t, CharT, std::mbstate_t>, wchar_t, CharT> conv(c);
            try
            {
                w = conv.from_bytes(sv.data(), sv.data() + sv.size());
            }
            catch (const std::range_error&)
            {
                set_last_error(std::errc::result_out_of_range);

                sv = sv.substr(0, conv.converted());

                w = conv.from_bytes(sv.data(), sv.data() + sv.size());
            }
        }

        // widechar to utf8
        {
            auto sv = asio2::to_basic_string_view(w);

            asio2::wstring_convert<asio2::codecvt_utf8<wchar_t, CharT>, wchar_t, CharT> conv;
            try
            {
                return conv.to_bytes(sv.data(), sv.data() + sv.size());
            }
            catch (const std::range_error&)
            {
                set_last_error(std::errc::result_out_of_range);

                sv = sv.substr(0, conv.converted());

                return conv.to_bytes(sv.data(), sv.data() + sv.size());
            }
        }
	}

	/**
	 * @brief Converts utf8 characters to gbk characters.
	 * @param str - gbk characters
	 * @return Converted value as std::string.
	 */
	template<class StringT>
    inline auto utf8_to_gbk(const StringT& str, const std::string& locale_name = "chs") noexcept
	{
        using CharT = typename detail::char_type<StringT>::type;

        clear_last_error();

		std::wstring w;

        std::codecvt_byname<wchar_t, char, std::mbstate_t>* c = nullptr;
        try
        {
            c = new std::codecvt_byname<wchar_t, char, std::mbstate_t>(locale_name);
        }
        catch (const std::exception&)
        {
            set_last_error(std::errc::invalid_argument);

            return std::string{};
        }

        // utf8 to widechar
        {
            auto sv = asio2::to_basic_string_view(str);

            asio2::wstring_convert<asio2::codecvt_utf8<wchar_t, CharT>, wchar_t, CharT> conv;
            try
            {
                w = conv.from_bytes(sv.data(), sv.data() + sv.size());
            }
            catch (const std::range_error&)
            {
                set_last_error(std::errc::result_out_of_range);

                sv = sv.substr(0, conv.converted());

                w = conv.from_bytes(sv.data(), sv.data() + sv.size());
            }
        }

        // widechar to gbk
        {
            auto sv = asio2::to_basic_string_view(w);

            asio2::wstring_convert<std::codecvt_byname<wchar_t, char, std::mbstate_t>> conv(c);
            try
            {
                return conv.to_bytes(sv.data(), sv.data() + sv.size());
            }
            catch (const std::range_error&)
            {
                set_last_error(std::errc::result_out_of_range);

                sv = sv.substr(0, conv.converted());

                return conv.to_bytes(sv.data(), sv.data() + sv.size());
            }
        }
	}

	/**
	 * @brief Converts wide characters to multibyte characters.
	 * @param str - wide characters
	 * @param locale_name - locale name
	 * @return Converted value as std::string.
	 */
	template<class StringT>
    inline std::string wcstombs(const StringT& str, const std::string& locale_name = asio2::get_codecvt_locale()) noexcept
	{
        clear_last_error();

        auto sv = asio2::to_basic_string_view(str);

        std::codecvt_byname<wchar_t, char, std::mbstate_t>* c = nullptr;
        try
        {
            c = new std::codecvt_byname<wchar_t, char, std::mbstate_t>(locale_name);
        }
        catch (const std::exception&)
        {
            set_last_error(std::errc::invalid_argument);

            return std::string{};
        }

		asio2::wstring_convert<std::codecvt_byname<wchar_t, char, std::mbstate_t>> conv(c);

        try
        {
            return conv.to_bytes(sv.data(), sv.data() + sv.size());
        }
        catch (const std::range_error&)
        {
            set_last_error(std::errc::result_out_of_range);

            sv = sv.substr(0, conv.converted());

            return conv.to_bytes(sv.data(), sv.data() + sv.size());
        }
	}

	/**
	 * @brief Converts multibyte characters to wide characters.
	 * @param str - wide characters
	 * @param locale_name - locale name
	 * @return Converted value as std::wstring.
	 */
	template<class StringT>
    inline std::wstring mbstowcs(const StringT& str, const std::string& locale_name = asio2::get_codecvt_locale()) noexcept
	{
        clear_last_error();

        auto sv = asio2::to_basic_string_view(str);

        std::codecvt_byname<wchar_t, char, std::mbstate_t>* c = nullptr;
        try
        {
            c = new std::codecvt_byname<wchar_t, char, std::mbstate_t>(locale_name);
        }
        catch (const std::exception&)
        {
            set_last_error(std::errc::invalid_argument);

            return std::wstring{};
        }

		asio2::wstring_convert<std::codecvt_byname<wchar_t, char, std::mbstate_t>> conv(c);

        try
        {
            return conv.from_bytes(sv.data(), sv.data() + sv.size());
        }
        catch (const std::range_error&)
        {
            set_last_error(std::errc::result_out_of_range);

            sv = sv.substr(0, conv.converted());

            return conv.from_bytes(sv.data(), sv.data() + sv.size());
        }
	}

	/**
	 * @brief Converts utf8 characters to current default locale characters.
	 * @param str - utf8 characters
	 * @return Converted value as std::string.
	 */
	template<class StringT>
	inline std::string utf8_to_locale(const StringT& str) noexcept
	{
		using CharT = typename detail::char_type<StringT>::type;

        clear_last_error();

        std::wstring w;

        std::codecvt_byname<wchar_t, char, std::mbstate_t>* c = nullptr;
        try
        {
            c = new std::codecvt_byname<wchar_t, char, std::mbstate_t>(asio2::get_codecvt_locale());
        }
        catch (const std::exception&)
        {
            set_last_error(std::errc::invalid_argument);

            return std::string{};
        }

        // utf8 to widechar
        {
            auto sv = asio2::to_basic_string_view(str);

            asio2::wstring_convert<asio2::codecvt_utf8<wchar_t, CharT>, wchar_t, CharT> conv;
            try
            {
                w = conv.from_bytes(sv.data(), sv.data() + sv.size());
            }
            catch (const std::range_error&)
            {
                set_last_error(std::errc::result_out_of_range);

                sv = sv.substr(0, conv.converted());

                w = conv.from_bytes(sv.data(), sv.data() + sv.size());
            }
        }

        // widechar to locale
        {
            auto sv = asio2::to_basic_string_view(w);

            asio2::wstring_convert<std::codecvt_byname<wchar_t, char, std::mbstate_t>> conv(c);
            try
            {
                return conv.to_bytes(sv.data(), sv.data() + sv.size());
            }
            catch (const std::range_error&)
            {
                set_last_error(std::errc::result_out_of_range);

                sv = sv.substr(0, conv.converted());

                return conv.to_bytes(sv.data(), sv.data() + sv.size());
            }
        }
	}

	/**
	 * @brief Converts current default locale characters to utf8 characters.
	 * @param str - current default locale characters
	 * @return Converted value as std::string.
	 */
	template<class StringT>
	inline std::string locale_to_utf8(const StringT& str) noexcept
	{
        using CharT = typename detail::char_type<StringT>::type;

        clear_last_error();

        std::wstring w;

        std::codecvt_byname<wchar_t, CharT, std::mbstate_t>* c = nullptr;
        try
        {
            c = new std::codecvt_byname<wchar_t, CharT, std::mbstate_t>(asio2::get_codecvt_locale());
        }
        catch (const std::exception&)
        {
            set_last_error(std::errc::invalid_argument);

            return std::string{};
        }

        // locale to widechar
        {
            auto sv = asio2::to_basic_string_view(str);

            asio2::wstring_convert<std::codecvt_byname<wchar_t, CharT, std::mbstate_t>, wchar_t, CharT> conv(c);
            try
            {
                w = conv.from_bytes(sv.data(), sv.data() + sv.size());
            }
            catch (const std::range_error&)
            {
                set_last_error(std::errc::result_out_of_range);

                sv = sv.substr(0, conv.converted());

                w = conv.from_bytes(sv.data(), sv.data() + sv.size());
            }
        }

        // widechar to utf8
        {
            auto sv = asio2::to_basic_string_view(w);

            asio2::wstring_convert<asio2::codecvt_utf8<wchar_t, CharT>, wchar_t, CharT> conv;
            try
            {
                return conv.to_bytes(sv.data(), sv.data() + sv.size());
            }
            catch (const std::range_error&)
            {
                set_last_error(std::errc::result_out_of_range);

                sv = sv.substr(0, conv.converted());

                return conv.to_bytes(sv.data(), sv.data() + sv.size());
            }
        }
	}
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_CODECVT_HPP__
