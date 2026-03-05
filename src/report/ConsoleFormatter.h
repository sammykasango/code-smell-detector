#pragma once
#include "ReportFormatter.h"
#include <iomanip>

// ── ConsoleFormatter ───────────────────────────────────────────────────────
// Renders a human-readable, aligned report to the terminal.
// Default formatter — used when no --format flag is supplied.
class ConsoleFormatter : public ReportFormatter {
public:
    void format(const Report& report, std::ostream& out) const override {
        const std::string LINE(65, '=');
        const std::string DASH(65, '-');

        out << "\n" << LINE << "\n";
        out << "  CODE SMELL REPORT\n";
        out << LINE << "\n\n";

        if (report.empty()) {
            out << "  No violations found.\n\n" << LINE << "\n";
            return;
        }

        std::string lastFile;
        for (auto& v : report.violations) {
            if (v.filePath != lastFile) {
                if (!lastFile.empty()) out << "\n";
                out << "  File: " << v.filePath << "\n\n";
                lastFile = v.filePath;
            }
            out << "  [" << std::left << std::setw(6) << severityToString(v.severity) << "] "
                << "Line " << std::setw(5) << v.line << " | "
                << v.smellType << "\n"
                << "               | " << v.message << "\n\n";
        }

        out << DASH << "\n";
        out << "  Summary: " << report.size() << " violation(s) across "
            << report.totalFiles << " file(s)\n";
        out << "  High: "    << report.countBySeverity(Severity::HIGH)
            << "   Medium: " << report.countBySeverity(Severity::MEDIUM)
            << "   Low: "    << report.countBySeverity(Severity::LOW) << "\n";
        out << LINE << "\n\n";
    }
};