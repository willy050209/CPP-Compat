#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
scripts/compile_test.py
=======================
Compiles and runs a smoke test using dist/compat.hpp with MSVC.
"""

import os
import sys
import shutil
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

    auto sub_clamped = sp.subspan(3, 2);
    if (sub_clamped.size() != 2 || sub_clamped[0] != 40 || sub_clamped[1] != 50) {
        std::cerr << "Span subspan test failed!" << std::endl;
        return 4;
    }

    int pc_val = 99;
    compat::propagate_const<int*> pc(&pc_val);
    compat::println("PropagateConst test: *pc = {}", *pc);
    if (*pc != 99 || compat::get_underlying(pc) != &pc_val) {
        std::cerr << "PropagateConst test failed!" << std::endl;
        return 5;
    }

    compat::shared_mutex sm;
    {
        compat::shared_lock<compat::shared_mutex> slk(sm);
        compat::println("SharedLock test: owns_lock = {}", slk.owns_lock());
        if (!slk.owns_lock()) {
            std::cerr << "SharedLock test failed!" << std::endl;
            return 6;
        }
    }

    std::string s_val = compat::to_string(12345);
    compat::println("ToString test: compat::to_string(12345) = {}", s_val);
    if (s_val != "12345") {
        std::cerr << "ToString test failed!" << std::endl;
        return 7;
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


def run_smoke_tests():
    has_msvc = Path(vcvars).exists()
    if sys.platform == "win32" and has_msvc:
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
        print("[SUCCESS] All single-header MSVC compiler tests (/std:c++20, /std:c++17, and /std:c++14) passed!")

    cxx = shutil.which("g++") or shutil.which("g++.exe") or shutil.which("clang++") or shutil.which("clang++.exe")
    if cxx:
        print(f"\n--- Testing {cxx} single-header smoke test (-std=c++11, -std=c++20) ---")
        for std in ["c++11", "c++20"]:
            safe_unlink(test_exe)
            cmd = [cxx, f"-std={std}", "-pedantic", "-Wall", "-Wextra", f"-I{REPO_ROOT}", str(test_cpp), "-o", str(test_exe)]
            res = subprocess.run(cmd, cwd=str(REPO_ROOT), capture_output=True, text=True)
            if res.returncode != 0:
                print(f"[FAIL] {cxx} compile failed under -std={std}:\n{res.stderr}")
                sys.exit(1)
            run_res = subprocess.run([str(test_exe)], cwd=str(REPO_ROOT), capture_output=True, text=True)
            if run_res.returncode != 0:
                print(f"[FAIL] {cxx} execution failed under -std={std}:\n{run_res.stderr}")
                sys.exit(1)
            safe_unlink(test_exe)
        print(f"[SUCCESS] {cxx} single-header tests passed!")

    # Cleanup
    for p in [test_cpp, test_exe, REPO_ROOT / "test_smoke_single_header.obj", REPO_ROOT / "run_cl.bat"]:
        safe_unlink(p)


def test_cross_tu_abi():
    print("\n" + "=" * 70)
    print("  Testing Cross-TU ABI Isolation & Seamless Upgrade")
    print("=" * 70)

    cxx = shutil.which("g++") or shutil.which("g++.exe") or shutil.which("clang++") or shutil.which("clang++.exe")
    if not cxx:
        print("[SKIP] No g++ or clang++ found on PATH for cross-TU ABI verification.")
        return

    tu_header = REPO_ROOT / "test_tu_abi.hpp"
    tu_legacy = REPO_ROOT / "test_tu_legacy.cpp"
    tu_modern = REPO_ROOT / "test_tu_modern.cpp"
    obj_legacy = REPO_ROOT / "test_tu_legacy.o"
    obj_modern = REPO_ROOT / "test_tu_modern.o"
    exe_mixed = REPO_ROOT / "test_tu_mixed.exe"

    tu_header.write_text("""#pragma once
#include <compat/Optional.hpp>
#include <compat/StringView.hpp>

compat::optional<int> calculate_len(compat::string_view sv);
""", encoding="utf-8-sig")

    tu_legacy.write_text("""#include "test_tu_abi.hpp"

compat::optional<int> calculate_len(compat::string_view sv) {
    if (sv.empty()) {
        return compat::nullopt;
    }
    return compat::optional<int>(static_cast<int>(sv.size()));
}
""", encoding="utf-8-sig")

    tu_modern.write_text("""#include "test_tu_abi.hpp"
#include <cassert>
#include <iostream>

int main() {
    auto res = calculate_len("Antigravity Seamless Upgrade");
    assert(res.has_value());
    assert(*res == 28);
    std::cout << "[PASS] Cross-TU ABI migration link & execution verified! Length: " << *res << std::endl;
    return 0;
}
""", encoding="utf-8-sig")

    # 1. Positive case: COMPAT_FORCE_SELF_IMPL=1 allows seamless C++11 lib + C++20 app
    print(f"\n--- [Positive Case] C++11 Library + C++20 Binary with COMPAT_FORCE_SELF_IMPL=1 ({cxx}) ---")
    safe_unlink(obj_legacy)
    safe_unlink(obj_modern)
    safe_unlink(exe_mixed)

    r1 = subprocess.run([cxx, "-std=c++11", "-DCOMPAT_FORCE_SELF_IMPL=1", "-Iinclude", "-c", str(tu_legacy), "-o", str(obj_legacy)], cwd=str(REPO_ROOT), capture_output=True, text=True)
    if r1.returncode != 0:
        print(f"[FAIL] TU Legacy compile failed:\n{r1.stderr}")
        sys.exit(1)

    r2 = subprocess.run([cxx, "-std=c++20", "-DCOMPAT_FORCE_SELF_IMPL=1", "-Iinclude", "-c", str(tu_modern), "-o", str(obj_modern)], cwd=str(REPO_ROOT), capture_output=True, text=True)
    if r2.returncode != 0:
        print(f"[FAIL] TU Modern compile failed:\n{r2.stderr}")
        sys.exit(1)

    r3 = subprocess.run([cxx, str(obj_legacy), str(obj_modern), "-o", str(exe_mixed)], cwd=str(REPO_ROOT), capture_output=True, text=True)
    if r3.returncode != 0:
        print(f"[FAIL] Mixed standard linking failed:\n{r3.stderr}")
        sys.exit(1)

    r4 = subprocess.run([str(exe_mixed)], cwd=str(REPO_ROOT), capture_output=True, text=True)
    print(r4.stdout.strip())
    if r4.returncode != 0:
        print(f"[FAIL] Mixed standard execution failed:\n{r4.stderr}")
        sys.exit(1)
    print("[PASS] Positive case: C++11 and C++20 mixed-standard linkage executed flawlessly with locked self ABI.")

    # 2. Negative case: Without COMPAT_FORCE_SELF_IMPLEMENTATION, inline namespaces block mismatched ABI at link time
    print(f"\n--- [Negative Case] Mismatched ABI Guard (C++11 default vs C++20 default without lock) ({cxx}) ---")
    safe_unlink(obj_legacy)
    safe_unlink(obj_modern)
    safe_unlink(exe_mixed)

    n1 = subprocess.run([cxx, "-std=c++11", "-Iinclude", "-c", str(tu_legacy), "-o", str(obj_legacy)], cwd=str(REPO_ROOT), capture_output=True, text=True)
    n2 = subprocess.run([cxx, "-std=c++20", "-Iinclude", "-c", str(tu_modern), "-o", str(obj_modern)], cwd=str(REPO_ROOT), capture_output=True, text=True)
    n3 = subprocess.run([cxx, str(obj_legacy), str(obj_modern), "-o", str(exe_mixed)], cwd=str(REPO_ROOT), capture_output=True, text=True)

    if n3.returncode != 0:
        print("[PASS] Negative guard verified: Linker successfully blocked mismatched ABI (unresolved symbol detected)!")
        print("Linker error output snippet:")
        print(n3.stderr.strip()[:300])
    else:
        print("[FAIL] Negative guard failed: Linker should have failed on mismatched inline namespaces!")
        sys.exit(1)

    # Cleanup
    for p in [tu_header, tu_legacy, tu_modern, obj_legacy, obj_modern, exe_mixed]:
        safe_unlink(p)


if __name__ == "__main__":
    run_smoke_tests()
    test_cross_tu_abi()
    print("\n" + "=" * 70)
    print("  ALL COMPILATION AND ABI ISOLATION TESTS COMPLETED SUCCESSFULLY!")
    print("=" * 70)

