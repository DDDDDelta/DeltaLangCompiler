#pragma once

#include "expression.hpp"
#include "typeinfo.hpp"

#include <vector>

struct Stmt {
public:
    virtual ~Stmt() = 0;
};

inline Stmt::~Stmt() = default;

using StmtList = std::vector<Stmt*>;

struct Parenmeter {
    std::string identifier;
    TypeInfo type;
};

using ParameterList = std::vector<Parenmeter>;

struct ExprStmt : public Stmt {
public:
    ~ExprStmt() override { delete expr; }

    Expr* expr;
};

struct CompoundStmt : public Stmt {
public:
    ~CompoundStmt() override {
        for (auto* stmt : stmts) {
            delete stmt;
        }
    }

    StmtList stmts;
};

struct DefStmt : public Stmt {
public:
    std::string identifier;
};

struct FuncDefStmt : public DefStmt {
public:
    ParameterList params;
    TypeInfo rettype;
    CompoundStmt funcbody;
};

struct LetDefStmt : public DefStmt {
public:
    TypeInfo vartype;
    Expr* defexpr;
};
