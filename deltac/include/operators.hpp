#pragma once

#include <optional>
#include <utility>

#include "token.hpp"
#include "tokentype.hpp"

namespace deltac {

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

namespace prec {
enum Binary {
    Unknown = 0,
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
    MaxPrecedence
};
}

// std::strong_ordering operator <=>(BinopPrecedence lhs, BinopPrecedence rhs);

prec::Binary get_precedence(BinaryOp op);

enum class UnaryOp {
    Plus,
    Minus,
    Not,
    BitwiseNot,
    Deref,
    AddressOf,
};

std::optional<BinaryOp> to_binary_operator(tok::Kind type);

inline bool is_binary_operator(const Token& tk) {
    return (bool)to_binary_operator(tk.get_type());
}

std::optional<UnaryOp> to_unary_operator(tok::Kind type);

inline bool is_unary_operator(const Token& tk) {
    return (bool)to_unary_operator(tk.get_type());
}

enum class AssignOp {
    Equal = 0,
    PlusEqual,
    MinusEqual,
    TimesEqual,
    DevideEqual,
    ModEqual,
    LeftShiftEqual,
    RightShiftEqual,
    OrEqual,
    AndEqual,
    XorEqual
};

std::optional<AssignOp> to_assignment_operator(tok::Kind type);

inline bool is_assignment_operator(const Token& tok) {
    return (bool)to_assignment_operator(tok.get_type());
}

}
