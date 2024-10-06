#include "sema.hpp"
#include "astcontext.hpp"
#include "expression.hpp"
#include "literal_support.hpp"
#include "operators.hpp"
#include "tokentype.hpp"
#include "utils.hpp"

namespace deltac {

TypeBuilder::TypeBuilder(Sema& action) : res(action.context.get_void_ty()), base(&res), action(action) {}

bool TypeBuilder::add_ptr(bool constness) {
    assert(!errored);

    res.add_ptr(constness);
    return true;
}

bool TypeBuilder::finalize(const Token& token, bool constness) {
    DELTA_ASSERT(!errored);

    finalized = true;

    if (token.is(tok::Void)) {
        base->raw_type(action.context.get_void_ty());
    }

    Type* ty = action.new_type_from_tok(token);

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
    DELTA_ASSERT(!errored);

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

static Expr* new_lval_cast(Expr* expr) {
    return new ImplicitCastExpr(expr, expr->type(), CastExpr::LValueToRValue);
}

static Expr* new_noop_cast(Expr* expr) {
    return new ExplicitCastExpr(expr, expr->type(), CastExpr::NoOp);
}

ExprResult Sema::act_on_int_literal(const Token& tok, u8 posix, QualType* ty) {
    if (ty != nullptr && !ty->is_integer_ty()) {
        // error incompatible specified type for int literal
        return action_error;
    }

    bool is_unsigned = true;
    auto bitwidth = 32u;

    if (ty) {
        is_unsigned = ty->is_signed_ty();
        bitwidth = (u32)ty->size();
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
            expr = new_lval_cast(expr);
        }
        break;
    case UnaryOp::AddressOf:
        if (expr->is_rval()) {
            return action_error;
        }
        break;
    }

    // lets leave implicit conversion to later...
    /*
     * 2) zero or one numeric promotion or numeric conversion;
     */
    /*
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
    */
    
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
        return new UnaryExpr(QualType::make_remove_ptr_ty(expr->type()), Expr::LValue, op, expr);

    case UnaryOp::AddressOf:
        if (expr->is_lval()) {
            // TODO: error cannot take address of rvalue
            return action_error;
        }

        return new UnaryExpr(QualType::make_ptr_ty(expr->type()), Expr::RValue, op, expr);
    }
}

RawTypeResult Sema::act_on_raw_type(const Token& id_token) {
    DELTA_ASSERT(id_token.is(tok::Identifier));


}

Expr* Sema::add_integer_promotion(Expr* expr) {
    DELTA_ASSERT(expr->is_rval());

    return new ImplicitCastExpr(expr, context.get_i32_ty(), CastExpr::IntCast);
}

} // namespace deltac
