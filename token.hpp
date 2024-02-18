#pragma once

#include "tokentype.hpp"
#include "charinfo.hpp"

#include <iostream>
#include <optional>
#include <any>
#include <typeinfo>

class Token {
public:
    Token() = default;
    Token(const Token&) = default;
    Token(Token&&) = default;

    bool is_one_of(TokenType type1) const { return type == type1; }
    template <typename... Ts>
    bool is_one_of(TokenType type, Ts... types) const {
        return is_one_of(type) || is_one_of(types...);
    }

    void concat(const Token& other) {
        code_view = std::string_view(std::min(code_view.begin(), other.code_view.begin()),
                                     std::max(code_view.end(), other.code_view.end()));
    }

    void start_token() {
        type = TokenType::ERROR;
        code_view = "";
        // data.reset();
    }

    /*
    void set_literal_data(std::string_view ld) { data = ld; }
    void set_bool_literal_data(bool b) { data = b ? "true" : "false"; }
    std::string_view get_literal_data() const { return data; }

    void set_identifier_data(std::string_view id) { data = id; }
    std::string_view get_identifier_data() const { return data; }
    */

    void set_view(std::string_view sv) { code_view = sv; }
    void set_view(const char* begin, const char* end) { code_view = std::string_view(begin, end); }
    void set_view(const char* begin, std::size_t size) { code_view = std::string_view(begin, size); }
    std::string_view get_view() const { return code_view; }

    void set_type(TokenType t) { type = t; }
    TokenType get_type() const { return type; }

    /*
    template <typename T>
    std::optional<const T&> get_data() const {
        if (data.has_value() && data.type() == typeid(T)) {
            return std::any_cast<const T&>(data);
        }
        return std::nullopt;
    }
    */
    /*
    template <typename T>
    void set_data(const T& d) { data = d; }
    */
    std::string_view get_name() const { return token_type_name(get_type()); }

private:
    TokenType type = TokenType::ERROR;
    std::string_view code_view;
    // std::any data;

    friend std::ostream& operator <<(std::ostream&, const Token& tk);
};

inline std::ostream& operator <<(std::ostream& os, const Token& tk) {
    os << token_type_enum_name(tk.type) << ": " << tk.code_view;
    return os;
}
