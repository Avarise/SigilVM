#pragma once

#include <thaumaturgy/thaumaturgy.h>
#include <filesystem>

namespace sigil::data {

/**
 * Deduplicate files from src into dst.
 *
 * - Hashes all regular files under src (recursively)
 * - Moves only unique files into dst
 * - Preserves directory structure
 * - Renames on filename collision by prefixing file hash
 * - Does not modify duplicates in src
 *
 * If dry_run == true:
 *   - No files are moved
 *   - Action plan is written to ~/.cache/sigilvm/dedup/run-<timestamp>.txt
 *
 * Returns thaumaturgy::yield describing success or failure.
 */
::thaumaturgy::yield dedup(
    const std::filesystem::path& src,
    const std::filesystem::path& dst,
    bool dry_run = true
) noexcept;

} // namespace sigil::tools
