#pragma once
#include "SmellDetector.h"
#include <sstream>

// ── GodClassDetector ───────────────────────────────────────────────────────
// Flags classes with too many methods AND/OR too many fields.
//
// Why it matters: A class with too many responsibilities becomes a
// maintenance magnet — everything depends on it and it becomes impossible
// to change safely. The fix: split into focused, single-purpose classes.
//
// Threshold keys: max_class_methods (default: 20), max_class_fields (default: 15)
// A full God Class violation requires BOTH thresholds to be exceeded.
// Exceeding only one generates a MEDIUM warning instead.
class GodClassDetector : public SmellDetector {
public:
    explicit GodClassDetector(const DetectorConfig& cfg)
        : SmellDetector("GodClass", Severity::HIGH, cfg) {}

    void visit(TranslationUnitNode& unit) override { walkUnit(unit); }

    void visit(ClassNode& cls) override {
        int methods         = cls.methodCount();
        int fields          = cls.fieldCount();
        bool tooManyMethods = methods > config_.maxClassMethods;
        bool tooManyFields  = fields  > config_.maxClassFields;

        if (tooManyMethods && tooManyFields) {
            std::ostringstream msg;
            msg << "Class '" << cls.name << "' has " << methods
                << " methods and " << fields << " fields — likely a God Class"
                << " (max: " << config_.maxClassMethods << " methods, "
                << config_.maxClassFields << " fields)";
            addViolation(cls.location(), msg.str(), Severity::HIGH);

        } else if (tooManyMethods) {
            std::ostringstream msg;
            msg << "Class '" << cls.name << "' has " << methods
                << " methods (max: " << config_.maxClassMethods << ")";
            addViolation(cls.location(), msg.str(), Severity::MEDIUM);

        } else if (tooManyFields) {
            std::ostringstream msg;
            msg << "Class '" << cls.name << "' has " << fields
                << " fields (max: " << config_.maxClassFields << ")";
            addViolation(cls.location(), msg.str(), Severity::MEDIUM);
        }

        walkClass(cls);
    }
};