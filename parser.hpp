#pragma once

#include "expression.hpp"
#include "statement.hpp"
#include "filebuffer.hpp"
#include "token.hpp"
#include "lexer.hpp"

#include <memory>

class Parser {
public:
    explicit Parser(const SourceBuffer& src);
    bool parse();

    Stmt* release_statement() {
        return stmt.release();
    }

private:
    TypeInfo type();

    Stmt* statement();

    DefStmt* definition_statement();
    FuncDefStmt* function_definition_statement();
    LetDefStmt* variable_definition_statement();
    ParenmeterList parameter_list();
    
    CompoundStmt* compound_statement();
    ExprStmt* expression_statement();
    ReturnStmt* return_statement();

    Expr* expression();
    LiteralExpr* literal_expression();
    PrimaryExpr* primary_expression();

    std::string_view advance_identifier() {
        std::string_view id = curr_token.get_view();
        advance_expected(TokenType::Identifier);
        return id;
    }

    // TypeInfo type_info();
    void advance_expected(TokenType type);
    bool advance() noexcept;
    
    void handel_lex_stop();

private:
    Lexer lexer;
    Token curr_token;
    Token next_token;

    std::unique_ptr<DefStmt> stmt;
};
