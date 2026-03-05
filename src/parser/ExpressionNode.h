#pragma once
#include "ASTNode.h"
#include <string>

// ── Represents an expression or literal inside a function body ─────────────
// Used primarily by MagicNumberDetector to locate numeric literals,
// and by DeadCodeDetector to harvest identifier references.
class ExpressionNode : public ASTNode {
public:
    enum class ExprKind {
        Literal,      // numeric or string literal
        Identifier,   // a named reference (variable, function call target)
        Call,         // function call expression
        BinaryOp,     // a op b
        UnaryOp,      // op a
        Other
    };

    ExprKind    exprKind;
    std::string value;              // literal value or identifier name
    bool        isNumericLiteral = false;

    ExpressionNode(ExprKind ek, std::string val, SourceLocation loc)
        : ASTNode(NodeKind::Expression, loc),
          exprKind(ek), value(std::move(val)) {}

    void accept(ASTVisitor& v) override;
};