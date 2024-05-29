#pragma once

#include <cstdint>

namespace deltac {

namespace charinfo {
extern const std::uint16_t info_table[256];

enum {
    CHAR_HORZ_WS  = 0x0001,  // '\t', '\f', '\v'.  Note, no '\0'
    CHAR_VERT_WS  = 0x0002,  // '\r', '\n'
    CHAR_SPACE    = 0x0004,  // ' '
    CHAR_DIGIT    = 0x0008,  // 0-9
    CHAR_XLETTER  = 0x0010,  // a-f,A-F
    CHAR_UPPER    = 0x0020,  // A-Z
    CHAR_LOWER    = 0x0040,  // a-z
    CHAR_UNDER    = 0x0080,  // _
    CHAR_PERIOD   = 0x0100,  // .
    CHAR_RAWDEL   = 0x0200,  // {}[]#<>%:;?*+-/^&|~!=,"'
    CHAR_PUNCT    = 0x0400   // `$@()
};

enum {
    CHAR_XUPPER = CHAR_XLETTER | CHAR_UPPER,
    CHAR_XLOWER = CHAR_XLETTER | CHAR_LOWER
};
}

inline bool is_ASCII(char c) {
    return static_cast<unsigned char>(c) <= 127;
}

inline bool is_ASCII(unsigned char c) { return c <= 127; }

/// Returns true if this character is horizontal ASCII whitespace:
/// ' ', '\\t', '\\f', '\\v'.
///
/// Note that this returns false for '\\0'.
inline bool is_horizontal_whitespace(unsigned char c) {
    using namespace charinfo;
    return (info_table[c] & (CHAR_HORZ_WS | CHAR_SPACE)) != 0;
}

/// Returns true if this character is vertical ASCII whitespace: '\\n', '\\r'.
///
/// Note that this returns false for '\\0'.
inline bool is_vertical_whitespace(unsigned char c) {
    using namespace charinfo;
    return (info_table[c] & CHAR_VERT_WS) != 0;
}

/// Return true if this character is horizontal or vertical ASCII whitespace:
/// ' ', '\\t', '\\f', '\\v', '\\n', '\\r'.
///
/// Note that this returns false for '\\0'.
inline bool is_whitespace(unsigned char c) {
    using namespace charinfo;
    return (info_table[c] & (CHAR_HORZ_WS | CHAR_VERT_WS | CHAR_SPACE)) != 0;
}

/// Return true if this character is an ASCII digit: [0-9]
inline bool is_digit(unsigned char c) {
    using namespace charinfo;
    return (info_table[c] & CHAR_DIGIT) != 0;
}

/// Return true if this character is a lowercase ASCII letter: [a-z]
inline bool is_lowercase(unsigned char c) {
    using namespace charinfo;
    return (info_table[c] & CHAR_LOWER) != 0;
}

/// Return true if this character is an uppercase ASCII letter: [A-Z]
inline bool is_uppercase(unsigned char c) {
    using namespace charinfo;
    return (info_table[c] & CHAR_UPPER) != 0;
}

/// Return true if this character is an ASCII letter: [a-zA-Z]
inline bool is_letter(unsigned char c) {
    using namespace charinfo;
    return (info_table[c] & (CHAR_UPPER | CHAR_LOWER)) != 0;
}

/// Return true if this character is an ASCII letter or digit: [a-zA-Z0-9]
inline bool is_alphanumeric(unsigned char c) {
    using namespace charinfo;
    return (info_table[c] & (CHAR_DIGIT | CHAR_UPPER | CHAR_LOWER)) != 0;
}

/// Return true if this character is an ASCII hex digit: [0-9a-fA-F]
inline bool is_hex_digit(unsigned char c) {
    using namespace charinfo;
    return (info_table[c] & (CHAR_DIGIT | CHAR_XLETTER)) != 0;
}

/// Return true if this character is an ASCII punctuation character.
///
/// Note that '_' is both a punctuation character and an identifier character!
inline bool is_punctuation(unsigned char c) {
    using namespace charinfo;
    return (info_table[c] & (CHAR_UNDER | CHAR_PERIOD | CHAR_RAWDEL | CHAR_PUNCT)) != 0;
}

/// Return true if this character is an ASCII printable character; that is, a
/// character that should take exactly one column to print in a fixed-width
/// terminal.
inline bool is_printable(unsigned char c) {
    using namespace charinfo;
    return (info_table[c] & (CHAR_UPPER | CHAR_LOWER | CHAR_PERIOD | CHAR_PUNCT |
                            CHAR_DIGIT | CHAR_UNDER | CHAR_RAWDEL | CHAR_SPACE)) != 0;
}

}
