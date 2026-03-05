#pragma once
#include "TranslationUnitNode.h"
#include "ClassNode.h"
#include "FunctionNode.h"
#include "BlockNode.h"
#include "ExpressionNode.h"
#include "ParameterNode.h"
#include "FieldNode.h"

// ── Abstract Visitor interface ─────────────────────────────────────────────
// Every SmellDetector implements this interface.
// Double dispatch: node.accept(visitor) → visitor.visit(node).
// Adding a new node type only requires one new visit() override here.
class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    virtual void visit(TranslationUnitNode& node) = 0;
    virtual void visit(ClassNode&           node) = 0;
    virtual void visit(FunctionNode&        node) = 0;
    virtual void visit(BlockNode&           node) = 0;
    virtual void visit(ExpressionNode&      node) = 0;
    virtual void visit(ParameterNode&       node) = 0;
    virtual void visit(FieldNode&           node) = 0;
};

// ── accept() implementations ───────────────────────────────────────────────
// Defined here (after ASTVisitor is fully declared) to resolve the
// circular dependency: ASTNode needs ASTVisitor, ASTVisitor needs ASTNode.
inline void TranslationUnitNode::accept(ASTVisitor& v) { v.visit(*this); }
inline void ClassNode          ::accept(ASTVisitor& v) { v.visit(*this); }
inline void FunctionNode       ::accept(ASTVisitor& v) { v.visit(*this); }
inline void BlockNode          ::accept(ASTVisitor& v) { v.visit(*this); }
inline void ExpressionNode     ::accept(ASTVisitor& v) { v.visit(*this); }
inline void ParameterNode      ::accept(ASTVisitor& v) { v.visit(*this); }
inline void FieldNode          ::accept(ASTVisitor& v) { v.visit(*this); }