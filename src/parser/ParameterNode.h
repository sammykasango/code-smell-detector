#pragma once
#include "ASTNode.h"
#include <string>

// ── Represents a single function parameter ─────────────────────────────────
// e.g. the 'int count' in void foo(int count)
class ParameterNode : public ASTNode {
public:
    std::string name;
    std::string typeName;

    ParameterNode(std::string n, std::string t, SourceLocation loc)
        : ASTNode(NodeKind::Parameter, loc),
          name(std::move(n)), typeName(std::move(t)) {}

    void accept(ASTVisitor& v) override;
};