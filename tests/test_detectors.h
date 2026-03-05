// tests/test_detectors.cpp
// Unit tests for all 7 SmellDetector implementations.
// Run: g++ -std=c++17 -Isrc tests/test_detectors.cpp src/lexer/Lexer.cpp src/parser/ASTBuilder.cpp -o test_detectors && ./test_detectors

#include <iostream>
#include <cassert>
#include <functional>
#include "../src/lexer/Lexer.h"
#include "../src/parser/ASTBuilder.h"
#include "../src/detectors/LongFunctionDetector.h"
#include "../src/detectors/LongParameterListDetector.h"
#include "../src/detectors/GodClassDetector.h"
#include "../src/detectors/MagicNumberDetector.h"
#include "../src/detectors/DeepNestingDetector.h"
#include "../src/detectors/DeadCodeDetector.h"
#include "../src/detectors/DuplicateCodeDetector.h"

static int passed = 0, failed = 0;

#define CHECK(cond, label) \
    if (cond) { ++passed; std::cout << "  [PASS] " << label << "\n"; } \
    else       { ++failed; std::cout << "  [FAIL] " << label << "\n"; }

// ── Helpers ────────────────────────────────────────────────────────────────
static std::unique_ptr<TranslationUnitNode> parse(const std::string& src) {
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    ASTBuilder builder(tokens, "test.cpp");
    return builder.build();
}

static DetectorConfig makeConfig() {
    DetectorConfig cfg;
    cfg.maxFunctionLines  = 10;
    cfg.maxParameters     = 3;
    cfg.maxClassMethods   = 5;
    cfg.maxClassFields    = 4;
    cfg.maxNestingDepth   = 2;
    cfg.minSeverity       = Severity::LOW;
    return cfg;
}

// Count violations of a specific smell type
static int countSmell(const std::vector<Violation>& v, const std::string& smell) {
    int n = 0;
    for (auto& x : v) if (x.smellType == smell) ++n;
    return n;
}

// ── 1. LongFunctionDetector ───────────────────────────────────────────────
void test_long_function_triggered() {
    // Build a function guaranteed to exceed 10 lines
    std::string src = "void bigFn() {\n";
    for (int i = 0; i < 15; ++i) src += "  int x" + std::to_string(i) + " = " + std::to_string(i) + ";\n";
    src += "}";
    auto unit = parse(src);
    auto cfg = makeConfig();
    LongFunctionDetector det(cfg);
    det.analyze(*unit);
    CHECK(!det.getViolations().empty(), "LongFunction: triggered on 15-line function");
    CHECK(det.getViolations()[0].smellType == "LongFunction", "LongFunction: correct smell type");
}

void test_long_function_not_triggered() {
    auto unit = parse("void small() { int x = 1; }");
    auto cfg = makeConfig();
    LongFunctionDetector det(cfg);
    det.analyze(*unit);
    CHECK(det.getViolations().empty(), "LongFunction: not triggered on short function");
}

// ── 2. LongParameterListDetector ─────────────────────────────────────────
void test_long_params_triggered() {
    auto unit = parse("void f(int a, int b, int c, int d) {}");
    auto cfg = makeConfig(); // max = 3
    LongParameterListDetector det(cfg);
    det.analyze(*unit);
    CHECK(!det.getViolations().empty(), "LongParamList: 4 params > max 3");
}

void test_long_params_not_triggered() {
    auto unit = parse("void f(int a, int b) {}");
    auto cfg = makeConfig();
    LongParameterListDetector det(cfg);
    det.analyze(*unit);
    CHECK(det.getViolations().empty(), "LongParamList: 2 params <= max 3");
}

void test_long_params_exact_threshold() {
    auto unit = parse("void f(int a, int b, int c) {}");
    auto cfg = makeConfig(); // max = 3, exactly at threshold
    LongParameterListDetector det(cfg);
    det.analyze(*unit);
    CHECK(det.getViolations().empty(), "LongParamList: 3 params == max 3, not flagged");
}

// ── 3. GodClassDetector ───────────────────────────────────────────────────
void test_god_class_triggered() {
    // 6 methods + 5 fields, both exceeding max (5/4)
    std::string src = "class Big {\npublic:\n";
    for (int i = 0; i < 6; ++i) src += "  void m" + std::to_string(i) + "() {}\n";
    for (int i = 0; i < 5; ++i) src += "  int f" + std::to_string(i) + ";\n";
    src += "};";
    auto unit = parse(src);
    auto cfg = makeConfig();
    GodClassDetector det(cfg);
    det.analyze(*unit);
    CHECK(!det.getViolations().empty(), "GodClass: triggered when both methods and fields exceed");
    CHECK(det.getViolations()[0].severity == Severity::HIGH, "GodClass: HIGH severity");
}

void test_god_class_methods_only() {
    // Many methods but few fields — MEDIUM, not HIGH
    std::string src = "class Wide {\npublic:\n";
    for (int i = 0; i < 8; ++i) src += "  void m" + std::to_string(i) + "() {}\n";
    src += "  int x;\n};";
    auto unit = parse(src);
    auto cfg = makeConfig();
    GodClassDetector det(cfg);
    det.analyze(*unit);
    bool hasMedium = false;
    for (auto& v : det.getViolations()) if (v.severity == Severity::MEDIUM) hasMedium = true;
    CHECK(hasMedium, "GodClass: MEDIUM when only methods exceed threshold");
}

void test_god_class_not_triggered() {
    auto unit = parse("class Small { public: void a() {} int x; };");
    auto cfg = makeConfig();
    GodClassDetector det(cfg);
    det.analyze(*unit);
    CHECK(det.getViolations().empty(), "GodClass: not triggered on small class");
}

// ── 4. MagicNumberDetector ────────────────────────────────────────────────
void test_magic_number_triggered() {
    auto unit = parse("void f() { int x = 86400; }");
    auto cfg = makeConfig();
    MagicNumberDetector det(cfg);
    det.analyze(*unit);
    CHECK(!det.getViolations().empty(), "MagicNumber: 86400 flagged");
}

void test_magic_number_allowed_literal() {
    auto unit = parse("void f() { int x = 0; int y = 1; }");
    auto cfg = makeConfig(); // 0 and 1 are in allowed set
    MagicNumberDetector det(cfg);
    det.analyze(*unit);
    CHECK(det.getViolations().empty(), "MagicNumber: 0 and 1 not flagged");
}

// ── 5. DeepNestingDetector ────────────────────────────────────────────────
void test_deep_nesting_triggered() {
    auto unit = parse(R"(
void f() {
    if (true) {
        for (;;) {
            while (true) {
                int x = 1;
            }
        }
    }
}
    )");
    auto cfg = makeConfig(); // max depth = 2
    DeepNestingDetector det(cfg);
    det.analyze(*unit);
    CHECK(!det.getViolations().empty(), "DeepNesting: triggered at depth > 2");
}

void test_deep_nesting_not_triggered() {
    auto unit = parse("void f() { int x = 1; }");
    auto cfg = makeConfig();
    DeepNestingDetector det(cfg);
    det.analyze(*unit);
    CHECK(det.getViolations().empty(), "DeepNesting: not triggered on flat function");
}

// ── 6. DeadCodeDetector ───────────────────────────────────────────────────
void test_dead_code_triggered() {
    // legacyHelper is defined but never called
    auto unit = parse(R"(
void legacyHelper() { int x = 1; }
int main() { return 0; }
    )");
    auto cfg = makeConfig();
    DeadCodeDetector det(cfg);
    det.analyze(*unit);
    bool found = false;
    for (auto& v : det.getViolations())
        if (v.message.find("legacyHelper") != std::string::npos) found = true;
    CHECK(found, "DeadCode: legacyHelper flagged as dead");
}

void test_dead_code_main_not_flagged() {
    auto unit = parse("int main() { return 0; }");
    auto cfg = makeConfig();
    DeadCodeDetector det(cfg);
    det.analyze(*unit);
    bool mainFlagged = false;
    for (auto& v : det.getViolations())
        if (v.message.find("main") != std::string::npos) mainFlagged = true;
    CHECK(!mainFlagged, "DeadCode: main() is never flagged");
}

// ── 7. DuplicateCodeDetector ──────────────────────────────────────────────
void test_duplicate_code_triggered() {
    // Two structurally identical functions (same line count, params, return type)
    std::string src = "int calcA(int x, int y) {\n";
    for (int i = 0; i < 6; ++i) src += "  int v" + std::to_string(i) + " = " + std::to_string(i) + ";\n";
    src += "  return x + y;\n}\n";
    src += "int calcB(int x, int y) {\n";
    for (int i = 0; i < 6; ++i) src += "  int v" + std::to_string(i) + " = " + std::to_string(i) + ";\n";
    src += "  return x + y;\n}\n";

    auto unit = parse(src);
    auto cfg = makeConfig();
    DuplicateCodeDetector det(cfg);
    det.analyze(*unit);
    CHECK(!det.getViolations().empty(), "DuplicateCode: calcA and calcB flagged");
}

void test_duplicate_code_different_not_flagged() {
    auto unit = parse(R"(
int funcA(int x) { return x + 1; }
void funcB(int x, int y, int z) { int r = x * y * z; }
    )");
    auto cfg = makeConfig();
    DuplicateCodeDetector det(cfg);
    det.analyze(*unit);
    CHECK(det.getViolations().empty(), "DuplicateCode: different functions not flagged");
}

// ── Main ───────────────────────────────────────────────────────────────────
int main() {
    std::cout << "\n=== Detector Tests ===\n\n";

    std::cout << "-- LongFunctionDetector --\n";
    test_long_function_triggered();
    test_long_function_not_triggered();

    std::cout << "-- LongParameterListDetector --\n";
    test_long_params_triggered();
    test_long_params_not_triggered();
    test_long_params_exact_threshold();

    std::cout << "-- GodClassDetector --\n";
    test_god_class_triggered();
    test_god_class_methods_only();
    test_god_class_not_triggered();

    std::cout << "-- MagicNumberDetector --\n";
    test_magic_number_triggered();
    test_magic_number_allowed_literal();

    std::cout << "-- DeepNestingDetector --\n";
    test_deep_nesting_triggered();
    test_deep_nesting_not_triggered();

    std::cout << "-- DeadCodeDetector --\n";
    test_dead_code_triggered();
    test_dead_code_main_not_flagged();

    std::cout << "-- DuplicateCodeDetector --\n";
    test_duplicate_code_triggered();
    test_duplicate_code_different_not_flagged();

    std::cout << "\n--- Results: " << passed << " passed, " << failed << " failed ---\n\n";
    return failed > 0 ? 1 : 0;
}