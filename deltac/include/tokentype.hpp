#pragma once

#include "utils.hpp"

#include <cstdint>
#include <string_view>

namespace deltac {

namespace tok {
    
enum Kind : std::uint16_t {
#define TOK(X) X,
#include "tokentype.inc"
};

}

std::string_view token_type_name(tok::Kind tkt);

std::string_view token_type_enum_name(tok::Kind tkt);

}
