#include <sigil/util/timer.h>
#include <sigil/util/fs.h>
#include <sigil/status.h>
#include <system_error>
#include <filesystem>
#include <iostream>
#include <cstdlib>
#include <cstddef>
#include <cctype>
#include <cstdio>
#include <string>
#include <vector>
#include <array>

using std::string;

namespace sigil::util {

file_handler_t::file_handler_t(const std::filesystem::path path) {
    this->path = path;
    this->file_size = 0;
    this->data = nullptr;
    this->status = VM_OK;

    sigil::util::timer_t tm;

    tm.start();

    std::error_code ec;
    auto sz = std::filesystem::file_size(this->path, ec);
    if (ec) {
        status = VM_IO_ERROR;
        return;
    }
    
    file_size = static_cast<size_t>(sz);

    std::cout << "[INFO] Size of " << path.string() << ": " << file_size / (1024 * 1024) << "MB" << std::endl;
    
    data = static_cast<uint8_t*>(std::malloc(file_size));

    if (!data) {
        status = VM_OUT_OF_MEMORY;
        file_size = 0;
        return;
    }

    FILE *fp = std::fopen(path.string().c_str(), "rb");
    if (!fp) {
        status = VM_IO_ERROR;
        std::free(data);
        data = nullptr;
        file_size = 0;
        return;
    }

    size_t read_size = std::fread(data, 1, file_size, fp);
    std::fclose(fp);

    if (read_size != file_size) {
        status = VM_IO_ERROR;
        std::free(data);
        data = nullptr;
        file_size = 0;
        return;
    }

    tm.stop();
    std::cout << "[INFO] Loaded file " << path.string() << " in " << tm.elapsed_milliseconds() << " ms" << std::endl;
}    


file_handler_t::~file_handler_t() {
    if (this->data) {
        std::free(data);
        this->data = nullptr;
    }
}

status_t file_handler_t::save_to(std::filesystem::path path) {
    if (!data || file_size == 0)
        return VM_SKIPPED;

    sigil::util::timer_t tm;
    tm.start();

    FILE *fp = std::fopen(path.string().c_str(), "wb");
    if (!fp)
        return VM_IO_ERROR;

    size_t written = std::fwrite(data, 1, file_size, fp);
    std::fclose(fp);

    if (written != file_size)
        return VM_IO_ERROR;

    tm.stop();
    std::cout << "[INFO] Saved file " << path.string() << " in " << tm.elapsed_milliseconds() << " ms" << std::endl;

    return VM_OK;
}




}








namespace sigil::util {
namespace {

// -----------------------------------------------------------------------------
// Protected paths table (immutable, platform-agnostic entries are POSIX-style)
// These are normalized in the hybrid representation by normalize_path()
// -----------------------------------------------------------------------------
static const std::array<const char*, 11> k_protected_list_cstr = {
    "/bin", "/sbin", "/etc", "/usr", "/boot", "/lib", "/lib64", "/dev", "/proc", "/sys", nullptr
};

// We will keep a small vector<string> of normalized protected paths that we can
// compare quickly. Build-once at first use.
static std::vector<std::string> s_protected_paths_normalized;

// Helper: ensure the protected list is normalized (lazy init)
static void ensure_protected_list_initialized();

// -----------------------------------------------------------------------------
// Platform helpers
// -----------------------------------------------------------------------------
#if defined(_WIN32) || defined(_WIN64)
static constexpr bool kIsWindows = true;
#else
static constexpr bool kIsWindows = false;
#endif

// -----------------------------------------------------------------------------
// String / path helpers
// -----------------------------------------------------------------------------

// Return true if input is empty (length == 0)
static bool is_null_or_empty(const std::string& p) noexcept {
    return p.empty();
}

// On Windows: replace backslashes with forward slashes. On non-Windows: no-op.
// We avoid touching escape sequences; we simply replace path separators.
static void normalize_slashes_for_platform(std::string& s) noexcept {
    if (!kIsWindows) return;
    for (char& c : s) {
        if (c == '\\') c = '/';
    }
}

// Convert our hybrid textual path to native std::filesystem::path for I/O.
// Hybrid rules:
//  - On Windows: "/C/Windows/..." -> "C:/Windows/..." (drive retained).
//  - On non-Windows: the path is returned as-is.
static std::filesystem::path to_native_path(const std::string& hybrid) {
    if (hybrid.empty()) return std::filesystem::path();

    if constexpr (kIsWindows) {
        // Hybrid absolute Windows path starts with "/<Letter>/" or "/<Letter>:" variant
        // Accept forms like "/C/..." or "/C:" or "C:/..." (if user passed native style)
        if (hybrid.size() >= 3 && hybrid[0] == '/' && std::isalpha(static_cast<unsigned char>(hybrid[1])) && hybrid[2] == '/') {
            // "/C/rest" -> "C:/rest"
            std::string native;
            native.reserve(hybrid.size());
            native.push_back(hybrid[1]); // drive letter
            native.push_back(':');
            native.append(hybrid.substr(2)); // includes the leading '/'
            // std::filesystem::path accepts "C:/..."
            return std::filesystem::path(native);
        }
        // If starts with "/C:" (e.g. "/C:")
        if (hybrid.size() >= 3 && hybrid[0] == '/' && std::isalpha(static_cast<unsigned char>(hybrid[1])) && hybrid[2] == ':') {
            std::string native = hybrid.substr(1); // remove leading '/'
            return std::filesystem::path(native);
        }

        // Otherwise, treat as native-like already: replace forward slashes with backslash if desired
        std::string tmp = hybrid;
        normalize_slashes_for_platform(tmp);
        return std::filesystem::path(tmp);
    } else {
        // POSIX: leave unchanged (std::filesystem uses /)
        return std::filesystem::path(hybrid);
    }
}

// Convert native std::filesystem::path into our hybrid string representation.
// - On Windows: "C:/Windows" -> "/C/Windows"
// - On POSIX: keep as-is and use generic_string() to ensure '/'
static std::string from_native_path(const std::filesystem::path& p) {
    if (p.empty()) return std::string();

    if constexpr (kIsWindows) {
        // canonical string using forward slashes
        std::string s = p.generic_string(); // "C:/Windows/..."
        // If starts with "X:/" convert to "/X/..."
        if (s.size() >= 2 && std::isalpha(static_cast<unsigned char>(s[0])) && s[1] == ':') {
            std::string out;
            out.reserve(s.size() + 1);
            out.push_back('/');
            out.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(s[0]))));
            // Append remainder after "X:"
            if (s.size() >= 3 && s[2] == '/') {
                out.append(s.substr(2)); // keep leading '/'
            } else {
                out.push_back('/'); // ensure slash
                if (s.size() > 2) out.append(s.substr(2));
            }
            return out;
        }
        // Otherwise, return generic_string unchanged but ensure it starts with '/'
        std::string s2 = p.generic_string();
        if (!s2.empty() && s2.front() != '/') {
            return std::string("/") + s2;
        }
        return s2;
    } else {
        // POSIX: generic_string preserves '/'
        return p.generic_string();
    }
}

// Normalize a path according to our hybrid rules:
// - convert slashes for platform
// - apply std::filesystem::absolute + lexically_normal to get canonical-like path
// - convert back to hybrid textual form (drive mapping on Windows)
static std::string normalize_path(std::string path) {
    if (path.empty()) return std::string();

    // On Windows convert backslashes to slashes first so generic_string/absolute works predictably
    normalize_slashes_for_platform(path);

    std::error_code ec;
    std::filesystem::path native = to_native_path(path);

    // Convert to absolute (relative paths resolved against current working directory)
    std::filesystem::path abs = std::filesystem::absolute(native, ec);
    if (ec) {
        // Fallback: use lexical normalization on the native path directly
        std::filesystem::path lex = native.lexically_normal();
        return from_native_path(lex);
    }

    // Lexically normal to collapse "." and ".."
    std::filesystem::path norm = abs.lexically_normal();

    // Convert to hybrid textual form
    return from_native_path(norm);
}

// Join two paths and normalize the result.
// If `right` is absolute (in hybrid or native sense), it will be normalized and returned.
static std::string join_paths(const std::string& left, const std::string& right) {
    if (right.empty()) {
        return normalize_path(left);
    }
    // Quick check: if right looks absolute in hybrid representation (starts with '/')
    // then return normalized right directly.
    // Note: On Windows right may be "/C/..." which is absolute in hybrid form.
    if (!right.empty() && right.front() == '/') {
        return normalize_path(right);
    }

    // Otherwise use native path join then normalize.
    std::error_code ec;
    std::filesystem::path native_left = to_native_path(normalize_path(left));
    std::filesystem::path native_right = to_native_path(right);
    std::filesystem::path combined = native_left / native_right;
    combined = combined.lexically_normal();
    return from_native_path(combined);
}

// Return the parent directory in hybrid form. If input is root, return root.
static std::string get_parent(const std::string& path) {
    std::string norm = normalize_path(path);
    if (norm.empty()) return std::string();

    std::filesystem::path native = to_native_path(norm);
    std::filesystem::path parent = native.parent_path();
    if (parent.empty()) {
        // parent of root -> root
        return normalize_path(std::string("/"));
    }
    std::string res = from_native_path(parent.lexically_normal());
    if (res.empty()) return normalize_path(std::string("/"));
    return res;
}

// Get filename (last path component) in textual form
static std::string get_filename(const std::string& path) {
    std::string norm = normalize_path(path);
    if (norm.empty()) return std::string();
    std::filesystem::path native = to_native_path(norm);
    return native.filename().generic_string();
}

// Is normalized path exactly root?
static bool is_root(const std::string& path) {
    std::string norm = normalize_path(path);
    if (norm.empty()) return false;
    // After normalization hybrid root should be "/"
    return norm == "/";
}

// Ensure protected list is initialized into s_protected_paths_normalized
static void ensure_protected_list_initialized() {
    if (!s_protected_paths_normalized.empty()) return;
    s_protected_paths_normalized.reserve(k_protected_list_cstr.size());
    for (const char* p = k_protected_list_cstr[0]; p != nullptr; ) {
        // iterate manually because array contains nullptr sentinel at end
        break;
    }
    // Simpler iteration:
    for (size_t i = 0; i < k_protected_list_cstr.size(); ++i) {
        const char* entry = k_protected_list_cstr[i];
        if (entry == nullptr) break;
        std::string norm = normalize_path(std::string(entry));
        if (!norm.empty()) s_protected_paths_normalized.push_back(std::move(norm));
    }
}

// Check if a normalized path is protected (prefix match at path boundary)
static bool is_protected_path(const std::string& path) {
    std::string norm = normalize_path(path);
    if (norm.empty()) return false;
    ensure_protected_list_initialized();
    for (const std::string& prot : s_protected_paths_normalized) {
        if (prot.empty()) continue;
        if (norm == prot) return true;
        // match prefix with directory boundary: prot + '/'
        if (norm.size() > prot.size() && norm.compare(0, prot.size(), prot) == 0) {
            // ensure next char is a slash
            if (norm[prot.size()] == '/') return true;
        }
    }
    return false;
}

// Validation helpers return sigil::status_t codes
static status_t validate_read(const std::string& path) {
    if (is_null_or_empty(path)) return VM_ARG_NULL;
    // normalization will throw? we handle with checking returned string
    std::string norm = normalize_path(path);
    if (norm.empty()) return VM_ARG_INVALID;
    return VM_OK;
}

static status_t validate_write(const std::string& path) {
    if (is_null_or_empty(path)) return VM_ARG_NULL;
    std::string norm = normalize_path(path);
    if (norm.empty()) return VM_ARG_INVALID;
    // forbids operating on root
    if (is_root(norm)) return VM_INVALID_STATE;
    // forbids writing into protected paths
    if (is_protected_path(norm)) return VM_PERMISSION_DENIED;
    return VM_OK;
}

} // anonymous namespace

// -----------------------------------------------------------------------------
// Exported (internal) helpers to be used by implementation in this translation unit
// (If you later want to expose them publicly, hoist declarations into a header)
// -----------------------------------------------------------------------------

// Note: these are not part of public header, they are small wrappers we expose
// within the sigil::util namespace for other cpp units to call if necessary.

std::string fs_internal_normalize_path(const std::string& p) {
    return normalize_path(p);
}

std::string fs_internal_join_paths(const std::string& left, const std::string& right) {
    return join_paths(left, right);
}

std::string fs_internal_get_parent(const std::string& p) {
    return get_parent(p);
}

std::string fs_internal_get_filename(const std::string& p) {
    return get_filename(p);
}

bool fs_internal_is_root(const std::string& p) {
    return is_root(p);
}

bool fs_internal_is_protected(const std::string& p) {
    return is_protected_path(p);
}

status_t fs_internal_validate_read(const std::string& p) {
    return validate_read(p);
}

status_t fs_internal_validate_write(const std::string& p) {
    return validate_write(p);
}







} // namespace sigil::util
