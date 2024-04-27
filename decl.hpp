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

    std::string_view get_identifier() { return identifier; }

private:
    std::string identifier;
};

inline Decl::~Decl() = default;

class VarDecl : public Decl {
public:
    VarDecl(std::string identifier, Expr* expr) : 
        Decl(std::move(identifier)), type(std::nullopt), expr(expr) {}

    VarDecl(std::string identifier, const Type& type) : 
        Decl(std::move(identifier)), type(type), expr(nullptr) {}

    VarDecl(std::string identifier, const Type& type, Expr* expr) : 
        Decl(std::move(identifier)), type(type), expr(expr) {}

    ~VarDecl() override = default;

    std::string get_decl_repr() override {
        return 
            "let " + 
            (std::string)get_identifier() + 
            ": " + 
            (type ? type->repr() : (std::string)"<not resolved>") +
            ";";
    }

private:
    std::optional<Type> type;
    Expr* expr;
};

struct Parameter {
    std::string name;
    Type type;
};

