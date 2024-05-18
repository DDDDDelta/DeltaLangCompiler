#pragma once

#include <cassert>
#include <type_traits>
#include <utility>
#include <vector>
#include <memory>

#define DELTA_UNREACHABLE(MSG) (assert(false && MSG && "unreachable"), __builtin_unreachable())

namespace util {

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

class use_move_t {} use_move;

class use_copy_t {} use_copy;

template <typename T>
std::underlying_type_t<T> to_underlying(T e) {
    return static_cast<std::underlying_type_t<T>>(e);
}

}