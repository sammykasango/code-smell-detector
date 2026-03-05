# How to Run the Code Smell Detector

A complete step-by-step guide from cloning to reading your first report.

---

## Step 1 — Get the project

```bash
git clone https://github.com/your-org/code-smell-detector.git
cd code-smell-detector
```

Or if you already have the folder just `cd` into it:

```bash
cd code-smell-detector-split
```

---

## Step 2 — Build the binary

You need **g++ with C++17 support** (GCC 7+ or Clang 5+). No other dependencies.

```bash
# Create a build folder to keep things clean
mkdir -p build

# Compile everything in one command
g++ -std=c++17 -Wall -O2 -Isrc \
    src/main.cpp \
    src/lexer/Lexer.cpp \
    src/parser/ASTBuilder.cpp \
    -o build/smell-detector
```

If the build succeeds you will see no output and the binary will be at `build/smell-detector`.

> **Using CMake instead?**
> ```bash
> mkdir -p build && cd build
> cmake ..
> make -j4
> cd ..
> ```

---

## Step 3 — Add your sample .cpp file

Drop any `.cpp` file you want to analyze into the project. For example:

```bash
# Put your file anywhere — project root, a subfolder, wherever makes sense
cp /path/to/your/MyCode.cpp .

# Or create a quick sample right now to try things out
cat > my_sample.cpp << 'EOF'
#include <iostream>
#include <string>

// A class with too many responsibilities
class OrderProcessor {
public:
    std::string customerName;
    std::string customerEmail;
    std::string shippingAddress;
    std::string billingAddress;
    std::string paymentMethod;
    double      orderTotal;
    int         itemCount;
    bool        isPriority;
    bool        isGift;
    std::string giftMessage;
    int         loyaltyPoints;
    std::string couponCode;
    double      discountAmount;
    bool        requiresSignature;
    std::string trackingNumber;
    int         estimatedDays;

    void processPayment()   {}
    void validateAddress()  {}
    void applyDiscount()    {}
    void sendConfirmation() {}
    void notifyWarehouse()  {}
    void updateInventory()  {}
    void generateInvoice()  {}
    void scheduleDelivery() {}
    void applyLoyaltyPoints() {}
    void checkFraud()       {}
    void logTransaction()   {}
    void notifyCustomer()   {}
    void handleRefund()     {}
    void archiveOrder()     {}
    void generateReport()   {}
    void syncWithCRM()      {}
    void updateDashboard()  {}
    void sendSMSAlert()     {}
    void applyTax()         {}
    void checkStock()       {}
    void reserveStock()     {}
    void releaseStock()     {}
};

// A function that is way too long
int calculateShipping(int weight) {
    std::cout << "Step 1\n";
    std::cout << "Step 2\n";
    std::cout << "Step 3\n";
    std::cout << "Step 4\n";
    std::cout << "Step 5\n";
    std::cout << "Step 6\n";
    std::cout << "Step 7\n";
    std::cout << "Step 8\n";
    std::cout << "Step 9\n";
    std::cout << "Step 10\n";
    std::cout << "Step 11\n";
    std::cout << "Step 12\n";
    std::cout << "Step 13\n";
    std::cout << "Step 14\n";
    std::cout << "Step 15\n";
    std::cout << "Step 16\n";
    std::cout << "Step 17\n";
    std::cout << "Step 18\n";
    std::cout << "Step 19\n";
    std::cout << "Step 20\n";
    return weight * 350;
}

// Magic numbers with no names
double applyDiscount(double price) {
    if (price > 100) {
        return price * 0.85;
    }
    return price * 0.95;
}

// Deep nesting
bool checkEligibility(bool isLoggedIn, bool hasPlan, bool isActive, bool isVerified) {
    if (isLoggedIn) {
        if (hasPlan) {
            if (isActive) {
                if (isVerified) {
                    if (true) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

int main() {
    OrderProcessor op;
    op.processPayment();
    int cost = calculateShipping(10);
    double price = applyDiscount(120.0);
    bool ok = checkEligibility(true, true, true, true);
    std::cout << cost << price << ok << "\n";
    return 0;
}
EOF
```

---

## Step 4 — Run the detector

### Analyze a single file (console output)

```bash
./build/smell-detector my_sample.cpp
```

You will see a formatted report printed directly to your terminal:

```
=================================================================
  CODE SMELL REPORT
=================================================================

  File: my_sample.cpp

  [HIGH  ] Line 4     | GodClass
               | Class 'OrderProcessor' has 22 methods and 16 fields — likely a God Class

  [HIGH  ] Line 48    | LongFunction
               | Function 'calculateShipping' is 24 lines long (max: 50)

  [LOW   ] Line 66    | MagicNumber
               | Magic number '0.85' — replace with a named constant
  ...

-----------------------------------------------------------------
  Summary: N violations found across 1 file(s)
=================================================================
```

### Analyze an entire directory

```bash
./build/smell-detector src/
```

Every `.cpp`, `.h`, `.hpp`, and `.cc` file in the directory (and subdirectories) will be analyzed.

---

## Step 5 — Choose your output format

### Save as JSON (great for CI pipelines or scripting)

```bash
./build/smell-detector --format json --output report.json my_sample.cpp
```

`report.json` will contain a structured breakdown of every violation — file, line, smell type, severity, and message.

### Save as HTML (open in a browser)

```bash
./build/smell-detector --format html --output report.html my_sample.cpp

# Then open it
open report.html        # macOS
xdg-open report.html    # Linux
start report.html       # Windows
```

---

## Step 6 — Filter by severity

Only show problems that matter most:

```bash
# Only HIGH severity violations
./build/smell-detector --min-severity high my_sample.cpp

# HIGH and MEDIUM
./build/smell-detector --min-severity medium my_sample.cpp

# Everything (default)
./build/smell-detector --min-severity low my_sample.cpp
```

---

## Step 7 — Tune the thresholds

Create or edit `.smellrc` in the directory where you run the tool:

```ini
# .smellrc — adjust these to match your team's standards

max_function_lines    = 40    # flag functions longer than 40 lines
max_parameters        = 3     # flag functions with more than 3 params
max_class_methods     = 15    # flag classes with more than 15 methods
max_class_fields      = 10    # flag classes with more than 10 fields
max_nesting_depth     = 3     # flag nesting deeper than 3 levels

# Numbers that are always fine as literals
allowed_literals = 0, 1, -1, 2, 100

# Turn off detectors you don't need
enable_dead_code      = false
enable_magic_numbers  = true
```

The tool will auto-load `.smellrc` from the current directory each run.

---

## Step 8 — Disable specific detectors on the fly

```bash
# Skip dead code and magic number checks for this run
./build/smell-detector \
    --disable dead_code \
    --disable magic_numbers \
    my_sample.cpp
```

Available detector names: `long_function`, `long_parameter_list`, `god_class`,
`duplicate_code`, `dead_code`, `magic_numbers`, `deep_nesting`

---

## Step 9 — Use in a CI pipeline

Add this to your GitHub Actions workflow (or any CI system):

```yaml
- name: Build smell detector
  run: |
    g++ -std=c++17 -O2 -Isrc \
        src/main.cpp src/lexer/Lexer.cpp src/parser/ASTBuilder.cpp \
        -o build/smell-detector

- name: Run smell detector
  run: |
    ./build/smell-detector \
        --min-severity high \
        --format json \
        --output smell-report.json \
        --exit-code \
        src/

- name: Upload smell report
  uses: actions/upload-artifact@v3
  with:
    name: smell-report
    path: smell-report.json
```

The `--exit-code` flag makes the process exit with code `1` if any violations at or above
`--min-severity` are found — which automatically fails the CI build.

---

## Step 10 — Run the tests

To verify your build is working correctly:

```bash
# Build the test binaries
g++ -std=c++17 -O2 -Isrc tests/test_lexer.cpp \
    src/lexer/Lexer.cpp \
    -o build/test_lexer

g++ -std=c++17 -O2 -Isrc tests/test_ast_builder.cpp \
    src/lexer/Lexer.cpp src/parser/ASTBuilder.cpp \
    -o build/test_ast

g++ -std=c++17 -O2 -Isrc tests/test_detectors.cpp \
    src/lexer/Lexer.cpp src/parser/ASTBuilder.cpp \
    -o build/test_detectors

# Run them
./build/test_lexer
./build/test_ast
./build/test_detectors
```

All three should print `0 failed` at the end.

---

## Quick reference card

```
ANALYZE A FILE         ./build/smell-detector myfile.cpp
ANALYZE A DIRECTORY    ./build/smell-detector src/
JSON OUTPUT            ./build/smell-detector --format json --output out.json src/
HTML OUTPUT            ./build/smell-detector --format html --output out.html src/
HIGH ONLY              ./build/smell-detector --min-severity high src/
DISABLE A DETECTOR     ./build/smell-detector --disable dead_code src/
CI MODE (fail on hit)  ./build/smell-detector --min-severity high --exit-code src/
CUSTOM CONFIG          ./build/smell-detector --config my_project.smellrc src/
SHOW HELP              ./build/smell-detector --help
```

---

## Troubleshooting

**"cannot open file" error**
Make sure the path you passed actually exists and you have read permission on it.

**Binary not found**
Run the build command in Step 2 first. The binary is created at `build/smell-detector`.

**No violations reported on a file you expect to be flagged**
Check your `.smellrc` thresholds — they may be set higher than the smell in your file.
Try running with `--min-severity low` and `--disable` nothing to see everything.

**Unexpected results on a large codebase**
The tool analyzes one file at a time. References across files are not resolved,
so Dead Code detection works best within a single file or small module.