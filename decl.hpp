#pragma once

#include "expression.hpp"
#include "typeinfo.hpp"

#include "llvm/ADT/SmallVector.h"

#include <optional>

class Decl {
public:
    Decl(std::string identifier) :
        identifier(std::move(identifier)) {}
    
    Decl(const Decl&) = delete;
    Decl(Decl&&) = delete;
    virtual ~Decl() = 0;

    virtual std::string get_decl_repr() = 0;
    virtual bool has_body() = 0;

    std::string_view get_identifier() { return identifier; }

private:
    std::string identifier;
};

inline Decl::~Decl() = default;

class VarDecl : public Decl {
public:
    VarDecl(std::string identifier, Expr* expr) : 
        Decl(std::move(identifier)), type(expr->type()), expr(expr) {}

    VarDecl(std::string identifier, QualType type) : 
        Decl(std::move(identifier)), type(std::move(type)), expr(nullptr) {}

    VarDecl(std::string identifier, QualType type, Expr* expr) : 
        Decl(std::move(identifier)), type(std::move(type)), expr(expr) {}

    ~VarDecl() override = default;

    std::string get_decl_repr() override {
        return 
            "let " + 
            (std::string)get_identifier() + 
            ": " + 
            type.repr() +
            ";";
    }

    const QualType& decl_type() { return type; }
    void decl_type(QualType type) { this->type = std::move(type); }

    Expr* get_expr() { return expr; }
    
    void reset_expr(Expr* e = nullptr) { 
        delete expr;
        this->expr = e; 
    }

    bool has_body() override { return expr != nullptr; }

private:
    QualType type;
    Expr* expr;
};

struct Parameter {
    std::string name;
    QualType type;
};

class FunctionDecl;

class TypeDecl;
