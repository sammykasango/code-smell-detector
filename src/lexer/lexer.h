#pragma once
#include "Token.h"
#include <string>
#include <vector>
#include <unordered_map>

class Lexer {
public:
    explicit Lexer(const std::string& source);

    std::vector<Token> tokenize();

private:
    std::string source_;
    size_t      pos_;
    int         line_;
    int         col_;

    static const std::unordered_map<std::string, TokenType> keywords_;

    char        current()  const;
    char        peek(int offset = 1) const;
    char        advance();
    bool        isAtEnd()  const;

    void        skipWhitespace();
    void        skipLineComment();
    void        skipBlockComment();

    Token       readIdentifierOrKeyword();
    Token       readNumber();
    Token       readString();
    Token       readOperatorOrSymbol();
    Token       readPreprocessor();

    Token       makeToken(TokenType type, const std::string& value) const;
};