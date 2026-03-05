#pragma once
#include "SourceLocation.h"

// Forward declaration — defined in ASTVisitor.h
class ASTVisitor;

// ── Abstract base for every node in the tree ───────────────────────────────
class ASTNode {
public:
    explicit ASTNode(NodeKind kind, SourceLocation loc)
        : kind_(kind), loc_(loc) {}

    virtual ~ASTNode() = default;

    NodeKind              kind()     const { return kind_; }
    const SourceLocation& location() const { return loc_;  }

    // Visitor entry point — implemented by each concrete node
    virtual void accept(ASTVisitor& visitor) = 0;

protected:
    NodeKind       kind_;
    SourceLocation loc_;
};