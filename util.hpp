#pragma once

#include "ffrmath.hpp"

#include <array>

namespace ffr
{
namespace util
{

template<class T, std::size_t S, auto FUNC>
constexpr auto makeTable() -> std::array<T, S>
{
    std::array<T,S> r{};
    for(std::size_t i = 0; i < S; ++i)
    {
        r[i] = FUNC(i);
    }
    return r;
}

constexpr auto createCube(math::fixed32 const xRadius,
                          math::fixed32 const yRadius,
                          math::fixed32 const zRadius) -> std::array<math::fixed32, 108>
{
    std::array<math::fixed32, 108> r =
    {
        // Front face (+Z)
        -xRadius, -yRadius,  zRadius,
        xRadius, -yRadius,  zRadius,
        xRadius,  yRadius,  zRadius,
        -xRadius, -yRadius,  zRadius,
        xRadius,  yRadius,  zRadius,
        -xRadius,  yRadius,  zRadius,

        // Back face (-Z)
        xRadius, -yRadius, -zRadius,
        -xRadius, -yRadius, -zRadius,
        -xRadius,  yRadius, -zRadius,
        xRadius, -yRadius, -zRadius,
        -xRadius,  yRadius, -zRadius,
        xRadius,  yRadius, -zRadius,

        // Left face (-X)
        -xRadius, -yRadius, -zRadius,
        -xRadius, -yRadius,  zRadius,
        -xRadius,  yRadius,  zRadius,
        -xRadius, -yRadius, -zRadius,
        -xRadius,  yRadius,  zRadius,
        -xRadius,  yRadius, -zRadius,

        // Right face (+X)
        xRadius, -yRadius,  zRadius,
        xRadius, -yRadius, -zRadius,
        xRadius,  yRadius, -zRadius,
        xRadius, -yRadius,  zRadius,
        xRadius,  yRadius, -zRadius,
        xRadius,  yRadius,  zRadius,

        // Top face (+Y)
        -xRadius,  yRadius,  zRadius,
        xRadius,  yRadius,  zRadius,
        xRadius,  yRadius, -zRadius,
        -xRadius,  yRadius,  zRadius,
        xRadius,  yRadius, -zRadius,
        -xRadius,  yRadius, -zRadius,

        // Bottom face (-Y)
        -xRadius, -yRadius, -zRadius,
        xRadius, -yRadius, -zRadius,
        xRadius, -yRadius,  zRadius,
        -xRadius, -yRadius, -zRadius,
        xRadius, -yRadius,  zRadius,
        -xRadius, -yRadius, zRadius
    };


    return r;
}




}
}
