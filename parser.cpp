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
 * FuncDefStmt
 *     : 'fn' Identifier ParenList ('->' Identifier)? CompoundStmt
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
 * FuncDefStmt
 *     : 'let' Identifier (':' Identifier)? CompoundStmt
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
    
    defstmt->vartype = advance_identifier();
}

/*
 * ParameterList
 *     : '(' (Parameter (',' Parameter)*)? ')'
 *     ;
 * 
 * Parameter
 *     : Identifier ':' Identifier
 *     ;
 */
ParenmeterList Parser::parameter_list() {
    ParameterList list;

    advance_expected(TokenType::LeftParen);

    // TODO: parse parameters

    return list;
}

/*
 * Stmt
 *     : CompoundStmt
 *     | LetDefStmt
 *     | FuncDefStmt
 *     | ExprStmt
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
    default:
        return expression_statement();
    }
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
    CompoundStmt* compstmt = new CompoundStmt;

    // statement list
    while (!curr_token.is_one_of(TokenType::RightBrace)) {
        compstmt->stmts.push_back(statement());
    }

    return compstmt;
}

/*
 * ExprStmt
 *     : Expr ';'
 *     ;
 */
ExprStmt* Parser::expression_statement() {
    ExprStmt* exprstmt = new ExprStmt;

    exprstmt->expr = expression();
    advance_expected(TokenType::Semicolon);

    return exprstmt;
}


/*
 * Expr
 *     : LiteralExpr
 *     ;
 */
Expr* Parser::expression() {
    if (is_literal_token(curr_token)) {
        return literal_expression();
    }
}

void Parser::advance_expected(TokenType type) {
    if (curr_token.is_one_of(type)) {
        if (!advance()) {
            handel_lex_stop();
        }
    }

    // TODO: diag expects token type
    throw ParseStop(ParseStop::ParserError);
}

bool Parser::advance() noexcept {
    curr_token = std::move(next_token);
    return lexer.lex(next_token);
}

void Parser::handel_lex_stop() {
    if (lexer.is_eof()) {
        throw ParseStop(ParseStop::EndOfFile);
    }

    // TODO: diag error type
    throw ParseStop(ParseStop::LexerError);
}
