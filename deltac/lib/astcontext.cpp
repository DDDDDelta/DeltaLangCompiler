#include "astcontext.hpp"

namespace deltac {

BuiltinType ASTContext::builtin_types[] = {
#define BUILTIN_TYPE(ID, NAME, SIZE) BuiltinType::ID,
#include "builtin_type.inc"
};

QualType ASTContext::get_i32_ty(bool is_const) const {
    return QualType(get_builtin_type(BuiltinType::I32), is_const ? qual::Const : qual::NoQual);
}

QualType ASTContext::get_int_ty_from_size(std::uint32_t bitwidth, bool is_signed, bool is_const) const {
    assert(bitwidth == 8 || bitwidth == 16 || bitwidth == 32 || bitwidth == 64);
    assert(is_signed || !is_signed);

    switch (bitwidth) {
    case 8:
        return QualType(get_builtin_type(is_signed ? BuiltinType::I8 : BuiltinType::U8), is_const ? qual::Const : qual::NoQual);
    case 16:
        return QualType(get_builtin_type(is_signed ? BuiltinType::I16 : BuiltinType::U16), is_const ? qual::Const : qual::NoQual);
    case 32:
        return QualType(get_builtin_type(is_signed ? BuiltinType::I32 : BuiltinType::U32), is_const ? qual::Const : qual::NoQual);
    case 64:
        return QualType(get_builtin_type(is_signed ? BuiltinType::I64 : BuiltinType::U64), is_const ? qual::Const : qual::NoQual);
    }
}

QualType ASTContext::get_bool_ty(bool is_const) const {
    return QualType(get_builtin_type(BuiltinType::Bool), is_const ? qual::Const : qual::NoQual);
}

} // namespace deltac
