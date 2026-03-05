#include "Lexer.h"
#include <cctype>
#include <stdexcept>

// ── Keyword map ────────────────────────────────────────────────────────────
const std::unordered_map<std::string, TokenType> Lexer::keywords_ = {
    {"class",     TokenType::KW_CLASS},
    {"struct",    TokenType::KW_STRUCT},
    {"void",      TokenType::KW_VOID},
    {"int",       TokenType::KW_INT},
    {"float",     TokenType::KW_FLOAT},
    {"double",    TokenType::KW_DOUBLE},
    {"char",      TokenType::KW_CHAR},
    {"bool",      TokenType::KW_BOOL},
    {"return",    TokenType::KW_RETURN},
    {"if",        TokenType::KW_IF},
    {"else",      TokenType::KW_ELSE},
    {"for",       TokenType::KW_FOR},
    {"while",     TokenType::KW_WHILE},
    {"do",        TokenType::KW_DO},
    {"switch",    TokenType::KW_SWITCH},
    {"case",      TokenType::KW_CASE},
    {"break",     TokenType::KW_BREAK},
    {"continue",  TokenType::KW_CONTINUE},
    {"public",    TokenType::KW_PUBLIC},
    {"private",   TokenType::KW_PRIVATE},
    {"protected", TokenType::KW_PROTECTED},
    {"static",    TokenType::KW_STATIC},
    {"const",     TokenType::KW_CONST},
    {"virtual",   TokenType::KW_VIRTUAL},
    {"override",  TokenType::KW_OVERRIDE},
    {"namespace", TokenType::KW_NAMESPACE},
    {"template",  TokenType::KW_TEMPLATE},
    {"define",    TokenType::KW_DEFINE},
    {"include",   TokenType::KW_INCLUDE},
};

// ── Constructor ────────────────────────────────────────────────────────────
Lexer::Lexer(const std::string& source)
    : source_(source), pos_(0), line_(1), col_(1) {}

// ── Helpers ────────────────────────────────────────────────────────────────
char Lexer::current() const {
    return isAtEnd() ? '\0' : source_[pos_];
}
char Lexer::peek(int offset) const {
    size_t idx = pos_ + offset;
    return idx < source_.size() ? source_[idx] : '\0';
}
char Lexer::advance() {
    char c = source_[pos_++];
    if (c == '\n') { ++line_; col_ = 1; }
    else            { ++col_; }
    return c;
}
bool Lexer::isAtEnd() const { return pos_ >= source_.size(); }

Token Lexer::makeToken(TokenType type, const std::string& value) const {
    return Token(type, value, line_, col_);
}

// ── Skip helpers ───────────────────────────────────────────────────────────
void Lexer::skipWhitespace() {
    while (!isAtEnd() && std::isspace(current()) && current() != '\n')
        advance();
}
void Lexer::skipLineComment() {
    while (!isAtEnd() && current() != '\n') advance();
}
void Lexer::skipBlockComment() {
    advance(); advance(); // consume /*
    while (!isAtEnd()) {
        if (current() == '*' && peek() == '/') {
            advance(); advance(); return;
        }
        advance();
    }
}

// ── Readers ────────────────────────────────────────────────────────────────
Token Lexer::readIdentifierOrKeyword() {
    int startLine = line_, startCol = col_;
    std::string val;
    while (!isAtEnd() && (std::isalnum(current()) || current() == '_'))
        val += advance();

    auto it = keywords_.find(val);
    TokenType type = (it != keywords_.end()) ? it->second : TokenType::IDENTIFIER;
    return Token(type, val, startLine, startCol);
}

Token Lexer::readNumber() {
    int startLine = line_, startCol = col_;
    std::string val;
    bool isFloat = false;
    while (!isAtEnd() && (std::isdigit(current()) || current() == '.')) {
        if (current() == '.') isFloat = true;
        val += advance();
    }
    // optional suffix: f, u, l, etc.
    while (!isAtEnd() && (current() == 'f' || current() == 'u' || current() == 'l' ||
                          current() == 'F' || current() == 'U' || current() == 'L'))
        val += advance();

    return Token(isFloat ? TokenType::FLOAT_LITERAL : TokenType::INTEGER_LITERAL,
                 val, startLine, startCol);
}

Token Lexer::readString() {
    int startLine = line_, startCol = col_;
    advance(); // opening "
    std::string val;
    while (!isAtEnd() && current() != '"') {
        if (current() == '\\') { advance(); }
        val += advance();
    }
    if (!isAtEnd()) advance(); // closing "
    return Token(TokenType::STRING_LITERAL, val, startLine, startCol);
}

Token Lexer::readPreprocessor() {
    int startLine = line_, startCol = col_;
    advance(); // consume #
    skipWhitespace();
    std::string directive;
    while (!isAtEnd() && std::isalpha(current()))
        directive += advance();
    // consume rest of line
    while (!isAtEnd() && current() != '\n') advance();
    return Token(TokenType::HASH, "#" + directive, startLine, startCol);
}

Token Lexer::readOperatorOrSymbol() {
    int startLine = line_, startCol = col_;
    char c = advance();
    char n = current();

    switch (c) {
        case '{': return Token(TokenType::LBRACE,    "{",  startLine, startCol);
        case '}': return Token(TokenType::RBRACE,    "}",  startLine, startCol);
        case '(': return Token(TokenType::LPAREN,    "(",  startLine, startCol);
        case ')': return Token(TokenType::RPAREN,    ")",  startLine, startCol);
        case '[': return Token(TokenType::LBRACKET,  "[",  startLine, startCol);
        case ']': return Token(TokenType::RBRACKET,  "]",  startLine, startCol);
        case ';': return Token(TokenType::SEMICOLON, ";",  startLine, startCol);
        case ',': return Token(TokenType::COMMA,     ",",  startLine, startCol);
        case '.': return Token(TokenType::DOT,       ".",  startLine, startCol);
        case '\n':return Token(TokenType::NEWLINE,   "\n", startLine, startCol);

        case ':':
            if (n == ':') { advance(); return Token(TokenType::SCOPE,  "::", startLine, startCol); }
            return Token(TokenType::COLON, ":", startLine, startCol);

        case '-':
            if (n == '>') { advance(); return Token(TokenType::ARROW,    "->",  startLine, startCol); }
            if (n == '-') { advance(); return Token(TokenType::MINUSMINUS,"--", startLine, startCol); }
            if (n == '=') { advance(); return Token(TokenType::MINUSEQ,  "-=",  startLine, startCol); }
            return Token(TokenType::MINUS, "-", startLine, startCol);

        case '+':
            if (n == '+') { advance(); return Token(TokenType::PLUSPLUS, "++",  startLine, startCol); }
            if (n == '=') { advance(); return Token(TokenType::PLUSEQ,   "+=",  startLine, startCol); }
            return Token(TokenType::PLUS, "+", startLine, startCol);

        case '*':
            if (n == '=') { advance(); return Token(TokenType::STAREQ,  "*=", startLine, startCol); }
            return Token(TokenType::STAR, "*", startLine, startCol);

        case '/':
            if (n == '=') { advance(); return Token(TokenType::SLASHEQ, "/=", startLine, startCol); }
            return Token(TokenType::SLASH, "/", startLine, startCol);

        case '=':
            if (n == '=') { advance(); return Token(TokenType::EQ,     "==", startLine, startCol); }
            return Token(TokenType::ASSIGN, "=", startLine, startCol);

        case '!':
            if (n == '=') { advance(); return Token(TokenType::NEQ,  "!=", startLine, startCol); }
            return Token(TokenType::NOT, "!", startLine, startCol);

        case '<':
            if (n == '=') { advance(); return Token(TokenType::LEQ,    "<=", startLine, startCol); }
            if (n == '<') { advance(); return Token(TokenType::LSHIFT, "<<", startLine, startCol); }
            return Token(TokenType::LT, "<", startLine, startCol);

        case '>':
            if (n == '=') { advance(); return Token(TokenType::GEQ,    ">=", startLine, startCol); }
            if (n == '>') { advance(); return Token(TokenType::RSHIFT, ">>", startLine, startCol); }
            return Token(TokenType::GT, ">", startLine, startCol);

        case '&':
            if (n == '&') { advance(); return Token(TokenType::AND,    "&&", startLine, startCol); }
            return Token(TokenType::BITAND, "&", startLine, startCol);

        case '|':
            if (n == '|') { advance(); return Token(TokenType::OR,    "||", startLine, startCol); }
            return Token(TokenType::BITOR, "|", startLine, startCol);

        case '^': return Token(TokenType::BITXOR,  "^", startLine, startCol);
        case '~': return Token(TokenType::BITNOT,  "~", startLine, startCol);
        case '%': return Token(TokenType::PERCENT, "%", startLine, startCol);

        default:
            return Token(TokenType::UNKNOWN, std::string(1, c), startLine, startCol);
    }
}

// ── Main tokenize loop ─────────────────────────────────────────────────────
std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (!isAtEnd()) {
        // Skip spaces/tabs (not newlines — they matter for line count)
        while (!isAtEnd() && (current() == ' ' || current() == '\t'))
            advance();

        if (isAtEnd()) break;

        char c = current();

        // Comments
        if (c == '/' && peek() == '/') { skipLineComment(); continue; }
        if (c == '/' && peek() == '*') { skipBlockComment(); continue; }

        // Preprocessor
        if (c == '#') { tokens.push_back(readPreprocessor()); continue; }

        // Identifiers / keywords
        if (std::isalpha(c) || c == '_') {
            tokens.push_back(readIdentifierOrKeyword());
            continue;
        }

        // Numbers
        if (std::isdigit(c) || (c == '.' && std::isdigit(peek()))) {
            tokens.push_back(readNumber());
            continue;
        }

        // Strings
        if (c == '"') { tokens.push_back(readString()); continue; }

        // Character literals — skip value, just note the literal
        if (c == '\'') {
            advance();
            std::string val;
            while (!isAtEnd() && current() != '\'') {
                if (current() == '\\') advance();
                val += advance();
            }
            if (!isAtEnd()) advance();
            tokens.emplace_back(TokenType::INTEGER_LITERAL, val, line_, col_);
            continue;
        }

        // Operators and symbols
        tokens.push_back(readOperatorOrSymbol());
    }

    tokens.emplace_back(TokenType::END_OF_FILE, "", line_, col_);
    return tokens;
}