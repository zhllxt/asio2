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
std::uint32_t header_limit_ = (std::numeric_limits<std::uint32_t>::max)();     // max header size
```

##### Modify file /beast/http/basic_file_body.hpp
```c++
// to support copyable
```

## Modify the code to be compatible with the mysql "IS_NUM" macro

##### Modify file /asio2/http/detail/http_parser.h
```c++
// Append "HTTP_" before all macro definitions.
```
