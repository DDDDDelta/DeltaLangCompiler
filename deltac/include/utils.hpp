#pragma once

#include <cassert>
#include <type_traits>
#include <utility>
#include <vector>
#include <string_view>
#include <memory>

#define DELTA_UNREACHABLE(MSG) (assert(false && MSG && "unreachable"), __builtin_unreachable())


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

inline class use_move_t {} use_move;

inline class use_copy_t {} use_copy;

template <typename T>
std::underlying_type_t<T> to_underlying(T e) {
    return static_cast<std::underlying_type_t<T>>(e);
}

template <typename CharT, class Traits = std::char_traits<CharT>>
constexpr std::basic_string_view<CharT, Traits> make_sv(const CharT* begin, const CharT* end) {
    return { begin, (std::size_t)(end - begin) };
}

template <
    typename It, 
    typename CharT = remove_cvref_t<decltype(*It())>, 
    typename Traits = std::char_traits<CharT>
>
constexpr std::basic_string_view<CharT, Traits> make_sv(It begin, It end) {
    return { &*begin, (std::size_t)(end - begin) };
}

} // namespace util
