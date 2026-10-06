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

    int clamped = compat::clamp(15, 1, 10);
    compat::println("Clamp test: compat::clamp(15, 1, 10) = {}", clamped);
    if (clamped != 10) {
        std::cerr << "Clamp test failed!" << std::endl;
        return 2;
    }

    int nums[] = {10, 20, 30, 40, 50};
    compat::span<int> sp(nums);
    auto sub = sp.subspan(1, 3);
    compat::println("Span test: size={}, front={}, back={}", sub.size(), sub.front(), sub.back());
    if (sub.size() != 3 || sub.front() != 20 || sub.back() != 40) {
        std::cerr << "Span test failed!" << std::endl;
        return 3;
    }

    auto sub_clamped = sp.subspan(3, 100);
    if (sub_clamped.size() != 2 || sub_clamped[0] != 40 || sub_clamped[1] != 50) {
        std::cerr << "Span clamping test failed!" << std::endl;
        return 4;
    }

    compat::println("Single-header test passed successfully!");
    return 0;
}
"""

test_cpp = REPO_ROOT / "test_smoke_single_header.cpp"
test_exe = REPO_ROOT / "test_smoke_single_header.exe"

with open(test_cpp, "w", encoding="utf-8-sig") as f:
    f.write(cpp_source)

candidate_vcvars = [
    r"C:\Program Files\Microsoft Visual Studio\18\Insiders\VC\Auxiliary\Build\vcvars64.bat",
    r"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat",
    r"C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat",
    r"C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat",
]
vcvars = next((p for p in candidate_vcvars if Path(p).exists()), candidate_vcvars[0])

def safe_unlink(path):
    import time
    for _ in range(10):
        try:
            if path.exists():
                path.unlink()
            return
        except OSError:
            time.sleep(0.2)

for std_flag in ["/std:c++20", "/std:c++17", "/std:c++14"]:
    print(f"\n--- Testing MSVC {std_flag} ---")
    safe_unlink(test_exe)
    safe_unlink(REPO_ROOT / "test_smoke_single_header.obj")

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

    safe_unlink(test_exe)

# Cleanup
for p in [test_cpp, test_exe, REPO_ROOT / "test_smoke_single_header.obj", REPO_ROOT / "run_cl.bat"]:
    if p.exists():
        p.unlink()

print("[SUCCESS] All single-header MSVC compiler tests (/std:c++20, /std:c++17, and /std:c++14) passed!")
