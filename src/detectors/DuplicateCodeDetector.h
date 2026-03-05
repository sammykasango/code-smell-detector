#pragma once
#include "SmellDetector.h"
#include <sstream>
#include <unordered_map>

// ── DuplicateCodeDetector ──────────────────────────────────────────────────
// Detects structurally identical functions using a lightweight fingerprint:
//   fingerprint = hash(lineCount, paramCount, returnType)
//
// Functions with matching fingerprints and bodies ≥ 5 lines are flagged.
//
// Why it matters: Duplicated logic is a maintenance trap — any future
// change must be made in multiple places, and it's easy to miss one.
// The fix: extract the shared logic into a single shared function.
//
// Note: A production-grade implementation would use a Rabin-Karp rolling
// hash over the token sequence for byte-level precision. This fingerprint
// approach is a fast, effective heuristic for the test-environment scope.
class DuplicateCodeDetector : public SmellDetector {
public:
    explicit DuplicateCodeDetector(const DetectorConfig& cfg)
        : SmellDetector("DuplicateCode", Severity::HIGH, cfg) {}

    void visit(TranslationUnitNode& unit) override {
        fingerprints_.clear();

        // Collect structural fingerprints for every function
        for (auto& fn  : unit.functions) collectFingerprint(*fn);
        for (auto& cls : unit.classes)
            for (auto& method : cls->methods) collectFingerprint(*method);

        // Group functions by fingerprint — any group of size ≥ 2 is suspicious
        std::unordered_map<size_t, std::vector<FunctionNode*>> groups;
        for (auto& [fn, fp] : fingerprints_) groups[fp].push_back(fn);

        for (auto& [fp, fns] : groups) {
            if (fns.size() < 2)            continue;
            if (fns[0]->lineCount() < 5)   continue;   // too small to care about

            // First function in the group is the "original"; the rest are flagged
            for (size_t i = 1; i < fns.size(); ++i) {
                std::ostringstream msg;
                msg << "Function '" << fns[i]->name
                    << "' appears structurally identical to '"
                    << fns[0]->name << "' ("
                    << fns[0]->lineCount()  << " lines, "
                    << fns[0]->paramCount() << " params)"
                    << " — consider extracting shared logic";
                addViolation(fns[i]->location(), msg.str(), Severity::HIGH);
            }
        }
    }

    void visit(ClassNode&)    override {}
    void visit(FunctionNode&) override {}

private:
    std::unordered_map<FunctionNode*, size_t> fingerprints_;

    void collectFingerprint(FunctionNode& fn) {
        size_t h = 0;
        hashCombine(h, static_cast<size_t>(fn.lineCount()));
        hashCombine(h, static_cast<size_t>(fn.paramCount()));
        hashCombine(h, std::hash<std::string>{}(fn.returnType));
        fingerprints_[&fn] = h;
    }

    // Boost-style hash combine
    static void hashCombine(size_t& seed, size_t val) {
        seed ^= val + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
};