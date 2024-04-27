#include "typeinfo.hpp"

static BasicType _void_ty("void");

BasicType* BasicType::void_ty = &_void_ty;
