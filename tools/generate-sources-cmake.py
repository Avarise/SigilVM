#!/usr/bin/env python3

"""
SigilVM — generate-sources-cmake.py

Purpose
-------
Automatically generate local source aggregation CMake fragments
for the SigilVM repository tree.

This tool DOES NOT replace CMake.
It only automates repetitive source enumeration and subtree traversal.

Philosophy
----------
- Human-written CMake remains authoritative.
- Generated files are structural only.
- No dependency inference.
- No install logic.
- No architecture decisions.
- No package semantics.

Generated files contain:
- local source aggregation
- add_subdirectory() calls
- grouped source variables

Recommended Usage
-----------------
Dry run:
    python tools/generate-sources-cmake.py --dry-run

Write generated files:
    python tools/generate-sources-cmake.py --write

Custom output filename:
    python tools/generate-sources-cmake.py --write --filename sources.generated.cmake

Repository Assumptions
----------------------
Expected repo layout:

repo/
    src/
    tests/
    include/

The tool recursively scans:
- src/
- tests/

and generates local CMake fragments for each subtree.

Generated fragments are intended to be INCLUDED
from manually-authored CMakeLists.txt files.

Example:
    include(sources.generated.cmake)

NOTES
-----
- Generated files are deterministic.
- Paths are always relative to owning directory.
- Hidden directories are ignored.
- build/ and .cache/ trees are ignored.
"""

from __future__ import annotations

import argparse
import pathlib
import sys
from dataclasses import dataclass, field
from typing import List


# ----------------------------------------------------------------------
# Configuration
# ----------------------------------------------------------------------

SOURCE_EXTENSIONS = {
    ".c",
    ".cc",
    ".cpp",
    ".cxx",
}

HEADER_EXTENSIONS = {
    ".h",
    ".hpp",
    ".hh",
    ".hxx",
}

IGNORED_DIRECTORIES = {
    ".git",
    ".cache",
    "build",
    "build-debug",
    "build-release",
    "out",
    "__pycache__",
}

DEFAULT_OUTPUT_FILENAME = "sources.generated.cmake"


# ----------------------------------------------------------------------
# Data Structures
# ----------------------------------------------------------------------

@dataclass
class DirectoryInfo:
    path: pathlib.Path
    sources: List[pathlib.Path] = field(default_factory=list)
    headers: List[pathlib.Path] = field(default_factory=list)
    subdirs: List[pathlib.Path] = field(default_factory=list)


# ----------------------------------------------------------------------
# Helpers
# ----------------------------------------------------------------------

def is_ignored(path: pathlib.Path) -> bool:
    return path.name in IGNORED_DIRECTORIES


def collect_directory_info(directory: pathlib.Path) -> DirectoryInfo:
    info = DirectoryInfo(path=directory)

    for child in sorted(directory.iterdir(), key=lambda p: p.name):
        if is_ignored(child):
            continue

        if child.is_dir():
            info.subdirs.append(child)
            continue

        if not child.is_file():
            continue

        suffix = child.suffix.lower()

        if suffix in SOURCE_EXTENSIONS:
            info.sources.append(child)

        elif suffix in HEADER_EXTENSIONS:
            info.headers.append(child)

    return info


def relative_name(path: pathlib.Path, base: pathlib.Path) -> str:
    return str(path.relative_to(base)).replace("\\", "/")


def variable_name_from_path(path: pathlib.Path, root: pathlib.Path) -> str:
    rel = path.relative_to(root)

    parts = ["SIGILVM"]

    for part in rel.parts:
        clean = (
            part.replace("-", "_")
                .replace(".", "_")
                .upper()
        )
        parts.append(clean)

    return "_".join(parts) + "_SOURCES"


# ----------------------------------------------------------------------
# Generation
# ----------------------------------------------------------------------

def generate_fragment(
    info: DirectoryInfo,
    repo_root: pathlib.Path,
) -> str:

    lines: List[str] = []

    rel_dir = relative_name(info.path, repo_root)

    lines.append("# ==================================================")
    lines.append(f"# Auto-generated source aggregation")
    lines.append(f"# Directory: {rel_dir}")
    lines.append("# DO NOT EDIT MANUALLY")
    lines.append("# ==================================================")
    lines.append("")

    variable_name = variable_name_from_path(info.path, repo_root)

    # ------------------------------------------------------------------
    # Subdirectories
    # ------------------------------------------------------------------

    if info.subdirs:
        lines.append("# Subdirectories")

        for subdir in info.subdirs:
            lines.append(f'add_subdirectory("{subdir.name}")')

        lines.append("")

    # ------------------------------------------------------------------
    # Sources
    # ------------------------------------------------------------------

    if info.sources:
        lines.append("# Local sources")
        lines.append(f"set({variable_name}")

        for source in info.sources:
            lines.append(f"    {source.name}")

        lines.append(")")
        lines.append("")

    # ------------------------------------------------------------------
    # Headers (optional informational grouping)
    # ------------------------------------------------------------------

    if info.headers:
        lines.append("# Local headers")

        for header in info.headers:
            lines.append(f"#   {header.name}")

        lines.append("")

    return "\n".join(lines)


# ----------------------------------------------------------------------
# Traversal
# ----------------------------------------------------------------------

def generate_for_tree(
    root: pathlib.Path,
    repo_root: pathlib.Path,
    output_filename: str,
    write: bool,
) -> None:

    for directory in sorted(root.rglob("*")):

        if not directory.is_dir():
            continue

        if is_ignored(directory):
            continue

        info = collect_directory_info(directory)

        has_content = (
            info.sources
            or info.headers
            or info.subdirs
        )

        if not has_content:
            continue

        content = generate_fragment(info, repo_root)

        output_path = directory / output_filename

        if write:
            output_path.write_text(content, encoding="utf-8")

            print(f"[WRITE] {relative_name(output_path, repo_root)}")

        else:
            print("=" * 80)
            print(relative_name(output_path, repo_root))
            print("=" * 80)
            print(content)
            print()


# ----------------------------------------------------------------------
# Main
# ----------------------------------------------------------------------

def main() -> int:

    parser = argparse.ArgumentParser(
        description="Generate local CMake source aggregation fragments."
    )

    parser.add_argument(
        "--write",
        action="store_true",
        help="Write generated files to disk."
    )

    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Print generated files without writing."
    )

    parser.add_argument(
        "--filename",
        default=DEFAULT_OUTPUT_FILENAME,
        help=f"Generated filename (default: {DEFAULT_OUTPUT_FILENAME})"
    )

    args = parser.parse_args()

    if not args.write and not args.dry_run:
        args.dry_run = True

    repo_root = pathlib.Path.cwd()

    targets = [
        repo_root / "src",
        repo_root / "tests",
    ]

    for target in targets:

        if not target.exists():
            continue

        generate_for_tree(
            root=target,
            repo_root=repo_root,
            output_filename=args.filename,
            write=args.write,
        )

    return 0


if __name__ == "__main__":
    raise SystemExit(main())