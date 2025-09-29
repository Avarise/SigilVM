#pragma once

#include <sigil/status.h>
#include <filesystem>
#include <cstdint>
#include <cstddef>
#include <string>

namespace sigil::util {

/**
 * @brief Filesystem operation flags.
 * Flags are bitwise combinable and control how path operations behave.
 */
enum fs_flags : uint32_t {

    FS_NONE                         = 0u,

    /**
     * Allow overwriting existing files or directories.
     *
     * - For file targets: allows truncation or replacement.
     * - For directory targets: allows replacing pre-existing trees.
     *
     * NOTE: If the target is inside a protected path,
     * overwrite is *never* allowed.
     */
    FS_OVERWRITE                    = SIGIL_BIT(0),

    /**
     * Recurse into directory contents when copying or nullifying.
     */
    FS_RECURSIVE                    = SIGIL_BIT(1),

    /**
     * Ignore symlink entries encountered inside directory traversal.
     *
     * - For copy: symlinks inside directories are skipped entirely.
     * - For nullify: symlinks are unlinked (deleted), but their targets
     *   are never followed.
     *
     * This flag does *not* affect behavior for the root source path.
     */
    FS_PRUNE_SYMLINKS               = SIGIL_BIT(2),

    /**
     * Do not delete the root path itself during nullification.
     *
     * - Directory: all contents removed, directory remains empty.
     * - File: file is truncated to zero size.
     */
    FS_PRESERVE_ROOT                = SIGIL_BIT(3),

    /**
     * Dereference a symlink only when the *source root* is itself a symlink.
     *
     * Semantics:
     * - If this flag is set and the source path is a symlink:
     *       → follow the symlink and operate on its resolved target.
     *
     * - If this flag is NOT set:
     *      - If FS_PRUNE_SYMLINKS is set: skip the symlink root entirely.
     *      - Otherwise: copy/nullify/compare the symlink itself (never follow).
     *
     * This flag affects *only the source root*, not interior traversal.
     */
    FS_DEREFERENCE_ROOT_SYMLINK     = SIGIL_BIT(4),
};

/**
 * @brief 
 * RAII based file handler, that exposes contents as raw, heap allocated chunk of memory.
 */
struct file_handler_t {
    std::filesystem::path path;
    size_t file_size;
    status_t status;
    uint8_t *data;

    // Standard Constructor
    file_handler_t(const std::filesystem::path path);
    ~file_handler_t();

    // Allows to save a file to a new path
    status_t save_to(const std::filesystem::path path);
};


/* ========================================================================== */
/*  Utility functions provided elsewhere                                      */
/* ========================================================================== */

/**
 * Returns true if the given path refers to an existing filesystem entry.
 */
bool fs_exists(const char* path);

/**
 * Returns true if the path is a directory.
 */
bool fs_is_directory(const char* path);

/**
 * Returns true if the path is a regular file.
 */
bool fs_is_file(const char* path);

/**
 * Returns true if the given path is considered "protected."
 *
 * Protected paths must never be nullified and must never have overwrite
 * operations applied inside them.
 */
bool fs_is_protected_path(const char* path);


/* ========================================================================== */
/*  Primary API                                                                */
/* ========================================================================== */

/**
 * @brief Copy a filesystem entry from source to target.
 *
 * Behavior summary:
 *
 * - If source is a file:
 *      - If target exists: FS_OVERWRITE is required.
 *
 * - If source is a directory:
 *      - If target does not exist: create directory.
 *      - If target exists:
 *           * Must be a directory
 *           * FS_OVERWRITE allows replacing contents
 *      - FS_RECURSIVE required to copy contents.
 *
 * - Symlink handling:
 *      * Interior symlinks obey FS_PRUNE_SYMLINKS.
 *      * Root symlink obeys FS_DEREFERENCE_ROOT_SYMLINK.
 *
 * - Protected path constraints:
 *      * Copying INTO a protected target is allowed only if no overwrite occurs.
 *
 * @return status code from sigil/status.h
 */
status_t fs_copy_from_path(
    const char* source,
    const char* target,
    uint32_t flags
);

/**
 * @brief std::string overload
 */
inline status_t fs_copy_from_path(
    const std::string& source,
    const std::string& target,
    uint32_t flags
) {
    return fs_copy_from_path(source.c_str(), target.c_str(), flags);
}


/**
 * @brief Nullify a filesystem entry.
 *
 * Behavior summary:
 *
 * - Protected paths cannot be nullified. Always fails.
 *
 * - If FS_PRESERVE_ROOT:
 *      - Directory: recursively remove all contents, directory remains.
 *      - File: truncate to zero size.
 *
 * - Without FS_PRESERVE_ROOT:
 *      - File: unlink/remove
 *      - Directory: remove entire subtree (FS_RECURSIVE required)
 *
 * - Symlink handling:
 *      * Symlink roots obey FS_DEREFERENCE_ROOT_SYMLINK.
 *      * Interior symlinks are never followed:
 *            - With PRUNE → skipped
 *            - Without PRUNE → unlink symlink, do not touch target
 *
 * @return status code from sigil/status.h
 */
status_t fs_nullify_path(
    const char* path,
    uint32_t flags
);

/**
 * @brief std::string overload
 */
inline status_t fs_nullify_path(
    const std::string& path,
    uint32_t flags
) {
    return fs_nullify_path(path.c_str(), flags);
}


/**
 * @brief Determine whether two paths refer to the same underlying filesystem object.
 *
 * Behavior:
 * - Uses canonical path resolution (realpath) unless both paths are
 *   symlinks and FS_PRUNE_SYMLINKS is set.
 *
 * - FS_DEREFERENCE_ROOT_SYMLINK:
 *       * If set, source symlink root and target symlink root are
 *         dereferenced before comparison.
 *       * If not set, comparing two symlinks compares the link itself.
 *
 * - Returns false if either path does not exist.
 *
 * @return true if paths point to the same underlying filesystem entry.
 */
bool fs_are_identical(
    const char* source,
    const char* target,
    uint32_t flags
);

/**
 * @brief std::string overload
 */
inline bool fs_are_identical(
    const std::string& source,
    const std::string& target,
    uint32_t flags
) {
    return fs_are_identical(source.c_str(), target.c_str(), flags);
}




} // namespace sigil::util
