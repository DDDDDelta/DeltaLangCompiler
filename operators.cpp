#include "operators.hpp"

std::optional<BinaryOp> to_binary_operator(TokenType type) {
    switch (type) {
    case TokenType::Plus: 
        return BinaryOp::Plus;
    case TokenType::Minus: 
        return BinaryOp::Minus;
    case TokenType::Star: 
        return BinaryOp::Multiply;
    case TokenType::Slash: 
        return BinaryOp::Divide;
    case TokenType::Percent: 
        return BinaryOp::Modulo;
    case TokenType::AmpAmp: 
        return BinaryOp::And;
    case TokenType::PipePipe: 
        return BinaryOp::Or;
    case TokenType::Equal: 
        return BinaryOp::Equal;
    case TokenType::ExclaimEqual: 
        return BinaryOp::NotEqual;
    case TokenType::Less:  
        return BinaryOp::Less;
    case TokenType::Greater: 
        return BinaryOp::Greater;
    case TokenType::LessEqual: 
        return BinaryOp::LessEqual;
    case TokenType::GreaterEqual: 
        return BinaryOp::GreaterEqual;
    case TokenType::LessLess: 
        return BinaryOp::LeftShift;
    case TokenType::GreaterGreater: 
        return BinaryOp::RightShift;
    case TokenType::Amp: 
        return BinaryOp::BitwiseAnd;
    case TokenType::Pipe: 
        return BinaryOp::BitwiseOr;
    case TokenType::Caret: 
        return BinaryOp::BitwiseXor;
    default: // suppress warning
        break;
    }
    
    return std::nullopt;
}

std::optional<UnaryOp> to_unary_operator(TokenType type) {
    switch (type) {
    case TokenType::Plus: 
        return UnaryOp::Plus;
    case TokenType::Minus: 
        return UnaryOp::Minus;
    case TokenType::Exclaim: 
        return UnaryOp::Not;
    default: // suppress warning
        break;
    }

    return std::nullopt;   
}

BinopPrecedence get_precedence(BinaryOp op) {
    switch (op) {
    case BinaryOp::Or:
        return BinopPrecedence::Or;
    case BinaryOp::And:
        return BinopPrecedence::And;
    case BinaryOp::BitwiseOr:
        return BinopPrecedence::BitwiseOr;
    case BinaryOp::BitwiseXor:
        return BinopPrecedence::BitwiseXor;
    case BinaryOp::BitwiseAnd:
        return BinopPrecedence::BitwiseAnd;
    case BinaryOp::Equal:
    case BinaryOp::NotEqual:
        return BinopPrecedence::Equality;
    case BinaryOp::Less:
    case BinaryOp::Greater:
    case BinaryOp::LessEqual:
    case BinaryOp::GreaterEqual:
        return BinopPrecedence::Relational;
    case BinaryOp::LeftShift:
    case BinaryOp::RightShift:
        return BinopPrecedence::Shift;
    case BinaryOp::Plus:
    case BinaryOp::Minus:
        return BinopPrecedence::Add;
    case BinaryOp::Multiply:
    case BinaryOp::Divide:
    case BinaryOp::Modulo:
        return BinopPrecedence::Multiply;
    default:
        DELTA_UNREACHABLE("logically unreachable");
    }
}