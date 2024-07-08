#pragma once

#include "utils.hpp"

#include "llvm/ADT/SmallVector.h"

#include <array>
#include <limits>
#include <memory>
#include <vector>
#include <algorithm>
#include <iterator>

namespace deltac {

template <typename T>
class Span;

namespace _impl {

// a LegacyRandomAccessIterator
template <typename T, typename P, typename R>
class SpanIter {
public:
    using difference_type   = isize;
    using value_type        = std::remove_cv_t<T>;
    using pointer           = P;
    using reference         = R;
    using iterator_category = std::random_access_iterator_tag;

private:
    SpanIter(pointer p) : p(p) {}

    friend ::deltac::Span<T>;

public:
    /*
     * a, b, objects of type It or const It
     * r, an lvalue of type It
     * n, an integer of type difference_type
     */

    // r++
    SpanIter operator ++(int) { return SpanIter(p++); }

    // ++r
    SpanIter& operator ++() {
        ++p;
        return *this;
    }

    // r--
    SpanIter operator --(int) { return SpanIter(p--); }

    // --r
    SpanIter& operator --() {
        --p;
        return *this;
    }

    // a + n
    SpanIter operator +(difference_type diff) const { return SpanIter(p + diff); }

    // n + a
    friend SpanIter operator +(difference_type diff, const SpanIter& it) { return it + diff; }

    // a - n
    SpanIter operator -(difference_type diff) const { return SpanIter(p - diff); }

    // r += n
    SpanIter& operator +=(difference_type diff) {
        p += diff;
        return *this;
    }

    // r -= n
    SpanIter& operator -=(difference_type diff) {
        p -= diff;
        return *this;
    }

    // b - a
    difference_type operator -(const SpanIter& other) const { return p - other.p; }

    // *a
    reference operator *() const { return *p; }

    // a[n]
    reference operator [](difference_type s) const { return p[s]; } 

    // a->
    pointer operator ->() const { return p; }

    // a (comp) b
    bool operator <(const SpanIter& other) const { return p < other.p; }

    bool operator >(const SpanIter& other) const { return p > other.p; }

    bool operator <=(const SpanIter& other) const { return p <= other.p; }

    bool operator >=(const SpanIter& other) const { return p >= other.p; }

    bool operator ==(const SpanIter& other) const { return p == other.p; }

    bool operator !=(const SpanIter& other) const { return p != other.p; }

public:
    void swap(SpanIter& other) {
        std::swap(p, other.p);
    }

private:
    pointer p;
};

}

// a ReversibleContainer
template <typename T>
class Span {
public:
    using element_type           = T;
    using value_type             = std::remove_cv_t<T>;
    using size_type              = usize;
    using difference_type        = isize;
    using pointer                = T*;
    using const_pointer          = const T*;
    using reference              = T&;
    using const_reference        = const T&;
    using iterator               = _impl::SpanIter<element_type, pointer, reference>;
    using const_iterator         = _impl::SpanIter<element_type, const_pointer, const_reference>;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

public:
    Span() : b(nullptr), e(nullptr) {}

    Span(T* pb, T* pe) : b(pb), e(pe) {}

    Span(T* pb, size_type size) : Span(pb, pb + size) {}

    template <usize Size>
    Span(std::array<T, Size>& arr) : b(arr.data()), e(b + Size) {}

    template <usize Size>
    Span(T(& arr)[Size]) : b(arr), e(b + Size) {}

    template <typename Allocator>
    Span(std::vector<T, Allocator>& vec) : b(vec.data()), e(b + vec.size()) {}

    template <usize Size>
    Span(llvm::SmallVector<T, Size>& svec) : b(svec.data()), e(b + svec.size()) {}

    Span(const Span&) = default;

    Span& operator =(const Span& rhs) = default;

public:
    bool operator ==(const Span& other) {
        return std::equal(begin(), end(), other.begin(), other.end());
    }

    bool operator !=(const Span& other) {
        return !(*this == other);
    }

public:
    T* data() const { return b; }

    size_type size() const { return e - b; }

    size_type max_size() const { return std::numeric_limits<size_type>::max(); }

    bool empty() const { return b == nullptr; }

    void swap(Span& other) {
        std::swap(b, other.b);
        std::swap(e, other.e);
    }

    iterator begin() {
        return b;
    }

    iterator end() {
        return e;
    }

    const_iterator begin() const {
        return cbegin();
    }

    const_iterator end() const {
        return cend();
    }

    const_iterator cbegin() const {
        return b;
    }

    const_iterator cend() const {
        return e;
    }

    reverse_iterator rbegin() {
        return e - 1;
    }

    reverse_iterator rend() {
        return b - 1;
    }

    const_reverse_iterator rbegin() const {
        return rbegin();
    }

    const_reverse_iterator rend() const {
        return rend();
    }

    const_reverse_iterator rcbegin() const {
        return e - 1;
    }

    const_reverse_iterator rcend() const {
        return b - 1;
    }

private:
    T* b;
    T* e;
};

}
