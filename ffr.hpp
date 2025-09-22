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
    Points = 1,
    Lines = 2,
    Triangles = 3
};


class VertexFunction
{
public:
    VertexFunction() = default;
    virtual ~VertexFunction() = default;
    virtual auto operator()(const ffr::math::vec4& in) -> ffr::math::vec4 = 0;
};


template<uint8_t MAX_VERTS>
class Context
{
public:
    Context() = default;
    virtual ~Context() = default;



    virtual auto plot(uint16_t x, uint16_t y, uint16_t color) -> void = 0;

    virtual auto line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) -> void
    {
        bool const steep = std::abs(y1 - y0) > std::abs(x1 - x0);

        if (steep)
        {
            uint16_t tmp = x0;
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

    void triangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, uint16_t color)
    {
        // Sort vertices by y-coordinate to ensure y1 <= y2 <= y3 (top to bottom)
        if (y1 > y2)
        {
            int16_t tempX = x1, tempY = y1;
            x1 = x2;
            y1 = y2;
            x2 = tempX;
            y2 = tempY;
        }

        if (y1 > y3)
        {
            int16_t tempX = x1, tempY = y1;
            x1 = x3;
            y1 = y3;
            x3 = tempX;
            y3 = tempY;
        }

        if (y2 > y3)
        {
            int16_t tempX = x2, tempY = y2;
            x2 = x3;
            y2 = y3;
            x3 = tempX;
            y3 = tempY;
        }

        // Step 2: Track edges using Bresenham's algorithm, and fill between them

        // From (x1, y1) to (x2, y2) - Left edge
        int16_t dx = (x2 >= x1) ? (x2 - x1) : (x1 - x2);
        int16_t dy = (y2 >= y1) ? (y2 - y1) : (y1 - y2);
        int16_t sx = (x1 < x2) ? 1 : -1;
        int16_t sy = (y1 < y2) ? 1 : -1;
        int16_t err = dx - dy;

        int16_t x = x1, y = y1;
        while (y <= y2)
        {
            if (y >= y1 && y <= y3)
            {
                // Find the leftmost x-coordinate (intersection with the first edge)
                int16_t x_left = x;

                // Find the rightmost x-coordinate (intersection with the second edge)
                int16_t dx2 = (x3 >= x2) ? (x3 - x2) : (x2 - x3);
                int16_t dy2 = (y3 >= y2) ? (y3 - y2) : (y2 - y3);
                int16_t sx2 = (x2 < x3) ? 1 : -1;
                int16_t sy2 = (y2 < y3) ? 1 : -1;
                int16_t err2 = dx2 - dy2;

                int16_t x_right = x2, y_right = y2;
                while (y_right < y3 && y != y_right)
                {
                    int16_t e2 = err2 * 2;
                    if (e2 > -dy2)
                    {
                        err2 -= dy2;
                        x_right += sx2;
                    }
                    if (e2 < dx2)
                    {
                        err2 += dx2;
                        y_right += sy2;
                    }
                }

                // Fill between x_left and x_right using lineHorizontal
                lineHorizontal(x_left, y, x_right, color); // Use the assumed lineHorizontal() function
            }

            if (x == x2 && y == y2)
                break;
            int16_t e2 = err * 2;
            if (e2 > -dy)
            {
                err -= dy;
                x += sx;
            }
            if (e2 < dx)
            {
                err += dx;
                y += sy;
            }
        }

        // From (x2, y2) to (x3, y3) - Right edge
        dx = (x3 >= x2) ? (x3 - x2) : (x2 - x3);
        dy = (y3 >= y2) ? (y3 - y2) : (y2 - y3);
        sx = (x2 < x3) ? 1 : -1;
        sy = (y2 < y3) ? 1 : -1;
        err = dx - dy;

        x = x2, y = y2;
        while (y <= y3)
        {
            if (y >= y1 && y <= y3)
            {
                // Fill between x_left and x_right using lineHorizontal
                int16_t x_left = x;

                // Find the rightmost x-coordinate (intersection with the third edge)
                int16_t dx3 = (x1 >= x3) ? (x1 - x3) : (x3 - x1);
                int16_t dy3 = (y1 >= y3) ? (y1 - y3) : (y3 - y1);
                int16_t sx3 = (x3 < x1) ? 1 : -1;
                int16_t sy3 = (y3 < y1) ? 1 : -1;
                int16_t err3 = dx3 - dy3;

                int16_t x_right = x3, y_right = y3;
                while (y_right > y1 && y != y_right)
                {
                    int16_t e2 = err3 * 2;
                    if (e2 > -dy3)
                    {
                        err3 -= dy3;
                        x_right += sx3;
                    }
                    if (e2 < dx3)
                    {
                        err3 += dx3;
                        y_right += sy3;
                    }
                }

                // Fill between x_left and x_right using lineHorizontal
                lineHorizontal(x_left, y, x_right, color); // Use the assumed lineHorizontal() function
            }

            if (x == x3 && y == y3)
                break;
            int16_t e2 = err * 2;
            if (e2 > -dy)
            {
                err -= dy;
                x += sx;
            }
            if (e2 < dx)
            {
                err += dx;
                y += sy;
            }
        }
    }




    virtual auto clear() -> void
    {

    }
    virtual auto present() -> void
    {

    }

    auto setVertexPointer(math::vec3* vp) -> void
    {
        vertex_pointer_ = vp;
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

    auto drawArray(DrawType dt, uint8_t first, uint8_t count) -> void
    {
        //copy verts and cols into bufs
        for(uint8_t i = first; i < first + count; ++i)
        {
            vert_buf_[i-first] = *(vertex_pointer_ + i);
        }
    }

private:
    uint16_t view_width_ = 0;
    uint16_t view_height_ = 0;

    math::vec3* vertex_pointer_ = nullptr;
    uint16_t* color_pointer_ = nullptr;

    std::array<math::vec3, MAX_VERTS> vert_buf_;
    uint8_t vert_buf_current_size_ = 0;
    std::array<uint16_t, (MAX_VERTS/3) + (MAX_VERTS%3)> color_buf_;
    uint8_t color_buf_current_size_ = 0;

    auto vertex_pipeline() -> void;


};


}
