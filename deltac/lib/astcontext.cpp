#include "astcontext.hpp"
#include "utils.hpp"

namespace deltac {

BuiltinType ASTContext::builtin_types[] = {
#define BUILTIN_TYPE(ID, NAME, SIZE) BuiltinType::ID,
#include "builtin_type.inc"
};

Type* ASTContext::get_i32_ty() const {
    return get_builtin_type(BuiltinType::I32);
}

Type* ASTContext::get_int_ty_size(u32 bitwidth, bool is_signed) const {
    assert(bitwidth == 8 || bitwidth == 16 || bitwidth == 32 || bitwidth == 64);
    assert(is_signed || !is_signed);

    switch (bitwidth) {
    case 8:
        return get_builtin_type(is_signed ? BuiltinType::I8 : BuiltinType::U8);
    case 16:
        return get_builtin_type(is_signed ? BuiltinType::I16 : BuiltinType::U16);
    case 32:
        return get_builtin_type(is_signed ? BuiltinType::I32 : BuiltinType::U32);
    case 64:
        return get_builtin_type(is_signed ? BuiltinType::I64 : BuiltinType::U64);
    default:
        DELTA_UNREACHABLE("impossible bitwidth");
    }
}

Type* ASTContext::get_bool_ty() const {
    return get_builtin_type(BuiltinType::Bool);
}

} // namespace deltac
