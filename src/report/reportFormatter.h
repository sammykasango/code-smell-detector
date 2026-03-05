#pragma once
#include "Report.h"
#include <iostream>
#include <fstream>
#include <stdexcept>

// ── ReportFormatter — abstract base ───────────────────────────────────────
// All output formatters inherit from this. Adding a new output format
// (e.g. XML, Markdown) only requires creating a new subclass and
// overriding format(). Nothing else in the system needs to change.
class ReportFormatter {
public:
    virtual ~ReportFormatter() = default;

    // Render the report to any output stream (stdout, file, string buffer)
    virtual void format(const Report& report, std::ostream& out) const = 0;

    // Convenience: write directly to a file path
    void formatToFile(const Report& report, const std::string& path) const {
        std::ofstream file(path);
        if (!file.is_open())
            throw std::runtime_error("Cannot open output file: " + path);
        format(report, file);
    }
};