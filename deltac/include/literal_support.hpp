#pragma once

#include "token.hpp"
#include "utils.hpp"

#include <string_view>

namespace llvm {
class APInt;
}

namespace deltac {

// This class is inspired by clang::NumericLiteralParser
class IntLiteralParser {
public:
    explicit IntLiteralParser(const Token& tok);
    IntLiteralParser(const Token& tok, std::uint8_t radix);

    std::uint8_t get_radix() const { return radix; }

    /// get_apint_val - Convert this numeric literal value to an APInt that
    /// matches Val's input width.  If there is an overflow (i.e., if the unsigned
    /// value read is larger than the APInt's bits will hold), set Val to the low
    /// bits of the result and return true.  Otherwise, return false.
    bool get_apint_val(llvm::APInt& val);

    /// Get the digits that comprise the literal. This excludes any prefix or
    /// suffix associated with the literal.
    std::string_view get_digits() const {
        return util::make_sv(digit_begin, end);
    }

private:
    const char* const begin;
    const char* const end;

    const char* digit_begin; 
    // there is no suffix
    const char* s; // cursor

    std::uint8_t radix;
};

}
