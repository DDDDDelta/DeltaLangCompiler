#include "lexer.hpp"
#include "filebuffer.hpp"
#include "token.hpp"
#include "keywordtrie.hpp"

#include <iostream>

int make_int() {
    return rand();
}

int main() {
    SourceBuffer buffer("./test/helloworld.dl");
    std::cout << "hi" << std::endl;

    
    std::cout << (buffer.ptr_cend() - buffer.ptr_cbegin()) << std::endl;
    std::cout << buffer.size() << std::endl;

    const char* begin = buffer.ptr_cbegin();

    Lexer lexer(buffer);
    Token result;

    while (true) {
        char c = *begin;
//        auto [c, size] = lexer.get_sized(begin);
        begin++;
//        begin += size;
        
//        std::cout << c << ' ' << size << "   ";
        std::cout << (int)c << ' ';
        if (c == 0)
            break;
    }

    std::cout << "segfault?" << std::endl;

    while (lexer.lex(result))
        std::cout << result << std::endl;

    std::cout << result << std::endl;
    return 0;
}
