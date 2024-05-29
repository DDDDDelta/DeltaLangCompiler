#include "tokentype.hpp"

namespace deltac {

static constexpr std::string_view token_name[] = {
#define TOK(X) #X,
#define PUNCTUATOR(X, Y) Y,
#define KEYWORD(X, Y) Y,
#include "tokentype.inc"
};

static constexpr std::string_view token_enum_name[] = {
#define TOK(X) #X,
#include "tokentype.inc"
};

std::string_view token_type_name(tok::Kind tkt) {
    return token_name[tkt];
}

std::string_view token_type_enum_name(tok::Kind tkt) {
    return token_enum_name[tkt];
}

}
