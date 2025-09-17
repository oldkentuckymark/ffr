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

template<uint8_t MAX_VERTS>
class Context
{
public:
    Context();
    virtual ~Context();



    virtual auto plot(uint16_t x, uint16_t y, uint16_t color) -> void;

    virtual auto line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) -> void
    {
        bool const steep = std::abs(y1 - y0) > std::abs(x1 - x0);

        if (steep)
        {
            int tmp = x0;
            x0 = y0;
            y0 = tmp;

            tmp = x1;
            x1 = y1;
            y1 = tmp;
        }

        if (x0 > x1)
        {
            int16_t tmp = x0;
            x0 = x1;
            x1 = tmp;

            tmp = y0;
            y0 = y1;
            y1 = tmp;
        }

        int16_t const dx    = x1 - x0;
        int16_t const dy    = std::abs(y1 - y0);
        int16_t error = dx / 2;
        int16_t const ystep = (y0 < y1) ? 1 : -1;
        int16_t y     = y0;

        for (int16_t x = x0; x <= x1; ++x)
        {
            if (steep)
            {
                plot(y, x, color);
            }
            else
            {
                plot(x, y, color);
            }

            error -= dy;
            if (error < 0)
            {
                y     += ystep;
                error += dx;
            }
        }
    }
    virtual auto lineHorizontal(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t color) -> void
    {
        line(x0,y0,x1,y0,color);
    }
    virtual auto lineVertical(uint16_t x0, uint16_t y0, uint16_t y1, uint16_t color) -> void
    {
        line(x0,y0,x0,y1,color);
    }

    virtual auto triangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) -> void
    {

    }


    virtual auto clear() -> void;
    virtual auto present() -> void;

    auto setVertexPointer(uint8_t size, void* vp) -> void
    {
        vertex_pointer_ = vp;
        vertex_size_ = size;
    }
    auto setColorPointer(uint16_t* cp)-> void
    {
        color_pointer_ = cp;
    }
    auto setViewPort(uint16_t w, uint16_t h)
    {
        view_width_ = w;
        view_height_ = h;
    }

    auto drawArray(DrawType dt, uint8_t first, uint8_t count) -> void;

private:
    uint16_t view_width_ = 0;
    uint16_t view_height_ = 0;

    void* vertex_pointer_ = nullptr;
    uint16_t* color_pointer_ = nullptr;
    uint8_t vertex_size_ = 0;

    std::array<math::vec4, MAX_VERTS> vert_buf_;
    std::array<uint16_t, (MAX_VERTS/3) + (MAX_VERTS%3)> color_buf_;

    auto vertex_pipeline() -> void;






};


}


