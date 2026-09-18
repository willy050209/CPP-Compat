#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
scripts/export_module.py
========================
CPP-Compat C++20 Module Exporter Script

Responsibilities:
1. Takes the unified single-header `dist/compat.hpp` (or automatically runs
   `scripts/bundle_header.py` if missing).
2. Converts and packages it into a standard C++20 Module Interface Unit `dist/compat.ixx`.
3. Places standard headers and configuration directives in the Global Module Fragment:
     module;
     #include <...>
     export module compat;
4. Transforms `namespace compat` blocks into:
     export namespace compat { ... }
   so all shim functions, types, and templates are cleanly exported.
5. Retains XML documentation comments (/// <summary>, etc.) and regular comments.
6. Enforces UTF-8 BOM encoding for the output file `dist/compat.ixx`.
"""

import os
import sys
import re
import argparse
from pathlib import Path

# Import bundle functionality from sibling script
sys.path.insert(0, str(Path(__file__).resolve().parent))
try:
    import bundle_header
except ImportError:
    bundle_header = None


RE_PRAGMA_ONCE = re.compile(r'^\s*#\s*pragma\s+once\b')
RE_INCLUDE_EXTERNAL = re.compile(r'^\s*#\s*include\s*<([^>]+)>')
RE_NAMESPACE_COMPAT = re.compile(r'^(?P<indent>\s*)namespace\s+compat\b(?!\s*::)(?P<rest>.*)$')
RE_NAMESPACE_COMPAT_DETAIL = re.compile(r'^(?P<indent>\s*)namespace\s+compat::detail\b(?P<rest>.*)$')


def find_repo_root(start_path: Path) -> Path:
    """Find repository root by looking for RULES.md, ARCHITECTURE.md, or .git."""
    curr = start_path.resolve()
    for _ in range(10):
        if (curr / "RULES.md").exists() or (curr / "ARCHITECTURE.md").exists() or (curr / ".git").exists():
            return curr
        if curr.parent == curr:
            break
        curr = curr.parent
    return start_path.resolve()


DEFAULT_CLEANUP_MACROS = [
    "COMPAT_CPLUSPLUS",
    "COMPAT_CXX_11",
    "COMPAT_CXX_14",
    "COMPAT_CXX_17",
    "COMPAT_CXX_20",
    "COMPAT_CXX_23",
    "COMPAT_CXX_26",
    "COMPAT_HAS_EXCEPTIONS",
    "COMPAT_THROW_OR_ABORT",
    "COMPAT_CONSTEXPR_14",
    "COMPAT_ABI_TAG",
    "COMPAT_HAS_STD_EXPECTED",
    "COMPAT_HAS_STD_PRINT",
    "COMPAT_HAS_STD_FORMAT",
    "COMPAT_HAS_STD_STRING_VIEW",
    "COMPAT_HAS_STD_VARIANT",
    "COMPAT_HAS_STD_RANGES",
    "COMPAT_HAS_STD_VIEWS_CONCAT",
    "COMPAT_HAS_STD_VIEWS_CACHE_LATEST",
    "COMPAT_HAS_STD_CONSTANT_RANGE",
    "COMPAT_HAS_STD_VIEWS_AS_CONST",
    "COMPAT_HAS_STD_RANGES_CONTAINS",
    "COMPAT_HAS_STD_RANGES_STARTS_WITH",
    "COMPAT_HAS_STD_RANGES_FOLD",
    "COMPAT_BAD_EXPECTED_ACCESS_DEFINED",
]


def convert_to_module(header_content: str, verbose: bool = False) -> str:
    """
    Transforms single-header content into a C++20 Module Interface Unit:
    - Global Module Fragment (GMF) holds STL includes and conditional standard headers:
        module;
        #include <...>
        export module compat;
    - All module sections (Config.hpp through Compat.hpp) live inside module purview.
    - Transforms `namespace compat` blocks into:
        export namespace compat { ... }
    - Strips textual #include <...> from module purview to avoid C5244.
    - Ensures internal preprocessor cleanup block at the end so no unwanted macros leak.
    """
    lines = header_content.splitlines()

    # Collect all external STL headers mentioned across the single-header
    external_includes: set[str] = set()
    for line in lines:
        stripped = line.strip()
        m_inc = RE_INCLUDE_EXTERNAL.match(stripped)
        if m_inc:
            external_includes.add(m_inc.group(1))

    # Core STL headers known to be needed across compat shim modules
    core_stl_headers = {
        "algorithm", "array", "cassert", "cstddef", "cstdint", "cstdlib", "cstring",
        "exception", "functional", "iostream", "iterator", "limits", "memory", "new",
        "ostream", "ranges", "sstream", "stdexcept", "string", "system_error",
        "tuple", "type_traits", "utility", "variant"
    }
    all_gmf_headers = sorted(list(external_includes.union(core_stl_headers)))

    purview_lines: list[str] = []
    in_header_preamble = True

    for line in lines:
        stripped = line.strip()

        # Pragma once is not used in module interface units
        if RE_PRAGMA_ONCE.match(stripped):
            continue

        # Skip the single-header distribution banner and top-level deduplicated STL includes
        # until the first module section is encountered
        if in_header_preamble:
            if stripped.startswith("// Module Section:"):
                in_header_preamble = False
            else:
                continue

        # In C++20 module purview, textual #include of standard headers is prohibited;
        # they are already included in the Global Module Fragment (GMF).
        if RE_INCLUDE_EXTERNAL.match(stripped):
            continue

        # Transform "namespace compat" into "export namespace compat"
        m_ns = RE_NAMESPACE_COMPAT.match(line)
        m_ns_detail = RE_NAMESPACE_COMPAT_DETAIL.match(line)
        if m_ns:
            indent = m_ns.group("indent")
            rest = m_ns.group("rest")
            purview_lines.append(f"{indent}export namespace compat{rest}")
        elif m_ns_detail:
            indent = m_ns_detail.group("indent")
            rest = m_ns_detail.group("rest")
            purview_lines.append(f"{indent}export namespace compat::detail{rest}")
        else:
            purview_lines.append(line)

    # Clean outer blank lines in purview
    while purview_lines and purview_lines[0].strip() == "":
        purview_lines.pop(0)
    while purview_lines and purview_lines[-1].strip() == "":
        purview_lines.pop()

    # Ensure preprocessor cleanup block is present at end of module purview
    purview_text = "\n".join(purview_lines)
    if "#undef COMPAT_CPLUSPLUS" not in purview_text:
        purview_lines.append("")
        purview_lines.append("// ============================================================================")
        purview_lines.append("// Internal Preprocessor Cleanup")
        purview_lines.append("// Undefine internal helper macros so no unwanted macros leak from module purview.")
        purview_lines.append("// ============================================================================")
        for macro in DEFAULT_CLEANUP_MACROS:
            purview_lines.append(f"#undef {macro}")
        purview_lines.append("")

    result_parts: list[str] = []

    # Headers requiring platform conditionals or language version checks
    platform_headers = {"windows.h", "unistd.h", "io.h"}
    conditional_stl_headers = {"expected", "print", "format", "version", "string_view", "ranges"}

    # Module banner
    result_parts.append("// ============================================================================")
    result_parts.append("// CPP-Compat: Zero-Dependency Modern C++ Backward Compatibility Shim Layer")
    result_parts.append("// C++20 Module Interface Unit (dist/compat.ixx)")
    result_parts.append("//")
    result_parts.append("// This file is automatically generated by scripts/export_module.py.")
    result_parts.append("// Do not edit this file directly.")
    result_parts.append("// ============================================================================")
    result_parts.append("")
    result_parts.append("module;")
    result_parts.append("")
    result_parts.append("// ----------------------------------------------------------------------------")
    result_parts.append("// Global Module Fragment (GMF): Standard Library Headers")
    result_parts.append("// ----------------------------------------------------------------------------")
    for header in all_gmf_headers:
        if header not in platform_headers and header not in conditional_stl_headers:
            result_parts.append(f"#include <{header}>")
    result_parts.append("")

    result_parts.append("// Platform-Specific Headers in GMF")
    result_parts.append("#if defined(_WIN32)")
    result_parts.append("#  define WIN32_LEAN_AND_MEAN")
    result_parts.append("#  define NOMINMAX")
    result_parts.append("#  include <windows.h>")
    result_parts.append("#  include <io.h>")
    result_parts.append("#else")
    result_parts.append("#  include <unistd.h>")
    result_parts.append("#endif")
    result_parts.append("")

    result_parts.append("// Conditional Standard Headers for C++20 / C++23 / C++26 in GMF")
    result_parts.append("#if defined(__has_include)")
    result_parts.append("#  if __has_include(<version>)")
    result_parts.append("#    include <version>")
    result_parts.append("#  endif")
    result_parts.append("#  if __has_include(<string_view>)")
    result_parts.append("#    include <string_view>")
    result_parts.append("#  endif")
    result_parts.append("#  if __has_include(<ranges>)")
    result_parts.append("#    include <ranges>")
    result_parts.append("#  endif")
    result_parts.append("#  if __has_include(<format>) && (defined(_MSVC_LANG) ? _MSVC_LANG >= 202002L : __cplusplus >= 202002L)")
    result_parts.append("#    include <format>")
    result_parts.append("#  endif")
    result_parts.append("#  if __has_include(<expected>) && (defined(_MSVC_LANG) ? _MSVC_LANG >= 202302L : __cplusplus >= 202302L)")
    result_parts.append("#    include <expected>")
    result_parts.append("#  endif")
    result_parts.append("#  if __has_include(<print>) && (defined(_MSVC_LANG) ? _MSVC_LANG >= 202302L : __cplusplus >= 202302L)")
    result_parts.append("#    include <print>")
    result_parts.append("#  endif")
    result_parts.append("#endif")
    result_parts.append("")
    result_parts.append("")

    result_parts.append("// ============================================================================")
    result_parts.append("// Module Purview: compat")
    result_parts.append("// ============================================================================")
    result_parts.append("export module compat;")
    result_parts.append("")

    if purview_lines:
        result_parts.extend(purview_lines)
        result_parts.append("")

    result_parts.append("// ============================================================================")
    result_parts.append("// End of C++20 Module Interface Unit: dist/compat.ixx")
    result_parts.append("// ============================================================================")
    result_parts.append("")

    return "\n".join(result_parts)


def export_module(input_header: Path, output_ixx: Path, verbose: bool = False) -> Path:
    """Main export function."""
    repo_root = find_repo_root(input_header.parent)
    if not input_header.is_absolute():
        input_header = (repo_root / input_header).resolve()
    if not output_ixx.is_absolute():
        output_ixx = (repo_root / output_ixx).resolve()

    # If input header does not exist, try bundling it first
    if not input_header.exists():
        if verbose:
            print(f"[INFO] '{input_header}' not found. Attempting to bundle from include/compat...")
        compat_dir = repo_root / "include" / "compat"
        if bundle_header and compat_dir.exists():
            bundle_header.bundle(compat_dir, input_header, verbose=verbose)
        else:
            raise FileNotFoundError(f"Cannot generate module: input header '{input_header}' does not exist and cannot be bundled.")

    with open(input_header, "r", encoding="utf-8-sig", errors="replace") as f:
        header_content = f.read()

    module_content = convert_to_module(header_content, verbose=verbose)

    # Ensure output directory exists
    output_ixx.parent.mkdir(parents=True, exist_ok=True)

    # Write output with UTF-8 BOM
    with open(output_ixx, "w", encoding="utf-8-sig", newline="\n") as f:
        f.write(module_content)

    # Verify UTF-8 BOM
    raw_bytes = output_ixx.read_bytes()
    assert raw_bytes.startswith(b"\xef\xbb\xbf"), "Error: Output module does not contain UTF-8 BOM!"

    line_count = len(module_content.splitlines())
    byte_count = len(raw_bytes)
    print(f"[SUCCESS] C++20 Module Interface Unit generated: {output_ixx} ({line_count} lines, {byte_count} bytes, UTF-8 BOM verified)")
    return output_ixx


def main():
    parser = argparse.ArgumentParser(description="CPP-Compat C++20 Module Exporter")
    parser.add_argument(
        "--input", "-i",
        type=str,
        default="dist/compat.hpp",
        help="Path to single-header dist/compat.hpp (default: dist/compat.hpp)"
    )
    parser.add_argument(
        "--output", "-o",
        type=str,
        default="dist/compat.ixx",
        help="Path to output module unit (default: dist/compat.ixx)"
    )
    parser.add_argument(
        "--verbose", "-v",
        action="store_true",
        help="Enable detailed diagnostic logging"
    )

    args = parser.parse_args()
    script_dir = Path(__file__).resolve().parent
    repo_root = find_repo_root(script_dir)

    input_path = Path(args.input)
    if not input_path.is_absolute():
        input_path = repo_root / input_path

    output_path = Path(args.output)
    if not output_path.is_absolute():
        output_path = repo_root / output_path

    try:
        export_module(input_path, output_path, verbose=args.verbose)
    except Exception as e:
        sys.stderr.write(f"[ERROR] Module export failed: {e}\n")
        sys.exit(1)


if __name__ == "__main__":
    main()
