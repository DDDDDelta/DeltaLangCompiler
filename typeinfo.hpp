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

class ASTContext;

class Type;
class PtrType;
class FunctionType;
class BuiltinType;

/*
 * TypeDeleter does not delete canonical types.
 */
class TypeDeleter {
public:
    TypeDeleter() = default;

    void operator()(const Type* ty) const;
};

enum class Qualification {
    NoQual,
    Const,
    N_A
};

/* 
 * Qual(lified) Type
 * act as a smart pointer to Type
 */
class QualType {
public:
    QualType() : type(/*  */), qual(Qualification::N_A) {}

    QualType(Type* ty);

    QualType(Type* ty, Qualification q) : 
        type(ty), qual(q) {
        // assumed to have the correct qualification
    }

    QualType(const QualType& other);

    QualType(QualType&& other) : type(other.type) {
        other.type = nullptr;
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
        std::swap(qual, other.qual);
    }

    ~QualType() { TypeDeleter()(type); }

public:
    std::string repr() const;

    bool can_be_vardecl_ty() const;
    std::size_t size() const;
    bool is_const() const;
    bool is_mutable() const;
    bool is_ptr_ty() const;
    bool is_func_ty() const;
    bool is_void_ty() const;
    void remove_ptr();
    void add_ptr(bool constness = false);

    friend bool operator ==(const QualType& lhs, const QualType& rhs);

private:
    Qualification qual;
    Type* type;
};


class Type {
public:
    virtual ~Type() = 0;

    virtual std::string repr() const = 0;

    virtual std::size_t size() const = 0;

    virtual Type* copy() const = 0;
};

inline Type::~Type() = default;

/* 
 * Represents the type of a pointer.
 * Can point to any type
 */
class PtrType : public Type {
public:
    PtrType(QualType type) : type_under(std::move(type)) {
        assert(type != nullptr);
    }
    ~PtrType() override = default;

    std::string repr() const override { return "*" + type_under.repr(); }

    std::size_t size() const override { return 8; }

    Type* copy() const override { return new PtrType(type_under); }

    QualType& pointee() { return type_under; }
    const QualType& pointee() const { return type_under; }

private:
    QualType type_under;
};

/*
 * Represents the type of a function.
 * FunctionType is always const. It cannot be the type of a VarDecl.
 */
class FunctionType : public Type {
public:
    FunctionType(llvm::ArrayRef<QualType> args_ty, QualType return_ty, use_copy_t = use_copy) : 
        args_ty(args_ty), return_ty(std::move(return_ty)) {}

    ~FunctionType() override = default;

    std::string repr() const override {
        std::string ret = "fn (";
        
        for (auto&& ty : args_ty) {
            ret += ty.repr() + ", ";
        }

        ret += ") -> " + return_ty.repr();
        return ret;
    }

    std::size_t size() const override { return 0; }

    Type* copy() const override {
        llvm::SmallVector<QualType> new_args_ty;
        for (auto ty : args_ty) {
            new_args_ty.push_back(ty);
        }
        return new FunctionType(new_args_ty, return_ty);
    }

private:
    llvm::SmallVector<QualType> args_ty;
    QualType return_ty;
};

/*
 * Represents the builtin type of the language.
 * Like other canonical types, its lifetime is managed by ASTContext.
 */
class BuiltinType : public Type {
public:
    enum class Kind : unsigned {
#define BUILTIN_TYPE(ID, NAME, SIZE) ID,
#include "builtin_type.inc"
    };

    using enum Kind;

protected:
    friend class ASTContext;

    BuiltinType(Kind kind) : kind(kind) {}

public:
    ~BuiltinType() override = default;

    std::string repr() const override;

    std::size_t size() const override;

    Kind get_kind() const { return kind; }

    // BuiltinType is guarenteed to be const since all public methods are const
    // We const_cast away the constness to keep the consistency with other Type objects
    Type* copy() const override { return const_cast<BuiltinType*>(this); }

private:
    Kind kind;
};

std::size_t get_size(BuiltinType::Kind kind);
std::string to_string(BuiltinType::Kind kind);