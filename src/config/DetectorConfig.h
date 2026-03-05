#pragma once
#include <string>
#include <set>
#include <fstream>
#include <sstream>
#include <iostream>
#include "../report/Violation.h"

struct DetectorConfig {
    // Thresholds
    int   maxFunctionLines   = 50;
    int   maxParameters      = 4;
    int   maxClassMethods    = 20;
    int   maxClassFields     = 15;
    int   maxNestingDepth    = 4;
    float duplicateThreshold = 0.85f;

    // Allowed literals (not flagged as magic numbers)
    std::set<std::string> allowedLiterals = { "0", "1", "-1", "2" };

    // Feature flags
    bool enableLongFunction       = true;
    bool enableLongParameterList  = true;
    bool enableGodClass           = true;
    bool enableDuplicateCode      = true;
    bool enableDeadCode           = true;
    bool enableMagicNumbers       = true;
    bool enableDeepNesting        = true;

    // Output
    std::string outputFormat = "console";  // console | json | html
    std::string outputPath   = "";
    Severity    minSeverity  = Severity::LOW;

    // ── Load from .smellrc ─────────────────────────────────────────────
    bool loadFromFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) return false;

        std::string line;
        while (std::getline(file, line)) {
            // Strip comments and whitespace
            auto commentPos = line.find('#');
            if (commentPos != std::string::npos) line = line.substr(0, commentPos);
            if (line.empty()) continue;

            auto eqPos = line.find('=');
            if (eqPos == std::string::npos) continue;

            std::string key   = trim(line.substr(0, eqPos));
            std::string value = trim(line.substr(eqPos + 1));

            applyKeyValue(key, value);
        }
        return true;
    }

    void validate() const {
        if (maxFunctionLines  <= 0) throw std::runtime_error("max_function_lines must be > 0");
        if (maxParameters     <= 0) throw std::runtime_error("max_parameters must be > 0");
        if (maxNestingDepth   <= 0) throw std::runtime_error("max_nesting_depth must be > 0");
        if (duplicateThreshold < 0 || duplicateThreshold > 1)
            throw std::runtime_error("duplicate_threshold must be in [0.0, 1.0]");
    }

private:
    static std::string trim(const std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        size_t end   = s.find_last_not_of(" \t\r\n");
        return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
    }

    static bool parseBool(const std::string& v) {
        return v == "true" || v == "1" || v == "yes";
    }

    void applyKeyValue(const std::string& key, const std::string& value) {
        if (key == "max_function_lines")   maxFunctionLines   = std::stoi(value);
        else if (key == "max_parameters")  maxParameters      = std::stoi(value);
        else if (key == "max_class_methods") maxClassMethods  = std::stoi(value);
        else if (key == "max_class_fields")  maxClassFields   = std::stoi(value);
        else if (key == "max_nesting_depth") maxNestingDepth  = std::stoi(value);
        else if (key == "duplicate_threshold") duplicateThreshold = std::stof(value);
        else if (key == "enable_long_function")      enableLongFunction      = parseBool(value);
        else if (key == "enable_long_parameter_list") enableLongParameterList = parseBool(value);
        else if (key == "enable_god_class")          enableGodClass          = parseBool(value);
        else if (key == "enable_duplicate_code")     enableDuplicateCode     = parseBool(value);
        else if (key == "enable_dead_code")          enableDeadCode          = parseBool(value);
        else if (key == "enable_magic_numbers")      enableMagicNumbers      = parseBool(value);
        else if (key == "enable_deep_nesting")       enableDeepNesting       = parseBool(value);
        else if (key == "output_format")             outputFormat            = value;
        else if (key == "output_path")               outputPath              = value;
        else if (key == "min_severity")              minSeverity             = severityFromString(value);
        else if (key == "allowed_literals") {
            allowedLiterals.clear();
            std::istringstream ss(value);
            std::string tok;
            while (std::getline(ss, tok, ','))
                allowedLiterals.insert(trim(tok));
        }
    }
};