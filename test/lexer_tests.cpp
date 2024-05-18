#include "../lexer.hpp"
#include "../filebuffer.hpp"
#include "../token.hpp"
#include "../keywordtrie.hpp"

#include <gtest/gtest.h>
#include <iostream>

static constexpr std::string_view token_literals[] = {
#define PUNCTUATOR(X, Y) Y,
#define KEYWORD(X, Y) Y,
#include "testtokentype.inc"
};

static constexpr TokenType token_types[] = {
#define TOK(X) TokenType::X,
#include "testtokentype.inc"
};

void TokenLexTest(const std::string& input, TokenType expectedType, const std::string& expectedValue) {
    std::istringstream iss(input);
    SourceBuffer buffer(iss);
    Lexer lexer(buffer);
    Token token;
    ASSERT_TRUE(lexer.lex(token)) << "Failed to lex input: " << input;
    EXPECT_EQ(token.get_type(), expectedType) << "Mismatched token type for input: " << input;
    EXPECT_EQ(token.get_view(), expectedValue) << "Mismatched token value for input: " << input;
    EXPECT_TRUE(lexer.lex(token)) << "Missing EOF token";
    EXPECT_EQ(token.get_type(), TokenType::EndOfFile);
}

class LexerTest : public ::testing::Test {
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(LexerTest, TokenTests) {
    for (size_t i = 0; i < std::size(token_literals); ++i) {
        SCOPED_TRACE("Testing token: " + std::string(token_literals[i]));
        TokenLexTest(std::string(token_literals[i]), token_types[i], std::string(token_literals[i]));
    }
}

TEST_F(LexerTest, geqTest) {
    std::istringstream iss2(">>= ");
    SourceBuffer buffer2(iss2);
    Lexer lexer2(buffer2);
    Token token2;
    lexer2.lex(token2);
    EXPECT_EQ(token2.get_type(), TokenType::GreaterGreaterEqual);
    EXPECT_EQ(token2.get_view(), ">>=");
    std::cout << "the actual view is " << token2.get_view() << " for >>= symbol" << std::endl;
}

TEST_F(LexerTest, HandlesNumericLiterals) {
    SourceBuffer buffer("./numbers.dl");
    Lexer lexer(buffer);
    Token token;
    EXPECT_TRUE(lexer.lex(token));
    EXPECT_EQ(token.get_type(), TokenType::DecIntLiteral);
    EXPECT_EQ(token.get_view(), "12323");
    EXPECT_TRUE(lexer.lex(token));
    EXPECT_EQ(token.get_type(), TokenType::HexIntLiteral);
    EXPECT_EQ(token.get_view(), "0x7FFFFFFF");
    EXPECT_TRUE(lexer.lex(token));
    EXPECT_EQ(token.get_type(), TokenType::FloatLiteral);
    EXPECT_EQ(token.get_view(), "12345.");
    EXPECT_TRUE(lexer.lex(token));
    EXPECT_EQ(token.get_type(), TokenType::DecIntLiteral);
    EXPECT_EQ(token.get_view(), "12345");
    EXPECT_TRUE(lexer.lex(token));
    EXPECT_EQ(token.get_type(), TokenType::FloatLiteral);
    EXPECT_EQ(token.get_view(), "123.123");
    EXPECT_TRUE(lexer.lex(token));
    EXPECT_EQ(token.get_type(), TokenType::DecIntLiteral);
    EXPECT_EQ(token.get_view(), "12314");
    EXPECT_TRUE(lexer.lex(token));
    EXPECT_EQ(token.get_type(), TokenType::EndOfFile);
    EXPECT_FALSE(lexer.lex(token));
    EXPECT_EQ(token.get_type(), TokenType::ERROR);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
