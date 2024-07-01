#pragma once

#include <cassert>
#include <type_traits>
#include <iterator>
#include <string_view>
#include <memory>
#include <cstdint>

#define DELTA_UNREACHABLE(MSG) (assert(false && MSG && "unreachable"), __builtin_unreachable())

#define DELTA_ASSERT(EXPR) assert(EXPR)
#define DELTA_ASSERT_MSG(EXPR, MSG) (DELTA_ASSERT(EXPR && MSG))

namespace deltac::util {

template <class T>
struct remove_cvref {
    using type = std::remove_cv_t<std::remove_reference_t<T>>;
};

template <class T>
using remove_cvref_t = typename remove_cvref<T>::type;

template <typename To, typename From>
inline bool isinstance(From* ptr) {
    return dynamic_cast<const To*>(ptr) != nullptr;
}

template <typename To, typename From>
inline bool isinstance(From&& obj) {
    return dynamic_cast<const To*>(&obj) != nullptr;
}

template <
    typename It, typename Deleter = 
        std::default_delete<std::remove_pointer_t<typename std::iterator_traits<It>::value_type>>
>
void cleanup_ptrs(It begin, It end, Deleter&& deleter = Deleter()) {
    for (; begin != end; ++begin) {
        deleter(*begin);
    }
}

inline constexpr class use_move_t {} use_move;

inline constexpr class use_copy_t {} use_copy;

template <typename T>
constexpr std::underlying_type_t<T> to_underlying(T e) {
    return static_cast<std::underlying_type_t<T>>(e);
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
constexpr std::basic_string_view<CharT, Traits> make_sv(const CharT* begin, const CharT* end) {
    return { begin, (std::size_t)(end - begin) };
}

template <
    typename It, 
    typename CharT = std::remove_reference_t<typename std::iterator_traits<It>::value_type>, 
    typename Traits = std::char_traits<CharT>
>
constexpr std::basic_string_view<CharT, Traits> make_sv(It begin, It end) {
    return { &*begin, (std::size_t)(end - begin) };
}

template <typename It, typename Delim>
constexpr typename std::iterator_traits<It>::value_type join(Delim delim, It begin, It end) {
    using Result = typename std::iterator_traits<It>::value_type;

    if (begin == end) {
        return Result();
    }

    Result first = *begin;
    std::advance(begin, 1);

    for (; begin != end; std::advance(begin, 1)) {
        first += *begin;
    }

    return first;
}

} // namespace util

namespace deltac {

using i8 = std::int8_t;
using u8 = std::uint8_t;
using i16 = std::int16_t;
using u16 = std::uint16_t;
using i32 = std::int32_t;
using u32 = std::uint32_t;
using i64 = std::int64_t;
using u64 = std::uint64_t;
using i128 = __int128;
using u128 = unsigned __int128;
using usize = std::size_t;

}
