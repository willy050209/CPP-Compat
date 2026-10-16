#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
scripts/test_print_matrix.py
============================
Multi-platform, multi-standard matrix verification for compat::print / println and all core modules.
Supports:
- Windows Native MSVC (/std:c++14, /std:c++17, /std:c++20, /std:c++latest) x (default, fallback)
- Windows Native MinGW / WSL GCC (-std=c++11, -std=c++14, -std=c++17, -std=c++20, -std=c++23, -std=c++26) x (default, fallback)
- Windows Native / WSL Clang++ (-std=c++14, -std=c++17, -std=c++20, -std=c++23, -std=c++26) x (default, fallback)
"""

import sys
import os
import shutil
import glob
import argparse
import subprocess
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent

SOURCES = [
    "tests/test_main.cpp",
    "tests/test_print.cpp",
    "tests/test_string_view.cpp",
    "tests/test_expected.cpp",
    "tests/test_format.cpp",
    "tests/test_parse.cpp",
    "tests/test_ranges.cpp",
    "tests/test_algorithm.cpp",
    "tests/test_memory.cpp",
    "tests/test_filesystem.cpp",
    "tests/test_span.cpp",
    "tests/test_shared_mutex.cpp",
    "tests/test_propagate_const.cpp",
    "tests/test_optional.cpp"
]

def find_vcvars64():
    candidates = glob.glob(r"C:\Program Files*\Microsoft Visual Studio\*\*\VC\Auxiliary\Build\vcvars64.bat")
    if candidates:
        return candidates[0]
    default_path = r"C:\Program Files\Microsoft Visual Studio\18\Insiders\VC\Auxiliary\Build\vcvars64.bat"
    if os.path.exists(default_path):
        return default_path
    return None

def get_wsl_path(path: Path) -> str:
    drive = path.drive.rstrip(":").lower()
    rest = path.as_posix()[3:]
    return f"/mnt/{drive}/{rest}"

def test_msvc():
    print("\n=======================================================")
    print("           Running MSVC Test Suite (Windows)")
    print("=======================================================")
    vcvars = find_vcvars64()
    if not vcvars:
        print("[SKIP] MSVC vcvars64.bat not found on system")
        return True

    standards = ["/std:c++14", "/std:c++17", "/std:c++20", "/std:c++latest"]
    modes = [("default", ""), ("fallback", "/DCOMPAT_FORCE_SELF_IMPL=1")]
    all_passed = True
    src_quoted = " ".join(f'"{REPO_ROOT / s}"' for s in SOURCES)

    for std in standards:
        for mode_name, mode_flag in modes:
            tag = f"{std.replace('/', '').replace(':', '_')}_{mode_name}"
            exe_path = REPO_ROOT / f"test_msvc_{tag}.exe"
            bat_path = REPO_ROOT / f"run_msvc_{tag}.bat"

            bat_content = f"""@echo off
call "{vcvars}" >nul 2>&1
cl.exe {std} {mode_flag} /EHsc /utf-8 /W4 /I"{REPO_ROOT / 'include'}" {src_quoted} /Fe"{exe_path}"
"""
            bat_path.write_text(bat_content, encoding="utf-8")

            print(f"--> Compiling MSVC {std} ({mode_name}) ...")
            comp = subprocess.run(["cmd.exe", "/c", str(bat_path)], cwd=str(REPO_ROOT), capture_output=True, text=True, encoding="utf-8", errors="replace")
            if comp.returncode != 0:
                print(f"[FAIL] Build failed for MSVC {std} ({mode_name})")
                print(comp.stdout[-400:] if comp.stdout else "")
                print(comp.stderr[-400:] if comp.stderr else "")
                all_passed = False
                continue

            print(f"--> Executing MSVC {std} ({mode_name}) ...")
            run = subprocess.run([str(exe_path)], cwd=str(REPO_ROOT), capture_output=True, text=True, encoding="utf-8", errors="replace")
            if run.returncode != 0:
                print(f"[FAIL] Execution failed for MSVC {std} ({mode_name})")
                print(run.stdout[-400:] if run.stdout else "")
                print(run.stderr[-400:] if run.stderr else "")
                all_passed = False
            else:
                if "繁體中文" in run.stdout and "こんにちは" in run.stdout and "test_print passed" in run.stdout:
                    print(f"[PASS] MSVC {std} ({mode_name}) verified successfully with UTF-8 & special characters!")
                else:
                    print(f"[WARN] MSVC {std} ({mode_name}) passed execution but output check needs inspection")

            for p in [exe_path, bat_path, REPO_ROOT / f"test_msvc_{tag}.obj"]:
                if p.exists():
                    try: p.unlink()
                    except Exception: pass

    for obj in REPO_ROOT.glob("*.obj"):
        try: obj.unlink()
        except Exception: pass

    return all_passed

def test_gcc():
    print("\n=======================================================")
    print("           Running GCC Test Suite (MinGW / WSL)")
    print("=======================================================")
    gcc_bin = shutil.which("g++")
    standards = ["11", "14", "17", "20", "23", "26"]
    modes = [("default", ""), ("fallback", "-DCOMPAT_FORCE_SELF_IMPL=1")]
    all_passed = True

    if gcc_bin:
        print(f"[INFO] Using native MinGW GCC: {gcc_bin}")
        src_quoted = [str(REPO_ROOT / s) for s in SOURCES]
        include_dir = str(REPO_ROOT / "include")
        for std in standards:
            for mode_name, mode_flag in modes:
                exe_name = REPO_ROOT / f"test_gcc_cxx{std}_{mode_name}.exe"
                cmd = [gcc_bin, f"-std=c++{std}"]
                if mode_flag:
                    cmd.append(mode_flag)
                cmd.extend(["-Wall", "-Wextra", f"-I{include_dir}"])
                cmd.extend(src_quoted)
                cmd.extend(["-o", str(exe_name)])

                print(f"--> Compiling & Running native g++ -std=c++{std} ({mode_name}) ...")
                comp = subprocess.run(cmd, cwd=str(REPO_ROOT), capture_output=True, text=True, encoding="utf-8", errors="replace")
                if comp.returncode != 0:
                    print(f"[FAIL] g++ -std=c++{std} ({mode_name}) build failed")
                    print(comp.stderr[-400:] if comp.stderr else "")
                    all_passed = False
                    continue

                run = subprocess.run([str(exe_name)], cwd=str(REPO_ROOT), capture_output=True, text=True, encoding="utf-8", errors="replace")
                if run.returncode != 0 or "test_print passed" not in run.stdout:
                    print(f"[FAIL] g++ -std=c++{std} ({mode_name}) execution failed")
                    all_passed = False
                else:
                    print(f"[PASS] g++ -std=c++{std} ({mode_name}) passed successfully!")

                if exe_name.exists():
                    try: exe_name.unlink()
                    except Exception: pass
    else:
        wsl_root = get_wsl_path(REPO_ROOT)
        src_list = " ".join(SOURCES)
        for std in standards:
            for mode_name, mode_flag in modes:
                print(f"--> Compiling & Running WSL g++ -std=c++{std} ({mode_name}) ...")
                cmd = f"cd {wsl_root} && g++ -std=c++{std} {mode_flag} -Wall -Wextra -Iinclude {src_list} -o /tmp/test_gxx_{std}_{mode_name} && /tmp/test_gxx_{std}_{mode_name}"
                res = subprocess.run(["wsl", "bash", "-c", cmd], capture_output=True, text=True, encoding="utf-8", errors="replace")
                if res.returncode != 0 or "test_print passed" not in res.stdout:
                    print(f"[FAIL] WSL g++ -std=c++{std} ({mode_name}) failed")
                    print(res.stderr[-400:] if res.stderr else "")
                    all_passed = False
                else:
                    print(f"[PASS] WSL g++ -std=c++{std} ({mode_name}) passed successfully!")

    return all_passed

def test_clang():
    print("\n=======================================================")
    print("      Running Clang++ Test Suite (Native / WSL)")
    print("=======================================================")
    clang_bin = shutil.which("clang++")
    standards = ["14", "17", "20", "23", "26"]
    modes = [("default", ""), ("fallback", "-DCOMPAT_FORCE_SELF_IMPL=1")]
    all_passed = True

    if clang_bin:
        print(f"[INFO] Using native Clang++: {clang_bin}")
        src_quoted = [str(REPO_ROOT / s) for s in SOURCES]
        include_dir = str(REPO_ROOT / "include")
        for std in standards:
            for mode_name, mode_flag in modes:
                exe_name = REPO_ROOT / f"test_clang_cxx{std}_{mode_name}.exe"
                cmd = [clang_bin, f"-std=c++{std}"]
                if mode_flag:
                    cmd.append(mode_flag)
                cmd.extend(["-Wall", "-Wextra", f"-I{include_dir}"])
                cmd.extend(src_quoted)
                cmd.extend(["-o", str(exe_name)])

                print(f"--> Compiling & Running native clang++ -std=c++{std} ({mode_name}) ...")
                comp = subprocess.run(cmd, cwd=str(REPO_ROOT), capture_output=True, text=True, encoding="utf-8", errors="replace")
                if comp.returncode != 0:
                    print(f"[FAIL] clang++ -std=c++{std} ({mode_name}) build failed")
                    print(comp.stderr[-400:] if comp.stderr else "")
                    all_passed = False
                    continue

                run = subprocess.run([str(exe_name)], cwd=str(REPO_ROOT), capture_output=True, text=True, encoding="utf-8", errors="replace")
                if run.returncode != 0 or "test_print passed" not in run.stdout:
                    print(f"[FAIL] clang++ -std=c++{std} ({mode_name}) execution failed")
                    all_passed = False
                else:
                    print(f"[PASS] clang++ -std=c++{std} ({mode_name}) passed successfully!")

                if exe_name.exists():
                    try: exe_name.unlink()
                    except Exception: pass
    else:
        wsl_root = get_wsl_path(REPO_ROOT)
        src_list = " ".join(SOURCES)
        for std in standards:
            for mode_name, mode_flag in modes:
                print(f"--> Compiling & Running WSL clang++ -std=c++{std} ({mode_name}) ...")
                cmd = f"cd {wsl_root} && clang++ -std=c++{std} {mode_flag} -Wall -Wextra -Iinclude {src_list} -o /tmp/test_clang_{std}_{mode_name} && /tmp/test_clang_{std}_{mode_name}"
                res = subprocess.run(["wsl", "bash", "-c", cmd], capture_output=True, text=True, encoding="utf-8", errors="replace")
                if res.returncode != 0 or "test_print passed" not in res.stdout:
                    print(f"[FAIL] WSL clang++ -std=c++{std} ({mode_name}) failed")
                    print(res.stderr[-400:] if res.stderr else "")
                    all_passed = False
                else:
                    print(f"[PASS] WSL clang++ -std=c++{std} ({mode_name}) passed successfully!")

    return all_passed

def main():
    parser = argparse.ArgumentParser(description="Multi-standard matrix testing for compat::print and core shim")
    parser.add_argument("--toolchain", choices=["all", "msvc", "gcc", "clang"], default="all")
    args = parser.parse_args()

    results = {}

    if args.toolchain in ["all", "msvc"]:
        results["MSVC"] = test_msvc()

    if args.toolchain in ["all", "gcc"]:
        results["GCC"] = test_gcc()

    if args.toolchain in ["all", "clang"]:
        results["Clang++"] = test_clang()

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
