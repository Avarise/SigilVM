#pragma once

#include <cstdint>
#include <cstddef>

namespace sigil {

// ============================================================
// 1. Channel layout (semantic meaning of channels)
// ============================================================

enum class channel_layout_t : uint8_t {
    unknown = 0,

    r,          // single channel
    rg,
    rgb,
    rgba,

    bgra,       // explicit alternative ordering
    argb,
    abgr,

    ya,         // luminance + alpha
};

// Number of channels for layout
constexpr inline uint8_t channel_count(channel_layout_t layout) {
    switch (layout) {
        case channel_layout_t::r:    return 1;
        case channel_layout_t::rg:   return 2;
        case channel_layout_t::rgb:  return 3;
        case channel_layout_t::rgba:
        case channel_layout_t::bgra:
        case channel_layout_t::argb:
        case channel_layout_t::abgr:
        case channel_layout_t::ya:   return 4;
        default:                     return 0;
    }
}

// ============================================================
// 2. Data representation (numeric storage)
// ============================================================

enum class data_type_t : uint8_t {
    unknown = 0,

    u8,     // uint8_t
    u16,    // uint16_t
    u32,    // uint32_t

    f16,    // half float (16-bit)
    f32,    // float
};

// Size in bytes for a single channel
constexpr inline uint8_t data_type_size(data_type_t t) {
    switch (t) {
        case data_type_t::u8:  return 1;
        case data_type_t::u16: return 2;
        case data_type_t::u32: return 4;
        case data_type_t::f16: return 2;
        case data_type_t::f32: return 4;
        default:               return 0;
    }
}

// ============================================================
// 3. Numeric interpretation (normalization)
// ============================================================

enum class numeric_format_t : uint8_t {
    unknown = 0,

    unorm,   // [0, max] -> [0.0, 1.0]
    snorm,   // signed normalized [-1, 1]
    uint,    // raw unsigned integer
    sint,    // raw signed integer
    float_,  // floating point
};

// ============================================================
// 4. Color space (meaning of RGB)
// ============================================================

enum class color_space_t : uint8_t {
    unknown = 0,

    srgb,          // standard display space (non-linear)
    linear_srgb,   // linearized sRGB primaries

    display_p3,
    rec2020,

    // analytical / perceptual (not for storage typically)
    cie_xyz,
    cie_lab,
    cie_lch,
};

// ============================================================
// 5. Encoding (transfer function)
// ============================================================

enum class encoding_t : uint8_t {
    unknown = 0,

    linear,    // no transform
    srgb,      // standard gamma curve
};

// ============================================================
// 6. Alpha representation
// ============================================================

enum class alpha_mode_t : uint8_t {
    none = 0,
    straight,
    premultiplied,
};

// ============================================================
// 7. Memory layout
// ============================================================

enum class memory_layout_t : uint8_t {
    unknown = 0,

    interleaved,   // RGBARGBA...
    planar,        // RRR... GGG... BBB...
};

// ============================================================
// 8. Image descriptor (fully specifies interpretation)
// ============================================================

struct image_desc_t {
    channel_layout_t layout      = channel_layout_t::unknown;
    data_type_t      type        = data_type_t::unknown;
    numeric_format_t numeric     = numeric_format_t::unknown;

    color_space_t    color_space = color_space_t::unknown;
    encoding_t       encoding    = encoding_t::unknown;

    alpha_mode_t     alpha       = alpha_mode_t::none;
    memory_layout_t  memory      = memory_layout_t::interleaved;

    // Optional flags (bitmask reserved for future use)
    uint32_t         flags       = 0;
};

// ============================================================
// 9. Image view (non-owning)
// ============================================================

struct image_view_t {
    const void* data   = nullptr;

    uint32_t width     = 0;
    uint32_t height    = 0;

    // Bytes between rows (may include padding)
    size_t   row_stride = 0;

    image_desc_t desc;
};

// Mutable variant
struct image_view_mut_t {
    void* data = nullptr;

    uint32_t width  = 0;
    uint32_t height = 0;

    size_t   row_stride = 0;

    image_desc_t desc;
};

// ============================================================
// 10. Helpers
// ============================================================

// Bytes per pixel (interleaved only)
constexpr inline size_t bytes_per_pixel(const image_desc_t& d) {
    return channel_count(d.layout) * data_type_size(d.type);
}

// Row size without padding
constexpr inline size_t row_size_bytes(const image_desc_t& d, uint32_t width) {
    return bytes_per_pixel(d) * width;
}

// Basic validity check (minimal, not exhaustive)
constexpr inline bool is_valid(const image_desc_t& d) {
    return
        d.layout      != channel_layout_t::unknown &&
        d.type        != data_type_t::unknown &&
        d.numeric     != numeric_format_t::unknown &&
        d.memory      != memory_layout_t::unknown;
}

// ============================================================
// 11. Canonical presets (explicit, no guessing)
// ============================================================

constexpr inline image_desc_t make_rgba8_srgb() {
    return {
        channel_layout_t::rgba,
        data_type_t::u8,
        numeric_format_t::unorm,
        color_space_t::srgb,
        encoding_t::srgb,
        alpha_mode_t::straight,
        memory_layout_t::interleaved,
        0
    };
}

constexpr inline image_desc_t make_rgba32f_linear() {
    return {
        channel_layout_t::rgba,
        data_type_t::f32,
        numeric_format_t::float_,
        color_space_t::linear_srgb,
        encoding_t::linear,
        alpha_mode_t::premultiplied,
        memory_layout_t::interleaved,
        0
    };
}

// ============================================================
// 12. Strongly-typed pixel structs (optional helpers)
// ============================================================

struct color_rgba8_t {
    uint8_t r, g, b, a;
};

struct color_rgba32f_t {
    float r, g, b, a;
};

static_assert(sizeof(color_rgba8_t)  == 4,  "Invalid packing");
static_assert(sizeof(color_rgba32f_t) == 16, "Invalid packing");

} // namespace sigil