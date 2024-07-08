#include "parser.hpp"

#include "literal_support.hpp"
#include "operators.hpp"
#include "tokentype.hpp"
#include "utils.hpp"

#include <string_view>

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

DeclResult Parser::declaration() {
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
TypeResult Parser::type() {
    TypeBuilder builder(action);

    while (true) {
        if (curr_token.is(tok::Identifier)) {
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
 * IdExpr
 *     : Identifier
 *     ;
 * 
 * ParenExpr
 *     : '(' Expr ')'
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
        if (!expr) {
            return action_error;
        }

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

        if (curr_token.is(As)) {
            advance();
            if (auto ty = type()) {
                return action.act_on_int_literal(curr_token, posix, &*ty);
            } else {
                return action_error;
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
 *     | PostFixExpression '(' ExpressionList[opt] ')'   (CallExpression)
 *     | PostFixExpression '[' Expression ']'       (IndexExpression) // TODO: implement this
 *     ;
 */
ExprResult Parser::postfix_expression() {
    ExprResult expr = primary_expression();

    if (!expr) {
        return action_error;
    }

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

/*
 * CastExpression
 *     : UnaryExpression
 *     | CastExpression 'as' Typename
 *     ;
 */
ExprResult Parser::cast_expression() {
    auto expr = unary_expression();
    return_if_not(expr);

    while (curr_token.is(tok::To)) {
        advance();

        if (auto opt_ty = type()) {
            action.act_on_cast_expr(*expr, *opt_ty);
        }
        else {
            // error invalid type
            expr.deletep();
            return action_error;
        }
    }

    // act on expr

    return expr;
}

ExprResult Parser::binary_expression() {
    return recursive_parse_binary_expression(prec::Unknown);
}

ExprResult Parser::recursive_parse_binary_expression(prec::Binary min_precedence) {
    ExprResult lhs = cast_expression();
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
        // lhs = new BinaryExpr(QualType(), Expr::Unclassified, lhs, opt_op.value(), rhs);
    }

    return lhs;
}

/*
 * AssignmentExpr
 *     : BinaryExpr
 *     : Binary AssignmentOperator AssignmentExpr
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
