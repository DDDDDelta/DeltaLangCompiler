#pragma once

#include "token.hpp"
#include "tokentype.hpp"
#include "utils.hpp"

#include <optional>
#include <utility>
#include <any>

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

struct BinaryExpr : public Expr {
public:
    ~BinaryExpr() override { delete lhs; delete rhs; };

    Expr* lhs;
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
    ~LiteralExpr() override = default;

    TokenType type; // must be one of the literal token types
    std::any data;
};

struct CastExpr : public PrimiaryExpr {
public:
    ~CastExpr() override { delete expr; }

    Expr* expr;
};
