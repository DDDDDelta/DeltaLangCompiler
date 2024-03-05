#pragma once

#include <string>
#include <utility>

class TypeManager {
    
};

class TypeInfo {
public:
    TypeInfo() = default;
    explicit TypeInfo(std::string type_name) : type_name(std::move(type_name)) {}
    explicit TypeInfo(TokenType tktype) {
        switch (tktype) {
        case TokenType::HexIntLiteral:
        case TokenType::DecIntLiteral:
            type_name = "i32";
            break;
        case TokenType::CharLiteral:
            type_name = "u8";
            break;
        case TokenType::FloatLiteral:
            type_name = "f64";
            break;
        case TokenType::StringLiteral:
            type_name = "u8";
            add_pointer();
            break;
        default:
            DELTA_UNREACHABLE("asserted to be valid type token");
        }
    }
    virtual ~TypeInfo() = default;
    
    std::string name() const { return type_name; }
    int pointer_count() const { return pointer_c; }

    TypeInfo& add_pointer(int count = 1) {
        pointer_c += count;
        return *this;
    }

private:
    std::string type_name;
    int pointer_c = 0;
};
