#pragma once

#include "utils.hpp"

#include <utility>
#include <type_traits>
#include <functional>

namespace deltac {

inline class action_error_t {} action_error;

namespace _impl {

template <typename T>
struct ARDestructBase {
    ARDestructBase() : dummy(), is_valid(false) {}

    template <typename... Args>
    ARDestructBase(std::in_place_t, Args&&... args) : val(std::forward<Args>(args)...), is_valid(true) {}

    ~ARDestructBase() {
        reset();
    }

    ARDestructBase(const ARDestructBase&) = default;
    ARDestructBase(ARDestructBase&&) = default;
    ARDestructBase& operator =(const ARDestructBase&) = default;
    ARDestructBase& operator =(ARDestructBase&&) = default;

    bool reset() {
        bool ret = is_valid;

        if (is_valid) {
            val.~T();
            is_valid = false;
        }

        return ret;
    }

    T& operator *() & {
        return val;
    }

    const T& operator *() const& {
        return val;
    }

    T&& operator *() && {
        return (T&&)val;
    }

    const T&& operator *() const&& {
        return (const T&&)val;
    }

    union {
        T val;
        action_error_t dummy; // why not
    };

    bool is_valid;
};

template <typename T>
struct ARBase : ARDestructBase<T> {
    using Self = ARBase<T>;

    using ARDestructBase<T>::ARDestructBase;

    template <typename Other>
    T& construct(Other&& other) {
        this->is_valid = other.is_valid;

        if (this->is_valid) {
            new(&this->val) T(std::forward<Other>(other));
        }

        return this->val;
    }

    template <typename Other>
    T& assign(Other&& other) {        
        if (this->is_valid) {
            this->val = T(std::forward<Other>(other.val));
        }
        else {
            construct(std::forward<Other>(other));
        }

        return this->val;
    }
};

}

// u happy now, copilot?
template <typename T>
class ActionResult : private _impl::ARBase<T> {
public:
    using Value = T;

    static inline constexpr bool is_ptr_specialization = false;

private:
    using Base = _impl::ARBase<T>;

    template <typename Other> 
    static inline constexpr bool enable_direct_conv = std::conjunction_v<
        std::negation<std::is_same<util::remove_cvref_t<Other>, ActionResult>>, // is not a copy construction
        std::negation<std::is_same<util::remove_cvref_t<Other>, std::in_place_t>>, // is not the in_place singleton
        std::is_constructible<Value, Other> // can be constructed from Other
    >;

public:
    // error constructor
    ActionResult(action_error_t = action_error) {}
    
    // valid value constructor
    template <typename Other = Value, typename = std::enable_if_t<enable_direct_conv<Other>>>
    ActionResult(Other&& val) : Base(std::in_place, std::forward<Other>(val)) {}

    // copy/move contructor
    template <typename Other, typename = std::enable_if_t<std::is_constructible_v<Value, const Other&>>>
    ActionResult(const ActionResult<Other>& other) : Base() {
        this->construct(*other);
    }

    template <typename Other, typename = std::enable_if_t<std::is_constructible_v<Value, Other&&>>>
    ActionResult(ActionResult<Other>&& other) : Base() {
        this->construct(std::move(*other));
    }

    // inplace valid value constructor
    template <typename... Args>
    explicit ActionResult(std::in_place_t, Args&&... args) : Base(std::in_place, std::forward<Args>(args)...) {}

    // error assignment
    ActionResult& operator =(action_error_t) noexcept {
        reset();
        return *this;
    }

    // valid value assignment
    template <typename Other, typename = std::enable_if_t<enable_direct_conv<Other>>>
    ActionResult& operator =(Other&& val) {
        this->assign(std::forward(val));
        return *this;
    }

    // copy/move assignment
    template <typename Other, typename = std::enable_if_t<std::is_assignable_v<Value, const Other&>>>
    ActionResult& operator =(const ActionResult<Other>& other) {
        this->assign(*other);
    }

    template <typename Other, typename = std::enable_if_t<std::is_assignable_v<Value, Other&&>>>
    ActionResult& operator =(ActionResult<Other>&& other) {
        this->assign(std::move(*other));
    }

    template <typename... Args>
    Value& emplace(Args&&... args) {
        reset();
        return this->construct(std::in_place, std::forward<Args>(args)...);
    }

    const Value& get() const { return this->val; }

private:
    Value& get() { return this->val; }

public:
    Value& operator *() & { return get(); }

    const Value& operator *() const& { return get(); }

    Value&& operator *() && { return std::move(get()); }

    const Value&& operator *() const&& { return std::move(get()); }

    using Base::reset;

    bool reset(Value& val) { 
        if (is_usable()) {
            return false;
        }

        val = std::move(this->val);
        reset();

        return true;
    }

    bool is_usable() const { return this->is_valid; }

    explicit operator bool() const { return is_usable(); }
};

template <typename T>
class ActionResult<T*> {
public:
    using Value = T*;

    static inline constexpr bool is_ptr_specialization = true;

public:
    ActionResult(action_error_t = action_error) : ptr(nullptr) {}

    ActionResult(std::nullptr_t) = delete;

    ActionResult(Value ptr) : ptr(ptr) {}

    Value get() const { return ptr; }

    Value& operator *() { return ptr; }

    // make the return type consistent with the non-pointer version
    const Value& operator *() const { return ptr; }
    
    bool is_usable() const { return ptr != nullptr; }

    void reset() { ptr = nullptr; }

    bool reset(Value& val) { 
        if (is_usable()) {
            return false;
        }

        val = ptr;
        reset();

        return true;
    }

    void swap(ActionResult& other) {
        std::swap(ptr, other.ptr);
    }

    void deletep() {
        delete ptr;
        reset();
    }

    explicit operator bool() const { return is_usable(); }

    friend bool operator ==(const ActionResult& lhs, const ActionResult& rhs) {
        return lhs.ptr == rhs.ptr;
    }

    friend bool operator !=(const ActionResult& lhs, const ActionResult& rhs) {
        return lhs.ptr != rhs.ptr;
    }

    friend bool operator ==(const ActionResult& lhs, std::nullptr_t) {
        return lhs.ptr == nullptr;
    }

    friend bool operator ==(std::nullptr_t, const ActionResult& rhs) {
        return nullptr == rhs.ptr;
    }

    friend bool operator !=(const ActionResult& lhs, std::nullptr_t) {
        return lhs.ptr != nullptr;
    }

    friend bool operator !=(std::nullptr_t, const ActionResult& rhs) {
        return nullptr != rhs.ptr;
    }

private:
    Value ptr;
};

template <typename T, typename D>
class GuardRAII {
public:
    using Guarded = T;
    using Deleter = D;

public:
    GuardRAII(Guarded&& guarded, Deleter&& d = Deleter(), bool active = true) : 
        g(std::forward(guarded)), d(std::forward(d)), enabled(active) {}

    ~GuardRAII() {
        if (enabled) {
            std::invoke(d, g);
        }
    }

    Guarded& guarded() { return g; }

    const Guarded& guarded() const { return g; }

    Deleter& deleter() { return d; }

    const Deleter& deleter() const { return d; }

    void activity(bool b) { enabled = b; }

    bool is_active() const { return enabled; }

private:
    Guarded&& g;
    Deleter&& d;
    bool enabled;
};

}
