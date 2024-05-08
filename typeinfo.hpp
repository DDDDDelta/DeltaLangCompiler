#pragma once

#include "utils.hpp"

#include <string>
#include <string_view>
#include <utility>
#include <cassert>
#include <set>
#include <compare>

#include "llvm/ADT/ArrayRef.h"

class BasicTypeManager;

class BasicType {
public:
    static BasicType* void_ty;

public:
    explicit BasicType(std::string name) : name(std::move(name)) {}
    BasicType(const BasicType&) = delete;
    BasicType(BasicType&&) = delete;

public:
    std::string_view repr() const {
        return name;
    }

    friend bool operator ==(const BasicType& lhs, const BasicType& rhs) = default;

private:
    std::string name;
};

/*
 * Type -+-> CompoundType -+-> PtrType
 *       |                 +-> ArrayRefType
 *       |                 `-> ArrayType
 *       +-> BaseType
 *       `-> FunctionType
 * 
 * 
 */
class QualType {
private:
    class Type;
    class FunctionType;
    class PtrType;
    class BaseType;

public:
    QualType() : type(new BaseType(BasicType::void_ty, false)), conctype((BaseType*)type) {}

    QualType(BasicType* basictype, bool constness = false) : 
        type(new BaseType(basictype, constness)), conctype((BaseType*)type) {}

    QualType(const QualType& other) : compoundlv(other.compoundlv) {
        conctype = new BaseType(other.conctype->basic_ty(), other.conctype->constness());
        type = other.type->copy(conctype);
    }

    QualType(QualType&& other) : 
        type(other.type), conctype(other.conctype), compoundlv(other.compoundlv) {
        other.type = nullptr;
        other.conctype = nullptr;
    }

    QualType& operator =(const QualType& rhs) {
        if (&rhs == this) {
            return *this;
        }

        QualType newty = rhs;
        swap(newty);
        return *this;
    }

    QualType& operator =(QualType&& rhs) {
        if (&rhs == this) {
            return *this;
        }

        swap(rhs);
        return *this;
    }

    void swap(QualType& other) {
        std::swap(type, other.type);
        std::swap(conctype, other.conctype);
        std::swap(compoundlv, other.compoundlv);
    }

    ~QualType() { delete type; }

public:
    std::string repr() const {
        return type->repr();
    }

    bool constness() const {
        return type->constness();
    }

    void constness(bool b) {
        type->constness(b);
    }

    bool is_compound_ty() const {
        return isinstance<CompoundType>(*type);
    }

    bool is_ptr_ty() const {
        return isinstance<PtrType>(*type);
    }

    bool is_func_ty() const {
        return isinstance<FunctionType>(type);
    }

    void remove_ptr() {
        assert(is_ptr_ty());
        auto* garbage = type;

        type = ((CompoundType*)type)->release();
        compoundlv--;

        delete garbage;
    }

    void add_ptr(bool constness = false) {
        type = new PtrType(type, constness);
        compoundlv++;
    }

    BasicType* get_basic_ty() const {
        return conctype->basic_ty();
    }

    void set_basic_ty(BasicType* type) {
        conctype->reset(type);
    }

    std::size_t get_compound_lv() const {
        return compoundlv;
    }

    friend bool operator ==(const QualType& lhs, const QualType& rhs) {
        if (*lhs.conctype->basic_ty() != *rhs.conctype->basic_ty())
            return false;
        
        // we only have ptrty currently
        return lhs.compoundlv == rhs.compoundlv;
    }

    std::size_t size() const {
        return 
    }

private:
    class Type {
    public:
        Type(bool constness) : is_const(constness) {}
        Type(const Type&) = delete;
        Type(Type&&) = delete;
        virtual ~Type() = default;
        
        std::string repr() const {
            return get_name() + (is_const ? " const" : "");
        }

        bool constness() const {
            return is_const;
        }

        void constness(bool c) {
            is_const = c;
        }

        [[nodiscard]] 
        virtual Type* copy(BaseType* concty) const = 0;

    protected:
        virtual std::string get_name() const = 0;
        friend class PtrType;

    private:
        bool is_const;
    };

    class CompoundType : public Type {
    public:
        CompoundType(Type* type, bool constness) : Type(constness), type_under(type) {}
        ~CompoundType() override {
            delete type_under;
        }

        Type* release() noexcept {
            Type* ret = type_under;
            type_under = nullptr;
            return ret;
        }

    protected:
        Type* type_under;
    };

    class FunctionType : public Type {
    public:
        FunctionType(llvm::ArrayRef<Type*> args_ty, Type* return_ty) : 
            Type(true), args_ty(args_ty), return_ty(return_ty) {}
        
        ~FunctionType() override {
            cleanup_ptrs(args_ty);
            delete return_ty;
        }

    private:
        llvm::SmallVector<Type*> args_ty;
        Type* return_ty;
    };

    class PtrType : public CompoundType {
    public:
        PtrType(Type* type, bool constness) : CompoundType(type, constness) {
            assert(type != nullptr);
        }
        ~PtrType() override = default;

        [[nodiscard]] 
        Type* copy(BaseType* concty) const override {
            return new PtrType(type_under->copy(concty), constness());
        }

    protected:
        std::string get_name() const override { return "*" + type_under->get_name(); }
    };

    class BaseType : public Type {
    public:
        BaseType(BasicType* type, bool constness) : Type(constness), type(type) {}
        ~BaseType() override = default;

        void reset(BasicType* newty) {
            assert(newty != nullptr);

            type = newty;
        }

        BasicType* basic_ty() const {
            return type;
        }

        [[nodiscard]] Type* copy(BaseType* concty) const override {
            return concty;
        }

    protected:
        std::string get_name() const override { return (std::string)type->repr(); }

    private:
        BasicType* type; // do not take ownership
    };

private:
    Type* type;
    BaseType* conctype;
    std::size_t compoundlv = 0;
};
