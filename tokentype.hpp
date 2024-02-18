#pragma once

#include "utils.hpp"

#include <cstdint>
#include <string_view>

enum class TokenType : std::uint16_t {
#define TOK(X) X,
#include "tokentype.inc"
};

std::string_view token_type_name(TokenType tkt);

std::string_view token_type_enum_name(TokenType tkt);