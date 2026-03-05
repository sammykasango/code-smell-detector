#pragma once
#include "SmellDetector.h"
#include <sstream>
#include <unordered_map>
#include <set>

// ── DeadCodeDetector ───────────────────────────────────────────────────────
// Two-pass heuristic detector:
//   Pass 1 — collect all defined function and class names
//   Pass 2 — collect all identifier references found in function bodies
//   Diff   — anything defined but never referenced is flagged
//
// Why it matters: Unreferenced code increases cognitive load, inflates
// build size, and may conceal old bugs. Delete it — that's what git is for.
//
// Note: This is a single-file heuristic. A linker-level analysis would
// be more precise but is out of scope for a static analyzer.
//
// Override: analyze() replaces the default visitor walk with a two-pass approach.
class DeadCodeDetector : public SmellDetector {
public:
    explicit DeadCodeDetector(const DetectorConfig& cfg)
        : SmellDetector("DeadCode", Severity::MEDIUM, cfg) {}

    // Two-pass override — does NOT use the default visitor walk
    void analyze(TranslationUnitNode& root) {
        violations_.clear();
        defined_.clear();
        referenced_.clear();

        collectDefinitions(root);
        collectReferences(root);

        for (auto& [name, loc] : defined_) {
            if (name == "main")              continue;   // entry point is always live
            if (!name.empty() && name[0] == '~') continue; // destructors always live

            if (referenced_.find(name) == referenced_.end()) {
                std::ostringstream msg;
                msg << "'" << name << "' is defined but never referenced — possible dead code";
                addViolation(loc, msg.str(), Severity::MEDIUM);
            }
        }
    }

    // No-op visits — analysis is done in the two-pass analyze() above
    void visit(TranslationUnitNode&) override {}
    void visit(ClassNode&)           override {}
    void visit(FunctionNode&)        override {}
    void visit(BlockNode&)           override {}
    void visit(ExpressionNode&)      override {}
    void visit(ParameterNode&)       override {}
    void visit(FieldNode&)           override {}

private:
    std::unordered_map<std::string, SourceLocation> defined_;
    std::set<std::string>                           referenced_;

    void collectDefinitions(TranslationUnitNode& unit) {
        for (auto& fn  : unit.functions)
            defined_[fn->name] = fn->location();

        for (auto& cls : unit.classes) {
            defined_[cls->name] = cls->location();
            for (auto& method : cls->methods)
                defined_[cls->name + "::" + method->name] = method->location();
        }
    }

    void collectReferences(TranslationUnitNode& unit) {
        // Every class name is considered referenced (used in variable declarations elsewhere)
        for (auto& cls : unit.classes) referenced_.insert(cls->name);

        for (auto& fn  : unit.functions)  scanBody(*fn);
        for (auto& cls : unit.classes)
            for (auto& method : cls->methods) scanBody(*method);
    }

    void scanBody(FunctionNode& fn) {
        if (fn.body) scanBlock(*fn.body);
    }

    void scanBlock(BlockNode& block) {
        for (auto& stmt : block.statements) {
            if (auto* expr = dynamic_cast<ExpressionNode*>(stmt.get())) {
                if (expr->exprKind == ExpressionNode::ExprKind::Identifier ||
                    expr->exprKind == ExpressionNode::ExprKind::Call) {
                    referenced_.insert(expr->value);
                }
            } else if (auto* blk = dynamic_cast<BlockNode*>(stmt.get())) {
                scanBlock(*blk);
            }
        }
    }
};