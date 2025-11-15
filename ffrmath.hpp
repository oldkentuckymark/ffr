#pragma once

#include <cmath>
#include <cstdint>

#include "util.hpp"

namespace ffr::math
{

class fixed32 //16.16
{
    static constexpr int32_t const FIX_SHIFT = 16;
    static constexpr int32_t const FIX_SCALE = 65536;
    static constexpr float const FIX_SCALEF = 65536.0f;

    int32_t data = 0;

public:
    constexpr fixed32() = default;

    constexpr explicit fixed32(int16_t const &that)
        : data(that << FIX_SHIFT)
    {}

    consteval explicit fixed32(float const &that)
        : data(static_cast<int32_t>(that * FIX_SCALEF))
    {}

    // constexpr operator bool () const
    // {
    //     return !(data == 0);
    // }

    constexpr auto operator=(int16_t const that) -> fixed32 &
    {
        data = that << FIX_SHIFT;
        return (*this);
    }

    consteval auto operator=(float const that) -> fixed32 &
    {
        data = static_cast<int32_t>(that * FIX_SCALEF);
        return (*this);
    }

    // constexpr explicit operator int32_t () const
    // {
    //     return data >> FIX_SHIFT;
    // }

    constexpr explicit operator int16_t() const { return data >> FIX_SHIFT; }

    consteval explicit operator float() const { return data / FIX_SCALEF; }

    constexpr auto operator+(fixed32 const that) const -> fixed32
    {
        fixed32 r;
        r.data = data + that.data;
        return r;
    }

    constexpr auto operator-(fixed32 const that) const -> fixed32
    {
        fixed32 r;
        r.data = data - that.data;
        return r;
    }

    constexpr auto operator*(fixed32 const that) const -> fixed32
    {
        fixed32 r;
        r.data = (int64_t(data) * that.data) >> FIX_SHIFT;
        return r;
    }

    constexpr auto operator/(fixed32 const that) const -> fixed32
    {
        fixed32 r;
        r.data = (int64_t(data) * FIX_SCALE) / (that.data);
        return r;
    }

    constexpr auto operator-() const -> fixed32
    {
        fixed32 r;
        r.data = this->data * -1;
        return r;
    }

    constexpr auto operator<=>(fixed32 const &that) const -> std::strong_ordering
    {
        return this->data <=> that.data;
    }
};

consteval auto operator""_fx(long double f) -> math::fixed32
{
    math::fixed32 r(static_cast<float>(f));
    return r;
}

static constexpr std::uint16_t GAMDEG_IN_CIRCLE = 256; //360 = degrees, 21600 = minutes

constexpr fixed32 PI = 3.14159265_fx;
constexpr fixed32 TAU = 6.28318530_fx;
constexpr float PIF = 3.14159265f;
constexpr float TAUF = 6.28318530f;
constexpr float RAD_TO_GAMDEGF = GAMDEG_IN_CIRCLE / TAUF;
constexpr fixed32 RAD_TO_GAMDEG = fixed32(GAMDEG_IN_CIRCLE / TAUF);

constexpr auto factorial(auto const n) -> decltype(n)
{
    decltype(n) r = 1;
    for (decltype(n) i = n; i > 1; --i)
    {
        r = r * i;
    }
    return r;
}

constexpr auto pow(auto const b, auto const e) -> decltype(b)
{
    decltype(b) r = 1;
    for (decltype(b) i = 0; i < e; ++i) {
        r *= b;
    }
    return r;
}

namespace
{
using LUT = util::array<fixed32, GAMDEG_IN_CIRCLE>;

consteval auto taylorSin(float const x) -> float
{
    float const ts = x - ((x * x * x) / 6.0f) + ((x * x * x * x * x) / 120.0f)
                     - ((x * x * x * x * x * x * x) / 5040.0f)
                     + ((x * x * x * x * x * x * x * x * x) / 32880.0f);
    return ts;
}

consteval auto makeSinTable() -> LUT
{
    uint16_t const quadrantSize = GAMDEG_IN_CIRCLE/4;
    LUT r{};

    uint16_t k = (quadrantSize*2);
    for (uint16_t i = 0; i <= quadrantSize; ++i)
    {
        float x = (TAUF / GAMDEG_IN_CIRCLE) * i;
        r[i] = taylorSin(x);
        r[k] = r[i];
        --k;
    }

    k = quadrantSize*2;
    for(uint16_t  i = 0; i < quadrantSize*2; ++i)
    {
        r[k] = -r[i];
        ++k;
    }

    r[quadrantSize * 0] = 0.0_fx;
    r[quadrantSize * 1] = 1.0_fx;
    r[quadrantSize * 2] = 0.0_fx;
    r[quadrantSize * 3] = -1.0_fx;

    return r;
}

consteval auto taylorCos(float const x) -> float
{
    float const tc = 1.0f - ((x * x) / 2.0f) + ((x * x * x * x) / 24.0f)
                     - ((x * x * x * x * x * x) / 720.0f)
                     + ((x * x * x * x * x * x * x * x) / 40320);
    return tc;
}

consteval auto makeCosTable() -> LUT
{
    const uint16_t quadrantSize = GAMDEG_IN_CIRCLE/4;
    LUT r{};

    uint16_t k = (quadrantSize*2);
    for (uint16_t i = 0; i <= quadrantSize; ++i)
    {
        float x = (TAUF / GAMDEG_IN_CIRCLE) * i;
        r[i] = taylorCos(x);
        r[k] = -r[i];
        --k;
    }

    k = quadrantSize*2;
    for(uint16_t  i = 0; i < quadrantSize*2; ++i)
    {
        r[k] = -r[i];
        ++k;
    }

    r[quadrantSize * 0] = 1.0_fx;
    r[quadrantSize * 1] = 0.0_fx;
    r[quadrantSize * 2] = -1.0_fx;
    r[quadrantSize * 3] = 0.0_fx;

    return r;
}
} // namespace

constexpr auto clampGamdeg(int16_t gamdeg) -> int16_t
{
    return (gamdeg % GAMDEG_IN_CIRCLE + GAMDEG_IN_CIRCLE) % GAMDEG_IN_CIRCLE;
}

constexpr auto sqrt(fixed32 const n) -> fixed32
{
    return 1.0_fx;
    //return static_cast<fixed32>(std::sqrtf(static_cast<float>(n)));
}

constexpr auto sin(fixed32 const a) -> fixed32
{
    constexpr auto SINTABLE = makeSinTable();

    if consteval
    {
        return static_cast<fixed32>(std::sinf(static_cast<float>(a)));
    }
    else
    {
        //return static_cast<fixed32>(std::sinf(static_cast<float>(a)));
        fixed32 const gd = a * RAD_TO_GAMDEG;
        int16_t const gdi = clampGamdeg(static_cast<int16_t>(gd));

        return fixed32{SINTABLE[gdi]};
    }
}

constexpr auto cos(fixed32 const a) -> fixed32
{
    constexpr LUT COSTABLE = makeCosTable();
    if consteval
    {
        return static_cast<fixed32>(std::cosf(static_cast<float>(a)));
    }
    else
    {
        //return static_cast<fixed32>(std::cosf(static_cast<float>(a)));

        fixed32 const gd = a * RAD_TO_GAMDEG;
        int16_t const gdi = clampGamdeg(static_cast<int16_t>(gd));

        return fixed32{COSTABLE[gdi]};
    }
}

constexpr auto tan(fixed32 const n) -> fixed32
{
    if consteval {
        return static_cast<fixed32>(
            std::sinf(static_cast<float>(n) / std::cosf(static_cast<float>(n))));
    } else {
        return sin(n) / cos(n);
    }
}

constexpr auto cot(fixed32 const n) -> fixed32
{
    if consteval {
        return static_cast<fixed32>(
            std::cosf(static_cast<float>(n) / std::sinf(static_cast<float>(n))));
    } else {
        return cos(n) / sin(n);
    }
}

constexpr auto abs(auto n) -> decltype(n)
{
    return (n > 0) ? n : -n;
}

constexpr auto abs(fixed32 n) -> fixed32
{
    return (n > 0.0_fx) ? n : -n;
}

class vec2
{
public:
    fixed32 x, y;

    constexpr auto operator+(vec2 const &that) const -> vec2 { return {x + that.x, y + that.y}; }

    constexpr auto operator-(vec2 const &that) const -> vec2 { return {x - that.x, y - that.y}; }

    constexpr auto operator*(fixed32 const &that) const -> vec2 { return {x * that, y * that}; }

    constexpr auto operator/(fixed32 const &that) const -> vec2 { return {x / that, y / that}; }

    [[nodiscard]] constexpr auto length() const -> fixed32
    {
        const auto x2 = x * x;
        const auto y2 = y * y;
        const auto sum = x2 + y2;
        return sqrt(sum);
    }

    auto operator[](uint8_t const p) -> fixed32 & { return *((&(this->x)) + p); }
};

class vec3 : public vec2
{
public:
    fixed32 z;

    constexpr auto operator+(vec3 const &that) -> vec3
    {
        return {this->x + that.x, this->y + that.y, z + that.x};
    }

    constexpr auto operator-(vec3 const &that) -> vec3
    {
        return {this->x - that.x, this->y - that.y, z - that.x};
    }

    constexpr auto operator*(fixed32 const &that) -> vec3
    {
        return {this->x * that, this->y * that, this->z * that};
    }

    constexpr auto operator/(fixed32 const &that) -> vec3
    {
        return {this->x / that, this->y / that, this->z / that};
    }

    constexpr auto operator*(vec3 const &that) -> fixed32
    {
        return {(this->x * that.x) + (this->y * that.y) + (this->z * that.z)};
    }

    [[nodiscard]] constexpr auto length() const -> fixed32
    {
        const auto x2 = x * x;
        const auto y2 = y * y;
        const auto z2 = z * z;
        const auto sum = x2 + y2 + z2;
        return sqrt(sum);
    }
};

class vec4 : public vec3
{
public:
    fixed32 w = 1.0_fx;

    constexpr auto operator+(vec4 const &that) const -> vec4
    {
        return {this->x + that.x, this->y + that.y, z + that.x, w + that.w};
    }

    constexpr auto operator-(vec4 const &that) const -> vec4
    {
        return {this->x - that.x, this->y - that.y, z - that.x, w - that.w};
    }

    constexpr auto operator*(fixed32 const &that) const -> vec4
    {
        return {this->x * that, this->y * that, this->z * that, w * that};
    }

    constexpr auto operator/(fixed32 const &that) const -> vec4
    {
        return {this->x / that, this->y / that, this->z / that, w / that};
    }

    constexpr auto operator*(vec4 const &that) const -> fixed32
    {
        return {(this->x * that.x) + (this->y * that.y) + (this->z * that.z) + (this->w * that.w)};
    }

    [[nodiscard]] constexpr auto length() const -> fixed32
    {
        const auto x2 = x * x;
        const auto y2 = y * y;
        const auto z2 = z * z;
        const auto w2 = w * w;
        const auto sum = x2 + y2 + z2 + w2;
        return sqrt(sum);
    }
};

class mat4
{
public:
    fixed32 m[4][4];

    constexpr mat4()
    {
        m[0][0] = 1.0_fx;
        m[0][1] = 0.0_fx;
        m[0][2] = 0.0_fx;
        m[0][3] = 0.0_fx;

        m[1][0] = 0.0_fx;
        m[1][1] = 1.0_fx;
        m[1][2] = 0.0_fx;
        m[1][3] = 0.0_fx;

        m[2][0] = 0.0_fx;
        m[2][1] = 0.0_fx;
        m[2][2] = 1.0_fx;
        m[2][3] = 0.0_fx;

        m[3][0] = 0.0_fx;
        m[3][1] = 0.0_fx;
        m[3][2] = 0.0_fx;
        m[3][3] = 1.0_fx;
    }

    constexpr auto operator+(mat4 const &that) -> mat4
    {
        mat4 n;

        for (uint8_t c = 0; c < 4; ++c) {
            for (uint8_t r = 0; r < 4; ++r) {
                n.m[c][r] = this->m[c][r] + that.m[c][r];
            }
        }

        return n;
    }

    constexpr auto operator-(mat4 const &that) -> mat4
    {
        mat4 n;

        for (uint8_t c = 0; c < 4; ++c) {
            for (uint8_t r = 0; r < 4; ++r) {
                n.m[c][r] = this->m[c][r] - that.m[c][r];
            }
        }

        return n;
    }

    constexpr auto operator*(fixed32 const &that) -> mat4
    {
        mat4 n;

        for (uint8_t c = 0; c < 4; ++c) {
            for (uint8_t r = 0; r < 4; ++r) {
                n.m[c][r] = this->m[c][r] * that;
            }
        }

        return n;
    }

    constexpr auto operator/(fixed32 const &that) -> mat4
    {
        mat4 n;

        for (uint8_t c = 0; c < 4; ++c) {
            for (uint8_t r = 0; r < 4; ++r) {
                n.m[c][r] = this->m[c][r] / that;
            }
        }

        return n;
    }

    constexpr auto operator*(mat4 const &that) -> mat4
    {
        mat4 n;

        for (uint8_t c = 0; c < 4; ++c) {
            for (uint8_t r = 0; r < 4; ++r) {
                n.m[c][r] = (this->m[0][r] * that.m[c][0]) + (this->m[1][r] * that.m[c][1])
                            + (this->m[2][r] * that.m[c][2]) + (this->m[3][r] * that.m[c][3]);
            }
        }

        return n;
    }

    constexpr auto operator*(vec4 const &that) -> vec4
    {
        vec4 n;

        n.x = (this->m[0][0] * that.x) + (this->m[1][0] * that.y) + (this->m[2][0] * that.z)
              + (this->m[3][0] * that.w);

        n.y = (this->m[0][1] * that.x) + (this->m[1][1] * that.y) + (this->m[2][1] * that.z)
              + (this->m[3][1] * that.w);

        n.z = (this->m[0][2] * that.x) + (this->m[1][2] * that.y) + (this->m[2][2] * that.z)
              + (this->m[3][2] * that.w);

        n.w = (this->m[0][3] * that.x) + (this->m[1][3] * that.y) + (this->m[2][3] * that.z)
              + (this->m[3][3] * that.w);

        return n;
    }

    static constexpr auto perspective(fixed32 const fovy,
                                      fixed32 const aspect,
                                      fixed32 const zNear,
                                      fixed32 const zFar)
    {
        mat4 n;
        fixed32 const fovR = fovy * (TAU / 360.0_fx);
        fixed32 const f = cot(fovR * 0.5_fx);

        n.m[0][0] = f / aspect;
        n.m[1][1] = f;
        n.m[2][2] = (zFar + zNear) / (zNear - zFar);
        n.m[3][2] = (2.0_fx * zFar * zNear) / (zNear - zFar);
        n.m[2][3] = -1.0_fx;
        n.m[3][3] = 0.0_fx;

        return n;
    }

    static constexpr auto perspective90DegSquare(fixed32 const zNear, fixed32 const zFar)
    {
        mat4 n;

        n.m[0][0] = 1.0_fx;
        n.m[1][1] = 1.0_fx;
        n.m[2][2] = (zFar + zNear) / (zNear - zFar);
        n.m[3][2] = (2.0_fx * zFar * zNear) / (zNear - zFar);
        n.m[2][3] = -1.0_fx;
        n.m[3][3] = 0.0_fx;

        return n;
    }

    static constexpr auto translation(vec3 const &v) -> mat4
    {
        mat4 n;

        n.m[3][0] = v.x;
        n.m[3][1] = v.y;
        n.m[3][2] = v.z;
        n.m[3][3] = 1.0_fx;

        return n;
    }

    static constexpr auto translation(vec4 const &v) -> mat4
    {
        mat4 n;

        n.m[3][0] = v.x;
        n.m[3][1] = v.y;
        n.m[3][2] = v.z;
        n.m[3][3] = v.w;

        return n;
    }

    static constexpr auto rotationX(fixed32 const &radians) -> mat4
    {
        mat4 r;

        r.m[1][1] = cos(radians);
        r.m[1][2] = sin(radians);
        r.m[2][1] = -sin(radians);
        r.m[2][2] = cos(radians);

        return r;
    }

    static constexpr auto rotationY(fixed32 const &radians) -> mat4
    {
        mat4 r;

        r.m[0][0] = cos(radians);
        r.m[0][2] = -sin(radians);
        r.m[2][0] = sin(radians);
        r.m[2][2] = cos(radians);

        return r;
    }

    static constexpr auto rotationZ(fixed32 const &radians) -> mat4
    {
        mat4 r;

        r.m[0][0] = cos(radians);
        r.m[0][1] = sin(radians);
        r.m[1][0] = -sin(radians);
        r.m[1][1] = cos(radians);

        return r;
    }
};

auto mix(auto x, auto y, auto a) -> auto
{
    return x * (1.0_fx - a) + y * a;
}

} // namespace ffr::math

consteval auto operator""_fx(long double f) -> ffr::math::fixed32
{
    ffr::math::fixed32 const r(static_cast<float>(f));
    return r;
}
