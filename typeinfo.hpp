#pragma once

#include <string>
#include <utility>

class TypeManager {
    
};

class TypeInfo {
public:
    explicit TypeInfo(std::string typename) : name(std::move(typename)) {}
    explicit TypeInfo(TokenType tktype) {
        switch (tktype) {
        case TokenType::HexIntLiteral:
        case TokenType::DecIntLiteral:
            name = "i32";
            break;
        case TokenType::CharLiteral:
            name = "u8";
            break;
        case TokenType::FloatLiteral:
            name = "f64";
            break;
        case TokenType::StringLiteral:
            name = "u8";
            add_pointer();
            break;
        default:
            DELTA_UNREACHABLE("asserted to be valid type token");
        }
    }
    virtual ~TypeInfo() = 0;
    
    std::string name() const { return name; }
    int pointer_count() const { return pointer_count; }

    TypeInfo& add_pointer(int count = 1) {
        pointer_c += count;
        return *this;
    }

private:
    std::string name;
    int pointer_c = 0;
};
