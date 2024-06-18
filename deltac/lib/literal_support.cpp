#include "literal_support.hpp"

#include "llvm/ADT/APSInt.h"
#include "llvm/ADT/StringExtras.h"


namespace deltac {

// directly from clang::alwaysFitsInto64Bits
static bool fits_into_64_bits(std::uint8_t radix, std::size_t numdigits) {
    switch (radix) {
    case 2:
        return numdigits <= 64;
    case 8:
        return numdigits <= 64 / 3; // Digits are groups of 3 bits.
    case 10:
        return numdigits <= 19; // floor(log10(2^64))
    case 16:
        return numdigits <= 64 / 4; // Digits are groups of 4 bits.
    default:
        DELTA_UNREACHABLE("impossible radix");
    }
}

IntLiteralParser::IntLiteralParser(const Token& t) : 
    begin(&t.get_view().front()), end(&t.get_view().back()) {
    const int prefix_size = 2;

    const std::string_view& sv = t.get_view();

    if (sv.size() <= prefix_size) {// cannot have 0x prefix
        digit_begin = begin;
        radix = 10;
    }
    else if (sv.substr(0, prefix_size) == "0x") {
        digit_begin = &s[prefix_size];
        radix = 16;
    }
    else {
        DELTA_UNREACHABLE("invalid prefix");
    }

    s = digit_begin;
}

IntLiteralParser::IntLiteralParser(const Token& t, std::uint8_t radix) :
    begin(&t.get_view().front()), end(&t.get_view().back()), 
    digit_begin(radix != 10 ? begin + 2 : begin), s(digit_begin),
    radix(radix) {}
    

bool IntLiteralParser::get_apint_val(llvm::APInt& val) {
    const std::size_t numdigits = end - begin;

    if (!fits_into_64_bits(radix, numdigits)) {
        // do not support over 64 bits for now
        return false;
    }
    else {
        uint64_t n = 0;

        for (char c : get_digits()) {
            n = n * radix + llvm::hexDigitValue(c);
        }

        val = n;
        return val.getZExtValue() != n;
    }
}

}
