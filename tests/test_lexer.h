// tests/test_lexer.cpp
// Unit tests for the Lexer module.
// Run: g++ -std=c++17 -Isrc tests/test_lexer.cpp src/lexer/Lexer.cpp -o test_lexer && ./test_lexer

#include <iostream>
#include <string>
#include <cassert>
#include "../src/lexer/Lexer.h"

static int passed = 0, failed = 0;

#define CHECK(cond, label) \
    if (cond) { ++passed; std::cout << "  [PASS] " << label << "\n"; } \
    else       { ++failed; std::cout << "  [FAIL] " << label << "\n"; }

// ── Helpers ────────────────────────────────────────────────────────────────
static std::vector<Token> lex(const std::string& src) {
    Lexer l(src);
    return l.tokenize();
}

static Token firstOf(const std::vector<Token>& tokens, TokenType type) {
    for (auto& t : tokens) if (t.type == type) return t;
    return Token(TokenType::UNKNOWN, "", 0, 0);
}

// ── Test cases ─────────────────────────────────────────────────────────────
void test_keywords() {
    auto tokens = lex("class void int return if for while");
    CHECK(firstOf(tokens, TokenType::KW_CLASS).value  == "class",  "keyword: class");
    CHECK(firstOf(tokens, TokenType::KW_VOID).value   == "void",   "keyword: void");
    CHECK(firstOf(tokens, TokenType::KW_INT).value    == "int",    "keyword: int");
    CHECK(firstOf(tokens, TokenType::KW_RETURN).value == "return", "keyword: return");
    CHECK(firstOf(tokens, TokenType::KW_IF).value     == "if",     "keyword: if");
    CHECK(firstOf(tokens, TokenType::KW_FOR).value    == "for",    "keyword: for");
    CHECK(firstOf(tokens, TokenType::KW_WHILE).value  == "while",  "keyword: while");
}

void test_identifiers() {
    auto tokens = lex("myVar _count CamelCase");
    int idCount = 0;
    for (auto& t : tokens) if (t.type == TokenType::IDENTIFIER) ++idCount;
    CHECK(idCount == 3, "three identifiers parsed");
    CHECK(tokens[0].value == "myVar",    "identifier: myVar");
    CHECK(tokens[1].value == "_count",   "identifier: _count");
    CHECK(tokens[2].value == "CamelCase","identifier: CamelCase");
}

void test_integer_literals() {
    auto tokens = lex("42 100 0 999");
    int litCount = 0;
    for (auto& t : tokens) if (t.type == TokenType::INTEGER_LITERAL) ++litCount;
    CHECK(litCount == 4, "four integer literals");
    CHECK(tokens[0].value == "42",  "literal: 42");
    CHECK(tokens[3].value == "999", "literal: 999");
}

void test_float_literals() {
    auto tokens = lex("3.14 0.5 99.0f");
    CHECK(tokens[0].type == TokenType::FLOAT_LITERAL, "3.14 is float literal");
    CHECK(tokens[0].value == "3.14", "3.14 value");
    CHECK(tokens[1].type == TokenType::FLOAT_LITERAL, "0.5 is float literal");
}

void test_operators() {
    auto tokens = lex("== != <= >= && || ++ -- -> ::");
    CHECK(firstOf(tokens, TokenType::EQ).value     == "==", "operator ==");
    CHECK(firstOf(tokens, TokenType::NEQ).value    == "!=", "operator !=");
    CHECK(firstOf(tokens, TokenType::LEQ).value    == "<=", "operator <=");
    CHECK(firstOf(tokens, TokenType::AND).value    == "&&", "operator &&");
    CHECK(firstOf(tokens, TokenType::OR).value     == "||", "operator ||");
    CHECK(firstOf(tokens, TokenType::ARROW).value  == "->", "operator ->");
    CHECK(firstOf(tokens, TokenType::SCOPE).value  == "::", "operator ::");
}

void test_symbols() {
    auto tokens = lex("{ } ( ) [ ] ; , .");
    CHECK(firstOf(tokens, TokenType::LBRACE).value    == "{", "symbol {");
    CHECK(firstOf(tokens, TokenType::RBRACE).value    == "}", "symbol }");
    CHECK(firstOf(tokens, TokenType::LPAREN).value    == "(", "symbol (");
    CHECK(firstOf(tokens, TokenType::RPAREN).value    == ")", "symbol )");
    CHECK(firstOf(tokens, TokenType::SEMICOLON).value == ";", "symbol ;");
}

void test_line_comment_skipped() {
    auto tokens = lex("int x; // this is a comment\nint y;");
    int idCount = 0;
    for (auto& t : tokens) if (t.type == TokenType::IDENTIFIER) ++idCount;
    CHECK(idCount == 2, "line comment skipped: only x and y as identifiers");
}

void test_block_comment_skipped() {
    auto tokens = lex("int /* block */ x;");
    int idCount = 0;
    for (auto& t : tokens) if (t.type == TokenType::IDENTIFIER) ++idCount;
    CHECK(idCount == 1, "block comment skipped: only x");
}

void test_string_literal() {
    auto tokens = lex("\"hello world\"");
    CHECK(tokens[0].type  == TokenType::STRING_LITERAL, "string literal type");
    CHECK(tokens[0].value == "hello world", "string literal value (no quotes)");
}

void test_line_numbers() {
    auto tokens = lex("int x;\nint y;\nint z;");
    // Find each identifier
    std::vector<Token> ids;
    for (auto& t : tokens) if (t.type == TokenType::IDENTIFIER) ids.push_back(t);
    CHECK(ids.size() == 3,     "three identifiers");
    CHECK(ids[0].line == 1,    "x on line 1");
    CHECK(ids[1].line == 2,    "y on line 2");
    CHECK(ids[2].line == 3,    "z on line 3");
}

void test_eof_token() {
    auto tokens = lex("");
    CHECK(!tokens.empty(), "empty source gives at least one token");
    CHECK(tokens.back().type == TokenType::END_OF_FILE, "last token is EOF");
}

// ── Main ───────────────────────────────────────────────────────────────────
int main() {
    std::cout << "\n=== Lexer Tests ===\n\n";
    test_keywords();
    test_identifiers();
    test_integer_literals();
    test_float_literals();
    test_operators();
    test_symbols();
    test_line_comment_skipped();
    test_block_comment_skipped();
    test_string_literal();
    test_line_numbers();
    test_eof_token();

    std::cout << "\n--- Results: " << passed << " passed, " << failed << " failed ---\n\n";
    return failed > 0 ? 1 : 0;
}