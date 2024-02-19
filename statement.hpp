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

template <typename FormatContext>
std::string format_stmt(const Stmt& stmt, FormatContext& ctx, int indent = 0) {
    std::string indent_str = std::string(indent*4, ' ');
    if (auto* expr_stmt = dynamic_cast<const ExprStmt*>(&stmt)) {
        return std::format("{0}{1}{2}{0}}}\n", indent_str, *expr_stmt, format_expr(*expr_stmt->expr, ctx, indent + 1));
    } else if (auto* comp_stmt = dynamic_cast<const CompoundStmt*>(&stmt)) {
        std::string result = indent_str + std::format("{}", *comp_stmt);
        for (const auto* inner_Stmt : comp_stmt->stmts) {
            result += format_stmt(*inner_Stmt, ctx, indent + 1);
        }
        return result + indent_str + "}\n";
    } else if (auto* funcdef_stmt = dynamic_cast<const FuncDefStmt*>(&stmt)) {
        std::string result = indent_str + std::format("{}", *funcdef_stmt);
        for (const auto& param : funcdef_stmt->params) {
            result += indent_str + std::string(4, ' ') + std::format("{}", param);
        }
        result += format_stmt(funcdef_stmt->funcbody, ctx, indent + 1);
        return result + indent_str + "}\n";
    } else if (auto* letdef_stmt = dynamic_cast<const LetDefStmt*>(&stmt)) {
        return std::format("{0}{1}{2}{0}}}\n", indent_str, *letdef_stmt, format_expr(*letdef_stmt->defexpr, ctx, indent + 1));
    }
    return "unknown_expr";
}

template<>
struct std::formatter<Parenmeter> : std::formatter<std::string> {
    template <typename FormatContext>
    auto format(const Parenmeter& param, FormatContext& ctx) -> decltype(ctx.out()) {
        return std::format_to(ctx.out(), "Parameter: {} '{}'\n", param.identifier, param.type.name);
    }
};

template<>
struct std::formatter<ExprStmt> : std::formatter<std::string> {
    template <typename FormatContext>
    auto format(const ExprStmt& stmt, FormatContext& ctx) -> decltype(ctx.out()) {
        return std::format_to(ctx.out(), "ExprStmt: {{\n");
    }
};

template<>
struct std::formatter<CompoundStmt> : std::formatter<std::string> {
    template <typename FormatContext>
    auto format(const CompoundStmt& stmt, FormatContext& ctx) -> decltype(ctx.out()) {
        return std::format_to(ctx.out(), "CompoundStmt: {{\n");
    }
};

template<>
struct std::formatter<FuncDefStmt> : std::formatter<std::string> {
    template <typename FormatContext>
    auto format(const FuncDefStmt& stmt, FormatContext& ctx) -> decltype(ctx.out()) {
        return std::format_to(ctx.out(), "FuncDefStmt: {} {} {{\n", stmt.rettype.name, stmt.identifier);
    }
};

template<>
struct std::formatter<LetDefStmt> : std::formatter<std::string> {
    template <typename FormatContext>
    auto format(const LetDefStmt& stmt, FormatContext& ctx) -> decltype(ctx.out()) {
        return std::format_to(ctx.out(), "LetDefStmt: {} {} {{\n", stmt.vartype.name, stmt.identifier);
    }
};
