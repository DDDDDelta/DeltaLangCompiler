#include "parser.hpp"

#include "literal_support.hpp"

Parser::Parser(const SourceBuffer& src) : lexer(src) {
    lexer.lex(curr_token); // must at least have an EOF token
    
    if (curr_token.is_one_of(TokenType::EndOfFile))
        void();// TODO: warm empty file
}

/*
 * Program
 *     : Decl*
 *     ;
 */
bool Parser::parse() {
    assert(!curr_token.is_one_of(TokenType::ERROR));

    if (curr_token.is_one_of(TokenType::EndOfFile)) {
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
    if (curr_token.is_one_of(TokenType::Let)) {
        return variable_declaration();
    } 
    else if (curr_token.is_one_of(TokenType::Fn)) {
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
        if (curr_token.is_one_of(TokenType::Identifier)) {
            if (bool b/* = action.get_basic_qualtype(curr_token.get_view()) */) {
                // TODO: error unrecognized basic type
                return false;
            }
            
            return true;
        }
        else if (curr_token.is_one_of(TokenType::Star)) {
            advance();

            bool is_const = false;
            if (curr_token.is_one_of(TokenType::Const)) {
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
 *     : 'return' Expr? ';'
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
        TokenType::HexIntLiteral, TokenType::DecIntLiteral
    )) {
        return integer_literal_expression();
    }
    // dont have sema yet...
    /*
    else if (curr_token.is_one_of(TokenType::Identifier)) {
        std::unique_ptr<IdExpr> idexpr = std::make_unique<IdExpr>();

        idexpr->identifier = advance_identifier();

        return idexpr.release();
    }
    */
    else if (curr_token.is_one_of(TokenType::LeftParen)) {
        advance();

        Expr* expr = expression();
        if (expr == nullptr) {
            return nullptr;
        }

        if (!advance_expected(TokenType::RightParen)) {
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
        using enum TokenType; 
    case HexIntLiteral: {
        posix = 16;
        [[fallthrough]];
    case DecIntLiteral:
        QualType spectype; // = action.get_i32_ty();

        if (next_token.is_one_of(As)) {
            if (!type(spectype)) {
                return nullptr;
            }
        }

        // sema.is_compatibile(spectype);
        // error incompatible specified type for int literal
        bool is_unsigned = true; // sema.is_signed_ty(spectype);
        std::uint32_t bitwidth = 32; // sema.type_size(spectype);

        IntLiteralParser literal_parser(curr_token.get_view(), posix);
        llvm::APSInt val(bitwidth, is_unsigned);

        literal_parser.get_apint_val(val);

        advance();

        return new IntLiteralExpr(std::move(spectype), std::move(val));
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

    // lets forget about this for a sec...
    /*
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
    */

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

        Expr* expr = unary_expression();
        if (!expr)
            return nullptr;

        QualType type; // = sema.unaryexpr_type(op, expr);

        return new UnaryExpr(std::move(type), op, expr);
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

    while (curr_token.is_one_of(TokenType::As)) {
        advance();

        QualType cast_to;
        if (!type(cast_to)) {
            // error invalid type
            delete expr;
            return nullptr;
        }

        // if (!sema.validate_cast(expr, type)) {
        //     delete expr;
        //     return nullptr;
        // }

        CastExpr* castexpr = new CastExpr(std::move(cast_to), expr);
    }

    // act on expr

    return expr;
}

bool Parser::advance_expected(TokenType type) {
    if (!curr_token.is_one_of(type)) {
        return false;
    }

    advance();
    return true;
}

void Parser::advance() {
    lexer.lex(curr_token);
}
