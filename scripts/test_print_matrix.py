#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
scripts/test_print_matrix.py
============================
Multi-platform, multi-standard matrix verification for compat::print / println.
Tests:
- Windows MSVC (/std:c++14, /std:c++17, /std:c++20, /std:c++latest)
- Linux WSL GCC (-std=c++11, -std=c++14, -std=c++17, -std=c++20, -std=c++23, -std=c++26)
- Mac (WSL Clang++) (-std=c++11, -std=c++14, -std=c++17, -std=c++20, -std=c++23, -std=c++26)
"""

import sys
import os
import argparse
import subprocess
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent

VCVARS_BAT = r"C:\Program Files\Microsoft Visual Studio\18\Insiders\VC\Auxiliary\Build\vcvars64.bat"

SOURCES = [
    "tests/test_print.cpp",
    "tests/test_string_view.cpp",
    "tests/test_expected.cpp",
    "tests/test_format.cpp",
    "tests/test_parse.cpp",
    "tests/test_ranges.cpp",
    "tests/test_algorithm.cpp",
    "tests/test_main.cpp"
]

def test_msvc():
    print("\n=======================================================")
    print("           Running MSVC Test Suite (Windows)")
    print("=======================================================")
    standards = ["/std:c++14", "/std:c++17", "/std:c++20", "/std:c++latest"]
    all_passed = True
    src_quoted = " ".join(f'"{REPO_ROOT / s}"' for s in SOURCES)

    for std in standards:
        tag = std.replace("/", "").replace(":", "_")
        exe_path = REPO_ROOT / f"test_msvc_{tag}.exe"
        bat_path = REPO_ROOT / f"run_msvc_{tag}.bat"

        bat_content = f"""@echo off
call "{VCVARS_BAT}" >nul 2>&1
cl.exe {std} /EHsc /utf-8 /W4 /I"{REPO_ROOT / 'include'}" {src_quoted} /Fe"{exe_path}"
"""
        bat_path.write_text(bat_content, encoding="utf-8")

        print(f"--> Compiling MSVC {std} ...")
        comp = subprocess.run(["cmd.exe", "/c", str(bat_path)], cwd=str(REPO_ROOT), capture_output=True, text=True, encoding="utf-8", errors="replace")
        if comp.returncode != 0:
            print(f"[FAIL] Build failed for MSVC {std}")
            print(comp.stdout[-500:] if comp.stdout else "")
            print(comp.stderr[-500:] if comp.stderr else "")
            all_passed = False
            continue

        print(f"--> Executing MSVC {std} ...")
        run = subprocess.run([str(exe_path)], cwd=str(REPO_ROOT), capture_output=True, text=True, encoding="utf-8", errors="replace")
        if run.returncode != 0:
            print(f"[FAIL] Execution failed for MSVC {std}")
            print(run.stdout[-500:] if run.stdout else "")
            print(run.stderr[-500:] if run.stderr else "")
            all_passed = False
        else:
            # Verify that special characters are properly printed without crash or mojibake in output
            if "繁體中文" in run.stdout and "こんにちは" in run.stdout and "test_print passed" in run.stdout:
                print(f"[PASS] MSVC {std} verified successfully with UTF-8 & special characters!")
            else:
                print(f"[WARN] MSVC {std} passed execution but check stdout content:\n{run.stdout[-300:]}")

        # Cleanup
        for p in [exe_path, bat_path, REPO_ROOT / f"test_msvc_{tag}.obj"]:
            if p.exists():
                try: p.unlink()
                except Exception: pass

    # Clean any leftover obj files in repo root
    for obj in REPO_ROOT.glob("*.obj"):
        try: obj.unlink()
        except Exception: pass

    return all_passed

def test_wsl_gcc():
    print("\n=======================================================")
    print("           Running GCC Test Suite (WSL Linux)")
    print("=======================================================")
    standards = ["11", "14", "17", "20", "23", "26"]
    all_passed = True
    src_list = " ".join(SOURCES)

    for std in standards:
        print(f"--> Compiling & Running WSL g++ -std=c++{std} ...")
        cmd = f"cd /mnt/d/program/C++/CPP-Compat && g++ -std=c++{std} -Wall -Wextra -Iinclude {src_list} -o /tmp/test_gxx_{std} && /tmp/test_gxx_{std}"
        res = subprocess.run(["wsl", "bash", "-c", cmd], capture_output=True, text=True, encoding="utf-8", errors="replace")
        if res.returncode != 0:
            print(f"[FAIL] WSL g++ -std=c++{std} failed")
            print(res.stdout[-400:] if res.stdout else "")
            print(res.stderr[-400:] if res.stderr else "")
            all_passed = False
        else:
            if "test_print passed" in res.stdout:
                print(f"[PASS] WSL g++ -std=c++{std} passed successfully!")
            else:
                print(f"[WARN] WSL g++ -std=c++{std} output didn't contain test_print passed")

    return all_passed

def test_wsl_clang():
    print("\n=======================================================")
    print("      Running Clang++ Test Suite (WSL Mac Simulation)")
    print("=======================================================")
    standards = ["11", "14", "17", "20", "23", "26"]
    all_passed = True
    src_list = " ".join(SOURCES)

    for std in standards:
        print(f"--> Compiling & Running WSL clang++ -std=c++{std} ...")
        cmd = f"cd /mnt/d/program/C++/CPP-Compat && clang++ -std=c++{std} -Wall -Wextra -Iinclude {src_list} -o /tmp/test_clang_{std} && /tmp/test_clang_{std}"
        res = subprocess.run(["wsl", "bash", "-c", cmd], capture_output=True, text=True, encoding="utf-8", errors="replace")
        if res.returncode != 0:
            print(f"[FAIL] WSL clang++ -std=c++{std} failed")
            print(res.stdout[-400:] if res.stdout else "")
            print(res.stderr[-400:] if res.stderr else "")
            all_passed = False
        else:
            if "test_print passed" in res.stdout:
                print(f"[PASS] WSL clang++ -std=c++{std} passed successfully!")
            else:
                print(f"[WARN] WSL clang++ -std=c++{std} output didn't contain test_print passed")

    return all_passed

def main():
    parser = argparse.ArgumentParser(description="Multi-standard matrix testing for compat::print")
    parser.add_argument("--toolchain", choices=["all", "msvc", "gcc", "clang"], default="all")
    args = parser.parse_args()

    results = {}

    if args.toolchain in ["all", "msvc"]:
        results["MSVC"] = test_msvc()

    if args.toolchain in ["all", "gcc"]:
        results["WSL GCC"] = test_wsl_gcc()

    if args.toolchain in ["all", "clang"]:
        results["WSL Clang++"] = test_wsl_clang()

    print("\n=======================================================")
    print("                   Final Summary")
    print("=======================================================")
    all_ok = True
    for name, ok in results.items():
        status = "PASSED" if ok else "FAILED"
        print(f"  {name:15}: {status}")
        if not ok:
            all_ok = False

    if all_ok:
        print("\nALL MATRIX COMPILER & RUNTIME TESTS PASSED!")
        sys.exit(0)
    else:
        print("\nSOME MATRIX TESTS FAILED!")
        sys.exit(1)

if __name__ == "__main__":
    main()
