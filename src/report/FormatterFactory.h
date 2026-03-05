#pragma once
#include "ConsoleFormatter.h"
#include "JSONFormatter.h"
#include "HTMLFormatter.h"
#include <memory>
#include <string>

// ── FormatterFactory ───────────────────────────────────────────────────────
// Returns the correct ReportFormatter* for a given format name string.
// Used by main.cpp to avoid depending on concrete formatter types directly.
inline std::unique_ptr<ReportFormatter> makeFormatter(const std::string& format) {
    if (format == "json") return std::make_unique<JSONFormatter>();
    if (format == "html") return std::make_unique<HTMLFormatter>();
    return std::make_unique<ConsoleFormatter>();   // default
}