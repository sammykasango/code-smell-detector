#pragma once
#include <string>

enum class TokenType {
    // Literals
    INTEGER_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,
    IDENTIFIER,

    // Keywords
    KW_CLASS,
    KW_STRUCT,
    KW_VOID,
    KW_INT,
    KW_FLOAT,
    KW_DOUBLE,
    KW_CHAR,
    KW_BOOL,
    KW_RETURN,
    KW_IF,
    KW_ELSE,
    KW_FOR,
    KW_WHILE,
    KW_DO,
    KW_SWITCH,
    KW_CASE,
    KW_BREAK,
    KW_CONTINUE,
    KW_PUBLIC,
    KW_PRIVATE,
    KW_PROTECTED,
    KW_STATIC,
    KW_CONST,
    KW_VIRTUAL,
    KW_OVERRIDE,
    KW_NAMESPACE,
    KW_TEMPLATE,
    KW_DEFINE,
    KW_INCLUDE,

    // Symbols
    LBRACE,       // {
    RBRACE,       // }
    LPAREN,       // (
    RPAREN,       // )
    LBRACKET,     // [
    RBRACKET,     // ]
    SEMICOLON,    // ;
    COLON,        // :
    COMMA,        // ,
    DOT,          // .
    ARROW,        // ->
    SCOPE,        // ::

    // Operators
    ASSIGN,       // =
    PLUS, MINUS, STAR, SLASH, PERCENT,
    EQ, NEQ, LT, GT, LEQ, GEQ,
    AND, OR, NOT,
    BITAND, BITOR, BITXOR, BITNOT,
    LSHIFT, RSHIFT,
    PLUSPLUS, MINUSMINUS,
    PLUSEQ, MINUSEQ, STAREQ, SLASHEQ,

    // Preprocessor
    HASH,
    NEWLINE,

    // Special
    END_OF_FILE,
    UNKNOWN
};

struct Token {
    TokenType   type;
    std::string value;
    int         line;
    int         col;

    Token(TokenType t, std::string v, int ln, int c)
        : type(t), value(std::move(v)), line(ln), col(c) {}

    bool is(TokenType t) const { return type == t; }
    bool isKeyword()     const { return type >= TokenType::KW_CLASS && type <= TokenType::KW_INCLUDE; }
    bool isLiteral()     const {
        return type == TokenType::INTEGER_LITERAL ||
               type == TokenType::FLOAT_LITERAL   ||
               type == TokenType::STRING_LITERAL;
    }

    std::string toString() const {
        return "[" + std::to_string(line) + ":" + std::to_string(col) + " " + value + "]";
    }
};