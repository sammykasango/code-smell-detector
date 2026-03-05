#pragma once
#include "../parser/ASTVisitor.h"
#include "../report/Violation.h"
#include "../config/DetectorConfig.h"
#include <vector>
#include <string>

// ── SmellDetector — abstract base for all detectors ────────────────────────
//
// Design: Template Method + Strategy + Visitor combined.
//
//   Template Method:  analyze() is the fixed skeleton — it resets state and
//                     kicks off the visitor walk. Subclasses implement their
//                     logic inside visit() overrides, not in analyze() directly.
//
//   Strategy:         DetectionEngine holds a vector<SmellDetector*> and calls
//                     analyze() on each. Detectors are interchangeable.
//
//   Visitor:          Each detector implements ASTVisitor. The AST calls back
//                     into the detector via visit() — classic double dispatch.
//
// To add a new detector:
//   1. Create NewDetector.h extending SmellDetector
//   2. Override visit() for the node types you care about
//   3. Call addViolation() when a smell is found
//   4. Register it in DetectorFactory.h
class SmellDetector : public ASTVisitor {
public:
    explicit SmellDetector(std::string name, Severity severity,
                           const DetectorConfig& config)
        : name_(std::move(name)), severity_(severity), config_(config) {}

    virtual ~SmellDetector() = default;

    // ── Template Method entry point ─────────────────────────────────────
    // Resets state, then drives the visitor walk over the full AST.
    void analyze(TranslationUnitNode& root) {
        violations_.clear();
        root.accept(*this);
    }

    // ── Default no-op visits ─────────────────────────────────────────────
    // Subclasses override only the nodes they need. Everything else is a no-op
    // that still walks down the tree so children are never silently skipped.
    void visit(TranslationUnitNode& node) override { walkUnit(node); }
    void visit(ClassNode&           node) override { walkClass(node); }
    void visit(FunctionNode&        node) override { (void)node; }
    void visit(BlockNode&           node) override { walkBlock(node); }
    void visit(ExpressionNode&      node) override { (void)node; }
    void visit(ParameterNode&       node) override { (void)node; }
    void visit(FieldNode&           node) override { (void)node; }

    // ── Results ──────────────────────────────────────────────────────────
    const std::vector<Violation>& getViolations() const { return violations_; }
    const std::string&            getName()        const { return name_; }
    Severity                      getSeverity()    const { return severity_; }

protected:
    std::string            name_;
    Severity               severity_;
    const DetectorConfig&  config_;
    std::vector<Violation> violations_;

    // Add a violation if it meets the configured minimum severity threshold
    void addViolation(const SourceLocation& loc,
                      const std::string&    message,
                      Severity              sev = Severity::HIGH) {
        if (sev >= config_.minSeverity)
            violations_.emplace_back(loc.file, loc.line, name_, message, sev);
    }

    // ── Tree-walking helpers ─────────────────────────────────────────────
    // Subclasses call these from overridden visit() to keep descending.
    void walkUnit(TranslationUnitNode& unit) {
        for (auto& cls : unit.classes)   cls->accept(*this);
        for (auto& fn  : unit.functions) fn->accept(*this);
    }
    void walkClass(ClassNode& cls) {
        for (auto& m : cls.methods) m->accept(*this);
    }
    void walkBlock(BlockNode& block) {
        for (auto& s : block.statements) s->accept(*this);
    }
};