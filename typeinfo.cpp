#include "typeinfo.hpp"

void TypeDeleter::operator()(const Type* ty) const {
    if (!isinstance<BuiltinType>(ty)) {
        delete ty;
    }
}

QualType::QualType(Type* ty) : type(ty) {
    assert(ty != nullptr);

    if (is_void_ty() || is_func_ty()) {
        qual = Qualification::N_A;
    } else {
        qual = Qualification::NoQual;
    }
}

QualType::QualType(const QualType& other) : type(other.type->copy()) {}

std::string QualType::repr() const { 
    return type->repr() + (qual == Qualification::Const ? " const" : ""); 
}

bool QualType::can_be_vardecl_ty() const { return size() != 0; }

std::size_t QualType::size() const {
    return type->size();
}

bool QualType::is_const() const { return qual == Qualification::Const;}

bool QualType::is_mutable() const { return qual == Qualification::NoQual;}

bool QualType::is_ptr_ty() const { return isinstance<PtrType>(type);}

bool QualType::is_func_ty() const { return isinstance<FunctionType>(type); }

bool QualType::is_void_ty() const { 
    return isinstance<BuiltinType>(type) && ((BuiltinType*)type)->get_kind() == BuiltinType::Void;
}

void QualType::remove_ptr() {
    assert(is_ptr_ty());
    auto* garbage = type;

    *this = std::move(((PtrType*)type)->pointee());

    TypeDeleter()(garbage);
}

// transfer data of current QualType to a new PtrType
// then set it back to current qualtype with new qualification(constness)
void QualType::add_ptr(bool constness) {
    type = new PtrType(std::move(*this));
    if (constness) {
        qual = Qualification::Const;
    } else {
        qual = Qualification::NoQual;
    }
}

std::string BuiltinType::repr() const { return to_string(kind); }

std::size_t BuiltinType::size() const { return get_size(kind); }

static std::size_t size_arr[] = {
#define BUILTIN_TYPE(ID, NAME, SIZE) SIZE,
#include "builtin_type.inc"
};

std::size_t get_size(BuiltinType::Kind kind) {
    return size_arr[std::to_underlying(kind)];
}

static const char* to_string_arr[] = {
#define BUILTIN_TYPE(ID, NAME, SIZE) #NAME,
#include "builtin_type.inc"
};

std::string to_string(BuiltinType::Kind kind) {
    return to_string_arr[std::to_underlying(kind)]; 
}
