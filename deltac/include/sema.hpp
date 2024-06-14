#include "expression.hpp"
#include "decl.hpp"
#include "astcontext.hpp"

#include "llvm/ADT/ArrayRef.h"

namespace deltac {

class Sema;

class TypeBuilder {
public:
    TypeBuilder(Sema& action);

    bool add_ptr(bool constness = false);
    // bool add_array(Expr* size);
    // bool add_array_ref(bool constness = false);
    bool finalize(std::string_view id);
    
    bool finalize(llvm::ArrayRef<QualType> param_ty, QualType ret_ty, util::use_move_t = util::use_move);

    bool has_error() const { return errored; }
    bool has_finalized() const { return finalized; }

    void reset();
    TypeResult release();
    TypeResult get();

private:
    bool errored = false;
    bool finalized = false;
    QualType res;
    Type** base_ty;
    Sema& action;
};

class Sema {
public:
    Sema(ASTContext& context) : context(context) {}
    
    const ASTContext& ast_context() const { return context; }

    ExprResult act_on_int_literal(const Token& tok, std::uint8_t posix, QualType* ty);
    ExprResult act_on_unary_expr(UnaryOp, Expr* expr);
    ExprResult act_on_cast_expr(Expr* expr, QualType ty);

private:
    Expr* add_integer_promotion(Expr* expr);
    Expr* add_bool_cast(Expr* expr);

    Type* new_type_from_id(std::string_view);
    Type* new_function_ty(llvm::ArrayRef<QualType> param_ty, QualType ret_ty);

private:
    friend class TypeBuilder;

    ASTContext& context;
};

}
