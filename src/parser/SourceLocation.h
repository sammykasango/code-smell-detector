#pragma once
#include <string>

// ── Source location ────────────────────────────────────────────────────────
struct SourceLocation {
    std::string file;
    int         line = 0;
    int         col  = 0;

    std::string toString() const {
        return file + ":" + std::to_string(line) + ":" + std::to_string(col);
    }
};

// ── Node kinds ─────────────────────────────────────────────────────────────
enum class NodeKind {
    TranslationUnit,
    Function,
    Class,
    Block,
    Expression,
    Parameter,
    Field,
    Variable
};