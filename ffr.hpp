#pragma once

#include <cstdint>
#include <cmath>

#include "ffrmath.hpp"

namespace ffr
{

constexpr auto Convert888to555(uint8_t const r, uint8_t const g, uint8_t const b) -> uint16_t
{
    return (((r >> 3) & 31) |
            (((g >> 3) & 31) << 5) |
            (((b >> 3) & 31) << 10) );

}

constexpr auto Convert555to888(uint16_t color) -> ffr::util::array<uint8_t, 4>
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
    virtual auto operator()(ffr::math::vec4& in) -> void = 0;
};


template<uint8_t MAX_VERTS>
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

    virtual auto line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) -> void
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
    virtual auto lineHorizontal(int16_t x0, int16_t y0, int16_t x1, uint16_t color) -> void
    {
        line(x0,y0,x1,y0,color);
    }
    virtual auto lineVertical(int16_t x0, int16_t y0, int16_t y1, uint16_t color) -> void
    {
        line(x0,y0,x0,y1,color);
    }

    virtual auto triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) -> void
    {

        //std::cout << x0 << ", " << x1 << ", " << y0 << ", " << y1 << ", " << x2 << ", " << y2 << "\n";

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

    auto setVertexPointer(uint8_t size, void* vp) -> void
    {
        vertex_pointer_ = vp;
        current_vertex_size_ = size;

    }

    //1 uint16_t per primitve
    auto setColorPointer(uint16_t* cp)-> void
    {
        color_pointer_ = cp;
    }
    auto setViewPort(int16_t w, int16_t h)
    {
        view_width_ = w;
        view_height_ = h;
    }

    auto drawArray(DrawType dt, uint16_t first, uint16_t count) -> void
    {

        if((!vertex_pointer_) || (!color_pointer_)) { return; }

        //copy verts and cols into bufs
        pre_clip_vert_buf_current_size_ = 0;
        pre_clip_color_buf_current_size_ = 0;
        post_clip_vert_buf_current_size_ = 0;
        post_clip_color_buf_current_size_ = 0;
        current_draw_type_ = dt;

        if(current_vertex_size_ == 2)
        {
            for(uint16_t i = first; i < first + count; ++i)
            {

                pre_clip_vert_buf_[pre_clip_vert_buf_current_size_] = {reinterpret_cast<math::vec2*>(vertex_pointer_)[i].x,
                                                                       reinterpret_cast<math::vec2*>(vertex_pointer_)[i].y,
                                                                       0.0_fx, 1.0_fx};
                pre_clip_vert_buf_current_size_++;

            }

        }
        if(current_vertex_size_ == 3)
        {
            for(uint16_t i = first; i < first + count; ++i)
            {

                pre_clip_vert_buf_[pre_clip_vert_buf_current_size_] = {reinterpret_cast<math::vec3*>(vertex_pointer_)[i].x,
                                                                       reinterpret_cast<math::vec3*>(vertex_pointer_)[i].y,
                                                                       reinterpret_cast<math::vec3*>(vertex_pointer_)[i].z,
                                                                       1.0_fx};
                pre_clip_vert_buf_current_size_++;

            }

        }



        if(current_draw_type_ == DrawType::Points)
        {
            for(uint16_t i = first; i < (first + count); ++i)
            {
                pre_clip_color_buf_[pre_clip_color_buf_current_size_] = color_pointer_[i];
                pre_clip_color_buf_current_size_ ++;
            }
        }
        if(current_draw_type_ == DrawType::Lines)
        {
            for(uint16_t i = first; i < (first + count) / 2; ++i)
            {
                pre_clip_color_buf_[pre_clip_color_buf_current_size_] = color_pointer_[i];
                pre_clip_color_buf_current_size_ ++;
            }

        }
        if(current_draw_type_ == DrawType::Triangles)
        {
            for(uint16_t i = first; i < (first + count) / 3; ++i)
            {
                pre_clip_color_buf_[pre_clip_color_buf_current_size_] = color_pointer_[i];
                pre_clip_color_buf_current_size_ ++;
            }

        }

        vertex_pipeline();

    }

    void terrain(math::vec2 p,
                 math::fixed32 phi,
                 int16_t height,
                 int16_t horizon,
                 int16_t scale_height,
                 int16_t distance,
                 int16_t screen_width, int16_t screen_height,
                 uint8_t const * const hm,
                 uint16_t const * const cm)
    {
        //# Call the render function with the camera parameters:
        //# position, viewing angle, height, horizon line position,
        //# scaling factor for the height, the largest distance,
        //# screen width and the screen height parameter
        //Render( Point(0, 0), 0, 50, 120, 120, 300, 800, 600 )

        uint8_t ybuffer[256];
        for (uint16_t i = 0; i < 256; ++i)
        {
            ybuffer[i] = screen_height;
        }

        //# precalculate viewing angle parameters
        math::fixed32 sinphi = ffr::math::sin(phi);
        math::fixed32 cosphi = ffr::math::cos(phi);

        //# initialize visibility array. Y position for each column on screen


        //# Draw from front to the back (low z coordinate to high z coordinate)
        math::fixed32 dz = 1.0_fx;
        math::fixed32 z = 1.0_fx;
        math::fixed32 dist32 (distance);
        while (z < dist32)
        {
            //# Find line on map. This calculation corresponds to a field of view of 90°
            auto pleft = math::vec2{ ((-cosphi*z - sinphi*z) + p.x, ( sinphi*z - cosphi*z) + p.y) };
            auto pright = math::vec2{ (( cosphi*z - sinphi*z) + p.x, (-sinphi*z - cosphi*z) + p.y) };

            //# segment the line
            math::fixed32 dx = static_cast<math::fixed32>(static_cast<int16_t>((pright.x - pleft.x) / math::fixed32(screen_width)));
            math::fixed32 dy = static_cast<math::fixed32>(static_cast<int16_t>((pright.y - pleft.y) / math::fixed32(screen_width)));

            //# Raster line and draw a vertical line for each segment
            for (math::fixed32 i=0.0_fx; i < math::fixed32(screen_width); i = i + 1.0_fx)
            {
                // auto height_on_screen = (height - hm[pleft.x, pleft.y]) / z * scale_height + horizon;
                // //lineVertical(i, height_on_screen, ybuffer[i], cm[pleft.x, pleft.y]);
                // if (height_on_screen < ybuffer[i])
                // {
                //     ybuffer[i] = height_on_screen;
                // }
                pleft.x = pleft.x + dx;
                pleft.y = pleft.y + dy;
            }
            //# Go to next line and increase step size when you are far away
            z = z + dz;
            dz = dz + 0.2_fx;
        }
    }


private:
    int16_t view_width_ = 0;
    int16_t view_height_ = 0;

    DrawType current_draw_type_ = DrawType::Points;
    uint8_t current_vertex_size_ = 0;

    void* vertex_pointer_ = nullptr;
    uint16_t* color_pointer_ = nullptr;

    ffr::util::array<math::vec4, MAX_VERTS> pre_clip_vert_buf_;
    uint16_t pre_clip_vert_buf_current_size_ = 0;
    ffr::util::array<uint16_t, MAX_VERTS > pre_clip_color_buf_;
    uint16_t pre_clip_color_buf_current_size_ = 0;

    ffr::util::array<math::vec4, MAX_VERTS> post_clip_vert_buf_;
    uint16_t post_clip_vert_buf_current_size_ = 0;
    ffr::util::array<uint16_t, MAX_VERTS> post_clip_color_buf_;
    uint16_t post_clip_color_buf_current_size_ = 0;

    VertexFunction* vertex_function_ = nullptr;

    auto vertex_pipeline() -> void
    {

        ffr::util::array<math::vec4, 27> post_clip_verts;
        uint16_t post_clip_verts_size = 0;

        //run vertex shader
        for(uint16_t i = 0; i < pre_clip_vert_buf_current_size_; ++i)
        {
            vertex_function_[0](pre_clip_vert_buf_[i]);
        }

        if(current_draw_type_ == DrawType::Points)
        {
            for(uint16_t i = 0; i < pre_clip_vert_buf_current_size_; ++i)
            {
                if(clip_point(pre_clip_vert_buf_[i]))
                {
                    post_clip_vert_buf_[post_clip_vert_buf_current_size_] = pre_clip_vert_buf_[i];
                    post_clip_vert_buf_current_size_++;

                    post_clip_color_buf_[post_clip_color_buf_current_size_] = pre_clip_color_buf_[i];
                    post_clip_color_buf_current_size_++;
                }
            }
        }
        else if(current_draw_type_ == DrawType::Lines)
        {

        }
        else    //DrawType::Triangles
        {
            for(uint16_t i = 0; i < pre_clip_vert_buf_current_size_ - 2; i = i + 3)
            {

                auto col = pre_clip_color_buf_[i/3];

                post_clip_verts_size = clip_triangle(pre_clip_vert_buf_[i+0],pre_clip_vert_buf_[i+1],pre_clip_vert_buf_[i+2],post_clip_verts);

                for(uint16_t ci = 0; ci < post_clip_verts_size/3; ci++)
                {
                    post_clip_color_buf_[post_clip_color_buf_current_size_ + ci] = col;
                }
                post_clip_color_buf_current_size_ += post_clip_verts_size/3;

                for(uint16_t vertIndex = 0; vertIndex < post_clip_verts_size; ++vertIndex)
                {
                    //do w divide to yield ndc coords
                    post_clip_verts[vertIndex].x = post_clip_verts[vertIndex].x / post_clip_verts[vertIndex].w;
                    post_clip_verts[vertIndex].y = post_clip_verts[vertIndex].y / post_clip_verts[vertIndex].w;
                    post_clip_verts[vertIndex].z = post_clip_verts[vertIndex].z / post_clip_verts[vertIndex].w;

                    post_clip_vert_buf_[post_clip_vert_buf_current_size_ + vertIndex] = post_clip_verts[vertIndex];
                }
                post_clip_vert_buf_current_size_ += post_clip_verts_size;

            }
        }

        //run ndc to window transform
        for(uint16_t  i = 0; i < post_clip_vert_buf_current_size_; ++i)
        {
            post_clip_vert_buf_[i].x = ((math::fixed32(view_width_) * 0.5_fx) * post_clip_vert_buf_[i].x) + (math::fixed32(view_width_) * 0.5_fx);
            post_clip_vert_buf_[i].y = -(((math::fixed32(view_height_) * 0.5_fx) * post_clip_vert_buf_[i].y)) + (math::fixed32(view_height_) * 0.5_fx);
            post_clip_vert_buf_[i].z = (0.5_fx * post_clip_vert_buf_[i].z) + (0.5_fx);
        }


        //fnally, draw
        if(current_draw_type_ == DrawType::Triangles)
        {
            for(uint16_t l = 0; l < post_clip_vert_buf_current_size_ - 2; l = l + 3)
            {
                if(frontFacing( {post_clip_vert_buf_[l].x, post_clip_vert_buf_[l].y},
                                {post_clip_vert_buf_[l+1].x,post_clip_vert_buf_[l+1].y},
                                {post_clip_vert_buf_[l+2].x, post_clip_vert_buf_[l+2].y} ))
                {
                triangle(static_cast<int16_t>(post_clip_vert_buf_[l].x), static_cast<int16_t>(post_clip_vert_buf_[l].y),
                        static_cast<int16_t>(post_clip_vert_buf_[l+1].x), static_cast<int16_t>(post_clip_vert_buf_[l+1].y),
                        static_cast<int16_t>(post_clip_vert_buf_[l+2].x), static_cast<int16_t>(post_clip_vert_buf_[l+2].y),
                         post_clip_color_buf_[l/3]);
                }

            }
        }





    }

    //true if inside, false if out of bounds
    auto clip_point(math::vec4 const & in) -> bool
    {
        if ((in.x <= -1.0_fx ||
             in.x >= 1.0_fx ||
             in.y <= -1.0_fx ||
             in.y >= 1.0_fx ||
             in.z <= -1.0_fx ||
             in.z >= 1.0_fx))
        {
            return false;
        }
        else
        {
            return true;
        }
    }


    // Clip a triangle against all 6 homogeneous clip planes and triangulate result
    // Returns number of output vertices (always a multiple of 3)
    // Output contains triangulated vertices (every 3 vertices form a triangle)
    auto clip_triangle(math::vec4 v0, math::vec4 v1, math::vec4 v2, ffr::util::array<math::vec4, 27>& output) -> int
    {
        // Define the 6 clipping planes in homogeneous space
        // For a vertex v = (x, y, z, w), the planes are:
        // -w <= x <= w  =>  x + w >= 0 and -x + w >= 0
        // -w <= y <= w  =>  y + w >= 0 and -y + w >= 0
        // -w <= z <= w  =>  z + w >= 0 and -z + w >= 0

        // Clip order: near, left, right, bottom, top, far
        const math::vec4 planes[6] = {
            math::vec4{ 0.0_fx,  0.0_fx,  1.0_fx,  1.0_fx},  // z + w >= 0  (near)
            math::vec4{ 1.0_fx,  0.0_fx,  0.0_fx,  1.0_fx},  // x + w >= 0  (left)
            math::vec4{-1.0_fx,  0.0_fx,  0.0_fx,  1.0_fx},  // -x + w >= 0 (right)
            math::vec4{ 0.0_fx,  1.0_fx,  0.0_fx,  1.0_fx},  // y + w >= 0  (bottom)
            math::vec4{ 0.0_fx, -1.0_fx,  0.0_fx,  1.0_fx},  // -y + w >= 0 (top)
            math::vec4{ 0.0_fx,  0.0_fx, -1.0_fx,  1.0_fx}   // -z + w >= 0 (far)
        };

        // Working buffers for polygon clipping (ping-pong between them)
        math::vec4 buffer1[9];  // Max vertices after clipping a triangle is 9
        math::vec4 buffer2[9];

        // Initialize with input triangle
        buffer1[0].x = v0.x; buffer1[0].y = v0.y; buffer1[0].z = v0.z; buffer1[0].w = v0.w;
        buffer1[1].x = v1.x; buffer1[1].y = v1.y; buffer1[1].z = v1.z; buffer1[1].w = v1.w;
        buffer1[2].x = v2.x; buffer1[2].y = v2.y; buffer1[2].z = v2.z; buffer1[2].w = v2.w;

        int vertCount = 3;

        math::vec4* currentBuffer = buffer1;
        math::vec4* nextBuffer = buffer2;

        // Clip against each plane sequentially
        for (int planeIdx = 0; planeIdx < 6; planeIdx++) {
            const math::vec4& plane = planes[planeIdx];
            int outCount = 0;

            // Clip current polygon against this plane
            for (int i = 0; i < vertCount; i++) {
                const math::vec4& curr = currentBuffer[i];
                const math::vec4& next = currentBuffer[(i + 1) % vertCount];

                math::fixed32 currDist = (plane * curr);
                math::fixed32 nextDist = (plane * next);

                bool currInside = currDist >= 0.0_fx;
                bool nextInside = nextDist >= 0.0_fx;

                if (currInside) {
                    nextBuffer[outCount++] = curr;
                }

                // If edge crosses the plane, compute intersection
                if (currInside != nextInside) {
                    math::fixed32 t = currDist / (currDist - nextDist);
                    nextBuffer[outCount++] = curr + ((next - curr)*t);
                }
            }

            vertCount = outCount;

            if (vertCount == 0) {
                return 0;  // Triangle completely clipped
            }

            // Swap buffers
            math::vec4* temp = currentBuffer;
            currentBuffer = nextBuffer;
            nextBuffer = temp;
        }

        // Triangulate the resulting polygon using fan triangulation
        // Polygon vertices are in currentBuffer[0..vertCount-1]
        int outIndex = 0;
        for (int i = 1; i < vertCount - 1; i++) {
            output[outIndex++] = currentBuffer[0];
            output[outIndex++] = currentBuffer[i];
            output[outIndex++] = currentBuffer[i + 1];
        }

        return outIndex;
    }

    auto frontFacing(math::vec2 v0, math::vec2 v1, math::vec2 v2) -> bool
    {
        return ((v1.x - v0.x) * (v2.y - v0.y) - (v2.x - v0.x) * (v1.y - v0.y)) < 0.0_fx;
    }

};



}
