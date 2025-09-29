#pragma once
#include <cmath>
#include <initializer_list>
#include <stdexcept>

namespace sigil::math {

    template <typename T, unsigned N>
    struct vector_t {
        T v[N];
    
        vector_t() = default;
    
        vector_t(std::initializer_list<T> init) {
            if (init.size() != N) {
                throw std::runtime_error("Invalid initializer size");
            }
    
            std::copy(init.begin(), init.end(), v);
        }
    };

namespace vector {

template <typename T, unsigned N> inline vector_t<T, N>
add(const vector_t<T, N>& a, const vector_t<T, N>& b) {
    vector_t<T, N> out{};
    for (unsigned i = 0; i < N; ++i)
        out.v[i] = a.v[i] + b.v[i];
    return out;
}

template <typename T, unsigned N> inline vector_t<T, N>
sub(const vector_t<T, N>& a, const vector_t<T, N>& b) {
    vector_t<T, N> out{};
    for (unsigned i = 0; i < N; ++i)
        out.v[i] = a.v[i] - b.v[i];
    return out;
}

template <typename T, unsigned N> inline T
dot(const vector_t<T, N>& a, const vector_t<T, N>& b) {
    T acc{};
    for (unsigned i = 0; i < N; ++i)
        acc += a.v[i] * b.v[i];
    return acc;
}

template <typename T, unsigned N> inline T
length_sq(const vector_t<T, N>& v) {
    return dot(v, v);
}

template <typename T, unsigned N> inline T
length(const vector_t<T, N>& v) {
    return static_cast<T>(sqrt(length_sq(v)));
}

template <typename T, unsigned N> inline vector_t<T, N>
scale(const vector_t<T, N>& v, T s) {
    vector_t<T, N> out{};
    for (unsigned i = 0; i < N; ++i)
        out.v[i] = v.v[i] * s;
    return out;
}

template <typename T, unsigned N> inline vector_t<T, N>
normalize(const vector_t<T, N>& v) {
    T len = length(v);
    vector_t<T, N> out{};
    for (unsigned i = 0; i < N; ++i)
        out.v[i] = v.v[i] / len;
    return out;
}

} // namespace sigil::math::vector
} // namespace sigil::math
