# 🔍 Code Smell Detector

> A developer tooling system written in C++ that parses source code and automatically detects common code smells in test environments — helping teams catch design problems before they become technical debt.

---

## Table of Contents

1. [What Is This?](#what-is-this)
2. [Why It Exists](#why-it-exists)
3. [What It Can Detect](#what-it-can-detect)
4. [How It Works — The Big Picture](#how-it-works--the-big-picture)
5. [Project Structure](#project-structure)
6. [Getting Started](#getting-started)
7. [Configuration](#configuration)
8. [Running the Detector](#running-the-detector)
9. [Understanding the Output](#understanding-the-output)
10. [Extending the System](#extending-the-system)
11. [Design Philosophy](#design-philosophy)
12. [Glossary](#glossary)
13. [Contributing](#contributing)
14. [License](#license)

---

## What Is This?

The **Code Smell Detector** is a static analysis tool — meaning it reads your source code *without running it* and looks for patterns that suggest poor design, hidden complexity, or maintenance hazards. Think of it as a code reviewer that never sleeps.

It is written entirely in **C++** using **Object-Oriented Programming (OOP)** principles. Every major concept in the system — from individual detectors to the reporting engine — is its own class, making the codebase itself a clean example of the patterns it helps you identify in others' code.

This tool is specifically designed for use in **test environments** and **CI pipelines**, where catching problems early is far cheaper than finding them in production.

---

## Why It Exists

Every codebase accumulates imperfection over time. Functions grow too long. Classes take on too many responsibilities. Numbers appear with no explanation. Blocks of code get copied and pasted instead of refactored. These patterns — called **code smells** — are not bugs. Your program still runs. But they are warning signs: places where the code will become harder to understand, harder to change, and more likely to break.

Most teams know these smells exist. Few have a reliable, automated way to catch them during development. This tool fills that gap. It plugs into your workflow and gives you a consistent, objective report on where your codebase's design is starting to strain.

---

## What It Can Detect

The system ships with seven built-in detectors, each targeting a specific code smell:

### 🔶 Long Function
A function that has grown too large is doing too much. Long functions are harder to read, harder to test, and harder to change safely. This detector flags any function whose body exceeds a configurable line threshold (default: **50 lines**).

**What to do:** Break the function into smaller, well-named helpers. Each function should do one thing clearly.

---

### 🔶 Long Parameter List
A function that takes many parameters is usually a sign that it knows too much about its callers, or that it should be receiving a structured object instead of raw values. This detector flags functions with more parameters than a configurable threshold (default: **4 parameters**).

**What to do:** Group related parameters into a struct or class. Consider whether some parameters are better held in object state.

---

### 🔶 God Class
A class that has too many methods and too many fields is trying to be everything at once. These "god classes" become magnets for unrelated logic and create tight coupling throughout the codebase. The detector flags classes exceeding configurable method and field count thresholds.

**What to do:** Identify the distinct responsibilities the class is handling and extract each into its own dedicated class.

---

### 🔶 Duplicate Code
Copied code is a maintenance trap. When the same logic exists in two places, any future change needs to happen twice — and it is easy to forget the second location. This detector uses a rolling-hash comparison to find blocks of code with high structural similarity across functions and files.

**What to do:** Extract the shared logic into a shared function or base class. Write it once and reuse it.

---

### 🔶 Dead Code
Code that is never called, never referenced, and never reached is noise. It increases cognitive load for readers, inflates build times, and sometimes masks old bugs. The detector builds a symbol reference graph and flags any function or class that is defined but never used.

**What to do:** Delete it confidently. If it might be needed later, that is what version control is for.

---

### 🔶 Magic Numbers
A number like `86400` embedded directly in an expression carries no meaning on its own. Is it a timeout? A daily limit? The seconds in a day? Magic numbers force readers to guess. This detector flags numeric literals that appear outside of named constants or `#define` directives.

**What to do:** Replace the literal with a named constant: `const int SECONDS_PER_DAY = 86400;`. Now the intent is self-documenting.

---

### 🔶 Deep Nesting
Code that is indented five or six levels deep is a sign of logic that has grown organically without refactoring. Deep nesting makes it very difficult to trace the flow of execution and to understand what conditions are in effect at any given point. This detector tracks brace depth during AST traversal and flags blocks that exceed a configurable threshold (default: **4 levels**).

**What to do:** Flatten the logic using early returns, guard clauses, or by extracting inner blocks into named functions.

---

## How It Works — The Big Picture

When you point the tool at a source file, it goes through five stages:

```
Your Source File
      │
      ▼
 [ 1. Lexer ]         Reads raw text → breaks it into tokens (keywords, names, symbols)
      │
      ▼
 [ 2. AST Builder ]   Assembles tokens into a tree representing the code's structure
      │
      ▼
 [ 3. Detectors ]     Seven analyzers independently walk the tree looking for smells
      │
      ▼
 [ 4. Report ]        All findings are collected, sorted, and counted
      │
      ▼
 [ 5. Formatter ]     Results are rendered to your terminal, a JSON file, or an HTML report
```

The key component is the **Abstract Syntax Tree (AST)** — a structured, hierarchical representation of your code. Rather than working with raw text, every detector works with this tree. That makes each detector precise, readable, and easy to maintain.

---

## Project Structure

```
code-smell-detector/
│
├── src/
│   ├── main.cpp                  # Entry point — wires everything together
│   │
│   ├── lexer/
│   │   ├── Lexer.h / Lexer.cpp   # Tokenizer — reads source text into token stream
│   │   └── Token.h               # Token data type (type, value, line, column)
│   │
│   ├── parser/
│   │   ├── ASTNode.h             # Abstract base class for all AST nodes
│   │   ├── FunctionNode.h/.cpp   # Represents a function definition
│   │   ├── ClassNode.h/.cpp      # Represents a class definition
│   │   ├── BlockNode.h/.cpp      # Represents a scoped block { ... }
│   │   ├── ExpressionNode.h/.cpp # Represents expressions and literals
│   │   └── ASTBuilder.h/.cpp     # Recursive-descent parser → builds the tree
│   │
│   ├── detectors/
│   │   ├── SmellDetector.h       # Abstract base class — all detectors extend this
│   │   ├── LongFunctionDetector.h/.cpp
│   │   ├── LongParameterListDetector.h/.cpp
│   │   ├── GodClassDetector.h/.cpp
│   │   ├── DuplicateCodeDetector.h/.cpp
│   │   ├── DeadCodeDetector.h/.cpp
│   │   ├── MagicNumberDetector.h/.cpp
│   │   └── DeepNestingDetector.h/.cpp
│   │
│   ├── engine/
│   │   ├── DetectionEngine.h/.cpp  # Manages all detectors; runs them against the AST
│   │   └── DetectorFactory.h/.cpp  # Creates detector instances from config
│   │
│   ├── report/
│   │   ├── Violation.h           # Data model for a single finding
│   │   ├── Report.h/.cpp         # Aggregates all violations into one object
│   │   ├── ReportFormatter.h     # Abstract base for output formatters
│   │   ├── ConsoleFormatter.h/.cpp
│   │   ├── JSONFormatter.h/.cpp
│   │   └── HTMLFormatter.h/.cpp
│   │
│   └── config/
│       └── DetectorConfig.h/.cpp # Loads and validates threshold settings
│
├── tests/
│   ├── test_lexer.cpp
│   ├── test_ast_builder.cpp
│   ├── test_detectors.cpp
│   └── fixtures/                 # Sample .cpp files with known smells for testing
│
├── .smellrc                      # Default configuration file
├── CMakeLists.txt                # Build configuration
└── README.md                     # This file
```

---

## Getting Started

### Prerequisites

Before building the project, make sure you have the following installed:

| Tool | Minimum Version | Purpose |
|------|----------------|---------|
| C++ Compiler (GCC or Clang) | C++17 | Compiling the source code |
| CMake | 3.16 | Build system generator |
| Make / Ninja | Any | Executing the build |

### Building from Source

Clone the repository and build using CMake:

```bash
# Clone the repository
git clone https://github.com/your-org/code-smell-detector.git
cd code-smell-detector

# Create a build directory (keeps things clean)
mkdir build && cd build

# Generate build files
cmake ..

# Compile
make -j4
```

If the build succeeds, you will find the executable at `build/smell-detector`.

### Running a Quick Test

To make sure everything is working, run the detector against one of the included fixture files:

```bash
./smell-detector ../tests/fixtures/sample_smelly.cpp
```

You should see a report in your terminal listing the smells found in that file.

---

## Configuration

The detector reads its settings from a `.smellrc` file in the directory where you run it. If no file is found, it falls back to sensible defaults.

### Example `.smellrc`

```ini
# Thresholds for each detector
max_function_lines    = 50
max_parameters        = 4
max_class_methods     = 20
max_class_fields      = 15
max_nesting_depth     = 4
duplicate_threshold   = 0.85

# Detectors to enable (comment out to disable)
enable_long_function        = true
enable_long_parameter_list  = true
enable_god_class            = true
enable_duplicate_code       = true
enable_dead_code            = true
enable_magic_numbers        = true
enable_deep_nesting         = true

# Numbers that are allowed as literals (won't be flagged as magic)
allowed_literals = 0, 1, -1, 2

# Output format: console | json | html
output_format = console

# Minimum severity to include in report: low | medium | high
min_severity = low
```

### Configuration Keys Explained

| Key | What It Controls | Default |
|-----|-----------------|---------|
| `max_function_lines` | Lines of code before a function is considered too long | 50 |
| `max_parameters` | Parameters before a function signature is flagged | 4 |
| `max_class_methods` | Method count that triggers the God Class detector | 20 |
| `max_class_fields` | Field count that triggers the God Class detector | 15 |
| `max_nesting_depth` | Brace levels before deep nesting is flagged | 4 |
| `duplicate_threshold` | Similarity ratio (0.0–1.0) for duplicate block detection | 0.85 |
| `allowed_literals` | Numeric literals that are always acceptable | 0, 1, -1 |
| `output_format` | Where and how results are written | console |
| `min_severity` | Filters out findings below this severity level | low |

---

## Running the Detector

### Basic Usage

```bash
# Analyze a single file
./smell-detector path/to/your/file.cpp

# Analyze all .cpp files in a directory
./smell-detector path/to/your/src/

# Use a custom config file
./smell-detector --config my_config.smellrc src/

# Output a JSON report
./smell-detector --format json --output report.json src/

# Only show high-severity findings
./smell-detector --min-severity high src/
```

### Command-Line Flags

| Flag | Description |
|------|-------------|
| `--config <path>` | Path to a `.smellrc` configuration file |
| `--format <type>` | Output format: `console`, `json`, or `html` |
| `--output <path>` | Write output to a file instead of the terminal |
| `--min-severity <level>` | Only report findings at this severity or above |
| `--disable <detector>` | Disable a specific detector by name |
| `--help` | Show usage information |

### Using in a CI Pipeline

Add this to your CI configuration (GitHub Actions example):

```yaml
- name: Run Code Smell Detector
  run: |
    ./smell-detector --format json --output smell-report.json src/
    # Fail the build if high-severity smells are found
    ./smell-detector --min-severity high --exit-code src/
```

The `--exit-code` flag causes the process to exit with a non-zero code if any violations at or above the minimum severity are found — which will fail most CI pipelines automatically.

---

## Understanding the Output

### Console Output

```
CODE SMELL REPORT
═══════════════════════════════════════════════════════
File: src/UserManager.cpp

  [HIGH]   Line  42  │ Long Function
                     │ Function 'processUserData' is 87 lines long (max: 50)

  [MEDIUM] Line  61  │ Long Parameter List
                     │ Function 'createSession' has 7 parameters (max: 4)

  [HIGH]   Line  12  │ God Class
                     │ Class 'UserManager' has 34 methods and 18 fields

───────────────────────────────────────────────────────
Summary: 3 violations found across 1 file
  High: 2   Medium: 1   Low: 0
═══════════════════════════════════════════════════════
```

### Reading a Violation

Each violation tells you:
- **Severity** — how urgent the finding is (High / Medium / Low)
- **Line number** — exactly where in the file the smell occurs
- **Smell type** — which category of code smell was detected
- **Message** — a plain-English explanation of what was found and why it was flagged

### JSON Output

The JSON format is designed for tool integration — feeding into dashboards, trend trackers, or custom scripts:

```json
{
  "summary": {
    "total_violations": 3,
    "files_analyzed": 1,
    "high": 2,
    "medium": 1,
    "low": 0
  },
  "violations": [
    {
      "file": "src/UserManager.cpp",
      "line": 42,
      "smell": "LongFunction",
      "severity": "HIGH",
      "message": "Function 'processUserData' is 87 lines long (max: 50)"
    }
  ]
}
```

---

## Extending the System

One of the primary goals of this project's design is that adding a new detector should be straightforward and should not require modifying any existing code. Here is how to add your own:

### Step 1 — Create your detector class

Create `src/detectors/MyNewDetector.h` extending `SmellDetector`:

```cpp
#pragma once
#include "SmellDetector.h"

class MyNewDetector : public SmellDetector {
public:
    MyNewDetector(const DetectorConfig& config);
    void analyze(ASTNode* root) override;

private:
    void visit(FunctionNode* node);
};
```

### Step 2 — Implement the analysis logic

In `MyNewDetector.cpp`, implement your `analyze()` method. Walk the AST and call `addViolation()` whenever you find a problem:

```cpp
void MyNewDetector::analyze(ASTNode* root) {
    // Walk the AST — visit each function node
    for (auto* fn : root->getFunctions()) {
        if (/* your condition */) {
            addViolation(fn->getLocation(), "Description of the problem found");
        }
    }
}
```

### Step 3 — Register it in the factory

In `DetectorFactory.cpp`, add your detector to the factory map:

```cpp
if (name == "my_new_smell") return new MyNewDetector(config);
```

### Step 4 — Add a config key (optional)

Add a threshold key to `DetectorConfig` and `.smellrc` if your detector needs tunable parameters.

That is all. Your detector will now run alongside the built-in ones, appear in all report formats, and respect the severity filter and config system automatically.

---

## Design Philosophy

This project is guided by a few core ideas that shaped every decision:

**One class, one job.** Every class in this codebase has a single, clearly defined responsibility. `Lexer` tokenizes. `ASTBuilder` parses. `SmellDetector` detects. `ReportFormatter` formats. Nothing bleeds into anything else.

**Code to interfaces, not implementations.** The `DetectionEngine` does not know or care which specific detectors it is running. The `Report` does not know which formatter will consume it. This indirection is what makes the system easy to extend without touching existing code.

**Configuration over hardcoding.** Every threshold that matters can be tuned. Teams have different standards, codebases have different contexts, and the tool respects that.

**The output should be actionable.** A violation report that says "code smell detected" is useless. Every finding in this system includes the exact location, a plain-language explanation of what was found, and enough context to act on it immediately.

**Test the detector with known smells.** The `tests/fixtures/` directory contains deliberately written bad code — files with known smells at known line numbers. Every detector has tests that verify it catches exactly what it should and ignores what it should not.

---

## Glossary

| Term | Plain-English Meaning |
|------|-----------------------|
| **Abstract Syntax Tree (AST)** | A tree-shaped data structure that represents your code's structure — functions, classes, blocks, and expressions as nodes in a hierarchy rather than raw text |
| **Code Smell** | A pattern in source code that suggests a design problem. Not a bug — the code may run fine — but a sign that something will be harder to maintain, understand, or change |
| **Static Analysis** | Analyzing code without running it. The tool reads the source files directly, the same way a human reviewer would |
| **Lexer / Tokenizer** | The component that reads raw source text and splits it into meaningful units (tokens) like keywords, identifiers, and operators |
| **Token** | A single meaningful unit of source code — for example, `void`, `myFunction`, `(`, `42`, or `}` |
| **Visitor Pattern** | A design pattern where an operation (like "check for deep nesting") is separated from the objects it operates on (AST nodes). Each detector is a Visitor that walks the tree |
| **Factory Pattern** | A design pattern for creating objects. Instead of writing `new LongFunctionDetector()` everywhere, you ask a factory to create it by name |
| **Violation** | A single instance of a detected code smell — one finding at one location in one file |
| **Severity** | A label (High / Medium / Low) indicating how urgently a violation should be addressed |
| **CI Pipeline** | A Continuous Integration pipeline — an automated system that runs checks (like this tool) on your code every time it is pushed or a pull request is opened |

---

## Contributing

Contributions are welcome. If you want to add a new detector, improve an existing one, or fix a bug:

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/my-new-detector`
3. Write your code and add tests in `tests/`
4. Make sure all existing tests still pass: `cd build && ctest`
5. Open a pull request with a clear description of what you added and why

Please keep the same design conventions used throughout the project — one class per file, descriptive names, and no magic numbers.

---

## License

This project is licensed under the MIT License. See `LICENSE` for details.

---

*Built to make codebases easier to understand — one smell at a time.*