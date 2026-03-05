cmake_minimum_required(VERSION 3.16)
project(CodeSmellDetector LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ── Shared source files (used by main binary and all tests) ────────────────
set(CORE_SOURCES
    src/lexer/Lexer.cpp
    src/parser/ASTBuilder.cpp
)

# ── Main executable ────────────────────────────────────────────────────────
add_executable(smell-detector
    src/main.cpp
    ${CORE_SOURCES}
)
target_include_directories(smell-detector PRIVATE src)

# ── Test: Lexer ────────────────────────────────────────────────────────────
add_executable(test_lexer
    tests/test_lexer.cpp
    src/lexer/Lexer.cpp
)
target_include_directories(test_lexer PRIVATE src)

# ── Test: ASTBuilder ──────────────────────────────────────────────────────
add_executable(test_ast_builder
    tests/test_ast_builder.cpp
    ${CORE_SOURCES}
)
target_include_directories(test_ast_builder PRIVATE src)

# ── Test: All Detectors ───────────────────────────────────────────────────
add_executable(test_detectors
    tests/test_detectors.cpp
    ${CORE_SOURCES}
)
target_include_directories(test_detectors PRIVATE src)

# ── CTest integration ─────────────────────────────────────────────────────
enable_testing()
add_test(NAME LexerTests     COMMAND test_lexer)
add_test(NAME ASTTests       COMMAND test_ast_builder)
add_test(NAME DetectorTests  COMMAND test_detectors)

# ── GCC < 9 needs explicit filesystem linkage ─────────────────────────────
foreach(target smell-detector test_ast_builder test_detectors)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND
        CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
        target_link_libraries(${target} stdc++fs)
    endif()
endforeach()

# ── Install ────────────────────────────────────────────────────────────────
install(TARGETS smell-detector DESTINATION bin)