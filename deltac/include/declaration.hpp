#pragma once

#include "expression.hpp"
#include "ownership.hpp"
#include "typeinfo.hpp"

#include "llvm/ADT/SmallVector.h"

#include <optional>
#include <string>
#include <utility>

namespace deltac {

class Decl {
public:
    Decl() = default;
    virtual ~Decl() = 0;

    virtual std::string get_decl_repr() = 0;
};

class NamedDecl : public Decl {
public:
    NamedDecl(std::string identifier) :
        identifier(std::move(identifier)) {}
    
    NamedDecl(const Decl&) = delete;
    NamedDecl(Decl&&) = delete;
    virtual ~NamedDecl() = 0;

    virtual std::string get_decl_repr() = 0;

    std::string_view get_identifier() { return identifier; }

private:
    std::string identifier;
};

inline Decl::~Decl() = default;

class VarDecl : public NamedDecl {
public:
    VarDecl(std::string identifier, Expr* expr) : 
        NamedDecl(std::move(identifier)), type(expr->type()), expr(expr) {}

    VarDecl(std::string identifier, QualType type, Expr* expr = nullptr) : 
        NamedDecl(std::move(identifier)), type(std::move(type)), expr(expr) {}

    ~VarDecl() override = default;

    std::string get_decl_repr() override {
        return 
            "let " + 
            (std::string)get_identifier() + 
            ": " + 
            type.repr() +
            ";";
    }

    const QualType& decl_type() const { return type; }
    void decl_type(QualType type) { this->type = std::move(type); }

    Expr* get_expr() { return expr; }
    
    void reset_expr(Expr* e = nullptr) { 
        delete expr;
        this->expr = e;
    }

    bool has_body() const { return expr != nullptr; }

private:
    QualType type;
    Expr* expr;
};

struct Parameter {
    std::string name;
    QualType type;
};

class FuncDecl;

class TypeDecl;

} // namespace deltac
