#pragma once

#include "token.hpp"
#include "tokentype.hpp"

enum class PostfixOp {
    Call,
    Index,
    LiteralTypeSpec
};

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

enum class BinopPrecedence {
    Unknown,
    Or,
    And,
    BitwiseOr,
    BitwiseXor,
    BitwiseAnd,
    Equality,
    Relational,
    Shift,
    Add,
    Multiply,
};

BinopPrecedence get_precedence(BinaryOp op);

enum class UnaryOp {
    Plus,
    Minus,
    Not,
    BitwiseNot,
    Deref,
    AddressOf,
};

std::optional<BinaryOp> to_binary_operator(TokenType type);

inline bool is_binary_operator(const Token& tk) {
    return to_binary_operator(tk.get_type()).has_value();
}

std::optional<UnaryOp> to_unary_operator(TokenType type);

inline bool is_unary_operator(const Token& tk) {
    return to_unary_operator(tk.get_type()).has_value();
}