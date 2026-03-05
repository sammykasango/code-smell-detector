#pragma once
#include "SmellDetector.h"
#include <sstream>

// ── MagicNumberDetector ────────────────────────────────────────────────────
// Flags numeric literals inside function bodies that are not in the
// configured set of allowed literals (0, 1, -1, 2 by default).
//
// Why it matters: A bare number in code carries no meaning. '86400' tells
// the reader nothing; 'SECONDS_PER_DAY' tells them everything.
// The fix: extract to a named const or #define.
//
// Config key: allowed_literals (default: 0, 1, -1, 2)
class MagicNumberDetector : public SmellDetector {
public:
    explicit MagicNumberDetector(const DetectorConfig& cfg)
        : SmellDetector("MagicNumber", Severity::LOW, cfg) {}

    void visit(TranslationUnitNode& unit) override { walkUnit(unit); }
    void visit(ClassNode& cls)            override { walkClass(cls); }

    void visit(FunctionNode& fn) override {
        inFunction_ = true;
        if (fn.body) fn.body->accept(*this);
        inFunction_ = false;
    }

    void visit(BlockNode& block) override {
        for (auto& stmt : block.statements) stmt->accept(*this);
    }

    void visit(ExpressionNode& expr) override {
        if (!inFunction_)          return;
        if (!expr.isNumericLiteral) return;
        if (config_.allowedLiterals.count(expr.value)) return;

        std::ostringstream msg;
        msg << "Magic number '" << expr.value << "' — replace with a named constant";
        addViolation(expr.location(), msg.str(), Severity::LOW);
    }

private:
    bool inFunction_ = false;
};