#pragma once
#include "ASTNode.h"
#include <vector>
#include <memory>

// ── Represents a scoped block: { statements... } ───────────────────────────
// Tracks nesting depth so DeepNestingDetector can work without re-walking.
// Every if/for/while/function body becomes a BlockNode with depth = parent+1.
class BlockNode : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> statements;
    int nestingDepth = 0;   // 1 = function body, 2 = first if/for, etc.
    int lineCount    = 0;   // end_line - start_line + 1

    explicit BlockNode(SourceLocation loc, int depth = 0)
        : ASTNode(NodeKind::Block, loc), nestingDepth(depth) {}

    void accept(ASTVisitor& v) override;
};