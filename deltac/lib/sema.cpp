#include "sema.hpp"
#include "expression.hpp"
#include "literal_support.hpp"
#include "operators.hpp"

namespace deltac {

TypeBuilder::TypeBuilder(Sema& action) : res(action.context.get_void_ty()), base(&res), action(action) {}

bool TypeBuilder::add_ptr(bool constness) {
    assert(!errored);

    res.add_ptr(constness);
    return true;
}

bool TypeBuilder::finalize(const Token& tok, bool constness) {
    assert(!errored);

    finalized = true;

    Type* ty = action.new_type_from_id(tok.get_view());

    if (!ty) {
        errored = true;
        return false;
    }

    base->raw_type(ty);
    
    if (constness) {
        base->add_const();
    }

    return true;
}

bool TypeBuilder::finalize(llvm::ArrayRef<QualType> param_ty, QualType ret_ty, util::use_move_t) {
    assert(!errored);

    finalized = true;

    Type* ty = action.new_function_ty(param_ty, std::move(ret_ty));
    if (!ty) {
        errored = true;
        return false;
    }

    base->raw_type(ty);

    return true;
}

bool TypeBuilder::reset() {
    bool ret = !errored && finalized;

    reset_internal();

    return ret;
}

TypeResult TypeBuilder::release() {
    if (!errored && finalized) {
        return std::move(res);
    }

    reset_internal();

    return action_error;
}

TypeResult TypeBuilder::get() {
    if (!errored && finalized) {
        return res;
    }

    return action_error;
}

void TypeBuilder::reset_internal() {
    errored = false;
    finalized = false;
    
    res = action.context.get_void_ty();
    base = &res;
}

ExprResult Sema::act_on_int_literal(const Token& tok, std::uint8_t posix, QualType* ty) {
    if (ty != nullptr && !ty->is_integer_ty()) {
        // error incompatible specified type for int literal
        return action_error;
    }

    bool is_unsigned = true;
    auto bitwidth = 32u;

    if (ty) {
        is_unsigned = ty->is_signed_ty();
        bitwidth = (std::uint32_t)ty->size();
    }

    IntLiteralParser literal_parser(tok, posix);
    llvm::APSInt val(bitwidth, is_unsigned);

    if (literal_parser.get_apint_val(val)) {
        // TODO: error overflow
        return action_error;
    }

    return new IntLiteralExpr(!ty ? context.get_i32_ty() : std::move(*ty), std::move(val));
}

ExprResult Sema::act_on_unary_expr(UnaryOp op, Expr* expr) {
    // TODO: handle error cases


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
            return action_error;
        }
        return new UnaryExpr(expr->type(), Expr::LValue, op, expr);

    case UnaryOp::AddressOf:
        return new UnaryExpr(QualType::make_ptr_ty(expr->type()), Expr::RValue, op, expr);
    }
}

ExprResult Sema::act_on_cast_expr(Expr* expr, QualType ty) {
    // if the expr is a pointer and is being casted to a pointer
    if (ty.is_ptr_ty() && expr->type().is_ptr_ty()) {
        if (expr->is_lval()) {
            expr = ImplicitCastExpr::new_lval_cast(expr);
        }
        
        return new CastExpr(expr, ty);
    }
}

Expr* Sema::add_integer_promotion(Expr* expr) {
    assert(expr->is_rval());

    return new ImplicitCastExpr(context.get_i32_ty(), expr, ImplicitCastExpr::IntCast);
}

Expr* Sema::add_bool_cast(Expr* expr) {
    assert(expr->is_rval());

    return new ImplicitCastExpr(context.get_bool_ty(), expr, ImplicitCastExpr::BoolCast);
}

Type* Sema::new_function_ty(llvm::ArrayRef<QualType> param_ty, QualType ret_ty) {
    return new FunctionType(param_ty, std::move(ret_ty), util::use_move);
}

} // namespace deltac
