#pragma once
#include "../detectors/SmellDetector.h"
#include "../detectors/DeadCodeDetector.h"
#include "../report/Report.h"
#include <vector>
#include <memory>

// ── DetectionEngine ────────────────────────────────────────────────────────
// Owns a list of SmellDetector instances and runs them all against a
// TranslationUnitNode, collecting their violations into a single Report.
//
// Usage:
//   DetectionEngine engine(config);
//   engine.registerDetector(std::make_unique<LongFunctionDetector>(config));
//   // ... register others ...
//   Report report = engine.runAll(*unit);
//
// New detectors are registered via registerDetector() — the engine itself
// never needs to change when a new smell is added.
class DetectionEngine {
public:
    explicit DetectionEngine(const DetectorConfig& config) : config_(config) {}

    void registerDetector(std::unique_ptr<SmellDetector> detector) {
        detectors_.push_back(std::move(detector));
    }

    // Run every registered detector against the parsed AST root.
    // Returns an aggregated, sorted Report.
    Report runAll(TranslationUnitNode& unit) {
        Report report;
        report.totalFiles = 1;

        for (auto& detector : detectors_) {
            // DeadCodeDetector overrides analyze() with a two-pass approach
            if (auto* dead = dynamic_cast<DeadCodeDetector*>(detector.get())) {
                dead->analyze(unit);
            } else {
                detector->analyze(unit);
            }

            for (auto& v : detector->getViolations())
                report.addViolation(v);
        }

        report.sort();
        return report;
    }

    int detectorCount() const { return static_cast<int>(detectors_.size()); }

private:
    const DetectorConfig&                       config_;
    std::vector<std::unique_ptr<SmellDetector>> detectors_;
};