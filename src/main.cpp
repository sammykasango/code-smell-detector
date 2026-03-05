#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>

#include "lexer/Lexer.h"
#include "parser/ASTBuilder.h"
#include "engine/DetectorFactory.h"
#include "report/FormatterFactory.h"
#include "config/DetectorConfig.h"

namespace fs = std::filesystem;

// ── File helpers ───────────────────────────────────────────────────────────
static std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) throw std::runtime_error("Cannot open file: " + path);
    std::ostringstream ss; ss << file.rdbuf(); return ss.str();
}

static std::vector<std::string> collectFiles(const std::string& path) {
    std::vector<std::string> files;
    if (fs::is_regular_file(path)) { files.push_back(path); return files; }
    if (fs::is_directory(path)) {
        for (auto& entry : fs::recursive_directory_iterator(path)) {
            if (!entry.is_regular_file()) continue;
            auto ext = entry.path().extension().string();
            if (ext == ".cpp" || ext == ".h" || ext == ".hpp" || ext == ".cc")
                files.push_back(entry.path().string());
        }
    }
    return files;
}

static void analyzeFile(const std::string& filePath,
                        DetectionEngine&   engine,
                        Report&            globalReport) {
    try {
        std::string source = readFile(filePath);
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        ASTBuilder builder(tokens, filePath);
        auto unit = builder.build();
        Report fileReport = engine.runAll(*unit);
        for (auto& v : fileReport.violations) globalReport.addViolation(v);
    } catch (const std::exception& ex) {
        std::cerr << "Warning: could not analyze " << filePath
                  << ": " << ex.what() << "\n";
    }
}

static void printUsage(const char* argv0) {
    std::cout
        << "Usage: " << argv0 << " [options] <path...>\n\n"
        << "Options:\n"
        << "  --config  <path>    Path to .smellrc config file\n"
        << "  --format  <type>    Output format: console (default), json, html\n"
        << "  --output  <path>    Write report to file instead of stdout\n"
        << "  --min-severity <s>  Minimum severity: low (default), medium, high\n"
        << "  --disable <name>    Disable a detector by name\n"
        << "  --exit-code         Exit 1 if any violations found\n"
        << "  --help              Show this message\n\n";
}

// ── Entry point ────────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {
    if (argc < 2) { printUsage(argv[0]); return 0; }

    DetectorConfig           config;
    std::vector<std::string> inputPaths;
    bool                     useExitCode = false;

    // Stash CLI values before config file can override them
    std::string cliFormat, cliOutput, cliMinSev, cliConfig;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") { printUsage(argv[0]); return 0; }
        if (arg == "--exit-code")                         { useExitCode = true; continue; }
        if (arg == "--config"       && i+1 < argc)        { cliConfig  = argv[++i]; continue; }
        if (arg == "--format"       && i+1 < argc)        { cliFormat  = argv[++i]; continue; }
        if (arg == "--output"       && i+1 < argc)        { cliOutput  = argv[++i]; continue; }
        if (arg == "--min-severity" && i+1 < argc)        { cliMinSev  = argv[++i]; continue; }
        if (arg == "--disable"      && i+1 < argc) {
            std::string det = argv[++i];
            if (det == "long_function")       config.enableLongFunction      = false;
            if (det == "long_parameter_list") config.enableLongParameterList = false;
            if (det == "god_class")           config.enableGodClass          = false;
            if (det == "duplicate_code")      config.enableDuplicateCode     = false;
            if (det == "dead_code")           config.enableDeadCode          = false;
            if (det == "magic_numbers")       config.enableMagicNumbers      = false;
            if (det == "deep_nesting")        config.enableDeepNesting       = false;
            continue;
        }
        inputPaths.push_back(arg);
    }

    if (inputPaths.empty()) {
        std::cerr << "Error: no input files or directories specified.\n";
        printUsage(argv[0]); return 1;
    }

    // Load config file — explicit path or auto-detect .smellrc
    if (!cliConfig.empty()) {
        if (!config.loadFromFile(cliConfig))
            std::cerr << "Warning: could not load config: " << cliConfig << "\n";
    } else {
        std::ifstream rc(".smellrc");
        if (rc.good()) { rc.close(); config.loadFromFile(".smellrc"); }
    }

    // CLI flags always win over config file
    if (!cliFormat.empty()) config.outputFormat = cliFormat;
    if (!cliOutput.empty()) config.outputPath   = cliOutput;
    if (!cliMinSev.empty()) config.minSeverity  = severityFromString(cliMinSev);

    try { config.validate(); }
    catch (const std::exception& ex) {
        std::cerr << "Config error: " << ex.what() << "\n"; return 1;
    }

    // Build engine with all enabled detectors
    auto engine = DetectorFactory::buildEngine(config);
    std::cerr << "Running " << engine.detectorCount() << " detector(s)...\n\n";

    // Analyze all target files
    Report globalReport;
    for (auto& path : inputPaths) {
        auto files = collectFiles(path);
        globalReport.totalFiles += static_cast<int>(files.size());
        for (auto& f : files) {
            std::cerr << "  Analyzing: " << f << "\n";
            analyzeFile(f, engine, globalReport);
        }
    }

    globalReport.sort();

    // Format and emit report
    auto formatter = makeFormatter(config.outputFormat);
    if (!config.outputPath.empty()) {
        formatter->formatToFile(globalReport, config.outputPath);
        std::cerr << "\nReport written to: " << config.outputPath << "\n";
    } else {
        formatter->format(globalReport, std::cout);
    }

    if (useExitCode && !globalReport.empty()) {
        std::cerr << "Exiting with code 1: "
                  << globalReport.size() << " violation(s) found.\n";
        return 1;
    }
    return 0;
}