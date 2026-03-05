#pragma once
#include "ASTNode.h"
#include <string>

// ── Represents a data member (field) inside a class ────────────────────────
// Tracked by GodClassDetector to count fields per class.
class FieldNode : public ASTNode {
public:
    std::string name;
    std::string typeName;
    std::string accessModifier;   // "public" | "private" | "protected"

    FieldNode(std::string n, std::string t, std::string access, SourceLocation loc)
        : ASTNode(NodeKind::Field, loc),
          name(std::move(n)), typeName(std::move(t)),
          accessModifier(std::move(access)) {}

    void accept(ASTVisitor& v) override;
};