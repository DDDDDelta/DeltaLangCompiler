#include "expression.hpp"
#include "decl.hpp"
#include "astcontext.hpp"

#include "llvm/ADT/ArrayRef.h"

#include <stack>

namespace deltac {

class TypeBuilder {
public:
    TypeBuilder(Sema& action);

    void add_ptr(bool constness = false);
    void add_array(Expr* size, bool constness = false);
    void add_array_ref();
    bool finalize(std::string_view id);
    bool finalize(llvm::ArrayRef<QualType> params, QualType ret_ty);

    void reset();

private:
    bool errored = false;
    bool finalized = false;
    QualType res;
    Sema& action;
};

class Sema {
public:
    Sema(ASTContext& context) : context(context) {}
    
    Expr* act_on_int_literal(const Token& tok, std::uint8_t posix, QualType* ty);
    Expr* act_on_unary_expr(UnaryOp, Expr* expr);

private:
    Expr* add_integer_promotion(Expr* expr);
    Expr* add_bool_cast(Expr* expr);

private:
    friend class TypeBuilder;

    ASTContext& context;
};

}
