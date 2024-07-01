#pragma once

#include "utils.hpp"

#include <memory>
#include <array>
#include <vector>

namespace deltac {

template <typename T>
class Span {
public:
    template <typename U>
    Span(U* pb, U* pe) : b(pb), e(pe) {}

    template <usize Size>
    Span(std::array<T, Size>& arr) : b(arr.data()), e(b + Size) {}

    template <usize Size>
    Span(T(& arr)[Size]) : b(arr), e(b + Size) {}

    template <typename Allocator = std::allocator<T>>
    Span(std::vector<T, Allocator>& vec) : b(vec.data()), e(b + vec.size()) {}

private:
    T* b;
    T* e;
};

}
