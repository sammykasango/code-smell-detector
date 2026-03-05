#pragma once
#include "SmellDetector.h"
#include <sstream>

// ── LongFunctionDetector ───────────────────────────────────────────────────
// Flags any function whose body exceeds config.maxFunctionLines.
//
// Why it matters: A function that is too long is doing too many things.
// Long functions are harder to read, test, and safely modify.
// The fix: decompose into smaller, well-named helper functions.
//
// Threshold key: max_function_lines (default: 50)
class LongFunctionDetector : public SmellDetector {
public:
    explicit LongFunctionDetector(const DetectorConfig& cfg)
        : SmellDetector("LongFunction", Severity::HIGH, cfg) {}

    void visit(TranslationUnitNode& unit) override { walkUnit(unit); }
    void visit(ClassNode& cls)            override { walkClass(cls); }

    void visit(FunctionNode& fn) override {
        int lines = fn.lineCount();
        if (lines > config_.maxFunctionLines) {
            std::ostringstream msg;
            msg << "Function '" << fn.name << "' is " << lines
                << " lines long (max: " << config_.maxFunctionLines << ")";
            addViolation(fn.location(), msg.str(), Severity::HIGH);
        }
        if (fn.body) fn.body->accept(*this);
    }
};