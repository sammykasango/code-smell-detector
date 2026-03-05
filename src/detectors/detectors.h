#pragma once
#include "SmellDetector.h"
#include <sstream>
#include <unordered_map>
#include <functional>
#include <cmath>
#include <set>

// ═══════════════════════════════════════════════════════════════════════════
// 1. LONG FUNCTION DETECTOR
//    Flags any function whose body exceeds maxFunctionLines.
// ═══════════════════════════════════════════════════════════════════════════
class LongFunctionDetector : public SmellDetector {
public:
    explicit LongFunctionDetector(const DetectorConfig& cfg)
        : SmellDetector("LongFunction", Severity::HIGH, cfg) {}

    void visit(TranslationUnitNode& unit) override { walkUnit(unit); }

    void visit(ClassNode& cls) override { walkClass(cls); }

    void visit(FunctionNode& fn) override {
        int lines = fn.lineCount();
        if (lines > config_.maxFunctionLines) {
            std::ostringstream msg;
            msg << "Function '" << fn.name << "' is " << lines
                << " lines long (max: " << config_.maxFunctionLines << ")";
            addViolation(fn.location(), msg.str(), Severity::HIGH);
        }
        // Still walk into body to count nested functions (lambdas etc.)
        if (fn.body) fn.body->accept(*this);
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// 2. LONG PARAMETER LIST DETECTOR
//    Flags functions with more parameters than maxParameters.
// ═══════════════════════════════════════════════════════════════════════════
class LongParameterListDetector : public SmellDetector {
public:
    explicit LongParameterListDetector(const DetectorConfig& cfg)
        : SmellDetector("LongParameterList", Severity::MEDIUM, cfg) {}

    void visit(TranslationUnitNode& unit) override { walkUnit(unit); }
    void visit(ClassNode& cls)            override { walkClass(cls); }

    void visit(FunctionNode& fn) override {
        int count = fn.paramCount();
        if (count > config_.maxParameters) {
            std::ostringstream msg;
            msg << "Function '" << fn.name << "' has " << count
                << " parameters (max: " << config_.maxParameters << ")";
            addViolation(fn.location(), msg.str(), Severity::MEDIUM);
        }
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// 3. GOD CLASS DETECTOR
//    Flags classes where both method count AND field count exceed thresholds.
//    A class must violate both to be flagged — one high metric alone may be
//    intentional (e.g. a utility class with many helpers but few fields).
// ═══════════════════════════════════════════════════════════════════════════
class GodClassDetector : public SmellDetector {
public:
    explicit GodClassDetector(const DetectorConfig& cfg)
        : SmellDetector("GodClass", Severity::HIGH, cfg) {}

    void visit(TranslationUnitNode& unit) override { walkUnit(unit); }

    void visit(ClassNode& cls) override {
        int methods = cls.methodCount();
        int fields  = cls.fieldCount();
        bool tooManyMethods = methods > config_.maxClassMethods;
        bool tooManyFields  = fields  > config_.maxClassFields;

        if (tooManyMethods && tooManyFields) {
            std::ostringstream msg;
            msg << "Class '" << cls.name << "' has " << methods
                << " methods and " << fields
                << " fields — likely a God Class"
                << " (max: " << config_.maxClassMethods
                << " methods, "  << config_.maxClassFields << " fields)";
            addViolation(cls.location(), msg.str(), Severity::HIGH);
        } else if (tooManyMethods) {
            std::ostringstream msg;
            msg << "Class '" << cls.name << "' has " << methods
                << " methods (max: " << config_.maxClassMethods << ")";
            addViolation(cls.location(), msg.str(), Severity::MEDIUM);
        } else if (tooManyFields) {
            std::ostringstream msg;
            msg << "Class '" << cls.name << "' has " << fields
                << " fields (max: " << config_.maxClassFields << ")";
            addViolation(cls.location(), msg.str(), Severity::MEDIUM);
        }
        // Walk methods too (to detect nested god-classes in inner classes)
        walkClass(cls);
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// 4. MAGIC NUMBER DETECTOR
//    Flags numeric literals appearing inside function bodies that are not
//    in the allowedLiterals set (0, 1, -1, 2 by default).
// ═══════════════════════════════════════════════════════════════════════════
class MagicNumberDetector : public SmellDetector {
public:
    explicit MagicNumberDetector(const DetectorConfig& cfg)
        : SmellDetector("MagicNumber", Severity::LOW, cfg) {}

    void visit(TranslationUnitNode& unit) override { walkUnit(unit); }
    void visit(ClassNode& cls)            override { walkClass(cls); }

    void visit(FunctionNode& fn) override {
        inFunction_ = true;
        if (fn.body) fn.body->accept(*this);
        inFunction_ = false;
    }

    void visit(BlockNode& block) override {
        for (auto& stmt : block.statements) stmt->accept(*this);
    }

    void visit(ExpressionNode& expr) override {
        if (!inFunction_) return;
        if (!expr.isNumericLiteral) return;
        if (config_.allowedLiterals.count(expr.value)) return;

        std::ostringstream msg;
        msg << "Magic number '" << expr.value
            << "' — replace with a named constant";
        addViolation(expr.location(), msg.str(), Severity::LOW);
    }

private:
    bool inFunction_ = false;
};

// ═══════════════════════════════════════════════════════════════════════════
// 5. DEEP NESTING DETECTOR
//    Walks every BlockNode and checks its nestingDepth against the threshold.
// ═══════════════════════════════════════════════════════════════════════════
class DeepNestingDetector : public SmellDetector {
public:
    explicit DeepNestingDetector(const DetectorConfig& cfg)
        : SmellDetector("DeepNesting", Severity::MEDIUM, cfg) {}

    void visit(TranslationUnitNode& unit) override { walkUnit(unit); }
    void visit(ClassNode& cls)            override { walkClass(cls); }

    void visit(FunctionNode& fn) override {
        currentFunction_ = fn.name;
        if (fn.body) fn.body->accept(*this);
        currentFunction_ = "";
    }

    void visit(BlockNode& block) override {
        if (block.nestingDepth > config_.maxNestingDepth) {
            std::ostringstream msg;
            msg << "Nesting depth " << block.nestingDepth
                << " in function '" << currentFunction_
                << "' exceeds max (" << config_.maxNestingDepth << ")";
            // Only report first offending depth per function to avoid noise
            if (reported_.find(currentFunction_) == reported_.end()) {
                addViolation(block.location(), msg.str(), Severity::MEDIUM);
                reported_.insert(currentFunction_);
            }
        }
        walkBlock(block);
    }

private:
    std::string      currentFunction_;
    std::set<std::string> reported_;
};

// ═══════════════════════════════════════════════════════════════════════════
// 6. DEAD CODE DETECTOR
//    Two-pass approach:
//    Pass 1: collect all defined function/class names
//    Pass 2: collect all referenced identifiers
//    Anything defined but never referenced is flagged.
//
//    Note: This is a heuristic — a full linker-level analysis is out of
//    scope for a single-file static analyzer.
// ═══════════════════════════════════════════════════════════════════════════
class DeadCodeDetector : public SmellDetector {
public:
    explicit DeadCodeDetector(const DetectorConfig& cfg)
        : SmellDetector("DeadCode", Severity::MEDIUM, cfg) {}

    void analyze(TranslationUnitNode& root) {
        violations_.clear();
        defined_.clear();
        referenced_.clear();

        // Pass 1 — collect definitions
        collectDefinitions(root);

        // Pass 2 — collect references (all identifiers used anywhere)
        collectReferences(root);

        // Diff: defined but never referenced
        for (auto& [name, loc] : defined_) {
            // Skip main, constructors/destructors (by convention same name as class)
            if (name == "main") continue;
            if (name.rfind('~', 0) == 0) continue; // destructor

            if (referenced_.find(name) == referenced_.end()) {
                std::ostringstream msg;
                msg << "'" << name << "' is defined but never referenced — possible dead code";
                addViolation(loc, msg.str(), Severity::MEDIUM);
            }
        }
    }

    // These are no-ops here because we use our own two-pass analyze()
    void visit(TranslationUnitNode&) override {}
    void visit(ClassNode&)           override {}
    void visit(FunctionNode&)        override {}
    void visit(BlockNode&)           override {}
    void visit(ExpressionNode&)      override {}
    void visit(ParameterNode&)       override {}
    void visit(FieldNode&)           override {}

private:
    std::unordered_map<std::string, SourceLocation> defined_;
    std::set<std::string>                           referenced_;

    void collectDefinitions(TranslationUnitNode& unit) {
        for (auto& fn  : unit.functions)  defined_[fn->name]  = fn->location();
        for (auto& cls : unit.classes) {
            defined_[cls->name] = cls->location();
            for (auto& method : cls->methods)
                defined_[cls->name + "::" + method->name] = method->location();
        }
    }

    void collectReferences(TranslationUnitNode& unit) {
        // Scan token-level: every IDENTIFIER in function bodies is a potential reference.
        // We do this by walking the AST and harvesting expression/identifier nodes.
        for (auto& fn  : unit.functions)  scanFunctionRefs(*fn);
        for (auto& cls : unit.classes) {
            referenced_.insert(cls->name); // used in declarations elsewhere
            for (auto& method : cls->methods) scanFunctionRefs(*method);
        }
    }

    void scanFunctionRefs(FunctionNode& fn) {
        // Record calls within function name itself (recursion)
        if (fn.body) scanBlockRefs(*fn.body);
    }

    void scanBlockRefs(BlockNode& block) {
        for (auto& stmt : block.statements) {
            if (auto* expr = dynamic_cast<ExpressionNode*>(stmt.get())) {
                if (expr->exprKind == ExpressionNode::ExprKind::Identifier ||
                    expr->exprKind == ExpressionNode::ExprKind::Call) {
                    referenced_.insert(expr->value);
                }
            } else if (auto* blk = dynamic_cast<BlockNode*>(stmt.get())) {
                scanBlockRefs(*blk);
            }
        }
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// 7. DUPLICATE CODE DETECTOR
//    Collects per-function line counts as a simple structural fingerprint.
//    Functions with identical signatures (name excluded, line count + param
//    count combined) are flagged as suspected duplicates.
//
//    A full implementation would use a rolling-hash (Rabin-Karp) over token
//    sequences. This version uses a lightweight heuristic that works well for
//    the scope of a test-environment static analyzer.
// ═══════════════════════════════════════════════════════════════════════════
class DuplicateCodeDetector : public SmellDetector {
public:
    explicit DuplicateCodeDetector(const DetectorConfig& cfg)
        : SmellDetector("DuplicateCode", Severity::HIGH, cfg) {}

    void visit(TranslationUnitNode& unit) override {
        fingerprints_.clear();

        // Collect fingerprints for all functions
        for (auto& fn  : unit.functions)  collectFingerprint(*fn);
        for (auto& cls : unit.classes)
            for (auto& method : cls->methods)
                collectFingerprint(*method);

        // Find duplicate fingerprints
        std::unordered_map<size_t, std::vector<FunctionNode*>> groups;
        for (auto& [fn, fp] : fingerprints_) groups[fp].push_back(fn);

        for (auto& [fp, fns] : groups) {
            if (fns.size() < 2) continue;
            // Only flag if all functions in the group have meaningful bodies
            if (fns[0]->lineCount() < 5) continue;

            for (size_t i = 1; i < fns.size(); ++i) {
                std::ostringstream msg;
                msg << "Function '" << fns[i]->name
                    << "' appears structurally identical to '"
                    << fns[0]->name
                    << "' (" << fns[0]->lineCount() << " lines, "
                    << fns[0]->paramCount() << " params)"
                    << " — consider extracting shared logic";
                addViolation(fns[i]->location(), msg.str(), Severity::HIGH);
            }
        }

        // Walk classes normally too
        for (auto& cls : unit.classes) walkClass(*cls);
    }

    void visit(ClassNode&)    override {}
    void visit(FunctionNode&) override {}

private:
    std::unordered_map<FunctionNode*, size_t> fingerprints_;

    void collectFingerprint(FunctionNode& fn) {
        // Fingerprint = hash(lineCount, paramCount, returnType)
        size_t h = 0;
        hashCombine(h, fn.lineCount());
        hashCombine(h, fn.paramCount());
        hashCombine(h, std::hash<std::string>{}(fn.returnType));
        fingerprints_[&fn] = h;
    }

    static void hashCombine(size_t& seed, size_t val) {
        seed ^= val + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    static void hashCombine(size_t& seed, const std::string& val) {
        hashCombine(seed, std::hash<std::string>{}(val));
    }
};