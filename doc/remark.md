## Modify the code to be compatible with the UE4 "check" macro

##### Modify file /fmt/ranges.h
```c++
// change the function "check(...)" to "(check)(...)"
// line 81,109
/// tuple_size and tuple_element check.
template <typename T> class is_tuple_like_ {
  template <typename U>
  static auto (check)(U* p) -> decltype(std::tuple_size<U>::value, int());
  template <typename> static void (check)(...);

 public:
  static FMT_CONSTEXPR_DECL const bool value =
      !std::is_void<decltype(check<T>(nullptr))>::value;
};
```

##### Modify file /beast/http/detail/type_traits.hpp
```c++
// change the function "check(...)" to "(check)(...)"
// line 35
template<class T>
class is_header_impl
{
    template<bool b, class F>
    static std::true_type (check)(
        header<b, F> const*);
    static std::false_type (check)(...);
public:
    using type = decltype((check)((T*)0));
};
```

##### Modify file /beast/http/basic_parser.hpp
```c++
// change the body and header limit to no limit.
// line 71
std::uint64_t body_limit_ = (std::numeric_limits<std::uint64_t>::max)();
    //default_body_limit(is_request{});   // max payload body
// line 78
std::uint32_t header_limit_ = 1 * 1024 * 1024;     // max header size
```

##### Modify file /beast/http/basic_file_body.hpp
```c++
// to support copyable
```

##### Modify file /beast/core/impl/basic_stream.hpp
```c++
// line 107 
// in function: impl_type::on_timer::operator()(error_code ec)
// beacuase the ec param maybe zero when the timer callback is
// called even if the timer cancel function has called already, 
// then the timer will never exited.
// add this code:
if (!sp->socket.is_open())
    return;
```

## Modify the code to be compatible with the mysql "IS_NUM" macro

##### Modify file /asio2/http/detail/http_parser.h
```c++
// Append "HTTP_" before all macro definitions.
```

##### Modify file /spdlog/spdlog.h
```c++
// Add header only define.
#ifndef ASIO2_DISABLE_AUTO_HEADER_ONLY
#ifndef SPDLOG_HEADER_ONLY
#define SPDLOG_HEADER_ONLY
#endif
#endif
```

##### Modify file /fmt/format.h
```c++
// Add header only define.
#ifndef ASIO2_DISABLE_AUTO_HEADER_ONLY
#ifndef FMT_HEADER_ONLY
#define FMT_HEADER_ONLY
#endif
#endif
```

##### Modify file /cereal/details/static_object.hpp
```c++
// Add No CEREAL_DLL_EXPORT define.
#if defined(_MSC_VER) && !defined(__clang__)
#if defined(ASIO2_ENABLE_CEREAL_DLL_EXPORT)
#   define CEREAL_DLL_EXPORT __declspec(dllexport)
#else
#   define CEREAL_DLL_EXPORT 
#endif
#   define CEREAL_USED
#else // clang or gcc
#if defined(ASIO2_ENABLE_CEREAL_DLL_EXPORT)
#   define CEREAL_DLL_EXPORT __attribute__ ((visibility("default")))
#else
#   define CEREAL_DLL_EXPORT 
#endif
#   define CEREAL_USED __attribute__ ((__used__))
#endif
```
