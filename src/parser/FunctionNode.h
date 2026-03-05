#pragma once
#include "ASTNode.h"
#include "ParameterNode.h"
#include "BlockNode.h"
#include <string>
#include <vector>
#include <memory>

// ── Represents a function or method definition ─────────────────────────────
// Covers free functions, class methods, constructors, and destructors.
// ownerClass is empty for free functions; set to the class name for methods.
class FunctionNode : public ASTNode {
public:
    std::string                name;
    std::string                returnType;
    std::string                ownerClass;     // "" for free functions
    std::vector<ParameterNode> parameters;
    std::unique_ptr<BlockNode> body;           // nullptr for pure declarations
    bool                       isVirtual    = false;
    bool                       isStatic     = false;
    bool                       isReferenced = false;

    // Convenience accessors used by detectors
    int lineCount()  const { return body ? body->lineCount : 0; }
    int paramCount() const { return static_cast<int>(parameters.size()); }

    FunctionNode(std::string n, std::string ret, SourceLocation loc)
        : ASTNode(NodeKind::Function, loc),
          name(std::move(n)), returnType(std::move(ret)) {}

    void accept(ASTVisitor& v) override;
};