#pragma once

#include "token.hpp"
#include "tokentype.hpp"
#include "typeinfo.hpp"
#include "utils.hpp"
#include "typeinfo.hpp"
#include "operators.hpp"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/APSInt.h"
#include "llvm/ADT/SmallVector.h"

#include <concepts>
#include <cstdint>
#include <cstddef>
#include <optional>
#include <utility>
#include <any>
#include <string>
#include <string_view>
// #include <format>

class Expr {
public:
    Expr() = default;
    Expr(QualType type) : exprtype(std::move(type)) {}
    Expr(const Expr&) = delete;
    Expr(Expr&&) = delete;
    virtual ~Expr() = 0;

    bool is_rval() const {
        return typecate == TypeCate::RValue;
    }

    bool is_lval() const {
        return typecate == TypeCate::LValue;
    }

    bool is_unclassified() const {
        return typecate == TypeCate::Unclassified;
    }

    bool can_modify() {
        return !exprtype.constness();
    }

    QualType& type() {
        return exprtype;
    }

    const QualType& type() const {
        return exprtype;
    }

protected:
    enum class TypeCate {
        RValue,
        LValue,
        Unclassified,
    } typecate;

private:
    QualType exprtype;
};

inline Expr::~Expr() = default;

class BinaryExpr : public Expr {
public:
    BinaryExpr(QualType type, Expr* lhs, BinaryOp op, Expr* rhs) : 
        Expr(std::move(type)), exprs { lhs, rhs }, op(op) {}

    ~BinaryExpr() override { delete exprs[LHS]; delete exprs[RHS]; };

    Expr* lhs() const { return exprs[LHS]; }
    void lhs(Expr* expr) { exprs[LHS] = expr; }
    Expr* rhs() const { return exprs[RHS]; }
    void rhs(Expr* expr) { exprs[RHS] = expr; }

    BinaryOp op_code() { return op; }

private:
    enum { LHS, RHS, EXPR_END };
    Expr* exprs[EXPR_END];
    BinaryOp op;
};

class UnaryExpr : public Expr {
public:
    UnaryExpr(QualType type, UnaryOp op, Expr* expr) :
        Expr(std::move(type)), op(op), mainexpr(expr) {}

    ~UnaryExpr() override { delete mainexpr; };

    Expr* expr() const { return mainexpr; }
    void expr(Expr* expr) { mainexpr = expr; }

private:
    UnaryOp op;
    Expr* mainexpr;
};

class PostfixExpr : public Expr {
public:
    PostfixExpr(const QualType& type, Expr* expr) :
        Expr(type), mainexpr(expr) {}
    ~PostfixExpr() override = 0;

    Expr* expr() const { return mainexpr; }
    void expr(Expr* expr) { mainexpr = expr; }

private:
    Expr* mainexpr;
};

inline PostfixExpr::~PostfixExpr() = default;

class CallExpr : public PostfixExpr {
public:
    CallExpr(QualType type, Expr* expr, llvm::ArrayRef<Expr*> arguments) : 
        PostfixExpr(std::move(type), expr), args(arguments) {}

    ~CallExpr() override {
        for (auto* arg : args) {
            delete arg;
        }
    }

private:
    llvm::SmallVector<Expr*> args;
};

class IndexExpr : public PostfixExpr {
public:
    IndexExpr(QualType type, Expr* expr, Expr* index) : 
        PostfixExpr(std::move(type), expr), index(index) {}

    ~IndexExpr() override { delete index; }

private:
    Expr* index = nullptr;
};

class CastExpr : public Expr {
public:
    CastExpr(QualType type, Expr* expr) : Expr(std::move(type)), expr(expr) {}
    ~CastExpr() override { delete expr; }

private:
    Expr* expr;
};

class IdExpr : public Expr {
public:
    IdExpr(QualType type, std::string_view id) : 
        Expr(std::move(type)), identifier(id) {}
    ~IdExpr() override = default;

private:
    std::string identifier;
};

class IntLiteralExpr : public Expr {
public:
    IntLiteralExpr(
        QualType type, 
        std::uint32_t numbits, 
        std::string_view literalrepr, 
        bool is_unsigned = true,
        std::uint8_t radix = 10
    ) : Expr(std::move(type)), data(llvm::APInt(numbits, literalrepr, radix), is_unsigned) {}

    IntLiteralExpr(QualType type, llvm::APInt data, bool is_unsigned = true) : 
        Expr(std::move(type)), data(std::move(data), is_unsigned) {}
    
    ~IntLiteralExpr() override = default;

private:
    llvm::APSInt data;
};

class ParenExpr : public Expr {
public:
    ParenExpr(Expr* expr) : expr(expr) {
        type() = expr->type();
    }

    ~ParenExpr() override { delete expr; }

private:
    Expr* expr;
};

/*

template <typename FormatContext>
std::string format_expr(const Expr& expr, FormatContext& ctx, int indent = 0) {
    std::string indent_str = std::string(indent*4, ' ');
    if (auto* literal = dynamic_cast<const LiteralExpr*>(&expr)) {
        return std::format("{}{}", indent_str, *literal);
    } else if (auto* id = dynamic_cast<const IdExpr*>(&expr)) {
        return std::format("{}{}", indent_str, *id);
    } else if (auto* bin = dynamic_cast<const BinaryExpr*>(&expr)) {
        return std::format("{0}{1}{2}{3}{0}}}\n", indent_str, *bin, format_expr(*bin->lhs, ctx, indent + 1),
                           format_expr(*bin->rhs, ctx, indent + 1));
    } else if (auto* unary = dynamic_cast<const UnaryExpr*>(&expr)) {
        return std::format("{0}{1}{2}{0}}}\n", indent_str, *unary, format_expr(*unary->expr, ctx, indent + 1));
    } else if (auto* cast = dynamic_cast<const CastExpr*>(&expr)) {
        return std::format("{0}{1}{2}{0}}}\n", indent_str, *cast, format_expr(*cast->expr, ctx, indent + 1));
    }
    return "unknown_expr";
}

template<>
struct std::formatter<BinaryExpr> : std::formatter<std::string> {
    template <typename FormatContext>
    auto format(const BinaryExpr& bin_expr, FormatContext& ctx) -> decltype(ctx.out()) {
        return std::format_to(ctx.out(), "BinaryExpr: some bin op {{\n");
    }
};

template<>
struct std::formatter<IdExpr> : std::formatter<std::string> {
    template <typename FormatContext>
    auto format(const IdExpr& expr, FormatContext& ctx) {
        std::string data_str = "some id";
        return std::format_to(ctx.out(), "IdExpr: {}\n", data_str);
    }
};

template<>
struct std::formatter<LiteralExpr> : std::formatter<std::string> {
    template <typename FormatContext>
    auto format(const LiteralExpr& expr, FormatContext& ctx) {
        std::string data_str = "some literal data";
        return std::format_to(ctx.out(), "LiteralExpr: {}\n", data_str);
    }
};

template<>
struct std::formatter<CastExpr> : std::formatter<std::string> {
    template <typename FormatContext>
    auto format(const CastExpr& expr, FormatContext& ctx) {
        return std::format_to(ctx.out(), "CastExpr: {{\n");
    }
};

template<>
struct std::formatter<UnaryExpr> : std::formatter<std::string> {
    template <typename FormatContext>
    auto format(const UnaryExpr& expr, FormatContext& ctx) {
        std::string op_str;
        switch (expr.op) {
            case UnaryOp::Plus:
                op_str = "Plus '+'";
                break;
            case UnaryOp::Minus:
                op_str = "Minus '-'";
                break;
            case UnaryOp::Not:
                op_str = "Not '!'";
                break;
            case UnaryOp::BitwiseNot:
                op_str = "BitwiseNot '~'";
                break;
            case UnaryOp::Deref:
                op_str = "Deref '*'";
                break;
            case UnaryOp::AddressOf:
                op_str = "AddressOf '&'";
                break;
            default:
                op_str = "Unknown";
        }
        return std::format_to(ctx.out(), "UnaryExpr: {}\n", op_str);
    }
};

*/
