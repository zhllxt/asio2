//
// unit_test.hpp
// ~~~~~~~~~~~~~
//
// Copyright (c) 2003-2022 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef __ASIO2_UNIT_TEST_HPP__
#define __ASIO2_UNIT_TEST_HPP__

#include <asio2/base/detail/push_options.hpp>

#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <limits>
#include <iomanip>
#include <mutex>
#include <iterator>
#include <string>
#include <string_view>
#include <fstream>
#include <thread>

#ifndef ASIO2_ENABLE_LOG
#define ASIO2_ENABLE_LOG
#endif

#include <asio2/external/asio.hpp>

#if defined(__sun)
# include <stdlib.h> // Needed for lrand48.
#endif // defined(__sun)

#if defined(__BORLANDC__) && !defined(__clang__)

// Prevent use of intrinsic for strcmp.
# include <cstring>
# undef strcmp
 
// Suppress error about condition always being true.
# pragma option -w-ccc

#endif // defined(__BORLANDC__) && !defined(__clang__)

#if !defined(ASIO2_TEST_IOSTREAM)
# define ASIO2_TEST_IOSTREAM std::cerr
#endif // !defined(ASIO2_TEST_IOSTREAM)

static const int   test_loop_times = 1;
static const int   test_client_count = int(std::thread::hardware_concurrency() * 2);
static const int   test_wait_count = 108000;
static const int   test_timer_deviation = 500;

bool test_has_error = false;

namespace asio2 {

namespace detail {

inline const char*& test_name()
{
  static const char* name = 0;
  return name;
}

inline std::atomic<long>& test_errors()
{
  static std::atomic<long> errors(0);
  return errors;
}

inline std::mutex& test_lock()
{
  static std::mutex mtx{};
  return mtx;
}

inline std::fstream& test_log()
{
    static std::fstream file("test.log", std::ios::out | std::ios::binary | std::ios::app);
    return file;
}

#define ASIO2_TEST_LOGSTREAM asio2::detail::test_log()

#define ASIO2_TEST_LOCK_GUARD \
  std::lock_guard guard(asio2::detail::test_lock());

inline void begin_test_suite(const char* name)
{
  asio2::detail::test_name();
  asio2::detail::test_errors();
  ASIO2_TEST_IOSTREAM << name << " test suite begins" << std::endl;
  ASIO2_TEST_LOGSTREAM << name << " test suite begins" << std::endl;
}

inline int end_test_suite(const char* name)
{
  ASIO2_TEST_IOSTREAM << name << " test suite ends" << std::endl;
  ASIO2_TEST_LOGSTREAM << name << " test suite ends" << std::endl;
  ASIO2_TEST_IOSTREAM << "\n*** ";
  ASIO2_TEST_LOGSTREAM << "\n*** ";
  long errors = asio2::detail::test_errors();
  ASIO2_TEST_IOSTREAM << errors << " errors detected." << std::endl;
  ASIO2_TEST_LOGSTREAM << errors << " errors detected." << std::endl;
  ASIO2_TEST_IOSTREAM << std::endl;
  ASIO2_TEST_LOGSTREAM << std::endl; ASIO2_TEST_LOGSTREAM.flush();
  return errors == 0 ? 0 : 1;
}

template <void (*Test)()>
inline void run_test(const char* name)
{
  test_name() = name;
  long errors_before = asio2::detail::test_errors();
  Test();
  if (!test_has_error)
  {
    ASIO2_TEST_IOSTREAM << std::endl;
    ASIO2_TEST_LOGSTREAM << std::endl;
  }
  if (test_errors() == errors_before)
  {
    ASIO2_TEST_IOSTREAM << name << " passed" << std::endl;
    ASIO2_TEST_LOGSTREAM << name << " passed" << std::endl;
  }
  else
  {
    ASIO2_TEST_IOSTREAM << name << " failed" << std::endl;
    ASIO2_TEST_LOGSTREAM << name << " failed" << std::endl;
  }
}

template <void (*)()>
inline void compile_test(const char* name)
{
  ASIO2_TEST_IOSTREAM << name << " passed" << std::endl;
  ASIO2_TEST_LOGSTREAM << name << " passed" << std::endl;
}

#if defined(ASIO_NO_EXCEPTIONS) || defined(BOOST_ASIO_NO_EXCEPTIONS)

template <typename T>
void throw_exception(const T& t)
{
  ASIO2_TEST_IOSTREAM << "Exception: " << t.what() << std::endl;
  ASIO2_TEST_LOGSTREAM << "Exception: " << t.what() << std::endl; ASIO2_TEST_LOGSTREAM.flush();
  std::abort();
}

#endif

void check_memory_leaks()
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
  // Detected memory leaks on windows system
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
}

void pause_cmd_window()
{
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
  //while (std::getchar() != '\n');
#endif
}

template<class... Args>
void test_print(Args&&... args)
{
    ((ASIO2_TEST_IOSTREAM << args << " "), ...);
    ((ASIO2_TEST_LOGSTREAM << args << " "), ...);
}

} // namespace detail
} // namespace asio2

#define ASIO2_CHECK(expr) \
  do { if (!(expr)) { \
    ASIO2_TEST_LOCK_GUARD \
    if (!test_has_error) \
    { \
      ASIO2_TEST_IOSTREAM << std::endl; \
      ASIO2_TEST_LOGSTREAM << std::endl; \
      test_has_error = true; \
    } \
    std::string_view file{__FILE__}; \
    ASIO2_TEST_IOSTREAM \
      << std::next(std::next(file.data(), file.find_last_of("\\/"))) << "(" << __LINE__ << "): " \
      << asio2::detail::test_name() << ": " \
      << "check '" << #expr << "' failed" << std::endl; \
    ASIO2_TEST_LOGSTREAM \
      << std::next(std::next(file.data(), file.find_last_of("\\/"))) << "(" << __LINE__ << "): " \
      << asio2::detail::test_name() << ": " \
      << "check '" << #expr << "' failed" << std::endl; ASIO2_TEST_LOGSTREAM.flush(); \
    ++asio2::detail::test_errors(); \
  } } while (0)

#define ASIO2_CHECK_VALUE(val, expr) \
  do { if (!(expr)) { \
    ASIO2_TEST_LOCK_GUARD \
    if (!test_has_error) \
    { \
      ASIO2_TEST_IOSTREAM << std::endl; \
      ASIO2_TEST_LOGSTREAM << std::endl; \
      test_has_error = true; \
    } \
    std::string_view file{__FILE__}; \
    ASIO2_TEST_IOSTREAM \
      << #val << "=" << val << " \t" \
      << std::next(std::next(file.data(), file.find_last_of("\\/"))) << "(" << __LINE__ << "): " \
      << asio2::detail::test_name() << ": " \
      << "check '" << #expr << "' failed" << std::endl; \
    ASIO2_TEST_LOGSTREAM \
      << #val << "=" << val << " \t" \
      << std::next(std::next(file.data(), file.find_last_of("\\/"))) << "(" << __LINE__ << "): " \
      << asio2::detail::test_name() << ": " \
      << "check '" << #expr << "' failed" << std::endl; ASIO2_TEST_LOGSTREAM.flush(); \
    ++asio2::detail::test_errors(); \
  } } while (0)

#define ASIO2_CHECK_MESSAGE(expr, msg) \
  do { if (!(expr)) { \
    ASIO2_TEST_LOCK_GUARD \
    if (!test_has_error) \
    { \
      ASIO2_TEST_IOSTREAM << std::endl; \
      ASIO2_TEST_LOGSTREAM << std::endl; \
      test_has_error = true; \
    } \
    std::string_view file{__FILE__}; \
    ASIO2_TEST_IOSTREAM \
      << std::next(std::next(file.data(), file.find_last_of("\\/"))) << "(" << __LINE__ << "): " \
      << asio2::detail::test_name() << ": " \
      << msg << std::endl; \
    ASIO2_TEST_LOGSTREAM \
      << std::next(std::next(file.data(), file.find_last_of("\\/"))) << "(" << __LINE__ << "): " \
      << asio2::detail::test_name() << ": " \
      << msg << std::endl; ASIO2_TEST_LOGSTREAM.flush(); \
    ++asio2::detail::test_errors(); \
  } } while (0)

#define ASIO2_WARN_MESSAGE(expr, msg) \
  do { if (!(expr)) { \
    ASIO2_TEST_LOCK_GUARD \
    if (!test_has_error) \
    { \
      ASIO2_TEST_IOSTREAM << std::endl; \
      ASIO2_TEST_LOGSTREAM << std::endl; \
      test_has_error = true; \
    } \
    std::string_view file{__FILE__}; \
    ASIO2_TEST_IOSTREAM \
      << std::next(std::next(file.data(), file.find_last_of("\\/"))) << "(" << __LINE__ << "): " \
      << asio2::detail::test_name() << ": " \
      << msg << std::endl; \
    ASIO2_TEST_LOGSTREAM \
      << std::next(std::next(file.data(), file.find_last_of("\\/"))) << "(" << __LINE__ << "): " \
      << asio2::detail::test_name() << ": " \
      << msg << std::endl; ASIO2_TEST_LOGSTREAM.flush(); \
  } } while (0)

#define ASIO2_ERROR(msg) \
  do { \
    ASIO2_TEST_LOCK_GUARD \
    std::string_view file{__FILE__}; \
    ASIO2_TEST_IOSTREAM \
      << std::next(std::next(file.data(), file.find_last_of("\\/"))) << "(" << __LINE__ << "): " \
      << asio2::detail::test_name() << ": " \
      << msg << std::endl; \
    ASIO2_TEST_LOGSTREAM \
      << std::next(std::next(file.data(), file.find_last_of("\\/"))) << "(" << __LINE__ << "): " \
      << asio2::detail::test_name() << ": " \
      << msg << std::endl; ASIO2_TEST_LOGSTREAM.flush(); \
    ++asio2::detail::test_errors(); \
  } while (0)

#define ASIO2_TEST_SUITE_IMPL(name, tests) \
  int main() \
  { \
    std::srand((unsigned int)time(nullptr)); \
    asio2::detail::check_memory_leaks(); \
    asio2::detail::begin_test_suite(name); \
    tests \
    int ret = asio2::detail::end_test_suite(name); \
    asio2::detail::pause_cmd_window(); \
    return ret; \
  }

#define ASIO2_TEST_SUITE(name, tests) \
  ASIO2_TEST_SUITE_IMPL(name, tests)

#define ASIO2_TEST_CASE(test) \
  asio2::detail::run_test<&test>(#test);

#define ASIO2_TEST_CASE2(test1, test2) \
  asio2::detail::run_test<&test1, test2>(#test1 "," #test2);

#define ASIO2_TEST_CASE3(test1, test2, test3) \
  asio2::detail::run_test<&test1, test2, test3>( \
    #test1 "," #test2 "," #test3);

#define ASIO2_TEST_CASE4(test1, test2, test3, test4) \
  asio2::detail::run_test<&test1, test2, test3, test4>( \
    #test1 "," #test2 "," #test3 "," #test4);

#define ASIO2_TEST_CASE5(test1, test2, test3, test4, test5) \
  asio2::detail::run_test<&test1, test2, test3, test4, test5>( \
    #test1 "," #test2 "," #test3 "," #test4 "," #test5);

#define ASIO2_COMPILE_TEST_CASE(test) \
  asio2::detail::compile_test<&test>(#test);

#define ASIO2_TEST_BEGIN_LOOP(__loops__) \
  int loops = __loops__; \
  for (int loop = 0; loop < loops; ++loop) \
  { \
    static auto __time1__ = std::chrono::steady_clock::now(); \
    if(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - __time1__).count() > 1) \
    { \
      __time1__ = std::chrono::steady_clock::now(); \
      ASIO2_TEST_LOCK_GUARD \
      ASIO2_TEST_IOSTREAM << '.'; \
      ASIO2_TEST_LOGSTREAM << '.'; \
    }

#define ASIO2_TEST_END_LOOP \
    if (loop + 1 < loops) \
    { \
      ASIO2_TEST_LOCK_GUARD \
      test_has_error = false; \
    } \
  }

#define ASIO2_TEST_WAIT_CHECK(...) \
std::this_thread::sleep_for(std::chrono::milliseconds(1)); \
static int waits = 0; \
if ((++waits) > test_wait_count) \
{ \
  ASIO2_TEST_LOCK_GUARD \
    std::string_view file{__FILE__}; \
    ASIO2_TEST_IOSTREAM \
      << "wait timeout: " \
      << std::next(std::next(file.data(), file.find_last_of("\\/"))) << "(" << __LINE__ << "): "; \
    ASIO2_TEST_LOGSTREAM \
      << "wait timeout: " \
      << std::next(std::next(file.data(), file.find_last_of("\\/"))) << "(" << __LINE__ << "): "; \
    asio2::detail::test_print(__VA_ARGS__); \
    ASIO2_TEST_IOSTREAM << std::endl; \
    ASIO2_TEST_LOGSTREAM << std::endl; ASIO2_TEST_LOGSTREAM.flush(); \
  waits = 0; \
  break; \
} \
std::ignore = waits

inline void null_test()
{
}

#if defined(__GNUC__) && defined(_AIX)

// AIX needs this symbol defined in asio, even if it doesn't do anything.
int test_main(int, char**)
{
}

#endif // defined(__GNUC__) && defined(_AIX)

#include <asio2/base/detail/pop_options.hpp>

#endif // __ASIO2_UNIT_TEST_HPP__
