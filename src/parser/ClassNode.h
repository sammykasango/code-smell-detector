#pragma once
#include "ASTNode.h"
#include "FunctionNode.h"
#include "FieldNode.h"
#include <string>
#include <vector>
#include <memory>

// ── Represents a class or struct definition ────────────────────────────────
// Owns all method and field nodes that belong to it.
// GodClassDetector reads methodCount() and fieldCount() directly.
class ClassNode : public ASTNode {
public:
    std::string                              name;
    std::vector<std::string>                 baseClasses;
    std::vector<std::unique_ptr<FunctionNode>> methods;
    std::vector<FieldNode>                   fields;
    bool                                     isReferenced = false;

    int methodCount() const { return static_cast<int>(methods.size()); }
    int fieldCount()  const { return static_cast<int>(fields.size());  }

    explicit ClassNode(std::string n, SourceLocation loc)
        : ASTNode(NodeKind::Class, loc), name(std::move(n)) {}

    void accept(ASTVisitor& v) override;
};