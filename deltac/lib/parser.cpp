#include "parser.hpp"

#include "literal_support.hpp"
#include "operators.hpp"
#include "tokentype.hpp"
#include "utils.hpp"

#include <string_view>

namespace deltac {

Parser::Parser(Lexer& lexer, Sema& s) : lexer(lexer), action(s) {
    lexer.lex(curr_token); // must at least have an EOF token
    
    if (curr_token.is(tok::EndOfFile))
        void();// TODO: diag empty file
}

/*
 * Program
 *     : TopLevelDeclaration
 *     | Program TopLevelDeclaration
 *     ;
 * 
 * TopLevelDeclaration
 *     : Declaration
 *     ;
 */
bool Parser::parse_top_level_decl(Decl*& res) {
    if (curr_token.is(tok::EndOfFile)) {
        return false;
    }

    auto declres = declaration();

    if (declres) {
        res = *declres;
        return true;
    }
    else {
        return false;
    }
}



/*
 * ParameterDeclaration
 *     : Identifier TypeSpecifier
 *     ;
 */
ParameterResult Parser::parameter() {
    if (!curr_token.is(tok::Identifier)) {
        return action_error;
    }

    std::string_view id = curr_token.get_view();

    advance();

    if (auto ty = type()) {
        return ParameterResult(std::in_place, id, std::move(ty));
    }
    
    return action_error;
}

/*
 * CompoundStmt
 *     : '{' StmtList '}'
 *     ;
 * 
 * StmtList
 *     : Stmt
 *     : StmtList Stmt
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
 * TypeSpecifier
 *     : 'void'
 *     | Typename 'const'[opt]
 *     | '*' 'const'[opt] Type
 *     ;
 */
TypeResult Parser::type() {
    TypeBuilder builder(action);

    while (true) {
        if (curr_token.is(tok::Void)) {
            advance();

            return (QualType)action.ast_context().get_void_ty();
        }
        else if (curr_token.is(tok::Identifier)) {
            Token t = curr_token;

            advance();

            bool is_const = try_advance(tok::Const);

            builder.finalize(t, is_const);
            
            return builder.release();
        }
        else if (curr_token.is(tok::Star)) {
            advance();
            
            bool is_const = try_advance(tok::Const);

            builder.add_ptr(is_const);
            // continue to parse compound type
        }
        else { // TODO: unrecognized token in type
            return action_error;
        }
    }

    DELTA_UNREACHABLE("logically unreachable");
}

/*
 * Typename
 *     : Identifier
 *     ;
 */
RawTypeResult Parser::raw_type() {
    if (!curr_token.is(tok::Identifier)) {
        return action_error;
    }

    return ;
}

/*
 * ReturnStmt
 *     : 'return' Expression[opt] ';'
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
 *     : CompoundStatement
 *     | DeclarationStatement
 *     | ExprStatement
 *     | ReturnStatement
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
 *     : AssignmentExpr
 *     ;
 */
ExprResult Parser::expression() {
    return assignment_expression();
}

/*
 * PrimaryExpr
 *     : IdExpr
 *     | ParenExpr
 *     | LiteralExpr
 *     ;
 * 
 * IdExpression
 *     : Identifier
 *     ;
 * 
 * ParenExpression
 *     : '(' Expression ')'
 *     ;
 * 
 */
ExprResult Parser::primary_expression() {
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

        ExprResult expr = expression();
        return_if_not(expr);

        if (!advance_expected(tok::RightParen)) {
            expr.deletep();
            return action_error;
        }

        // return new ParenExpr(expr);
    }

    // TODO: unrecognized token for primary expression
    return action_error;
}

ExprResult Parser::integer_literal_expression() {
    u8 posix = 10;
    switch (curr_token.get_type()) {
        using namespace tok; 
    case HexIntLiteral: {
        posix = 16;
        [[fallthrough]];
    case DecIntLiteral:
        advance();

        return action.act_on_int_literal(curr_token, posix, nullptr);
    }
    default:
        DELTA_UNREACHABLE("must be a literal expression type token");
    }
}

/* PostfixExpression:
 *     : PrimaryExpression
 *     | PostFixExpression '(' ExpressionList[opt] ')'   (CallExpression)
 *     ;
 */
ExprResult Parser::postfix_expression() {
    ExprResult expr = primary_expression();
    return_if_not(expr);

    // CallExpression or IndexExpression
    while (true) {
        if (curr_token.is_one_of(tok::LeftParen)) { // callexpr
            llvm::SmallVector<Expr*> args;
            bool is_valid = parse_list_of(
                std::back_inserter(args), 
                bind_this(&Parser::expression),
                tok::LeftParen,
                tok::RightParen
            );
            if (!is_valid) {
                util::cleanup_ptrs(args.begin(), args.end());

                expr.deletep();

                return action_error;
            }
        } else {
            break;
        }
        // TODO: index expression
    }

    return expr;
}

/*
 * UnaryExpression
 *     : PostfixExpression
 *     | UnaryOperator CastExpression
 *     ;
 */
ExprResult Parser::unary_expression() {
    if (auto op = to_unary_operator(curr_token.get_type())) {
        advance();

        auto expr = unary_expression();
        return_if_not(expr);

        return action.act_on_unary_expr(*op, *expr);
    }
    else {
        return postfix_expression();
    }
}

// parsed according to precedence
ExprResult Parser::binary_expression() {
    return recursive_parse_binary_expression(prec::Unknown);
}

ExprResult Parser::recursive_parse_binary_expression(prec::Binary min_precedence) {
    ExprResult lhs = unary_expression();
    return_if_not(lhs);

    while (true) {
        auto opt_op = to_binary_operator(curr_token.get_type()); // not a binary operator, binary expression ends
        if (!opt_op) 
            break;

        prec::Binary cur_op_precedence = get_precedence(opt_op.value());

        if (cur_op_precedence < min_precedence) 
            break;

        advance();

        prec::Binary next_min_precedence = (prec::Binary)(cur_op_precedence + 1);

        ExprResult rhs = recursive_parse_binary_expression(next_min_precedence);

        if (!rhs) {
            // TODO: diag
            lhs.deletep();
            return action_error;
        }

        lhs = action.act_on_binary_expr(*lhs, *opt_op, *rhs);

        if (!lhs) {
            // TODO: diag
            break;
        }
    }

    return lhs;
}

/*
 * AssignmentExpr
 *     : BinaryExpr
 *     : BinaryExpr AssignmentOperator AssignmentExpr
 *     ;
 */
ExprResult Parser::assignment_expression() {
    ExprResult lhs = binary_expression();
    return_if_not(lhs);

    if (auto op = to_assignment_operator(curr_token.get_type())) {
        if (auto ae = assignment_expression()) {
            return action.act_on_assignment_expr(*lhs, *op, *ae);
        }
        else {
            lhs.deletep();
            // TODO: diag invalid expr
            return action_error;
        }
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

bool Parser::try_advance(tok::Kind type) {
    return advance_expected(type);
}

void Parser::advance() {
    lexer.lex(curr_token);
}

}
