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
    virtual auto operator()(const ffr::math::vec4& in) -> ffr::math::vec4 = 0;
};


template<uint8_t MAX_VERTS = 128>
class Context
{
public:
    Context() = default;
    virtual ~Context() = default;



    virtual auto plot(uint16_t x, uint16_t y, uint16_t color) -> void = 0;

    auto setVertexFunction(VertexFunction* vf) -> void
    {
        vertex_function_ = vf;
    }

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

    void triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t color) {
        // This implementation uses only 16-bit integer math (Bresenham-style)
        // and avoids all C++ standard library functions.
        int16_t v_top_x = x0, v_top_y = y0;
        int16_t v_mid_x = x1, v_mid_y = y1;
        int16_t v_bot_x = x2, v_bot_y = y2;
        int16_t temp_x, temp_y;

        // --- 1. Manual Sort ---
        // Sort points so v_top_y <= v_mid_y <= v_bot_y
        if (v_top_y > v_mid_y) { temp_x = v_top_x; v_top_x = v_mid_x; v_mid_x = temp_x; temp_y = v_top_y; v_top_y = v_mid_y; v_mid_y = temp_y; }
        if (v_mid_y > v_bot_y) { temp_x = v_mid_x; v_mid_x = v_bot_x; v_bot_x = temp_x; temp_y = v_mid_y; v_mid_y = v_bot_y; v_bot_y = temp_y; }
        if (v_top_y > v_mid_y) { temp_x = v_top_x; v_top_x = v_mid_x; v_mid_x = temp_x; temp_y = v_top_y; v_top_y = v_mid_y; v_mid_y = temp_y; }

        // --- 2. Trivial Case: Horizontal line ---
        if (v_top_y == v_bot_y) {
            int16_t min_x = v_top_x;
            int16_t max_x = v_top_x;
            if (v_mid_x < min_x) min_x = v_mid_x;
            if (v_mid_x > max_x) max_x = v_mid_x;
            if (v_bot_x < min_x) min_x = v_bot_x;
            if (v_bot_x > max_x) max_x = v_bot_x;
            lineHorizontal(min_x, v_top_y, max_x, color);
            return;
        }

        // --- 3. Setup Bresenham Edge Steppers ---
        // Stepper A traces the long edge (top -> bottom)
        int16_t dx_a = v_bot_x - v_top_x;
        int16_t dy_a = v_bot_y - v_top_y;
        int16_t x_step_a = 1;
        if (dx_a < 0) { dx_a = -dx_a; x_step_a = -1; }
        int16_t error_a = dy_a >> 1;
        int16_t x_a = v_top_x;

        // Stepper B will trace the upper int16_t edge (top -> middle) first
        int16_t dx_b = v_mid_x - v_top_x;
        int16_t dy_b = v_mid_y - v_top_y;
        int16_t x_step_b = 1;
        if (dx_b < 0) { dx_b = -dx_b; x_step_b = -1; }
        int16_t error_b = dy_b >> 1;
        int16_t x_b = v_top_x;

        // --- 4. Top half of triangle ---
        // This part is skipped if the triangle is flat-top (top_y == mid_y)
        for (int16_t y = v_top_y; y < v_mid_y; y++) {
            lineHorizontal(x_a, y, x_b, color);

            // Advance stepper A along the long edge
            error_a -= dx_a;
            while (error_a < 0) {
                x_a += x_step_a;
                error_a += dy_a;
            }

            // Advance stepper B along the upper int16_t edge
            if (dy_b > 0) { // Avoid division by zero on a horizontal top edge
                error_b -= dx_b;
                while (error_b < 0) {
                    x_b += x_step_b;
                    error_b += dy_b;
                }
            }
        }

        // --- 5. Bottom half of triangle ---
        // Re-setup stepper B for the lower int16_t edge (middle -> bottom)
        dx_b = v_bot_x - v_mid_x;
        dy_b = v_bot_y - v_mid_y;
        x_step_b = 1;
        if (dx_b < 0) { dx_b = -dx_b; x_step_b = -1; }
        error_b = dy_b >> 1;
        x_b = v_mid_x;

        for (int16_t y = v_mid_y; y <= v_bot_y; y++) {
            lineHorizontal(x_a, y, x_b, color);

            // Advance stepper A along the long edge
            error_a -= dx_a;
            while (error_a < 0) {
                x_a += x_step_a;
                error_a += dy_a;
            }

            // Advance stepper B along the lower int16_t edge
            if (dy_b > 0) { // Avoid division by zero on a horizontal bottom edge
                error_b -= dx_b;
                while (error_b < 0) {
                    x_b += x_step_b;
                    error_b += dy_b;
                }
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

        if((!vertex_pointer_) || (!color_pointer_)) { return; }

        //copy verts and cols into bufs
        vert_buf_current_size_ = 0;
        color_buf_current_size_ = 0;
        current_draw_type_ = dt;

        for(uint8_t i = first; i < first + count; ++i)
        {
            vert_buf_[i-first] = *(vertex_pointer_ + i);
            vert_buf_current_size_ = vert_buf_current_size_ + 1;

            color_buf_[i-first] = *(color_pointer_ + i);
            color_buf_current_size_ = color_buf_current_size_ + 1;
        }

        vertex_pipeline();

    }

private:
    uint16_t view_width_ = 0;
    uint16_t view_height_ = 0;

    DrawType current_draw_type_ = DrawType::Lines;

    math::vec3* vertex_pointer_ = nullptr;
    uint16_t* color_pointer_ = nullptr;

    std::array<math::vec3, MAX_VERTS> vert_buf_;
    uint8_t vert_buf_current_size_ = 0;
    std::array<uint16_t, (MAX_VERTS/3) + (MAX_VERTS%3)> color_buf_;
    uint8_t color_buf_current_size_ = 0;

    VertexFunction* vertex_function_ = nullptr;

    auto vertex_pipeline() -> void
    {

    }

    // std::vector<Vertex> run_vertex_function(std::vector<Vertex>& in)
    // {
    //     std::vector<Vertex> out;
    //     for (auto& i : in)
    //     {
    //         out.push_back(  Vertex{ vertex_function[0](i.pos) , i.col }  );
    //     }
    //     return out;
    // }
    // std::vector<Vertex> run_clip_function(std::vector<Vertex>& in)
    // {
    //     std::vector<Vertex> out;

    //     for(uint8_t i = 0; i < in.size() - 1; i = i + 2)
    //     {
    //         Vertex pi1, pi2, po1, po2;
    //         pi1 = in[i];
    //         pi2 = in[i+1];
    //         bool pi1in = clip_point(pi1);
    //         bool pi2in = clip_point(pi2);
    //         if(pi1in && pi2in)
    //         {
    //             out.push_back(pi1);
    //             out.push_back(pi2);
    //         }
    //         else if(pi1in || pi2in)
    //         {
    //             clip_line_component(pi1,pi2, 0, 1.0_fx, po1, po2);
    //             clip_line_component(po1,po2, 0, -1.0_fx, pi1, pi2);
    //             clip_line_component(pi1,pi2, 1, 1.0_fx, po1, po2);
    //             clip_line_component(po1,po2, 1, -1.0_fx, pi1, pi2);
    //             clip_line_component(pi1,pi2, 2, 1.0_fx, po1, po2);
    //             clip_line_component(po1,po2, 2, -1.0_fx, pi1, pi2);

    //             out.push_back( pi1 );
    //             out.push_back( pi2 );
    //         }


    //     }

    //     return  out;
    // }
    // std::vector<Vertex> run_ndc_function(std::vector<Vertex>& in)
    // {
    //     std::vector<Vertex> out;
    //     for (auto& i : in)
    //     {
    //         out.push_back( Vertex{ { i.pos / i.pos.w }, i.col });
    //     }
    //     return out;
    // }
    // std::vector<Vertex> run_windowtransform_function(std::vector<Vertex>& in)
    // {
    //     std::vector<Vertex> out;
    //     for (auto& i : in)
    //     {
    //         out.push_back
    //             (
    //                 Vertex
    //                 {
    //                     {
    //                         ((math::fixed32(xres) / 2.0_fx) * i.pos.x) + (static_cast<math::fixed32>(xres) / 2.0_fx),
    //                         -((static_cast<math::fixed32>(yres) / 2.0_fx) * i.pos.y) + (static_cast<math::fixed32>(yres) / 2.0_fx),
    //                         ((1.0_fx / 2.0_fx) * i.pos.z) + (1.0_fx / 2.0_fx),
    //                         i.pos.w
    //                     },
    //                     i.col
    //                 }
    //                 );
    //     }
    //     return out;
    // }

    // void run_draw_function(std::vector<Vertex>& in)
    // {
    //     if(in.empty())
    //     {
    //         return;
    //     }

    //     for(uint32_t i = 0; i < in.size() - 1; i = i + 2)
    //     {


    //         //laserOff();
    //         //laserMove(in[i].pos.x, in[i].pos.y);
    //         //laserOn();
    //         //laserColor(in[i].col.r, in[i].col.g, in[i].col.b);

    //         line(static_cast<int16_t>(in[i].pos.x),
    //              static_cast<int16_t>(in[i].pos.y),
    //              static_cast<int16_t>(in[i+1].pos.x),
    //              static_cast<int16_t>(in[i+1].pos.y),
    //              in[i].col);


    //         //laserMove(in[i+1].pos.x, in[i+1].pos.y);
    //         //laserColor(in[i+1].col.r, in[i+1].col.g, in[i].col.b);
    //     }
    //     //laserOff();
    // }

    // // returns true if point is inside volume
    // bool clip_point(const Vertex& in)
    // {
    //     if ((in.pos.x < -in.pos.w ||
    //          in.pos.x > in.pos.w ||
    //          in.pos.y < -in.pos.w ||
    //          in.pos.y > in.pos.w ||
    //          in.pos.z < -in.pos.w ||
    //          in.pos.z > in.pos.w))
    //     {
    //         return false;
    //     }
    //     else
    //     {
    //         return true;
    //     }
    // }


};


}
