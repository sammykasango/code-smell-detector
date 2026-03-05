#pragma once
#include <string>

// ── Severity levels ────────────────────────────────────────────────────────
enum class Severity { LOW, MEDIUM, HIGH };

inline std::string severityToString(Severity s) {
    switch (s) {
        case Severity::HIGH:   return "HIGH";
        case Severity::MEDIUM: return "MEDIUM";
        case Severity::LOW:    return "LOW";
    }
    return "UNKNOWN";
}

inline Severity severityFromString(const std::string& s) {
    if (s == "high")   return Severity::HIGH;
    if (s == "medium") return Severity::MEDIUM;
    return Severity::LOW;
}

inline bool operator>=(Severity a, Severity b) {
    return static_cast<int>(a) >= static_cast<int>(b);
}

// ── Violation ──────────────────────────────────────────────────────────────
// Represents one detected code smell instance at one location in one file.
// This is the fundamental output unit of every SmellDetector.
struct Violation {
    std::string filePath;
    int         line;
    std::string smellType;
    std::string message;
    Severity    severity;

    Violation(std::string fp, int ln, std::string smell,
              std::string msg, Severity sev)
        : filePath(std::move(fp)), line(ln),
          smellType(std::move(smell)), message(std::move(msg)),
          severity(sev) {}

    std::string toString() const {
        return "[" + severityToString(severity) + "] " +
               filePath + ":" + std::to_string(line) +
               " | " + smellType + " — " + message;
    }
};