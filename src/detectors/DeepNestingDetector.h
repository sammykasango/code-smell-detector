#pragma once
#include "SmellDetector.h"
#include <sstream>
#include <set>

// ── DeepNestingDetector ────────────────────────────────────────────────────
// Flags BlockNodes whose nestingDepth exceeds config.maxNestingDepth.
// The AST builder stamps each block with its depth at parse time, so
// this detector simply reads the value — no re-traversal needed.
//
// Why it matters: Code nested 5–6 levels deep is nearly impossible to
// follow. Each level adds a condition the reader must hold in memory.
// The fix: early returns, guard clauses, and extracted helper functions.
//
// Threshold key: max_nesting_depth (default: 4)
// Only the first offending block per function is reported to reduce noise.
class DeepNestingDetector : public SmellDetector {
public:
    explicit DeepNestingDetector(const DetectorConfig& cfg)
        : SmellDetector("DeepNesting", Severity::MEDIUM, cfg) {}

    void visit(TranslationUnitNode& unit) override { walkUnit(unit); }
    void visit(ClassNode& cls)            override { walkClass(cls); }

    void visit(FunctionNode& fn) override {
        currentFunction_ = fn.name;
        if (fn.body) fn.body->accept(*this);
        currentFunction_ = "";
    }

    void visit(BlockNode& block) override {
        if (block.nestingDepth > config_.maxNestingDepth) {
            // Only report once per function to avoid flooding the report
            if (reported_.find(currentFunction_) == reported_.end()) {
                std::ostringstream msg;
                msg << "Nesting depth " << block.nestingDepth
                    << " in function '" << currentFunction_
                    << "' exceeds max (" << config_.maxNestingDepth << ")";
                addViolation(block.location(), msg.str(), Severity::MEDIUM);
                reported_.insert(currentFunction_);
            }
        }
        walkBlock(block);
    }

private:
    std::string           currentFunction_;
    std::set<std::string> reported_;   // tracks which functions already have a violation
};