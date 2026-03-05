#pragma once
#include "ReportFormatter.h"

// ── JSONFormatter ──────────────────────────────────────────────────────────
// Emits a structured JSON report. Designed for CI pipeline integration —
// output can be consumed by dashboards, trend trackers, or custom scripts.
// Use: --format json  or  --format json --output report.json
class JSONFormatter : public ReportFormatter {
public:
    void format(const Report& report, std::ostream& out) const override {
        out << "{\n";
        out << "  \"summary\": {\n";
        out << "    \"total_violations\": " << report.size()                             << ",\n";
        out << "    \"files_analyzed\": "   << report.totalFiles                         << ",\n";
        out << "    \"high\": "             << report.countBySeverity(Severity::HIGH)    << ",\n";
        out << "    \"medium\": "           << report.countBySeverity(Severity::MEDIUM)  << ",\n";
        out << "    \"low\": "              << report.countBySeverity(Severity::LOW)     << "\n";
        out << "  },\n";
        out << "  \"violations\": [\n";

        for (size_t i = 0; i < report.violations.size(); ++i) {
            const auto& v = report.violations[i];
            out << "    {\n";
            out << "      \"file\": \""      << escape(v.filePath)              << "\",\n";
            out << "      \"line\": "        << v.line                           << ",\n";
            out << "      \"smell\": \""     << escape(v.smellType)             << "\",\n";
            out << "      \"severity\": \""  << severityToString(v.severity)    << "\",\n";
            out << "      \"message\": \""   << escape(v.message)               << "\"\n";
            out << "    }" << (i + 1 < report.violations.size() ? "," : "") << "\n";
        }

        out << "  ]\n}\n";
    }

private:
    static std::string escape(const std::string& s) {
        std::string out;
        for (char c : s) {
            switch (c) {
                case '"':  out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\n': out += "\\n";  break;
                case '\r': out += "\\r";  break;
                case '\t': out += "\\t";  break;
                default:   out += c;
            }
        }
        return out;
    }
};