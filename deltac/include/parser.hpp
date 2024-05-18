#pragma once

#include "decl.hpp"
#include "expression.hpp"
#include "statement.hpp"
#include "filebuffer.hpp"
#include "token.hpp"
#include "lexer.hpp"
#include "utils.hpp"

#include <ranges>
#include <memory>
#include <iterator>

class Parser {
public:
    explicit Parser(const SourceBuffer& src);
    bool parse();

    Decl* release_decl() {
        return decl.release();
    }

private:
    bool type(QualType&);

    Decl* declaration();
    Decl* variable_declaration();

    template <typename Container>
    bool parameter_list(std::back_insert_iterator<Container> out) {
        if (!advance_expected(TokenType::LeftParen))
            return false;

        while (true) {
            if (!curr_token.is_one_of(TokenType::Identifier)) {
                return false;
            }

            std::string_view identifier = curr_token.get_view();
            advance();
            if (!advance_expected(TokenType::Colon)) {
                return false;
            }

            QualType ty;
            if (!type(ty)) {
                return false;
            }

            out = (Parameter) { (std::string)identifier, std::move(ty) };

            if (curr_token.is_one_of(TokenType::Comma)) { // next parameter
                continue;
            }
            else if (curr_token.is_one_of(TokenType::RightParen)) { // end of list
                break;
            }
            else { // diag unrecognized token after type
                return false;
            }
        }

        advance(/* TokenType::RightParen */);

        return true;
    }

    Stmt* statement();
    Stmt* compound_statement();
    Stmt* expression_statement();
    Stmt* return_statement();

    template <typename Container>
    bool expression_list(
        std::back_insert_iterator<Container> out, 
        TokenType start_token, 
        TokenType end_token
    ) {
        if (!advance_expected(start_token)) {
            return false;
        }

        while (!curr_token.is_one_of(end_token)) {
            if (Expr* expr = expression()) {
                out = expr;
            }
            else {
                return false;
            }
        }

        advance();
        return true;
    }

    Expr* expression();
    Expr* primary_expression();
    Expr* integer_literal_expression();
    Expr* postfix_expression();
    Expr* unary_expression();
    Expr* cast_expression();
    // Expr* rhs_of_binary_expression(Expr* lhs);

    // TypeInfo type_info();
    bool advance_expected(TokenType type);
    void advance();

private:
    Lexer lexer;
    ASTContext& context;

    Token curr_token;

    std::unique_ptr<Decl> decl;
};
