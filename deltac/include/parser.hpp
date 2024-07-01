#pragma once

#include "declaration.hpp"
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
#include <functional>

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
    TypeResult type();

    DeclResult declaration();
    DeclResult variable_declaration();

    ParameterResult parameter();

    template <typename Container, typename Fn>
    bool parse_list_of(
        std::back_insert_iterator<Container> out, 
        Fn&& fn, 
        tok::Kind start, 
        tok::Kind end,
        tok::Kind delimiter = tok::Comma, 
        bool accept_empty = true,
        bool allow_trailing_delim = false
    ) {
        if (!advance_expected(start)) {
            return false;
        }

        // do not accept empty but got an empty list
        if (!accept_empty && curr_token.is(end)) {
            // TODO: error empty list
            return false;
        }

        while (!curr_token.is(end)) {
            auto res = std::invoke(fn);

            if (!res) {
                return false;
            }

            *out = *res;

            if (curr_token.is(delimiter)) {
                advance();

                if (curr_token.is(end) && allow_trailing_delim) {
                    advance();
                    return true;
                }
                else if (curr_token.is(end) && !allow_trailing_delim) {
                    // TODO: diag no trailing delimiter

                    return false;
                }

                // else continue to parse the next element
            }
        }

        return true;
    }

    StmtResult statement();
    StmtResult compound_statement();
    StmtResult expression_statement();
    StmtResult return_statement();

    ExprResult expression();
    ExprResult primary_expression();
    ExprResult integer_literal_expression();
    ExprResult postfix_expression();
    ExprResult unary_expression();
    ExprResult cast_expression();
    ExprResult binary_expression();
    ExprResult recursive_parse_binary_expression(prec::Binary);
    ExprResult assignment_expression();

    // TypeInfo type_info();
    bool advance_expected(tok::Kind type);
    bool try_advance(tok::Kind type);
    void advance();

    template <typename Fn>
    auto bind_this(Fn&& fn) {
        return std::bind(fn, this);
    }

private:
    Lexer lexer;
    Sema& action;

    Token curr_token;

    std::unique_ptr<Decl> decl;
};

} // namespace deltac
