#pragma once

#include <cassert>
#include <concepts>
#include <type_traits>
#include <utility>
#include <vector>
#include <memory>

#define DELTA_UNREACHABLE(MSG) (assert(false && MSG && "unreachable"), std::unreachable())

template <typename To>
inline bool isinstance(auto* ptr) requires
    std::derived_from<To, std::remove_pointer_t<std::decay_t<decltype(ptr)>>> && 
    std::is_polymorphic_v<std::remove_pointer_t<std::decay_t<decltype(ptr)>>> {
    return dynamic_cast<const To*>(ptr) != nullptr;
}

template <typename To>
inline bool isinstance(auto&& obj) requires 
    std::derived_from<To, std::decay_t<decltype(obj)>> && 
    std::is_polymorphic_v<std::decay_t<decltype(obj)>> {
    return dynamic_cast<const To*>(&obj) != nullptr;
}

template <
    std::ranges::input_range Range, 
    std::invocable<std::ranges::range_value_t<std::remove_cvref_t<Range>>> Deleter 
        = std::default_delete<std::ranges::rangex_value_t<Range>>
> requires std::is_pointer_v<std::ranges::range_value_t<Range>>
void cleanup_ptrs(Range&& range, Deleter&& deleter = Deleter()) {
    for (auto* p : range) {
        deleter(p);
    }
}

class use_move_t {} use_move;

class use_copy_t {} use_copy;
