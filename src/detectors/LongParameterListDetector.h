#pragma once
#include "SmellDetector.h"
#include <sstream>

// ── LongParameterListDetector ──────────────────────────────────────────────
// Flags functions with more parameters than config.maxParameters.
//
// Why it matters: Too many parameters indicate a function knows too much
// about its callers, or that a struct/class should replace the raw values.
// The fix: group related parameters into a config or data object.
//
// Threshold key: max_parameters (default: 4)
class LongParameterListDetector : public SmellDetector {
public:
    explicit LongParameterListDetector(const DetectorConfig& cfg)
        : SmellDetector("LongParameterList", Severity::MEDIUM, cfg) {}

    void visit(TranslationUnitNode& unit) override { walkUnit(unit); }
    void visit(ClassNode& cls)            override { walkClass(cls); }

    void visit(FunctionNode& fn) override {
        int count = fn.paramCount();
        if (count > config_.maxParameters) {
            std::ostringstream msg;
            msg << "Function '" << fn.name << "' has " << count
                << " parameters (max: " << config_.maxParameters << ")";
            addViolation(fn.location(), msg.str(), Severity::MEDIUM);
        }
    }
};