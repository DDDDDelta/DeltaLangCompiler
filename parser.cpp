#include "parser.hpp"

class ParseStop : public std::exception {
public:
    enum StopType {
        LexerError,
        ParserError,
        EndOfFile,
    };

public:
    explicit ParseStop(/* fileid */ StopType type = ParserError) : type(type) {}
    StopType get_type() const { return type; }
    const char* what() const noexcept override { return /* file */ "compilation ended"; }
    ~ParseStop() override = default; // TODO: emit error message

private:
    StopType type;
};

explicit Parser::Parser(const SourceBuffer& src) : lexer(src) {
    lexer.lex(curr_token); // must at least have an EOF token
    bool hastwo = lexer.lex(next_token);
    
    if (!hastwo) {
        assert(next_token.is_one_of(TokenType::EndOfFile));
        // TODO: error/warn empty file
    }
}

/*
 * Program
 *     : DefStmt*
 *     ;
 */
bool Parser::parse() {
    assert(!curr_token.is_one_of(TokenType::ERROR));

    if (curr_token.is_one_of(TokenType::EndOfFile)) {
        return false;
    }

    DefStmt* new_stmt = nullptr;
    try {
        new_stmt = definition_statement();
        assert(new_stmt != nullptr); // must succeed or throw

        stmt.reset(new_stmt); // noexcept here
    } catch (const ParseStop& e) {
        delete new_stmt; // just to make sure

        return false;
    }

    return true;
}

/*
 * DefStmt
 *     : FuncDefStmt
 *     | LetDefStmt
 *     ;
 */
DefStmt* Parser::definition_statement() {
    if (curr_token.is_one_of(TokenType::Fn)) {
        return function_definition_statement();
    } else if (curr_token.is_one_of(TokenType::Let)) {
        return variable_definition_statement();
    } else {
        // unrecognized token
        throw ParseStop(ParseStop::ParserError);
    }
}

/*
 * ParameterList
 *     : '(' (Parameter (',' Parameter)*)? ')'
 *     ;
 * 
 * Parameter
 *     : Identifier ':' Type
 *     ;
 */
ParenmeterList Parser::parameter_list() {
    ParameterList list;

    advance_expected(TokenType::LeftParen);

    while (!curr_token.is_one_of(TokenType::RightParen)) {
        Parenmeter param;
        param.identifier = advance_identifier();
        advance_expected(TokenType::Colon);
        param.type = type();
        // parenmeter

        list.emplace_back(std::move(param));

        // does not forbid trailing comma
        if (curr_token.is_one_of(TokenType::Comma)) {
            advance_expected(TokenType::Comma);
        }
    }

    advance_expected(TokenType::RightParen);

    return list;
}

/*
 * FuncDefStmt
 *     : 'fn' Identifier ParenList '->' Type CompoundStmt
 *     ;
 */
FuncDefStmt* Parser::function_definition_statement() {
    std::unique_ptr<FuncDefStmt> defstmt = std::make_unique<FuncDefStmt>(); // exception safe

    advance_expected(TokenType::Fn);

    if (!curr_token.is_one_of(TokenType::Identifier)) {
        throw ParseStop(ParseStop::ParserError);
    }

    defstmt->identifier = advance_identifier();
    defstmt->paren_list = parameter_list();
    
    advance_expected(TokenType::MinusGreater);

    defstmt->rettype = advance_identifier();
    defstmt->funcbody = compound_statement();

    return defstmt.release();
}

/*
 * LetDefStmt
 *     : 'let' Identifier ':' Type '=' Expr
 *     ;
 */
LetDefStmt* Parser::variable_definition_statement() {
    std::unique_ptr<LetDefStmt> defstmt = std::make_unique<LetDefStmt>(); // exception safe

    advance_expected(TokenType::Let);

    if (!curr_token.is_one_of(TokenType::Identifier)) {
        throw ParseStop(ParseStop::ParserError);
    }

    defstmt->identifier = advance_identifier();

    advance_expected(TokenType::Colon);
    
    defstmt->vartype = type();

    advance_expected(TokenType::Equal);

    defstmt->defexpr = expression();

    return defstmt.release();
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

/*
 * ExprStmt
 *     : Expr ';'
 *     ;
 */
ExprStmt* Parser::expression_statement() {
    std::unique_ptr<ExprStmt> exprstmt = std::make_unique<ExprStmt>();

    exprstmt->expr = expression();
    advance_expected(TokenType::Semicolon);

    return exprstmt;
}

/*
 * Type
 *     : Identifier
 *     | '*' Type
 *     ;
 */
TypeInfo Parser::type() {
    if (curr_token.is_one_of(TokenType::Identifier)) {
        return TypeInfo(advance_identifier());
    }
    else if (curr_token.is_one_of(TokenType::Star)) {
        advance_expected(TokenType::Star);
        return type().add_pointer();
    }
    else { // TODO: unrecognized token in type
        throw ParseStop(ParseStop::ParserError);
    }
}

/*
 * ReturnStmt
 *     : 'return' Expr? ';'
 */
ReturnStmt* Parser::return_statement() {
    std::unique_ptr<ReturnStmt> retstmt = std::make_unique<ReturnStmt>();

    advance_expected(TokenType::Return);

    if (!curr_token.is_one_of(TokenType::Semicolon)) {
        retstmt->expr = expression();
    }

    advance_expected(TokenType::Semicolon);

    return retstmt.release();
}

/*
 * Stmt
 *     : CompoundStmt
 *     | LetDefStmt
 *     | FuncDefStmt
 *     | ExprStmt
 *     | ReturnStmt
 *     ;
 */
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

ExprList Parser::expression_list(TokenType start_token, TokenType end_token) {
    ExprList list;

    advance_expected(start_token);

    while (curr_token.is_one_of(end_token)) {
        try {
            list.push_back(expression()); // expression() may throw and push_back is noexcept
        } catch (...) {
            for (auto* expr : list) {
                delete expr;
            }
            throw;
        }

        if (curr_token.is_one_of(TokenType::Comma)) {
            advance();
        }
        else {
            break;
        }
    }

    advance_expected(end_token);

    return list;
}

/*
 * Expr
 *     : PostfixExpr
 *     ;
 */
Expr* Parser::expression() {
    return PostfixExpr();
}

/*
 * LiteralExpr
 *     : IntLiteralExpr
 *     | FloatLiteralExpr
 *     | StringLiteralExpr
 *     ;
 */
Expr* Parser::literal_expression() {
    std::unique_ptr<LiteralExpr> litexpr;

    litexpr->type = TypeInfo(curr_token.get_type());

    switch (curr_token.get_type()) {
        using enum TokenType;
    case HexIntLiteral:
    case DecIntLiteral:
        litexpr->reset(new IntLiteralExpr);
        break;
    case FloatLiteral:
        litexpr->reset(new FloatLiteralExpr);
        break;
    case StringLiteral:
        litexpr->reset(new StringLiteralExpr);
        break;
    default:
        throw ParseStop(ParseStop::ParserError);
        // asserted to be valid literal token
    }

    advance(); // consume the literal token

    return litexpr.release();
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
    if (is_literal_token(curr_token)) {
        return literal_expression();
    }
    else if (curr_token.is_one_of(TokenType::Identifier)) {
        std::unique_ptr<IdExpr> idexpr = std::make_unique<IdExpr>();

        idexpr->identifier = advance_identifier();

        return idexpr.release();
    }
    else if (curr_token.is_one_of(TokenType::LeftParen)) {
        advance_expected(TokenType::LeftParen);

        std::unique_ptr<ParenExpr> expr = std::make_unique<ParenExpr>();
        expr->expr = expression();
        expr->type = expr->expr->type;

        advance_expected(TokenType::RightParen);

        return expr.release();
    }

    throw ParseStop(ParseStop::ParserError); // TODO: unrecognized token for primary expression
}

/* PostfixExpression:
 *     : PrimaryExpression
 *     | PostFixExpression '(' ExpressionList ')'   (CallExpression)
 *     | PostFixExpression '[' Expression ']'       (IndexExpression) // TODO: implement this
 *     | LiteralExpression '::' Typename
 *     ;
 */
Expr* Parser::postfix_expression() {
    std::unique_ptr<Expr> primexpr(primary_expression());

    // LiteralExpression '::' Typename
    // literal type specifier
    if (isinstance<LiteralExpr>(primexpr.get())) {
        if (curr_token.is_one_of(TokenType::ColonColon)) {
            advance();

            TypeInfo cast_to = type();
            // TODO: check if cast_to is valid type for literal
            primexpr->type = std::move(cast_to);
        }
    }

    // CallExpression or IndexExpression
    while (true) {
        if (curr_token.is_one_of(TokenType::LeftParen)) {
            std::unique_ptr<CallExpr> callexpr = std::make_unique<CallExpr>();
            callexpr->args = expression_list(TokenType::LeftParen, TokenType::RightParen);

            callexpr->mainexpr = primeexpr.release(); // noexcept
            primexpr.reset(callexpr.release()); // noexcept
        } else {
            break;
        }
        // TODO: index expression
    }
    /* cannot have mutable explicit this
    [&primeexpr, parser = this](this auto&& self) mutable -> void {
        if (curr_token.is_one_of(TokenType::LeftParen)) {
            std::unique_ptr<CallExpr> callexpr = std::make_unique<CallExpr>();
            callexpr->args = parser->expression_list(TokenType::LeftParen, TokenType::RightParen);

            callexpr->mainexpr = primeexpr.release(); // noexcept
            primexpr.reset(callexpr.release()); // noexcept

            self();
        }
        // TODO: index expression
    } ();
    */

    return primeexpr.release();
}

/*
 * UnaryExpression
 *     : PostfixExpression
 *     | UnaryOperator CastExpression
 *     ;
 */
Expr* Parser::unary_expression() {
    if (is_unary_operator(curr_token)) {
        std::unique_ptr<UnaryExpr> unaryexpr = std::make_unique<UnaryExpr>();

        unaryexpr->op = to_unary_operator(curr_token);
        advance();

        unaryexpr->mainexpr = unary_expression();

        return unaryexpr.release();
    }
    else {
        return postfix_expression();
    }
}

/*
 * CastExpression
 *     : UnaryExpression
 *     | CastExpression '::' Typename
 *     ;
 */
Expr* Parser::cast_expression() {
    std::unique_ptr<Expr> unaryexpr(unary_expression());

    while (curr_token.is_one_of(TokenType::ColonColon)) {
        advance();

        std::unique_ptr<CastExpr> castexpr = std::make_unique<CastExpr>();
        castexpr->expr = unaryexpr.release(); // noexcept
        castexpr->type = type();

        unaryexpr.reset(castexpr.release()); // noexcept
    }

    return unaryexpr.release();
}



void Parser::advance_expected(TokenType type) {
    if (curr_token.is_one_of(type)) {
        if (!advance_noexc()) {
            handel_lex_stop();
        }
    }

    // TODO: diag expects token type
    throw ParseStop(ParseStop::ParserError);
}

bool Parser::advance_noexc() noexcept {
    curr_token = std::move(next_token);
    return lexer.lex(next_token);
}

void Parser::advance() {
    if (!advance_noexc()) {
        handel_lex_stop();
    }
}

void Parser::handel_lex_stop() {
    if (lexer.is_eof()) {
        throw ParseStop(ParseStop::EndOfFile);
    }

    // TODO: diag error type
    throw ParseStop(ParseStop::LexerError);
}
