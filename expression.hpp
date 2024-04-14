#pragma once

#include "token.hpp"
#include "tokentype.hpp"
#include "typeinfo.hpp"
#include "utils.hpp"
#include "typeinfo.hpp"
#include "operators.hpp"

#include <concepts>
#include <cstdint>
#include <cstddef>
#include <optional>
#include <utility>
#include <any>
#include <string>
#include <string_view>
#include <format>
#include <charconv>

struct Expr {
public:
    TypeInfo type;

    virtual ~Expr() = 0;
};

inline Expr::~Expr() = default;


struct BinaryExpr : public Expr {
public:
    ~BinaryExpr() override { delete lhs; delete rhs; };

    Expr* lhs  = nullptr;
    BinaryOp op;
    Expr* rhs  = nullptr;
};

struct UnaryExpr : public Expr {
public:
    ~UnaryExpr() override { delete mainexpr; };

    UnaryOp op;
    Expr* mainexpr = nullptr;
};

using ExprList = std::vector<Expr*>;

struct PostfixExpr : public Expr {
public:
    ~PostfixExpr() override = 0;

    Expr* mainexpr = nullptr;
    PostfixOp op;
};

inline PostfixExpr::~PostfixExpr() = default;

struct CallExpr : public PostfixExpr {
public:
    ~CallExpr() override {
        for (auto* arg : args) {
            delete arg;
        }
    }

    ExprList args;
};

struct IndexExpr : public PostfixExpr {
public:
    ~IndexExpr() override { delete index; }

    Expr* index = nullptr;
};

struct CastExpr : public Expr {
public:
    ~CastExpr() override { delete expr; }

    Expr* expr = nullptr;
};

struct PrimaryExpr : public Expr {
public:
    ~PrimaryExpr() override = 0;
};

inline PrimaryExpr::~PrimaryExpr() = default;

struct IdExpr : public PrimaryExpr {
public:
    ~IdExpr() override = default;

    std::string identifier;
};

inline bool is_literal_token(const Token& tk) {
    return tk.is_one_of(
        TokenType::HexIntLiteral, 
        TokenType::DecIntLiteral, 
        TokenType::FloatLiteral, 
        TokenType::StringLiteral, 
        TokenType::CharLiteral
    );
}

struct LiteralExpr : public PrimaryExpr {
public:
    ~LiteralExpr() override = 0;
};

inline LiteralExpr::~LiteralExpr() = default;

struct IntLiteralExpr : public LiteralExpr {
public:
    ~IntLiteralExpr() override = default;
    
    template <std::integral T>
    T get_int() {
        T val;
        std::memcpy(&val, data, sizeof(T));
        return val;
    }

    template <std::integral T>
    void set_int(T val) {
        std::memcpy(data, &val, sizeof(T));
    }

private:
    std::byte data[8] = {};
};

struct FloatLiteralExpr : public LiteralExpr {
public:
    ~FloatLiteralExpr() override = default;

    double data = 0.0;
};

struct StringLiteralExpr : public LiteralExpr {
public:
    ~StringLiteralExpr() override = default;

    std::string_view data;
};

struct ParenExpr : public PrimaryExpr {
public:
    ~ParenExpr() override { delete expr; }

    Expr* expr = nullptr;
};

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
