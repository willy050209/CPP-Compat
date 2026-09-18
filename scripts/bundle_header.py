#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
scripts/bundle_header.py
========================
CPP-Compat Single-Header Bundler / Amalgamation Script

Responsibilities:
1. Recursively scans `include/compat/` for all C++ headers (.hpp, .h).
2. Parses internal `#include "..."` dependencies and resolves their targets.
3. Performs a topological sort so prerequisites appear before dependents.
4. Removes internal relative `#include "..."` and individual `#pragma once` directives.
5. Collects and deduplicates top-level external STL headers (<cstdint>, <iostream>, etc.).
6. Retains XML documentation comments (/// <summary>, etc.) and regular code comments.
7. Emits a clean, self-contained single-header distribution to `dist/compat.hpp`.
8. Enforces UTF-8 BOM encoding for output files per project specifications.
"""

import os
import sys
import re
import argparse
from pathlib import Path
from collections import defaultdict, deque


# Regex patterns
RE_PRAGMA_ONCE = re.compile(r'^\s*#\s*pragma\s+once\b')
RE_INCLUDE_INTERNAL = re.compile(r'^\s*#\s*include\s*"([^"]+)"')
RE_INCLUDE_EXTERNAL = re.compile(r'^\s*#\s*include\s*<([^>]+)>')
RE_PP_IF = re.compile(r'^\s*#\s*(if|ifdef|ifndef)\b')
RE_PP_ENDIF = re.compile(r'^\s*#\s*endif\b')
RE_DEFINE = re.compile(r'^\s*#\s*define\s+(COMPAT_[A-Za-z0-9_]+)')

# Private helper macros that must be cleaned up at the end of the bundled single-header
# to prevent polluting consumer code, while retaining public API macros (e.g. COMPAT_NODISCARD).
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
    "COMPAT_BAD_EXPECTED_ACCESS_DEFINED",
]

# Public API macros that must be retained and NEVER undef'ed
PUBLIC_API_MACROS = {
    "COMPAT_NODISCARD",
    "COMPAT_FORCE_FALLBACK",
    "COMPAT_FORCE_SELF_IMPLEMENTATION",
    "COMPAT_FORCE_STD_IMPLEMENTATION",
    "COMPAT_ENABLE_VARIANT_EXPECTED",
    "COMPAT_ENABLE_UNION_EXPECTED",
}


def collect_cleanup_macros(parsed_data: dict[Path, dict]) -> list[str]:
    """
    Collects internal private helper macros from parsed headers to emit in the cleanup block.
    Retains public API macros while ensuring all internal helper macros (COMPAT_CXX_*,
    COMPAT_HAS_*, COMPAT_CPLUSPLUS, etc.) are undefined.
    """
    cleanup_set = set(DEFAULT_CLEANUP_MACROS)

    for data in parsed_data.values():
        for line in data["cleaned_lines"]:
            m = RE_DEFINE.match(line)
            if m:
                macro_name = m.group(1)
                if macro_name in PUBLIC_API_MACROS:
                    continue
                if (macro_name.startswith("COMPAT_CXX_") or
                    macro_name.startswith("COMPAT_HAS_") or
                    macro_name.startswith("COMPAT_INTERNAL_") or
                    macro_name in cleanup_set):
                    cleanup_set.add(macro_name)

    ordered = [m for m in DEFAULT_CLEANUP_MACROS if m in cleanup_set]
    extras = sorted([m for m in cleanup_set if m not in DEFAULT_CLEANUP_MACROS])
    return ordered + extras


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


def scan_headers(compat_dir: Path) -> list[Path]:
    """Recursively scan for .hpp and .h files in include/compat/."""
    if not compat_dir.exists():
        return []
    files = [p.resolve() for p in compat_dir.rglob("*") if p.is_file() and p.suffix.lower() in (".hpp", ".h")]
    return sorted(files)


def resolve_include(source_file: Path, target_str: str, compat_dir: Path, include_dir: Path, all_headers: set[Path]) -> Path | None:
    """Resolve an internal #include "..." path to a known header file."""
    candidates = [
        (source_file.parent / target_str).resolve(),
        (compat_dir / target_str).resolve(),
        (include_dir / target_str).resolve(),
        (compat_dir / target_str.replace("compat/", "")).resolve(),
        (compat_dir / "detail" / target_str).resolve() if "detail" not in target_str else None,
    ]
    for c in candidates:
        if c and c in all_headers:
            return c
    return None


def parse_header_file(file_path: Path, compat_dir: Path, include_dir: Path, all_headers: set[Path]):
    """
    Parse a single header file:
    - Identifies dependencies on other internal headers.
    - Extracts top-level unconditional external STL headers for deduplication.
    - Preserves conditional external STL headers in-place.
    - Strips #pragma once and internal #include "...".
    - Preserves XML comments and implementation code.
    """
    with open(file_path, "r", encoding="utf-8-sig", errors="replace") as f:
        content = f.read()

    lines = content.splitlines()
    cleaned_lines: list[str] = []
    internal_deps: set[Path] = set()
    top_external_includes: list[str] = []

    pp_depth = 0

    for line in lines:
        stripped = line.strip()

        # Track preprocessor nesting depth
        if RE_PP_IF.match(stripped):
            pp_depth += 1
            cleaned_lines.append(line)
            continue
        elif RE_PP_ENDIF.match(stripped):
            pp_depth = max(0, pp_depth - 1)
            cleaned_lines.append(line)
            continue

        # Strip #pragma once
        if RE_PRAGMA_ONCE.match(stripped):
            continue

        # Check internal include: #include "..."
        m_int = RE_INCLUDE_INTERNAL.match(stripped)
        if m_int:
            target_str = m_int.group(1)
            resolved = resolve_include(file_path, target_str, compat_dir, include_dir, all_headers)
            if resolved:
                internal_deps.add(resolved)
                # Internal include is inlined / resolved by topological order, remove the directive
                continue
            else:
                # If cannot resolve internally, treat as relative external or preserve
                cleaned_lines.append(line)
                continue

        # Check external include: #include <...>
        m_ext = RE_INCLUDE_EXTERNAL.match(stripped)
        if m_ext:
            header_name = m_ext.group(1)
            if pp_depth == 0:
                # Top-level unconditional external include: extract for global deduplication
                top_external_includes.append(header_name)
                continue
            else:
                # Conditional external include (e.g. inside #if COMPAT_HAS_...): keep in-place!
                cleaned_lines.append(line)
                continue

        # Regular line (code, XML comment, standard comment, empty line)
        cleaned_lines.append(line)

    return {
        "file": file_path,
        "internal_deps": internal_deps,
        "top_external_includes": top_external_includes,
        "cleaned_lines": cleaned_lines,
    }


def topological_sort(headers: list[Path], parsed_data: dict[Path, dict]) -> list[Path]:
    """
    Topologically sort headers using Kahn's algorithm with deterministic tie-breaking.
    Prerequisites appear before dependents.
    """
    in_degree = {h: 0 for h in headers}
    dependents = defaultdict(list)

    for h in headers:
        for dep in parsed_data[h]["internal_deps"]:
            if dep in in_degree:
                in_degree[h] += 1
                dependents[dep].append(h)

    # Deterministic sorting function for ready nodes
    def sort_key(p: Path):
        name = p.name.lower()
        rel = str(p).lower()
        # Config.hpp should always be first if possible
        is_config = 0 if "config" in name else 1
        # detail/ files should precede higher level files with equal in-degree
        is_detail = 0 if "detail" in rel else 1
        # Compat.hpp (umbrella header) should ideally be last
        is_compat_main = 1 if name == "compat.hpp" else 0
        return (is_config, is_detail, is_compat_main, rel)

    ready = [h for h in headers if in_degree[h] == 0]
    ready.sort(key=sort_key)
    queue = deque(ready)

    sorted_result: list[Path] = []

    while queue:
        # To maintain priority at every stage, sort queue if multiple elements
        if len(queue) > 1:
            sorted_q = sorted(list(queue), key=sort_key)
            queue = deque(sorted_q)

        curr = queue.popleft()
        sorted_result.append(curr)

        for dep in dependents[curr]:
            in_degree[dep] -= 1
            if in_degree[dep] == 0:
                queue.append(dep)

    if len(sorted_result) != len(headers):
        # Cycle detected, append remaining in deterministic order
        unresolved = [h for h in headers if h not in sorted_result]
        unresolved.sort(key=sort_key)
        sys.stderr.write(f"[WARN] Dependency cycle detected or unresolvable dependencies in {len(unresolved)} files. Appending remaining.\n")
        sorted_result.extend(unresolved)

    return sorted_result


def clean_blank_lines(lines: list[str]) -> list[str]:
    """Collapse consecutive blank lines to at most two."""
    res = []
    blank_count = 0
    for line in lines:
        if line.strip() == "":
            blank_count += 1
            if blank_count <= 2:
                res.append("")
        else:
            blank_count = 0
            res.append(line)
    return res


def bundle(compat_dir: Path, output_file: Path, verbose: bool = False) -> Path:
    """Main bundling function."""
    repo_root = find_repo_root(compat_dir)
    include_dir = repo_root / "include"
    if not compat_dir.is_absolute():
        compat_dir = (repo_root / compat_dir).resolve()
    if not output_file.is_absolute():
        output_file = (repo_root / output_file).resolve()

    headers = scan_headers(compat_dir)
    if not headers:
        raise FileNotFoundError(f"No header files found in '{compat_dir}'.")

    all_headers_set = set(headers)
    parsed_data = {}
    all_top_externals: list[str] = []

    for h in headers:
        data = parse_header_file(h, compat_dir, include_dir, all_headers_set)
        parsed_data[h] = data
        for ext in data["top_external_includes"]:
            if ext not in all_top_externals:
                all_top_externals.append(ext)

    sorted_headers = topological_sort(headers, parsed_data)

    if verbose:
        print(f"[INFO] Scanned {len(headers)} header(s).")
        print("[INFO] Topological order:")
        for idx, h in enumerate(sorted_headers, start=1):
            rel = h.relative_to(repo_root) if repo_root in h.parents else h.name
            deps = [d.name for d in parsed_data[h]["internal_deps"]]
            print(f"  {idx:2d}. {rel} (depends on: {', '.join(deps) if deps else 'none'})")

    # Sort external STL includes alphabetically for clean presentation
    all_top_externals_sorted = sorted(all_top_externals)

    # Build bundled file content
    parts: list[str] = []

    # Header banner
    parts.append("// ============================================================================")
    parts.append("// CPP-Compat: Zero-Dependency Modern C++ Backward Compatibility Shim Layer")
    parts.append("// Single-Header Distribution (dist/compat.hpp)")
    parts.append("//")
    parts.append("// This file is automatically generated by scripts/bundle_header.py.")
    parts.append("// Do not edit this file directly. Edit files in include/compat/ instead.")
    parts.append("// ============================================================================")
    parts.append("")
    parts.append("#pragma once")
    parts.append("")

    # Deduplicated external STL headers
    if all_top_externals_sorted:
        parts.append("// ----------------------------------------------------------------------------")
        parts.append("// Standard Library Headers (Deduplicated)")
        parts.append("// ----------------------------------------------------------------------------")
        for ext in all_top_externals_sorted:
            parts.append(f"#include <{ext}>")
        parts.append("")

    # Section for each header in topological order
    for h in sorted_headers:
        rel_path = h.relative_to(repo_root) if repo_root in h.parents else h.name
        norm_rel = str(rel_path).replace("\\", "/")

        data = parsed_data[h]
        cleaned_body = clean_blank_lines(data["cleaned_lines"])
        # Strip leading and trailing empty lines in section
        while cleaned_body and cleaned_body[0].strip() == "":
            cleaned_body.pop(0)
        while cleaned_body and cleaned_body[-1].strip() == "":
            cleaned_body.pop()

        if cleaned_body:
            parts.append("// ============================================================================")
            parts.append(f"// Module Section: {norm_rel}")
            parts.append("// ============================================================================")
            parts.append("")
            parts.extend(cleaned_body)
            parts.append("")

    # Internal preprocessor cleanup block
    cleanup_macros = collect_cleanup_macros(parsed_data)
    if cleanup_macros:
        parts.append("// ============================================================================")
        parts.append("// Internal Preprocessor Cleanup")
        parts.append("// Undefine internal helper macros to prevent macro leakage into consumer code,")
        parts.append("// while retaining public API macros (e.g. COMPAT_NODISCARD).")
        parts.append("// ============================================================================")
        for macro in cleanup_macros:
            parts.append(f"#undef {macro}")
        parts.append("")

    parts.append("// ============================================================================")
    parts.append("// End of Single-Header Distribution: dist/compat.hpp")
    parts.append("// ============================================================================")
    parts.append("")

    final_content = "\n".join(parts)

    # Ensure output directory exists
    output_file.parent.mkdir(parents=True, exist_ok=True)

    # Write output with UTF-8 BOM
    with open(output_file, "w", encoding="utf-8-sig", newline="\n") as f:
        f.write(final_content)

    # Verify UTF-8 BOM
    raw_bytes = output_file.read_bytes()
    assert raw_bytes.startswith(b"\xef\xbb\xbf"), "Error: Output file does not contain UTF-8 BOM!"

    line_count = len(final_content.splitlines())
    byte_count = len(raw_bytes)
    print(f"[SUCCESS] Bundled single-header generated: {output_file} ({line_count} lines, {byte_count} bytes, UTF-8 BOM verified)")
    return output_file


def main():
    parser = argparse.ArgumentParser(description="CPP-Compat Single-Header Bundler")
    parser.add_argument(
        "--input-dir", "-i",
        type=str,
        default="include/compat",
        help="Path to include/compat directory (default: include/compat)"
    )
    parser.add_argument(
        "--output", "-o",
        type=str,
        default="dist/compat.hpp",
        help="Path to output header (default: dist/compat.hpp)"
    )
    parser.add_argument(
        "--verbose", "-v",
        action="store_true",
        help="Enable detailed diagnostic logging"
    )

    args = parser.parse_args()
    script_dir = Path(__file__).resolve().parent
    repo_root = find_repo_root(script_dir)

    input_dir = Path(args.input_dir)
    if not input_dir.is_absolute():
        input_dir = repo_root / input_dir

    output_path = Path(args.output)
    if not output_path.is_absolute():
        output_path = repo_root / output_path

    try:
        bundle(input_dir, output_path, verbose=args.verbose)
    except Exception as e:
        sys.stderr.write(f"[ERROR] Bundling failed: {e}\n")
        sys.exit(1)


if __name__ == "__main__":
    main()
