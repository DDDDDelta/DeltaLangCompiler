#include "astcontext.hpp"

BuiltinType builtin_types[] = {
#define BUILTIN_TYPE(ID, NAME, SIZE) BuiltinType::ID,
#include "builtin_type.inc"
};

static BuiltinType* get_builtin_type(BuiltinType::Kind kind) {
    return &builtin_types[util::to_underlying(kind)];
}

QualType ASTContext::get_i32_ty(bool is_const) const {
    return QualType(get_builtin_type(BuiltinType::I32), is_const);
}


