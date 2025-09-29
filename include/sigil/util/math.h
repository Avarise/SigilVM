#pragma once

#include <cmath>

namespace sigil {

// Basic vector types
struct vec2 { float_t x, y; };
struct vec3 { float_t x, y, z; };
struct vec4 { float_t x, y, z, w; };

// Common operations
inline vec3 add(const vec3& a, const vec3& b) { return {a.x+b.x, a.y+b.y, a.z+b.z}; }
inline float_t dot(const vec3& a, const vec3& b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
inline float_t length(const vec3& v) { return std::sqrt(dot(v,v)); }
inline vec3 normalize(const vec3& v) {
    float_t len = length(v);
    return (len > 0) ? vec3{v.x/len, v.y/len, v.z/len} : vec3{0,0,0};
}

// TODO: mat4, quaternions, transforms

} // namespace sigil
