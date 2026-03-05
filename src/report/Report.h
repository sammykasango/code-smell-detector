#pragma once
#include "Violation.h"
#include <vector>
#include <map>
#include <algorithm>

// ── Report ─────────────────────────────────────────────────────────────────
// Collects all Violation instances from all detectors across all files.
// DetectionEngine populates this; ReportFormatter consumes it.
class Report {
public:
    std::vector<Violation> violations;
    int                    totalFiles = 0;

    void addViolation(Violation v) {
        violations.push_back(std::move(v));
    }

    // Sort by file path → line number → severity descending (HIGH first)
    void sort() {
        std::sort(violations.begin(), violations.end(),
            [](const Violation& a, const Violation& b) {
                if (a.filePath != b.filePath) return a.filePath < b.filePath;
                if (a.line     != b.line)     return a.line     < b.line;
                return static_cast<int>(a.severity) > static_cast<int>(b.severity);
            });
    }

    std::map<std::string, int> smellCounts() const {
        std::map<std::string, int> counts;
        for (auto& v : violations) counts[v.smellType]++;
        return counts;
    }

    int countBySeverity(Severity s) const {
        int n = 0;
        for (auto& v : violations) if (v.severity == s) ++n;
        return n;
    }

    bool empty() const { return violations.empty(); }
    int  size()  const { return static_cast<int>(violations.size()); }
};