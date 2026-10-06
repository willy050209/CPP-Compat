#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
scripts/run_full_matrix.py
==========================
Comprehensive cross-compiler and cross-standard test matrix runner:
- Compilers: g++ (GCC 16.1), clang++ (LLVM 23.1), MSVC (VS 18 Insiders / 19.51)
- Standards: C++11 to latest (C++14/17/20/23/26/latest)
- Modes: default (native) and fallback (COMPAT_FORCE_FALLBACK / COMPAT_FORCE_SELF_IMPLEMENTATION)
"""

import os
import sys
import subprocess
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent

TEST_SOURCES = [
    "tests/test_main.cpp",
    "tests/test_string_view.cpp",
    "tests/test_span.cpp",
    "tests/test_optional.cpp",
    "tests/test_expected.cpp",
    "tests/test_format.cpp",
    "tests/test_print.cpp",
    "tests/test_parse.cpp",
    "tests/test_ranges.cpp",
    "tests/test_algorithm.cpp",
    "tests/test_memory.cpp",
    "tests/test_filesystem.cpp",
]

CANDIDATE_VCVARS = [
    r"C:\Program Files\Microsoft Visual Studio\18\Insiders\VC\Auxiliary\Build\vcvars64.bat",
    r"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat",
    r"C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat",
    r"C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat",
]
VCVARS = next((p for p in CANDIDATE_VCVARS if Path(p).exists()), CANDIDATE_VCVARS[0])


def run_cmd(cmd_list_or_str, cwd=REPO_ROOT, shell=False):
    res = subprocess.run(cmd_list_or_str, cwd=str(cwd), capture_output=True, text=True, shell=shell)
    return res.returncode, res.stdout, res.stderr


def main():
    try:
        sys.stdout.reconfigure(line_buffering=True)
    except Exception:
        pass
    print("=" * 70, flush=True)
    print("  CPP-Compat Full Matrix Verification (g++, clang++, MSVC)", flush=True)
    print("=" * 70, flush=True)

    results = {}

    test_sources_rel = " ".join(TEST_SOURCES)
    out_exe = REPO_ROOT / "temp_matrix_test.exe"

    # 1. GCC (MinGW / UCRT64)
    gcc_stds = ["c++11", "c++14", "c++17", "c++20", "c++23", "c++26"]
    for std in gcc_stds:
        for mode in ["default", "fallback"]:
            tag = f"g++ ({std}, {mode})"
            print(f"\n[RUNNING] {tag} ...")
            if out_exe.exists():
                out_exe.unlink()

            extra_defs = ["-DCOMPAT_FORCE_FALLBACK=1", "-DCOMPAT_FORCE_SELF_IMPLEMENTATION=1"] if mode == "fallback" else []
            cmd = ["g++.exe", f"-std={std}", "-O2", "-I", "include"] + extra_defs + TEST_SOURCES + ["-o", str(out_exe)]
            code, stdout, stderr = run_cmd(cmd)
            if code != 0:
                print(f"[BUILD FAIL] {tag}\n{stderr}")
                results[tag] = "BUILD_FAIL"
                continue

            code, stdout, stderr = run_cmd([str(out_exe)])
            if code != 0:
                print(f"[TEST FAIL] {tag}\n{stdout}\n{stderr}")
                results[tag] = "TEST_FAIL"
            else:
                print(f"[PASS] {tag}")
                results[tag] = "PASSED"

            if out_exe.exists():
                out_exe.unlink()

    # 2. Clang++ (x86_64-pc-windows-msvc)
    # Note: MSVC STL headers reject C++11 (<yvals.h> requires C++14+), so test C++14 to C++26
    clang_stds = ["c++14", "c++17", "c++20", "c++23", "c++26"]
    for std in clang_stds:
        for mode in ["default", "fallback"]:
            tag = f"clang++ ({std}, {mode})"
            print(f"\n[RUNNING] {tag} ...")
            if out_exe.exists():
                out_exe.unlink()

            extra_defs = ["-DCOMPAT_FORCE_FALLBACK=1", "-DCOMPAT_FORCE_SELF_IMPLEMENTATION=1"] if mode == "fallback" else []
            cmd = ["clang++.exe", f"-std={std}", "-O2", "-I", "include"] + extra_defs + TEST_SOURCES + ["-o", str(out_exe)]
            code, stdout, stderr = run_cmd(cmd)
            if code != 0:
                print(f"[BUILD FAIL] {tag}\n{stderr}")
                results[tag] = "BUILD_FAIL"
                continue

            code, stdout, stderr = run_cmd([str(out_exe)])
            if code != 0:
                print(f"[TEST FAIL] {tag}\n{stdout}\n{stderr}")
                results[tag] = "TEST_FAIL"
            else:
                print(f"[PASS] {tag}")
                results[tag] = "PASSED"

            if out_exe.exists():
                out_exe.unlink()

    # 3. MSVC (cl.exe via vcvars64)
    # MSVC supports /std:c++14, /std:c++17, /std:c++20, /std:c++latest
    msvc_stds = ["c++14", "c++17", "c++20", "c++latest"]
    bat_file = REPO_ROOT / "run_msvc_temp.bat"

    for std in msvc_stds:
        for mode in ["default", "fallback"]:
            tag = f"MSVC (/std:{std}, {mode})"
            print(f"\n[RUNNING] {tag} ...")
            if out_exe.exists():
                out_exe.unlink()

            extra_defs = "/DCOMPAT_FORCE_FALLBACK=1 /DCOMPAT_FORCE_SELF_IMPLEMENTATION=1" if mode == "fallback" else ""
            bat_content = f"""@echo off
call "{VCVARS}" >nul 2>&1
cl.exe /std:{std} /EHsc /O2 /W4 /utf-8 /I"include" {extra_defs} {test_sources_rel} /Fe"{out_exe}" >nul 2>&1
"""
            bat_file.write_text(bat_content, encoding="utf-8")
            code, stdout, stderr = run_cmd(["cmd.exe", "/c", str(bat_file)])
            if not out_exe.exists():
                print(f"[BUILD FAIL] {tag}")
                results[tag] = "BUILD_FAIL"
                continue

            code, stdout, stderr = run_cmd([str(out_exe)])
            if code != 0:
                print(f"[TEST FAIL] {tag}\n{stdout}\n{stderr}")
                results[tag] = "TEST_FAIL"
            else:
                print(f"[PASS] {tag}")
                results[tag] = "PASSED"

            if out_exe.exists():
                out_exe.unlink()
            for obj in REPO_ROOT.glob("*.obj"):
                obj.unlink()

    if bat_file.exists():
        bat_file.unlink()

    print("\n" + "=" * 70)
    print("  Full Matrix Test Results Summary")
    print("=" * 70)
    all_passed = True
    for tag, status in results.items():
        print(f"  {tag:<35} : {status}")
        if status != "PASSED":
            all_passed = False

    print("=" * 70)
    if all_passed:
        print("  🎉 ALL MATRIX COMBINATIONS PASSED SUCCESSFULLY! (100% GREEN)")
    else:
        print("  ❌ SOME MATRIX COMBINATIONS FAILED.")
        sys.exit(1)


if __name__ == "__main__":
    main()
