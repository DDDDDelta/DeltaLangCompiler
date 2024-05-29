#include "operators.hpp"

namespace deltac {

std::optional<BinaryOp> to_binary_operator(tok::Kind type) {
    switch (type) {
        using namespace tok;
    case Plus: 
        return BinaryOp::Plus;
    case Minus: 
        return BinaryOp::Minus;
    case Star: 
        return BinaryOp::Multiply;
    case Slash: 
        return BinaryOp::Divide;
    case Percent: 
        return BinaryOp::Modulo;
    case AmpAmp: 
        return BinaryOp::And;
    case PipePipe: 
        return BinaryOp::Or;
    case EqualEqual:
        return BinaryOp::Equal;
    case ExclaimEqual: 
        return BinaryOp::NotEqual;
    case Less:  
        return BinaryOp::Less;
    case Greater: 
        return BinaryOp::Greater;
    case LessEqual: 
        return BinaryOp::LessEqual;
    case GreaterEqual: 
        return BinaryOp::GreaterEqual;
    case LessLess: 
        return BinaryOp::LeftShift;
    case GreaterGreater: 
        return BinaryOp::RightShift;
    case Amp: 
        return BinaryOp::BitwiseAnd;
    case Pipe: 
        return BinaryOp::BitwiseOr;
    case Caret: 
        return BinaryOp::BitwiseXor;
    default: // suppress warning
        break;
    }
    
    return std::nullopt;
}

std::optional<UnaryOp> to_unary_operator(tok::Kind type) {
    using namespace tok;
    switch (type) {
    case Plus: 
        return UnaryOp::Plus;
    case Minus: 
        return UnaryOp::Minus;
    case Exclaim: 
        return UnaryOp::Not;
    case Tilde:
        return UnaryOp::BitwiseNot;
    case Star:
        return UnaryOp::Deref;
    case Amp:
        return UnaryOp::AddressOf;
    default:
        return std::nullopt;
    }
}

prec::Binary get_precedence(BinaryOp op) {
    using namespace prec;
    switch (op) {
    case BinaryOp::Or:
        return Or;
    case BinaryOp::And:
        return And;
    case BinaryOp::BitwiseOr:
        return BitwiseOr;
    case BinaryOp::BitwiseXor:
        return BitwiseXor;
    case BinaryOp::BitwiseAnd:
        return BitwiseAnd;
    case BinaryOp::Equal:
    case BinaryOp::NotEqual:
        return Equality;
    case BinaryOp::Less:
    case BinaryOp::Greater:
    case BinaryOp::LessEqual:
    case BinaryOp::GreaterEqual:
        return Relational;
    case BinaryOp::LeftShift:
    case BinaryOp::RightShift:
        return Shift;
    case BinaryOp::Plus:
    case BinaryOp::Minus:
        return Add;
    case BinaryOp::Multiply:
    case BinaryOp::Divide:
    case BinaryOp::Modulo:
        return Multiply;
    default:
        DELTA_UNREACHABLE("logically unreachable");
    }
}

}
