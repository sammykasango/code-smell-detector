#pragma once
#include "../lexer/Token.h"
#include "ASTVisitor.h"
#include <vector>
#include <memory>
#include <string>

class ASTBuilder {
public:
    ASTBuilder(const std::vector<Token>& tokens, std::string filePath);
    std::unique_ptr<TranslationUnitNode> build();

private:
    const std::vector<Token>& tokens_;
    std::string               filePath_;
    size_t                    pos_;
    int                       nestingDepth_;
    std::string               currentAccess_;

    const Token& current()             const;
    const Token& peek(int offset = 1)  const;
    const Token& consume();
    const Token& expect(TokenType type, const std::string& context = "");
    bool         check(TokenType type) const;
    bool         match(TokenType type);
    bool         isAtEnd()             const;
    void         skipToSemicolon();
    void         skipToClosingBrace();
    SourceLocation loc()               const;

    std::string  parseTypeName();
    bool         isTypeToken()         const;
    bool         lookaheadIsFunction();

    std::unique_ptr<ClassNode>    parseClass();
    std::unique_ptr<FunctionNode> parseFunction(const std::string& returnType,
                                                const std::string& name,
                                                const std::string& ownerClass = "");
    std::vector<ParameterNode>    parseParameterList();
    std::unique_ptr<BlockNode>    parseBlock(int depth);
    FieldNode                     parseField(const std::string& typeName,
                                             const std::string& name,
                                             const std::string& access);
    void parseStatement(BlockNode& block, int depth);
};