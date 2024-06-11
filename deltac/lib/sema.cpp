#include "sema.hpp"
#include "literal_support.hpp"
#include "operators.hpp"

namespace deltac {

TypeBuilder::TypeBuilder(Sema& action) : res(action.context.get_void_ty()), action(action) {}



Expr* Sema::act_on_int_literal(const Token& tok, std::uint8_t posix, QualType* ty) {
    if (ty != nullptr && !ty->is_integer_ty()) {
        // error incompatible specified type for int literal
        return nullptr;
    }

    bool is_unsigned = !ty ? true : ty->is_signed_ty();
    auto bitwidth = !ty ? 32u : (std::uint32_t)ty->size();

    IntLiteralParser literal_parser(tok.get_view(), posix);
    llvm::APSInt val(bitwidth, is_unsigned);

    if (literal_parser.get_apint_val(val)) {
        // TODO: error overflow
        return nullptr;
    }

    return new IntLiteralExpr(!ty ? context.get_i32_ty() : std::move(*ty), std::move(val));
}

Expr* Sema::act_on_unary_expr(UnaryOp op, Expr* expr) {
    assert(!expr->type().is_void_ty());

    /*
     * 1) zero or one conversion from the following set:
     *   lvalue-to-rvalue conversion,
     *   array-to-pointer conversion, and
     *   function-to-pointer conversion;
     */
    switch (op) {
    case UnaryOp::Plus:
    case UnaryOp::Minus:
    case UnaryOp::Not:
    case UnaryOp::BitwiseNot:
    case UnaryOp::Deref:
        if (expr->is_lval()) {
            expr = ImplicitCastExpr::new_lval_cast(expr);
        }
        break;
    case UnaryOp::AddressOf:
        break;
    }

    /*
     * 2) zero or one numeric promotion or numeric conversion;
     */
    switch (op) {
    case UnaryOp::Plus:
    case UnaryOp::Minus:
    case UnaryOp::BitwiseNot:
        if (expr->type().is_integer_ty() && expr->type().size() < 32) {
            expr = add_integer_promotion(expr);
        }
        break;
    case UnaryOp::Not:
        if (!expr->type().is_bool_ty()) {
            expr = add_bool_cast(expr);
        }
        break;
    case UnaryOp::Deref:
    case UnaryOp::AddressOf:
        break;
    }
    
    /* 
     * constructs the unary expression 
     */
    switch (op) {
    case UnaryOp::Plus:
    case UnaryOp::Minus:
    case UnaryOp::Not:
    case UnaryOp::BitwiseNot:
        return new UnaryExpr(expr->type(), Expr::RValue, op, expr);

    case UnaryOp::Deref:
        if (!expr->type().is_ptr_ty()) {
            // error dereferencing non-pointer type
            return nullptr;
        }
        return new UnaryExpr(expr->type(), Expr::LValue, op, expr);

    case UnaryOp::AddressOf:
        return new UnaryExpr(QualType::make_ptr_ty(expr->type()), Expr::RValue, op, expr);
    }
}

Expr* Sema::add_integer_promotion(Expr* expr) {
    assert(expr->is_rval());

    return ImplicitCastExpr::new_int_cast(expr, context.get_i32_ty());
}

Expr* Sema::add_bool_cast(Expr* expr) {
    assert(expr->is_rval());

    return ImplicitCastExpr::new_bool_cast(expr, context.get_bool_ty());
}

} // namespace deltac
