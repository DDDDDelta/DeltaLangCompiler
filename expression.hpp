#pragma once

#include "token.hpp"
#include "tokentype.hpp"
#include "utils.hpp"
#include "typeinfo.hpp"

#include <optional>
#include <utility>
#include <any>
#include <string>

struct Expr {
public:
    TypeInfo type;

    virtual ~Expr() = default;
    virtual std::string to_string() const = 0; // for test
};

// inline Expr::~Expr() = default;

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

    std::string to_string() const override; // for test
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

    std::string to_string() const override; // for test
};

struct PrimaryExpr : public Expr {
public:
    ~PrimaryExpr() override = 0;
};

inline PrimaryExpr::~PrimaryExpr() = default;

struct IdExpr : public PrimaryExpr {
public:
    ~IdExpr() override = default;
    std::string to_string() const override; // for test
};

bool is_literal_token(const Token& type);

struct LiteralExpr : public PrimaryExpr {
public:
    ~LiteralExpr() override = default;

    TokenType type; // must be one of the literal token types
    std::any data;

    std::string to_string() const override; // for test
};

struct CastExpr : public PrimaryExpr {
public:
    ~CastExpr() override { delete expr; }

    Expr* expr;

    std::string to_string() const override; // for test
};
