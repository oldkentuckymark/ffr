#pragma once

#include <cstdint>
#include <array>
#include <cmath>
#include <vector>
#include <climits>

#include "ffrmath.hpp"

namespace ffr
{

constexpr auto Convert888to555(uint8_t const r, uint8_t const g, uint8_t const b) -> uint16_t
{
    return (((r >> 3) & 31) |
            (((g >> 3) & 31) << 5) |
            (((b >> 3) & 31) << 10) );

}

constexpr auto Convert555to888(uint16_t color) -> std::array<uint8_t, 4>
{
    uint8_t const red = (color & 31) << 3;
    uint8_t const green = ((color >> 5) & 31) << 3;
    uint8_t const blue = ((color >> 10) & 31) << 3;
    uint8_t const alpha = 255;
    return {red,green,blue,alpha};
}

enum class DrawType : uint8_t
{
    Points,
    Lines,
    Triangles
};

class Context
{
public:
    Context();
    virtual ~Context();


    virtual auto plot(uint16_t x, uint16_t y, uint16_t color) -> void;

    virtual auto line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) -> void;
    virtual auto lineHorizontal(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t color) -> void;
    virtual auto lineVertical(uint16_t x0, uint16_t y0, uint16_t y1, uint16_t color) -> void;

    virtual auto clear() -> void;
    virtual auto present() -> void;

    auto setVertexPointer(uint8_t size, uint8_t stride, void* vp) -> void;
    auto setColorPointer(uint8_t stride, uint16_t* cp)-> void;

    auto drawArray(DrawType dt, uint8_t first, uint8_t count) -> void;

private:
    void* vertex_pointer_ = nullptr;
    uint16_t* color_pointer_ = nullptr;

    std::vector<math::vec4> vert_buf;
    std::vector<uint16_t> color_buf;






};


}


