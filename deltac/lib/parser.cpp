#include "parser.hpp"

#include "literal_support.hpp"

namespace deltac {

Parser::Parser(const SourceBuffer& src, Sema& s) : lexer(src), action(s) {
    lexer.lex(curr_token); // must at least have an EOF token
    
    if (curr_token.is(tok::EndOfFile))
        void();// TODO: warm empty file
}

/*
 * Program
 *     : Decl*
 *     ;
 */
bool Parser::parse() {
    assert(!curr_token.is(tok::ERROR));

    if (curr_token.is(tok::EndOfFile)) {
        // diag empty file
        return false;
    }

    Decl* new_decl = declaration();
    if (new_decl != nullptr) {
        decl.reset(new_decl);
        return true;
    } else {
        return false;
    }
}

Decl* Parser::declaration() {
    if (curr_token.is(tok::Let)) {
        return variable_declaration();
    } 
    else if (curr_token.is(tok::Fn)) {
        return nullptr;
    }
    else {
        return nullptr;
    }
}



/*
 * ParameterList
 *     : '(' (Parameter (',' Parameter)*)? ')'
 *     ;
 * 
 * Parameter
 *     : Identifier ':' QualType
 *     ;
 */

/*
 * CompoundStmt
 *     : '{' StmtList '}'
 *     ;
 * 
 * StmtList
 *     : Stmt*
 *     ;
 */
/*
CompoundStmt* Parser::compound_statement() {
    std::unique_ptr<CompoundStmt> compstmt = std::make_unique<CompoundStmt>();

    advance_expected(TokenType::LeftBrace);

    // statement list
    while (!curr_token.is_one_of(TokenType::RightBrace)) {
        compstmt->stmts.push_back(statement());
    }

    advance_expected(TokenType::RightBrace);

    return compstmt.release();
}
*/

/*
 * ExprStmt
 *     : Expr ';'
 *     ;
 */
/*
ExprStmt* Parser::expression_statement() {
    std::unique_ptr<ExprStmt> exprstmt = std::make_unique<ExprStmt>();

    exprstmt->expr = expression();
    advance_expected(TokenType::Semicolon);

    return exprstmt.release();
}
*/

/*
 * Type
 *     : Identifier
 *     | '*' Type
 *     ;
 */
bool Parser::type(QualType& newtype) {
    newtype = QualType(); // reset type object just in case

    while (true) {
        if (curr_token.is(tok::Identifier)) {
            bool is_const = false;
            if (curr_token.is(tok::Const)) {
                is_const = true;
                advance();
            }

            if (false/* = action.get_basic_qualtype(curr_token.get_view()) */) {
                // TODO: error unrecognized basic type
                return false;
            }
            
            return true;
        }
        else if (curr_token.is(tok::Star)) {
            advance();

            bool is_const = false;
            if (curr_token.is(tok::Const)) {
                is_const = true;
                advance();
            }

            newtype.add_ptr(is_const);
            // continue to parse compound type
        }
        else { // TODO: unrecognized token in type
            return false;
        }
    }

    DELTA_UNREACHABLE("logically unreachable");
}

/*
 * ReturnStmt
 *     : 'return' Expr[opt] ';'
 */
/*
ReturnStmt* Parser::return_statement() {
    std::unique_ptr<ReturnStmt> retstmt = std::make_unique<ReturnStmt>();

    advance_expected(TokenType::Return);

    if (!curr_token.is_one_of(TokenType::Semicolon)) {
        retstmt->expr = expression();
    }

    advance_expected(TokenType::Semicolon);

    return retstmt.release();
}
*/

/*
 * Stmt
 *     : CompoundStmt
 *     | LetDefStmt
 *     | FuncDefStmt
 *     | ExprStmt
 *     | ReturnStmt
 *     ;
 */
/*
Stmt* Parser::statement() {
    switch (curr_token.get_type()) {
        using enum TokenType;
    case LeftBrace:
        return compound_statement();
    case Let:
        return variable_definition_statement();
    case Fn:
        return function_definition_statement();
    case Return:
        return return_statement();
    default:
        return expression_statement();
    }
}
*/
/*
 * Expr
 *     : PostfixExpr
 *     ;
 */
Expr* Parser::expression() {
    return cast_expression();
}

/*
 * PrimaryExpr
 *     : IdExpr
 *     | ParenExpr
 *     | LiteralExpr
 *     ;
 * 
 * IdExpr
 *     : Identifier
 *     ;
 * 
 * ParenExpr
 *     : '(' Expr ')'
 *     ;
 * 
 */
Expr* Parser::primary_expression() {
    if (curr_token.is_one_of(
        tok::HexIntLiteral, tok::DecIntLiteral
    )) {
        return integer_literal_expression();
    }
    else if (curr_token.is(tok::Identifier)) {
        // return action.act_on_id_expr(curr_token);
    }
    else if (curr_token.is(tok::LeftParen)) {
        advance();

        Expr* expr = expression();
        if (expr == nullptr) {
            return nullptr;
        }

        if (!advance_expected(tok::RightParen)) {
            delete expr;
            return nullptr;
        }

        return new ParenExpr(expr);
    }

    // TODO: unrecognized token for primary expression
    return nullptr;
}

Expr* Parser::integer_literal_expression() {
    std::uint8_t posix = 10;
    switch (curr_token.get_type()) {
        using namespace tok; 
    case HexIntLiteral: {
        posix = 16;
        [[fallthrough]];
    case DecIntLiteral:
        advance();

        if (curr_token.is(As)) {
            advance();
            if (auto ty = type()) {
                return action.act_on_int_literal(curr_token, posix, &*ty);
            } else {
                return nullptr;
            }
        }

        return action.act_on_int_literal(curr_token, posix, nullptr);
    }
    default:
        DELTA_UNREACHABLE("must be a literal expression type token");
    }
}

/* PostfixExpression:
 *     : PrimaryExpression
 *     | PostFixExpression '(' ExpressionList ')'   (CallExpression)
 *     | PostFixExpression '[' Expression ']'       (IndexExpression) // TODO: implement this
 *     ;
 */
Expr* Parser::postfix_expression() {
    // CallExpression or IndexExpression
    while (true) {
        if (curr_token.is_one_of(TokenType::LeftParen)) {
            std::vector<Expr*> args;
            bool is_valid = expression_list(args, TokenType::LeftParen, TokenType::RightParen);
            if (!is_valid) {
                for (auto* p : args)
                    delete p;

                return nullptr;
            }

            callexpr->mainexpr = primexpr.release(); // noexcept
            primexpr.reset(callexpr.release()); // noexcept
        } else {
            break;
        }
        // TODO: index expression
    }

    return primary_expression();
}

/*
 * UnaryExpression
 *     : PostfixExpression
 *     | UnaryOperator CastExpression
 *     ;
 */
Expr* Parser::unary_expression() {
    if (is_unary_operator(curr_token)) {
        UnaryOp op = *to_unary_operator(curr_token.get_type());
        advance();

        if (Expr* expr = unary_expression()) {
            // act on expr
            return action.act_on_unary_expr(op, expr);
        }
        else {
            return nullptr;
        }
    }
    else {
        return postfix_expression();
    }
}

/*
 * CastExpression
 *     : UnaryExpression
 *     | CastExpression 'as' Typename
 *     ;
 */
Expr* Parser::cast_expression() {
    Expr* expr = unary_expression();
    if (!expr)
        return nullptr;

    while (curr_token.is(tok::To)) {
        advance();

        if (auto opt_ty = type()) {
            action.act_on_cast_expr(expr, *opt_ty);
        }
        else {
            // error invalid type
            delete expr;
            return nullptr;
        }
    }

    // act on expr

    return expr;
}

Expr* Parser::binary_expression() {
    return recursive_parse_binary_expression(prec::Unknown);
}

Expr* Parser::recursive_parse_binary_expression(prec::Binary min_precedence) {
    Expr* lhs = cast_expression();

    while (true) {
        auto opt_op = to_binary_operator(curr_token.get_type()); // not a binary operator, binary expression ends
        if (!opt_op) 
            break;

        prec::Binary cur_op_precedence = get_precedence(opt_op.value());

        if (cur_op_precedence < min_precedence) 
            break;

        advance();

        prec::Binary next_min_precedence = (prec::Binary)(cur_op_precedence + 1);

        Expr* rhs = recursive_parse_binary_expression(next_min_precedence);

        if (!rhs) {
            delete lhs;
            return nullptr;
        }

        lhs = action.act_on_binary_expr(lhs, *opt_op, rhs);

        if (!lhs) {
            return nullptr;
        }
        // lhs = new BinaryExpr(QualType(), Expr::Unclassified, lhs, opt_op.value(), rhs);
    }

    return lhs;
}

bool Parser::advance_expected(tok::Kind type) {
    if (!curr_token.is(type)) {
        return false;
    }

    advance();
    return true;
}

void Parser::advance() {
    lexer.lex(curr_token);
}

}
