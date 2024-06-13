#pragma once

#include "decl.hpp"
#include "expression.hpp"
#include "statement.hpp"
#include "filebuffer.hpp"
#include "token.hpp"
#include "lexer.hpp"
#include "utils.hpp"
#include "astcontext.hpp"
#include "sema.hpp"

#include <memory>
#include <iterator>

namespace deltac {

class Parser {
public:
    Parser(const SourceBuffer& src, Sema& s);

    Parser(const Parser&) = delete;
    Parser(Parser&&) = delete;

    bool parse();

    Decl* release_decl() {
        return decl.release();
    }

private:
    std::optional<QualType> type();

    Decl* declaration();
    Decl* variable_declaration();

    template <typename Container, typename Fn>
    bool list_of(
        std::back_insert_iterator<Container> out, 
        Fn&& fn, 
        tok::Kind start, 
        tok::Kind end, 
        bool accept_empty = true
    ) {
        if (!advance_expected(start)) {
            return false;
        }

        if (accept_empty && curr_token.is(end)) {
            advance();
            return true;
        }
        else if (!accept_empty && curr_token.is(end)) {
            return false;
        }

        while (!curr_token.is(end)) {
            if (auto res = fn()) {
                
            } 
            else {
                return false;
            }
        }
    }

    template <typename Container>
    bool parameter_list(std::back_insert_iterator<Container> out) {
        if (!advance_expected(tok::LeftParen))
            return false;

        while (true) {
            if (!curr_token.is(tok::Identifier)) {
                return false;
            }

            std::string_view identifier = curr_token.get_view();
            advance();
            if (!advance_expected(tok::Colon)) {
                return false;
            }

            auto ty = type();
            if (!ty) {
                return false;
            }

            out = Parameter((std::string)identifier, std::move(ty));

            if (curr_token.is(tok::Comma)) { // next parameter
                continue;
            }
            else if (curr_token.is(tok::RightParen)) { // end of list
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
        tok::Kind start_token, 
        tok::Kind end_token
    ) {
        if (!advance_expected(start_token)) {
            return false;
        }

        while (!curr_token.is(end_token)) {
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
    Expr* binary_expression();
    Expr* recursive_parse_binary_expression(prec::Binary);

    // TypeInfo type_info();
    bool advance_expected(tok::Kind type);
    void advance();

private:
    Lexer lexer;
    Sema& action;

    Token curr_token;

    std::unique_ptr<Decl> decl;
};

} // namespace deltac
