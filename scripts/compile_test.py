#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
scripts/compile_test.py
=======================
Compiles and runs a smoke test using dist/compat.hpp with MSVC.
"""

import os
import sys
import subprocess
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent

cpp_source = """// Smoke test for single-header dist/compat.hpp
#include "dist/compat.hpp"
#include <iostream>

int main() {
    compat::string_view sv = "Hello from CPP-Compat single-header!";
    compat::println("StringView test: {}", sv);

    auto parsed_int = compat::parse<int32_t>("42");
    if (parsed_int.has_value()) {
        compat::println("Parse<int32_t> test: {}", *parsed_int);
    } else {
        std::cerr << "Parse failed!" << std::endl;
        return 1;
    }

    auto formatted = compat::format("Formatted {} and {}", "alpha", 999);
    compat::println("Format test: {}", formatted);

    compat::println("Single-header test passed successfully!");
    return 0;
}
"""

test_cpp = REPO_ROOT / "test_smoke_single_header.cpp"
test_exe = REPO_ROOT / "test_smoke_single_header.exe"

with open(test_cpp, "w", encoding="utf-8-sig") as f:
    f.write(cpp_source)

vcvars = r"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

for std_flag in ["/std:c++20", "/std:c++17"]:
    print(f"\n--- Testing MSVC {std_flag} ---")
    bat_script = f"""@echo off
call "{vcvars}" >nul 2>&1
cl.exe {std_flag} /EHsc /W4 /I"{REPO_ROOT}" "{test_cpp}" /Fe"{test_exe}"
"""
    bat_path = REPO_ROOT / "run_cl.bat"
    bat_path.write_text(bat_script, encoding="utf-8")

    res = subprocess.run(["cmd.exe", "/c", str(bat_path)], cwd=str(REPO_ROOT), capture_output=True, text=True)
    print(res.stdout)
    if res.stderr:
        print(res.stderr)
    if res.returncode != 0:
        print(f"[FAIL] Compilation failed under {std_flag}")
        sys.exit(1)

    run_res = subprocess.run([str(test_exe)], cwd=str(REPO_ROOT), capture_output=True, text=True)
    print("Execution output:\n" + run_res.stdout)
    if run_res.returncode != 0:
        print(f"[FAIL] Execution failed under {std_flag}")
        sys.exit(1)

# Cleanup
for p in [test_cpp, test_exe, REPO_ROOT / "test_smoke_single_header.obj", REPO_ROOT / "run_cl.bat"]:
    if p.exists():
        p.unlink()

print("[SUCCESS] All single-header MSVC compiler tests (/std:c++20 and /std:c++17) passed!")
