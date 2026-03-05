#pragma once
#include "ASTNode.h"
#include "ClassNode.h"
#include "FunctionNode.h"
#include <string>
#include <vector>
#include <memory>

// ── Root node of the AST — represents one parsed source file ───────────────
// DetectionEngine receives this node and passes it to every detector.
// It owns all top-level classes and free functions found in the file.
class TranslationUnitNode : public ASTNode {
public:
    std::string                                filePath;
    std::vector<std::unique_ptr<ClassNode>>    classes;
    std::vector<std::unique_ptr<FunctionNode>> functions;   // free functions

    explicit TranslationUnitNode(std::string fp, SourceLocation loc)
        : ASTNode(NodeKind::TranslationUnit, loc), filePath(std::move(fp)) {}

    void accept(ASTVisitor& v) override;
};