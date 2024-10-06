#include "lexer.hpp"

namespace deltac {

const KeywordTrie Lexer::kwtrie = {
#define KEYWORD(X, Y) tok::X,
#include "tokentype.inc"
};

Lexer::Lexer(const char* begin, const char* end) : 
    buffer_start(begin), buffer_end(end), buffer_curr(buffer_start) {}

void Lexer::form_token(Token& result, const char* token_end, tok::Kind type) {
    result.set_type(type);
    result.set_view(buffer_curr, token_end);
    buffer_curr = token_end;
}

// prev_char is numeric and consumed
bool Lexer::lex_numeric_literal(Token& result, const char* curr_ptr) {
    bool dot = false;
    tok::Kind type = tok::DecIntLiteral;
    
    while (true) {
        char digit = *curr_ptr;

        if (is_digit(digit)) {
            curr_ptr++;
            continue;
        }
        else if (is_punctuation(digit) || is_whitespace(digit)) {
        // a dec number can only be followed by a punctuation or whitespace
            if (digit == '_') // punctuation but part of an identifier
                break;

            if (digit == '.' && !dot) { // dot can only appear once in a float literal
                type = tok::FloatLiteral;
                dot = true;

                curr_ptr++;
                continue;
            } 
            else if (digit == '.') {
                break;
            }

            form_token(result, curr_ptr, type);
            return true;
        }
        else { // unexpected character
            break;
        }
    }
    
    // TODO: Error invalid numeric literal
    form_token(result, curr_ptr, tok::ERROR);
    return false;
}

bool Lexer::lex_hex(Token& result, const char* curr_ptr) {
    while (true) {
        char digit = *curr_ptr;
        if (!is_hex_digit(digit) && is_alphanumeric(digit)) {
            // TODO: Error invalid hexdec literal
            form_token(result, curr_ptr, tok::ERROR);
            return false;
        }
        else if (is_hex_digit(digit)) {
            curr_ptr++;
            continue;
        }
        else {
            form_token(result, curr_ptr, tok::HexIntLiteral);
            return true;
        }
    }

    DELTA_UNREACHABLE("physically unreachable");
}


bool Lexer::lex_string_literal(Token& result, const char* curr_ptr) {
    bool escaped = false;
    while (true) {
        char c = *curr_ptr++;
        if (c == '\\' && !escaped) {
            escaped = true;
        }
        else if (c == '"' && !escaped) {
            // end of string literal
            break;
        }
        else {
            escaped = false;
        }
    }

    form_token(result, curr_ptr, tok::StringLiteral);
    return true;
}


bool Lexer::lex_identifier_continue(Token& result, const char* curr_ptr) {
    while (is_alphanumeric(*curr_ptr) || *curr_ptr == '_') {
        curr_ptr++;
    }

    form_token(result, curr_ptr, tok::Identifier);
    return true;
}

bool Lexer::lex(Token& result) {
    result.start_token();
    
    if (is_eof())
        return false;

    const char* curr_ptr = buffer_curr;

    // buffer_curr points to c, curr_ptr points to the next char
    char c;
    char next;
    tok::Kind type;

    // skips all the whitespaces before a token
    // buffer_curr points to the start of the token
    // curr_ptr points to the next character to lex
    while (is_whitespace(c = *curr_ptr++)) {
        buffer_curr++;
    }

    switch (c) {
    case 0: // reached eof?
        if (curr_ptr == buffer_end) {
            form_token(result, curr_ptr, tok::EndOfFile);
            return true;
        } else {
            // TODO: warn null in buffer
            form_token(result, curr_ptr, tok::ERROR);
            return false;
        }
        
    case '0': 
        // 0x || 0X
        next = *curr_ptr;
        if (next == 'x' || next == 'X')
            return lex_hex(result, ++curr_ptr);

        [[fallthrough]];

    case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        // encountered a number
        return lex_numeric_literal(result, curr_ptr);

    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
    case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
    case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
    case 'v': case 'w': case 'x': case 'y': case 'z':
        // encountered keyword or identifier
        // keywords will never start with capital letters
        curr_ptr--; // return back to the first character for trie matching
        type = kwtrie.tok_search(curr_ptr).value_or(tok::Identifier);
        if (type != tok::Identifier) {
            break;
        }

        [[fallthrough]];

    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
    case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
    case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
    case 'V': case 'W': case 'X': case 'Y': case 'Z':
    case '_':
        // encountered identifier
        return lex_identifier_continue(result, curr_ptr);
    
    // handling simple punctuations
    case '[':
        type = tok::LeftSquare;
        break;

    case ']':
        type = tok::RightSquare;
        break;

    case '{':
        type = tok::LeftBrace;
        break;

    case '}':
        type = tok::RightBrace;
        break;

    case '(':
        type = tok::LeftParen;
        break;

    case ')':
        type = tok::RightParen;
        break;

    case '.': 
        next = *curr_ptr;
        if (is_digit(next))
            // float starts with a dot, must go back to lex the dot
            return lex_numeric_literal(result, curr_ptr - 1);
        type = tok::Dot;
        break;

    case '=':
        next = *curr_ptr;

        switch (next) {
        case '>':
            type = tok::EqualGreater;
            break;
        
        case '=':
            type = tok::EqualEqual;
            break;

        default:
            type = tok::Equal;
            goto SuccessEnd;
        }

        curr_ptr++;
        break;

    case '+':
        next = *curr_ptr;

        if (next == '=') {
            curr_ptr++;
            type = tok::PlusEqual;
            break;
        }

        type = tok::Plus;
        break;

    case '-':
        next = *curr_ptr;

        switch (next) {
        case '>':
            type = tok::MinusGreater;
            break;

        case '=':
            type = tok::MinusEqual;
            break;

        default:
            type = tok::Minus;
            goto SuccessEnd;
        }

        curr_ptr++;
        break;

    case '*':
        next = *curr_ptr;

        if (next == '=') {
            curr_ptr++;
            type = tok::StarEqual;
            break;
        }

        type = tok::Star;
        break;

    case '/':
        next = *curr_ptr;

        if (next == '=') {
            curr_ptr++;
            type = tok::SlashEqual;
            break;
        }

        type = tok::Slash;
        break;

    case '%':
        next = *curr_ptr;

        if (next == '=') {
            curr_ptr++;
            type = tok::PercentEqual;
            break;
        }

        type = tok::Percent;
        break;

    case '~':
        next = *curr_ptr;

        if (next == '=') {
            curr_ptr++;
            type = tok::TildeEqual;
            break;
        }

        type = tok::Tilde;
        break;

    case '|':
        next = *curr_ptr;

        switch (next) {
        case '|':
            type = tok::PipePipe;
            break;

        case '=':
            type = tok::PipeEqual;
            break;

        default:
            type = tok::Pipe;
            goto SuccessEnd;
        }

        curr_ptr++;
        break;

    case '&':
        next = *curr_ptr;

        switch (next) {
        case '&':
            type = tok::AmpAmp;
            break;

        case '=':
            type = tok::AmpEqual;
            break;

        default:
            type = tok::Amp;
            goto SuccessEnd;
        }

        curr_ptr++;
        break;

    case '^':
        next = *curr_ptr;

        if (next == '=') {
            curr_ptr++;
            type = tok::CaretEqual;
            break;
        }

        type = tok::Caret;
        break;

    case '!':
        next = *curr_ptr;

        if (next == '=') {
            curr_ptr++;
            type = tok::ExclaimEqual;
            break;
        }

        type = tok::Exclaim;
        break;

    case '>':
        next = *curr_ptr;

        switch (next) {
        case '=':
            curr_ptr++;
            type = tok::GreaterEqual;
            goto SuccessEnd;

        case '>':
            next = *++curr_ptr; // points the next char after >>

            if (next == '=') {
                curr_ptr++; // points to the next char after >>=
                type = tok::GreaterGreaterEqual;
            } else {
                type = tok::GreaterGreater;
            }
            
            goto SuccessEnd;
        }

        type = tok::Greater;
        break;

    case '<':
        next = *curr_ptr;

        switch (next) {
        case '=':
            curr_ptr++;
            type = tok::LessEqual;
            goto SuccessEnd;

        case '<':
            next = *++curr_ptr; // points to the next char after <<

            if (next == '=') {
                curr_ptr++; // points to the next char after <<=
                type = tok::LessLessEqual;
            } else {
                type = tok::LessLess;
            }
            
            goto SuccessEnd;
        }

        type = tok::Less;
        break;

    case ',':
        type = tok::Comma;
        break;

    case ';':
        type = tok::Semicolon;
        break;

    case ':':
        next = *curr_ptr;

        if (next == ':') {
            curr_ptr++;
            type = tok::ColonColon;
            break;
        }

        type = tok::Colon;
        break;

    case '"':
        // encountered a string literal
        return lex_string_literal(result, curr_ptr);

    case '\'':
        // encountered a char literal
        next = *curr_ptr++;

        if (next == '\\') {
            // skipping through escaped char
            curr_ptr++;
        }
        
        if (*curr_ptr == '\'') {
            form_token(result, curr_ptr, tok::CharLiteral);
            return true;
        }

        // TODO: Error invalid char literal
        // skips until the next single quote
        do {
            curr_ptr++;
        } while (*curr_ptr != '\'' && *curr_ptr != 0);
        
        [[fallthrough]];

    default:
        // unrecognized character or bad char literal
        form_token(result, curr_ptr, tok::ERROR);
        return false;
    }

SuccessEnd:
    // success fully lexed a token
    form_token(result, curr_ptr, type);
    return true;
}

}
