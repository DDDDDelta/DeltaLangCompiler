#ifndef TOK
#define TOK(X)
#endif

#ifndef PUNCTUATOR
#define PUNCTUATOR(X,Y) TOK(X)
#endif

#ifndef KEYWORD
#define KEYWORD(X,Y) TOK(X)
#endif

PUNCTUATOR(LeftParen, "(")
PUNCTUATOR(RightParen, ")")
PUNCTUATOR(LeftBrace, "{")
PUNCTUATOR(RightBrace, "}")
PUNCTUATOR(LeftSquare, "[")
PUNCTUATOR(RightSquare, "]")

PUNCTUATOR(Equal, "=")
PUNCTUATOR(Plus, "+")
PUNCTUATOR(Minus, "-")
PUNCTUATOR(Star, "*")
PUNCTUATOR(Slash, "/")
PUNCTUATOR(Percent, "%")
PUNCTUATOR(Tilde, "~")
PUNCTUATOR(Pipe, "|")
PUNCTUATOR(Amp, "&")
PUNCTUATOR(Caret, "^")
PUNCTUATOR(GreaterGreater, ">>")
PUNCTUATOR(LessLess, "<<")
PUNCTUATOR(PlusEqual, "+=")
PUNCTUATOR(MinusEqual, "-=")
PUNCTUATOR(StarEqual, "*=")
PUNCTUATOR(SlashEqual, "/=")
PUNCTUATOR(PercentEqual, "%=")
PUNCTUATOR(TildeEqual, "~=")
PUNCTUATOR(PipeEqual, "|=")
PUNCTUATOR(AmpEqual, "&=")
PUNCTUATOR(CaretEqual, "^=")
PUNCTUATOR(GreaterGreaterEqual, ">>=")
PUNCTUATOR(LessLessEqual, "<<=")

PUNCTUATOR(Exclaim, "!")
PUNCTUATOR(AmpAmp, "&&")
PUNCTUATOR(PipePipe, "||")

PUNCTUATOR(Greater, ">")
PUNCTUATOR(Less, "<")

PUNCTUATOR(EqualEqual, "==")
PUNCTUATOR(ExclaimEqual, "!=")
PUNCTUATOR(GreaterEqual, ">=")
PUNCTUATOR(LessEqual, "<=")

PUNCTUATOR(Dot, ".")
PUNCTUATOR(Comma, ",")
PUNCTUATOR(MinusGreater, "->")
PUNCTUATOR(Semicolon, ";")
PUNCTUATOR(Colon, ":")
PUNCTUATOR(ColonColon, "::")
PUNCTUATOR(EqualGreater, "=>")

KEYWORD(Void, "void")
KEYWORD(Type, "type")
KEYWORD(Fn, "fn")
KEYWORD(Let, "let")
KEYWORD(Const, "const")
KEYWORD(If, "if")
KEYWORD(Else, "else")
KEYWORD(Return, "return")
KEYWORD(Loop, "loop")
KEYWORD(Break, "break")
KEYWORD(Continue, "continue")

#undef TOK
#undef PUNCTUATOR
#undef KEYWORD