#include "expression.hpp"
#include "decl.hpp"
#include "astcontext.hpp"

class Sema {
public:
    Sema(const ASTContext& context) : context(context) {}
    

private:
    const ASTContext& context;
};