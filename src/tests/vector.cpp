#include <sigil/math/vector.h>
#include <gtest/gtest.h>

using sigil::math::vector_t;
namespace vops = sigil::math::vector;

template <typename T, unsigned N>
static void 
expect_vec_eq(const vector_t<T, N>& a, const vector_t<T, N>& b, T eps = static_cast<T>(1e-6))
{
    for (unsigned i = 0; i < N; ++i)
        EXPECT_NEAR(a.v[i], b.v[i], eps);
}

TEST(Vector, Construction) {
    vector_t<float, 3> v{{1.0f, 2.0f, 3.0f}};

    EXPECT_FLOAT_EQ(v.v[0], 1.0f);
    EXPECT_FLOAT_EQ(v.v[1], 2.0f);
    EXPECT_FLOAT_EQ(v.v[2], 3.0f);
}

TEST(Vector, Add) {
    vector_t<float, 3> a{{1.0f, 2.0f, 3.0f}};
    vector_t<float, 3> b{{4.0f, 5.0f, 6.0f}};

    auto c = vops::add(a, b);
    vector_t<float, 3> expected{{5.0f, 7.0f, 9.0f}};

    expect_vec_eq(c, expected);
}

TEST(Vector, Sub) {
    vector_t<float, 3> a{{5.0f, 7.0f, 9.0f}};
    vector_t<float, 3> b{{1.0f, 2.0f, 3.0f}};

    auto c = vops::sub(a, b);
    vector_t<float, 3> expected{{4.0f, 5.0f, 6.0f}};

    expect_vec_eq(c, expected);
}


TEST(Vector, Dot) {
    vector_t<float, 3> a{{1.0f, 2.0f, 3.0f}};
    vector_t<float, 3> b{{4.0f, 5.0f, 6.0f}};

    float d = vops::dot(a, b);

    EXPECT_FLOAT_EQ(d, 32.0f); // 1*4 + 2*5 + 3*6
}

TEST(Vector, LengthSquared) {
    vector_t<float, 3> v{{2.0f, 3.0f, 6.0f}};

    float len_sq = vops::length_sq(v);

    EXPECT_FLOAT_EQ(len_sq, 49.0f);
}

TEST(Vector, Length) {
    vector_t<float, 3> v{{2.0f, 3.0f, 6.0f}};

    float len = vops::length(v);

    EXPECT_NEAR(len, 7.0f, 1e-6f);
}

TEST(Vector, Scale) {
    vector_t<float, 3> v{{1.0f, -2.0f, 3.0f}};

    auto s = vops::scale(v, 2.0f);
    vector_t<float, 3> expected{{2.0f, -4.0f, 6.0f}};

    expect_vec_eq(s, expected);
}

TEST(Vector, Normalize) {
    vector_t<float, 3> v{{0.0f, 3.0f, 4.0f}};

    auto n = vops::normalize(v);

    // length should be 1
    float len = vops::length(n);
    EXPECT_NEAR(len, 1.0f, 1e-6f);

    vector_t<float, 3> expected{{0.0f, 0.6f, 0.8f}};
    expect_vec_eq(n, expected);
}


TEST(Vector, WorksForDifferentDimensions) {
    vector_t<int, 2> a{{1, 2}};
    vector_t<int, 2> b{{3, 4}};

    auto c = vops::add(a, b);

    EXPECT_EQ(c.v[0], 4);
    EXPECT_EQ(c.v[1], 6);
}
