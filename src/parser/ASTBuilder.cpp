#include "ASTBuilder.h"
#include <stdexcept>
#include <algorithm>
#include <set>

ASTBuilder::ASTBuilder(const std::vector<Token>& tokens, std::string filePath)
    : tokens_(tokens), filePath_(std::move(filePath)), pos_(0), nestingDepth_(0),
      currentAccess_("private") {}

const Token& ASTBuilder::current() const { return tokens_[pos_]; }
const Token& ASTBuilder::peek(int offset) const {
    size_t idx = pos_ + offset;
    return idx < tokens_.size() ? tokens_[idx] : tokens_.back();
}
const Token& ASTBuilder::consume() { return tokens_[pos_++]; }
bool ASTBuilder::check(TokenType t) const { return current().type == t; }
bool ASTBuilder::match(TokenType t) {
    if (check(t)) { consume(); return true; }
    return false;
}
bool ASTBuilder::isAtEnd() const { return current().type == TokenType::END_OF_FILE; }
SourceLocation ASTBuilder::loc() const {
    return { filePath_, current().line, current().col };
}
const Token& ASTBuilder::expect(TokenType type, const std::string& /*ctx*/) {
    if (check(type)) return consume();
    return consume();
}
void ASTBuilder::skipToSemicolon() {
    while (!isAtEnd() && !check(TokenType::SEMICOLON) && !check(TokenType::RBRACE))
        consume();
    if (check(TokenType::SEMICOLON)) consume();
}
void ASTBuilder::skipToClosingBrace() {
    int depth = 1;
    while (!isAtEnd() && depth > 0) {
        if (check(TokenType::LBRACE)) ++depth;
        else if (check(TokenType::RBRACE)) --depth;
        consume();
    }
}
static const std::set<TokenType> typeTokens = {
    TokenType::KW_VOID, TokenType::KW_INT, TokenType::KW_FLOAT,
    TokenType::KW_DOUBLE, TokenType::KW_CHAR, TokenType::KW_BOOL,
    TokenType::IDENTIFIER
};
bool ASTBuilder::isTypeToken() const { return typeTokens.count(current().type) > 0; }
std::string ASTBuilder::parseTypeName() {
    std::string type;
    if (check(TokenType::KW_CONST)) { type += consume().value + " "; }
    if (!isAtEnd() && isTypeToken()) type += consume().value;
    if (check(TokenType::LT)) {
        type += consume().value;
        int depth = 1;
        while (!isAtEnd() && depth > 0) {
            if (check(TokenType::LT))  ++depth;
            if (check(TokenType::GT))  --depth;
            type += consume().value;
        }
    }
    while (check(TokenType::STAR) || check(TokenType::BITAND))
        type += consume().value;
    return type;
}
bool ASTBuilder::lookaheadIsFunction() {
    size_t saved = pos_;
    while (!isAtEnd() && (current().type == TokenType::KW_STATIC  ||
                          current().type == TokenType::KW_VIRTUAL  ||
                          current().type == TokenType::KW_CONST    ||
                          current().type == TokenType::KW_OVERRIDE))
        pos_++;
    size_t typeStart = pos_;
    while (!isAtEnd() && isTypeToken()) pos_++;
    if (pos_ == typeStart) { pos_ = saved; return false; }
    while (check(TokenType::STAR) || check(TokenType::BITAND)) pos_++;
    if (check(TokenType::SCOPE)) { pos_++; if (!isAtEnd()) pos_++; }
    if (!check(TokenType::IDENTIFIER)) { pos_ = saved; return false; }
    pos_++;
    bool result = check(TokenType::LPAREN);
    pos_ = saved;
    return result;
}
std::vector<ParameterNode> ASTBuilder::parseParameterList() {
    std::vector<ParameterNode> params;
    expect(TokenType::LPAREN);
    while (!isAtEnd() && !check(TokenType::RPAREN)) {
        if (check(TokenType::COMMA)) { consume(); continue; }
        if (check(TokenType::DOT))   { skipToSemicolon(); break; }
        while (check(TokenType::KW_CONST) || check(TokenType::KW_STATIC)) consume();
        if (!isTypeToken()) { consume(); continue; }
        std::string typeName = parseTypeName();
        std::string name;
        if (check(TokenType::IDENTIFIER)) name = consume().value;
        if (check(TokenType::ASSIGN)) {
            consume();
            while (!isAtEnd() && !check(TokenType::COMMA) && !check(TokenType::RPAREN))
                consume();
        }
        params.emplace_back(name, typeName, loc());
    }
    if (check(TokenType::RPAREN)) consume();
    return params;
}
std::unique_ptr<BlockNode> ASTBuilder::parseBlock(int depth) {
    auto block = std::make_unique<BlockNode>(loc(), depth);
    expect(TokenType::LBRACE);
    int startLine = current().line;
    while (!isAtEnd() && !check(TokenType::RBRACE))
        parseStatement(*block, depth);
    int endLine = current().line;
    block->lineCount = endLine - startLine + 1;
    if (check(TokenType::RBRACE)) consume();
    return block;
}
void ASTBuilder::parseStatement(BlockNode& block, int depth) {
    while (check(TokenType::NEWLINE)) consume();
    if (check(TokenType::LBRACE)) {
        auto nested = parseBlock(depth + 1);
        block.statements.push_back(std::move(nested));
        return;
    }
    if (check(TokenType::KW_IF)    || check(TokenType::KW_FOR)   ||
        check(TokenType::KW_WHILE) || check(TokenType::KW_DO)    ||
        check(TokenType::KW_SWITCH)) {
        consume();
        if (check(TokenType::LPAREN)) {
            int pDepth = 1; consume();
            while (!isAtEnd() && pDepth > 0) {
                if (check(TokenType::LPAREN)) ++pDepth;
                if (check(TokenType::RPAREN)) --pDepth;
                consume();
            }
        }
        if (check(TokenType::LBRACE)) {
            auto nested = parseBlock(depth + 1);
            block.statements.push_back(std::move(nested));
        } else {
            parseStatement(block, depth + 1);
        }
        if (check(TokenType::KW_ELSE)) {
            consume();
            if (check(TokenType::LBRACE)) {
                auto nested = parseBlock(depth + 1);
                block.statements.push_back(std::move(nested));
            } else {
                parseStatement(block, depth + 1);
            }
        }
        return;
    }
    if (check(TokenType::INTEGER_LITERAL) || check(TokenType::FLOAT_LITERAL)) {
        auto expr = std::make_unique<ExpressionNode>(
            ExpressionNode::ExprKind::Literal, current().value, loc());
        expr->isNumericLiteral = true;
        block.statements.push_back(std::move(expr));
        consume();
        return;
    }
    while (!isAtEnd() && !check(TokenType::SEMICOLON) &&
           !check(TokenType::RBRACE) && !check(TokenType::LBRACE)) {
        if (check(TokenType::INTEGER_LITERAL) || check(TokenType::FLOAT_LITERAL)) {
            auto expr = std::make_unique<ExpressionNode>(
                ExpressionNode::ExprKind::Literal, current().value, loc());
            expr->isNumericLiteral = true;
            block.statements.push_back(std::move(expr));
        }
        consume();
    }
    if (check(TokenType::SEMICOLON)) consume();
}
std::unique_ptr<FunctionNode> ASTBuilder::parseFunction(
        const std::string& returnType, const std::string& name,
        const std::string& ownerClass) {
    auto fn = std::make_unique<FunctionNode>(name, returnType, loc());
    fn->ownerClass = ownerClass;
    fn->parameters = parseParameterList();
    while (check(TokenType::KW_CONST) || check(TokenType::KW_OVERRIDE) ||
           check(TokenType::ASSIGN))
        consume();
    if (check(TokenType::INTEGER_LITERAL) && current().value == "0") consume();
    if (check(TokenType::SEMICOLON))      { consume(); }
    else if (check(TokenType::LBRACE))    { fn->body = parseBlock(1); }
    else                                   { skipToSemicolon(); }
    return fn;
}
std::unique_ptr<ClassNode> ASTBuilder::parseClass() {
    consume();
    if (!check(TokenType::IDENTIFIER)) { skipToClosingBrace(); return nullptr; }
    std::string name = consume().value;
    auto cls = std::make_unique<ClassNode>(name, loc());
    if (check(TokenType::COLON)) {
        consume();
        while (!isAtEnd() && !check(TokenType::LBRACE)) {
            if (check(TokenType::IDENTIFIER)) cls->baseClasses.push_back(consume().value);
            else consume();
        }
    }
    if (!check(TokenType::LBRACE)) { skipToSemicolon(); return cls; }
    consume();
    currentAccess_ = "private";
    while (!isAtEnd() && !check(TokenType::RBRACE)) {
        while (check(TokenType::NEWLINE)) consume();
        if (check(TokenType::RBRACE)) break;
        if (check(TokenType::KW_PUBLIC))    { currentAccess_ = "public";    consume(); match(TokenType::COLON); continue; }
        if (check(TokenType::KW_PRIVATE))   { currentAccess_ = "private";   consume(); match(TokenType::COLON); continue; }
        if (check(TokenType::KW_PROTECTED)) { currentAccess_ = "protected"; consume(); match(TokenType::COLON); continue; }
        bool isVirt = false, isStat = false;
        while (check(TokenType::KW_VIRTUAL) || check(TokenType::KW_STATIC) ||
               check(TokenType::KW_CONST)   || check(TokenType::KW_OVERRIDE) ||
               check(TokenType::HASH)) {
            if (check(TokenType::KW_VIRTUAL)) isVirt = true;
            if (check(TokenType::KW_STATIC))  isStat = true;
            consume();
        }
        if (!isTypeToken() && !check(TokenType::SCOPE)) { consume(); continue; }
        std::string typeName = parseTypeName();
        if (check(TokenType::SCOPE)) { consume(); }
        if (!check(TokenType::IDENTIFIER)) { skipToSemicolon(); continue; }
        std::string memberName = consume().value;
        if (check(TokenType::LPAREN)) {
            auto fn = parseFunction(typeName, memberName, name);
            fn->isVirtual = isVirt;
            fn->isStatic  = isStat;
            cls->methods.push_back(std::move(fn));
        } else {
            while (!isAtEnd() && !check(TokenType::SEMICOLON) && !check(TokenType::RBRACE))
                consume();
            if (check(TokenType::SEMICOLON)) consume();
            cls->fields.emplace_back(memberName, typeName, currentAccess_, loc());
        }
    }
    if (check(TokenType::RBRACE)) consume();
    if (check(TokenType::SEMICOLON)) consume();
    return cls;
}
std::unique_ptr<TranslationUnitNode> ASTBuilder::build() {
    SourceLocation rootLoc = { filePath_, 1, 1 };
    auto unit = std::make_unique<TranslationUnitNode>(filePath_, rootLoc);
    while (!isAtEnd()) {
        while (check(TokenType::NEWLINE) || check(TokenType::SEMICOLON)) consume();
        if (isAtEnd()) break;
        if (check(TokenType::HASH)) { consume(); continue; }
        if (check(TokenType::KW_NAMESPACE)) {
            consume();
            if (check(TokenType::IDENTIFIER)) consume();
            if (check(TokenType::LBRACE))     consume();
            continue;
        }
        if (check(TokenType::KW_CLASS) || check(TokenType::KW_STRUCT)) {
            auto cls = parseClass();
            if (cls) unit->classes.push_back(std::move(cls));
            continue;
        }
        while (check(TokenType::KW_STATIC)  || check(TokenType::KW_VIRTUAL)  ||
               check(TokenType::KW_CONST)   || check(TokenType::KW_OVERRIDE) ||
               check(TokenType::KW_TEMPLATE)) {
            consume();
            if (check(TokenType::LT)) {
                consume(); int d = 1;
                while (!isAtEnd() && d > 0) {
                    if (check(TokenType::LT)) ++d;
                    if (check(TokenType::GT)) --d;
                    consume();
                }
            }
        }
        if (isTypeToken()) {
            std::string retType = parseTypeName();
            if (check(TokenType::SCOPE)) consume();
            if (check(TokenType::IDENTIFIER)) {
                std::string name = consume().value;
                if (check(TokenType::LPAREN)) {
                    auto fn = parseFunction(retType, name);
                    unit->functions.push_back(std::move(fn));
                    continue;
                }
            }
            skipToSemicolon();
            continue;
        }
        consume();
    }
    return unit;
}