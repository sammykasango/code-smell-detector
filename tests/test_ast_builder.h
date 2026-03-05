// tests/test_ast_builder.cpp
// Unit tests for the ASTBuilder module.
// Run: g++ -std=c++17 -Isrc tests/test_ast_builder.cpp src/lexer/Lexer.cpp src/parser/ASTBuilder.cpp -o test_ast && ./test_ast

#include <iostream>
#include <cassert>
#include <functional>
#include "../src/lexer/Lexer.h"
#include "../src/parser/ASTBuilder.h"

static int passed = 0, failed = 0;

#define CHECK(cond, label) \
    if (cond) { ++passed; std::cout << "  [PASS] " << label << "\n"; } \
    else       { ++failed; std::cout << "  [FAIL] " << label << "\n"; }

// ── Helper: parse source → TranslationUnitNode ────────────────────────────
static std::unique_ptr<TranslationUnitNode> parse(const std::string& src) {
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    ASTBuilder builder(tokens, "test.cpp");
    return builder.build();
}

// ── Test cases ─────────────────────────────────────────────────────────────
void test_empty_source() {
    auto unit = parse("");
    CHECK(unit != nullptr,            "empty source: unit not null");
    CHECK(unit->classes.empty(),      "empty source: no classes");
    CHECK(unit->functions.empty(),    "empty source: no functions");
}

void test_free_function_detected() {
    auto unit = parse("int add(int a, int b) { return a + b; }");
    CHECK(unit->functions.size() == 1,          "one free function");
    CHECK(unit->functions[0]->name == "add",    "function named add");
    CHECK(unit->functions[0]->returnType == "int", "return type int");
}

void test_function_param_count() {
    auto unit = parse("void foo(int a, int b, int c) {}");
    CHECK(!unit->functions.empty(),                 "function parsed");
    CHECK(unit->functions[0]->paramCount() == 3,    "3 parameters");
    CHECK(unit->functions[0]->parameters[0].name == "a", "first param: a");
    CHECK(unit->functions[0]->parameters[2].name == "c", "third param: c");
}

void test_void_no_params() {
    auto unit = parse("void doNothing() {}");
    CHECK(unit->functions.size() == 1,           "one function");
    CHECK(unit->functions[0]->paramCount() == 0, "zero parameters");
    CHECK(unit->functions[0]->name == "doNothing", "named doNothing");
}

void test_class_detected() {
    auto unit = parse("class Dog { public: void bark() {} };");
    CHECK(unit->classes.size() == 1,          "one class");
    CHECK(unit->classes[0]->name == "Dog",    "class named Dog");
}

void test_class_method_count() {
    auto unit = parse(R"(
        class Shape {
        public:
            void draw() {}
            void resize() {}
            int area() { return 0; }
        };
    )");
    CHECK(!unit->classes.empty(),                      "class parsed");
    CHECK(unit->classes[0]->methodCount() == 3,        "3 methods");
}

void test_class_field_count() {
    auto unit = parse(R"(
        class Point {
        public:
            int x;
            int y;
            int z;
        };
    )");
    CHECK(!unit->classes.empty(),                   "class parsed");
    CHECK(unit->classes[0]->fieldCount() == 3,      "3 fields");
}

void test_function_line_count() {
    // A function with a body of several lines
    auto unit = parse(R"(
int multiLine() {
    int a = 1;
    int b = 2;
    int c = 3;
    return a + b + c;
}
    )");
    CHECK(!unit->functions.empty(),               "function parsed");
    CHECK(unit->functions[0]->lineCount() > 3,    "body has multiple lines");
}

void test_nested_block_depth() {
    auto unit = parse(R"(
void deep() {
    if (true) {
        for (int i = 0; i < 10; i++) {
            while (true) {
                int x = 1;
            }
        }
    }
}
    )");
    CHECK(!unit->functions.empty(), "function parsed");
    // Walk body to find deepest block
    int maxDepth = 0;
    std::function<void(BlockNode*)> walk = [&](BlockNode* b) {
        if (!b) return;
        if (b->nestingDepth > maxDepth) maxDepth = b->nestingDepth;
        for (auto& s : b->statements) {
            if (auto* blk = dynamic_cast<BlockNode*>(s.get())) walk(blk);
        }
    };
    walk(unit->functions[0]->body.get());
    CHECK(maxDepth >= 3, "nesting depth reaches at least 3");
}

void test_class_with_base() {
    auto unit = parse("class Cat : public Animal { public: void meow() {} };");
    CHECK(!unit->classes.empty(),                          "class parsed");
    CHECK(!unit->classes[0]->baseClasses.empty(),          "has base class");
    CHECK(unit->classes[0]->baseClasses[0] == "Animal",    "base is Animal");
}

void test_translation_unit_file_path() {
    Lexer lexer("int x;");
    auto tokens = lexer.tokenize();
    ASTBuilder builder(tokens, "src/myfile.cpp");
    auto unit = builder.build();
    CHECK(unit->filePath == "src/myfile.cpp", "file path stored in unit");
}

// ── Main ───────────────────────────────────────────────────────────────────
int main() {
    std::cout << "\n=== ASTBuilder Tests ===\n\n";
    test_empty_source();
    test_free_function_detected();
    test_function_param_count();
    test_void_no_params();
    test_class_detected();
    test_class_method_count();
    test_class_field_count();
    test_function_line_count();
    test_nested_block_depth();
    test_class_with_base();
    test_translation_unit_file_path();

    std::cout << "\n--- Results: " << passed << " passed, " << failed << " failed ---\n\n";
    return failed > 0 ? 1 : 0;
}