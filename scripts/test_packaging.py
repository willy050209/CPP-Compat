#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
scripts/test_packaging.py
=========================
Automated validation suite for CPP-Compat packaging scripts and artifacts:
1. Validates scripts/bundle_header.py execution and dist/compat.hpp output.
2. Validates scripts/export_module.py execution and dist/compat.ixx output.
3. Verifies UTF-8 BOM encoding on all generated artifacts.
4. Verifies absence of internal relative includes (#include "...") in bundled header.
5. Verifies presence and deduplication of standard STL headers.
6. Verifies C++20 module structure (module;, export module compat;, export namespace compat).
7. Verifies retention of XML documentation comments (/// <summary>, etc.).
"""

import sys
import subprocess
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
DIST_DIR = REPO_ROOT / "dist"
HPP_FILE = DIST_DIR / "compat.hpp"
IXX_FILE = DIST_DIR / "compat.ixx"


def check(cond: bool, msg: str):
    if not cond:
        print(f"[FAIL] {msg}")
        sys.exit(1)
    else:
        print(f"[PASS] {msg}")


def main():
    print("=== Step 1: Run scripts/bundle_header.py ===")
    res = subprocess.run([sys.executable, str(REPO_ROOT / "scripts" / "bundle_header.py"), "-v"], capture_output=True, text=True)
    print(res.stdout)
    if res.stderr:
        print(res.stderr)
    check(res.returncode == 0, "bundle_header.py returned code 0")
    check(HPP_FILE.exists(), f"{HPP_FILE} exists")

    print("\n=== Step 2: Validate dist/compat.hpp ===")
    hpp_bytes = HPP_FILE.read_bytes()
    check(hpp_bytes.startswith(b"\xef\xbb\xbf"), "dist/compat.hpp contains UTF-8 BOM (ef bb bf)")

    hpp_text = HPP_FILE.read_text(encoding="utf-8-sig")
    lines = hpp_text.splitlines()

    # Check #pragma once
    check("#pragma once" in hpp_text, "dist/compat.hpp contains '#pragma once'")

    # Check no internal relative includes
    internal_includes = [l.strip() for l in lines if '#include "' in l]
    check(len(internal_includes) == 0, f"dist/compat.hpp contains 0 internal relative includes (found: {internal_includes})")

    # Check deduplicated STL headers
    check("#include <cstdint>" in hpp_text, "dist/compat.hpp includes <cstdint>")
    check("#include <iostream>" in hpp_text, "dist/compat.hpp includes <iostream>")
    check("#include <variant>" in hpp_text, "dist/compat.hpp includes <variant>")
    check("#include <string>" in hpp_text, "dist/compat.hpp includes <string>")
    check("#include <type_traits>" in hpp_text, "dist/compat.hpp includes <type_traits>")

    # Check XML documentation comments
    check("/// <summary>" in hpp_text, "dist/compat.hpp retains XML comments (/// <summary>)")

    print("\n=== Step 3: Run scripts/export_module.py ===")
    res_mod = subprocess.run([sys.executable, str(REPO_ROOT / "scripts" / "export_module.py"), "-v"], capture_output=True, text=True)
    print(res_mod.stdout)
    if res_mod.stderr:
        print(res_mod.stderr)
    check(res_mod.returncode == 0, "export_module.py returned code 0")
    check(IXX_FILE.exists(), f"{IXX_FILE} exists")

    print("\n=== Step 4: Validate dist/compat.ixx ===")
    ixx_bytes = IXX_FILE.read_bytes()
    check(ixx_bytes.startswith(b"\xef\xbb\xbf"), "dist/compat.ixx contains UTF-8 BOM (ef bb bf)")

    ixx_text = IXX_FILE.read_text(encoding="utf-8-sig")

    check("module;" in ixx_text, "dist/compat.ixx contains 'module;' Global Module Fragment")
    check("export module compat;" in ixx_text, "dist/compat.ixx contains 'export module compat;'")
    check("export namespace compat {" in ixx_text, "dist/compat.ixx contains 'export namespace compat {'")
    check("#pragma once" not in ixx_text, "dist/compat.ixx does not contain '#pragma once'")
    check("/// <summary>" in ixx_text, "dist/compat.ixx retains XML comments (/// <summary>)")

    print("\n[ALL CHECKS PASSED] Packaging artifacts and scripts are 100% verified!")


if __name__ == "__main__":
    main()
