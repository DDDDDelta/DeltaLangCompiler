#pragma once

#include "token.hpp"
#include "tokentype.hpp"
#include "typeinfo.hpp"
#include "utils.hpp"

#include <concept>
#include <stdfloat>
#include <cstdint>
#include <cstddef>
#include <optional>
#include <utility>

struct Expr {
public:
    TypeInfo type = nullptr;

    virtual ~Expr() = 0;
};

inline Expr::~Expr() = default;

enum class BinaryOp {
    Plus,
    Minus,
    Multiply,
    Divide,
    Modulo,
    And,
    Or,
    Equal,
    NotEqual,
    Less,
    Greater,
    LessEqual,
    GreaterEqual,
    LeftShift,
    RightShift,
    BitwiseAnd,
    BitwiseOr,
    BitwiseXor,
};

std::optional<BinaryOp> to_binary_operator(TokenType type);

bool is_binary_operator(const Token& tk) {
    return to_binary_operator(tk.type).has_value();
}

struct BinaryExpr : public Expr {
public:
    ~BinaryExpr() override { delete lhs; delete rhs; };

    Expr* lhs;
    BinaryOp op;
    Expr* rhs;
};

enum class UnaryOp {
    Plus,
    Minus,
    Not,
    BitwiseNot,
    Deref,
    AddressOf,
};

std::optional<UnaryOp> to_unary_operator(TokenType type);

bool is_unary_operator(const Token& tk) {
    return to_unary_operator(tk.type).has_value();
}

struct UnaryExpr : public Expr {
public:
    ~UnaryExpr() override { delete expr; };

    UnaryOp op;
    Expr* expr;
};

struct PrimaryExpr : public Expr {
public:
    ~PrimaryExpr() override = 0;
};

inline PrimaryExpr::~PrimaryExpr() = default;

struct IdExpr : public PrimaryExpr {
public:
    ~IdExpr() override = default;
};

bool is_literal_token(const Token& type);

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
}

struct StringLiteralExpr : public LiteralExpr {
public:
    ~StringLiteralExpr() override = default;

    std::string_view data;
};

struct ParenExpr : public PrimaryExpr {
public:
    ~ParenExpr() override { delete expr; }

    Expr* expr;
};

struct CastExpr : public PrimiaryExpr {
public:
    ~CastExpr() override { delete expr; }

    Expr* expr;
};
