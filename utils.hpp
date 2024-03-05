#pragma once

#include <cassert>
#include <concepts>
#include <type_traits>
#include <utility>
#include <vector>
#include <memory>

#define DELTA_UNREACHABLE(MSG) (assert(false && MSG && "unreachable"), std::unreachable())

template <typename To>
inline bool instanceof(auto* ptr) requires
    std::derived_from<To, std::remove_pointer_t<std::decay_t<decltype(ptr)>>> && 
    std::is_polymorphic_v<std::remove_pointer_t<std::decay_t<decltype(ptr)>>> {
    return dynamic_cast<const To*>(ptr) != nullptr;
}

template <typename To>
inline bool instanceof(auto&& obj) requires 
    std::derived_from<To, std::decay_t<decltype(obj)>> && 
    std::is_polymorphic_v<std::decay_t<decltype(obj)>> {
    return dynamic_cast<const To*>(&obj) != nullptr;
}
