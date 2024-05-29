#include "typeinfo.hpp"

namespace deltac {

void TypeDeleter::operator ()(const Type* ty) const {
    if (!util::isinstance<BuiltinType>(ty)) {
        delete ty;
    }
}

QualType::QualType(Type* ty) : type(ty) {
    assert(ty != nullptr);

    if (is_void_ty() || is_func_ty()) {
        qualification = qual::N_A;
    } else {
        qualification = qual::NoQual;
    }
}

QualType::QualType(const QualType& other) : type(other.type->copy()) {}

std::string QualType::repr() const { 
    return type->repr() + (qualification == qual::Const ? " const" : ""); 
}

bool QualType::can_be_vardecl_ty() const { return size() != 0; }

std::size_t QualType::size() const {
    return type->size();
}

bool QualType::is_const() const { return qualification == qual::Const; }

bool QualType::is_mutable() const { return qualification == qual::NoQual; }

bool QualType::is_builtin_ty() const { return util::isinstance<BuiltinType>(type); }

bool QualType::is_ptr_ty() const { return util::isinstance<PtrType>(type); }

bool QualType::is_func_ty() const { return util::isinstance<FunctionType>(type); }

bool QualType::is_void_ty() const { 
    return is_builtin_ty() && ((BuiltinType*)type)->get_kind() == BuiltinType::Void;
}

bool QualType::is_bool_ty() const {
    return is_builtin_ty() && ((BuiltinType*)type)->get_kind() == BuiltinType::Bool;
}

void QualType::remove_ptr() {
    assert(is_ptr_ty());
    auto* garbage = type;

    *this = std::move(((PtrType*)type)->pointee());

    TypeDeleter()(garbage);
}

void QualType::remove_const() {
    assert(is_const());
    qualification = qual::NoQual;
}

// transfer data of current QualType to a new PtrType
// then set it back to current qualtype with new qualification(constness)
void QualType::add_ptr(bool constness) {
    type = new PtrType(std::move(*this));
    if (constness) {
        qualification = qual::Const;
    } else {
        qualification = qual::NoQual;
    }
}

bool QualType::is_signed_ty() const { 
    if (auto* bt = dynamic_cast<BuiltinType*>(type)) {
        return is_signed(bt->get_kind());
    } else {
        return false;
    }
 }

bool QualType::is_unsigned_ty() const { 
    if (auto* bt = dynamic_cast<BuiltinType*>(type)) {
        return is_unsigned(bt->get_kind());
    } else {
        return false;
    }
 }

bool QualType::is_integer_ty() const { 
    if (auto* bt = dynamic_cast<BuiltinType*>(type)) {
        return is_integer(bt->get_kind());
    } else {
        return false;
    }
}

std::string BuiltinType::repr() const { return to_string(kind); }

std::size_t BuiltinType::size() const { return get_size(kind); }

static std::size_t size_arr[] = {
#define BUILTIN_TYPE(ID, NAME, SIZE) SIZE,
#include "builtin_type.inc"
};

std::size_t get_size(BuiltinType::Kind kind) {
    return size_arr[util::to_underlying(kind)];
}

static const char* to_string_arr[] = {
#define BUILTIN_TYPE(ID, NAME, SIZE) #NAME,
#include "builtin_type.inc"
};

std::string to_string(BuiltinType::Kind kind) {
    return to_string_arr[util::to_underlying(kind)]; 
}

static const signed char signedness_arr[] = {
#define BUILTIN_TYPE(ID, NAME, SIZE) 0,
#define SIGNED_BUILTIN_TYPE(ID, NAME, SIZE) 1,
#define UNSIGNED_BUILTIN_TYPE(ID, NAME, SIZE) -1,
#include "builtin_type.inc"
};

bool is_signed(BuiltinType::Kind kind) {
    return signedness_arr[util::to_underlying(kind)] == 1;
}

bool is_unsigned(BuiltinType::Kind kind) {
    return signedness_arr[util::to_underlying(kind)] == -1;
}

bool is_integer(BuiltinType::Kind kind) {
    return is_signed(kind) || is_unsigned(kind);
}

}
