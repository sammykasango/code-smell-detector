#pragma once
#include "DetectionEngine.h"
#include "../detectors/LongFunctionDetector.h"
#include "../detectors/LongParameterListDetector.h"
#include "../detectors/GodClassDetector.h"
#include "../detectors/MagicNumberDetector.h"
#include "../detectors/DeepNestingDetector.h"
#include "../detectors/DeadCodeDetector.h"
#include "../detectors/DuplicateCodeDetector.h"

// ── DetectorFactory ────────────────────────────────────────────────────────
// Reads a DetectorConfig and builds a fully-configured DetectionEngine
// with all enabled detectors registered and ready to run.
//
// To add a new detector:
//   1. #include its header above
//   2. Add one `if (cfg.enableXxx)` block in buildEngine()
//   Nothing else in the system needs to change.
class DetectorFactory {
public:
    static DetectionEngine buildEngine(const DetectorConfig& cfg) {
        DetectionEngine engine(cfg);

        if (cfg.enableLongFunction)
            engine.registerDetector(
                std::make_unique<LongFunctionDetector>(cfg));

        if (cfg.enableLongParameterList)
            engine.registerDetector(
                std::make_unique<LongParameterListDetector>(cfg));

        if (cfg.enableGodClass)
            engine.registerDetector(
                std::make_unique<GodClassDetector>(cfg));

        if (cfg.enableMagicNumbers)
            engine.registerDetector(
                std::make_unique<MagicNumberDetector>(cfg));

        if (cfg.enableDeepNesting)
            engine.registerDetector(
                std::make_unique<DeepNestingDetector>(cfg));

        if (cfg.enableDeadCode)
            engine.registerDetector(
                std::make_unique<DeadCodeDetector>(cfg));

        if (cfg.enableDuplicateCode)
            engine.registerDetector(
                std::make_unique<DuplicateCodeDetector>(cfg));

        return engine;
    }
};