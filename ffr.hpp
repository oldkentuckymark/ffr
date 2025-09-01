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

enum class DrawType
{
    Points,
    Lines,
    Line_Strip,
    Line_Loop
};

class VertexFunction
{
public:
    VertexFunction() {}
    virtual ~VertexFunction() {}
    virtual ffr::math::vec4 operator()(const ffr::math::vec4& in) = 0;
};


//color of line is primitive by color of first vertex
class Context
{
public:

    virtual void plot(uint16_t x, uint16_t y, uint16_t color) {};

    virtual void line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
    {

        bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);

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
        int16_t ystep = (y0 < y1) ? 1 : -1;
        int16_t y     = y0;

        for (int x = x0; x <= x1; ++x)
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
    virtual void lineHorizontal(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t color) {}
    virtual void lineVertical(uint16_t x1, uint16_t y1, uint16_t y2, uint16_t color) {}

    virtual void clear() {}
    virtual void present() {}


    Context()
    {
        vertex_pointer = nullptr;
        color_pointer = nullptr;
        index_pointer = nullptr;
        vertex_function = nullptr;

        xres = 0;
        yres = 0;
    }
    virtual ~Context()
    {

    }

    void setVertexFunction(VertexFunction* vf)
    {
        vertex_function = vf;
    }

    void setViewPort(const uint16_t x, const uint16_t y)
    {
        xres = x;
        yres = y;
    }

    void VertexPointer(const uint8_t size, void* pointer)
    {
        vertex_pointer = pointer;
        vertex_size = size;
    }
    void ColorPointer(uint16_t* pointer)
    {
        color_pointer = pointer;
    }
    void IndexPointer(uint8_t* pointer)
    {
        index_pointer = pointer;
    }

    void DrawArray(DrawType drawtype, const uint32_t first, const uint32_t count)
    {
        if (!vertex_pointer)
        {
            return;
        }

        draw_type = drawtype;

        //gather pos into buffer
        work_buff.clear();

        if(vertex_size == 2)
        {
            ffr::math::vec2* vp = reinterpret_cast<ffr::math::vec2*>(vertex_pointer);
            auto ggg = vp[0].y;
            for(uint32_t i = first ;i < count; ++i)
            {
                work_buff.push_back( Vertex{ {vp[i].x, vp[i].y,0.0_fx,1.0_fx}, UINT16_MAX } );
            }
        }
        else if(vertex_size == 3)
        {
            ffr::math::vec3* vp = reinterpret_cast<ffr::math::vec3*>(vertex_pointer);
            for(uint32_t i = first ;i < count; ++i)
            {
                work_buff.push_back( Vertex{ {vp[i].x, vp[i].y,vp[i].z,1.0_fx}, UINT16_MAX } );
            }
        }
        else if(vertex_size == 4)
        {
            ffr::math::vec4* vp = reinterpret_cast<ffr::math::vec4*>(vertex_pointer);
            for(uint32_t i = first ;i < count; ++i)
            {
                work_buff.push_back( Vertex{ vp[i], UINT16_MAX } );
            }
        }

        //gather col into buffer
        if(color_pointer)
        {
            uint16_t* cp = color_pointer;
            for(uint32_t i = first; i < count; ++i)
            {
                work_buff[i].col = cp[i];
            }
        }

        vertex_pipeline();
    }

    void DrawElements(DrawType const drawtype, uint32_t const count)
    {
        if (!index_pointer || !vertex_pointer)
        {
            return;
        }

        draw_type = drawtype;

        //gather pos into buffer
        work_buff.clear();

        if(vertex_size == 2)
        {
            ffr::math::vec2* vp = reinterpret_cast<ffr::math::vec2*>(vertex_pointer);
            for(uint32_t i = 0 ;i < count; ++i)
            {
                work_buff.push_back( Vertex{ {vp[index_pointer[i]].x,vp[index_pointer[i]].y, 0.0_fx, 1.0_fx} , UINT16_MAX } );
            }
        }
        else if(vertex_size == 3)
        {
            ffr::math::vec3* vp = reinterpret_cast<ffr::math::vec3*>(vertex_pointer);
            for(uint32_t i = 0 ;i < count; ++i)
            {
                work_buff.push_back( Vertex{ {vp[index_pointer[i]].x, vp[index_pointer[i]].y, vp[index_pointer[i]].z, 1.0_fx} , UINT16_MAX } );
            }
        }
        else if(vertex_size == 4)
        {
            ffr::math::vec4* vp = reinterpret_cast<ffr::math::vec4*>(vertex_pointer);
            for(uint32_t i = 0 ;i < count; ++i)
            {
                work_buff.push_back( Vertex{ {vp[index_pointer[i]]} , UINT16_MAX } );
            }
        }

        //gather col into buffer
        if(color_pointer)
        {
            uint16_t* cp = color_pointer;
            for(uint32_t i = 0; i < count; ++i)
            {
                work_buff[i].col = cp[index_pointer[i]];
            }
        }

        vertex_pipeline();
    }

    struct Vertex
    {
        ffr::math::vec4 pos;
        uint16_t col;
    };

protected:

    uint16_t xres, yres;
    void* vertex_pointer;
    uint16_t* color_pointer;
    uint8_t* index_pointer;

    uint8_t vertex_size;

    DrawType draw_type;

    VertexFunction* vertex_function;

    std::vector<Vertex> work_buff;


    std::vector<Vertex> convert_to_lines(std::vector<Vertex>& in, DrawType dt)
    {
        std::vector<Vertex> out;

        if(dt == DrawType::Points)
        {
            for(uint8_t i = 0; i  <in.size(); ++i)
            {
                out.push_back(in[i]);
                out.push_back(in[i]);
            }
        }
        else if(dt == DrawType::Lines)
        {
            out = in;
        }
        else if(dt == DrawType::Line_Strip)
        {
            for(uint8_t i = 0; i < in.size() - 1; ++i)
            {
                out.push_back( in[i] );
                out.push_back( in[i + 1] );
            }
        }
        else if (dt == DrawType::Line_Loop)
        {
            for(uint8_t i = 0; i < in.size() - 1; ++i)
            {
                out.push_back( in[i] );
                out.push_back( in[i + 1] );
            }
            out.push_back(in[in.size() - 1]);
            out.push_back(in[0]);
        }

        return out;
    }


    void vertex_pipeline()
    {
        std::vector<Vertex> clip_buff = run_vertex_function(work_buff);
        std::vector<Vertex> line_buff = convert_to_lines(clip_buff, draw_type);
        std::vector<Vertex> ndc_buff = run_clip_function(line_buff);
        std::vector<Vertex> wt_buff = run_ndc_function(ndc_buff);
        std::vector<Vertex> draw_buff = run_windowtransform_function(wt_buff);
        run_draw_function(draw_buff);
    }

    std::vector<Vertex> run_vertex_function(std::vector<Vertex>& in)
    {
        std::vector<Vertex> out;
        for (auto& i : in)
        {
            out.push_back(  Vertex{ vertex_function[0](i.pos) , i.col }  );
        }
        return out;
    }
    std::vector<Vertex> run_clip_function(std::vector<Vertex>& in)
    {
        std::vector<Vertex> out;

        for(uint8_t i = 0; i < in.size() - 1; i = i + 2)
        {
            Vertex pi1, pi2, po1, po2;
            pi1 = in[i];
            pi2 = in[i+1];
            bool pi1in = clip_point(pi1);
            bool pi2in = clip_point(pi2);
            if(pi1in && pi2in)
            {
                out.push_back(pi1);
                out.push_back(pi2);
            }
            else if(pi1in || pi2in)
            {
                clip_line_component(pi1,pi2, 0, 1.0_fx, po1, po2);
                clip_line_component(po1,po2, 0, -1.0_fx, pi1, pi2);
                clip_line_component(pi1,pi2, 1, 1.0_fx, po1, po2);
                clip_line_component(po1,po2, 1, -1.0_fx, pi1, pi2);
                clip_line_component(pi1,pi2, 2, 1.0_fx, po1, po2);
                clip_line_component(po1,po2, 2, -1.0_fx, pi1, pi2);

                out.push_back( pi1 );
                out.push_back( pi2 );
            }


        }

        return  out;
    }
    std::vector<Vertex> run_ndc_function(std::vector<Vertex>& in)
    {
        std::vector<Vertex> out;
        for (auto& i : in)
        {
            out.push_back( Vertex{ { i.pos / i.pos.w }, i.col });
        }
        return out;
    }
    std::vector<Vertex> run_windowtransform_function(std::vector<Vertex>& in)
    {
        std::vector<Vertex> out;
        for (auto& i : in)
        {
            out.push_back
                (
                    Vertex
                    {
                        {
                     ((math::fixed32(xres) / 2.0_fx) * i.pos.x) + (static_cast<math::fixed32>(xres) / 2.0_fx),
                            -((static_cast<math::fixed32>(yres) / 2.0_fx) * i.pos.y) + (static_cast<math::fixed32>(yres) / 2.0_fx),
                            ((1.0_fx / 2.0_fx) * i.pos.z) + (1.0_fx / 2.0_fx),
                            i.pos.w
                        },
                        i.col
                    }
                    );
        }
        return out;
    }

    void run_draw_function(std::vector<Vertex>& in)
    {
        if(in.empty())
        {
            return;
        }

        for(uint32_t i = 0; i < in.size() - 1; i = i + 2)
        {


            //laserOff();
            //laserMove(in[i].pos.x, in[i].pos.y);
            //laserOn();
            //laserColor(in[i].col.r, in[i].col.g, in[i].col.b);

            line(static_cast<int16_t>(in[i].pos.x),
                 static_cast<int16_t>(in[i].pos.y),
                 static_cast<int16_t>(in[i+1].pos.x),
                 static_cast<int16_t>(in[i+1].pos.y),
                 in[i].col);


            //laserMove(in[i+1].pos.x, in[i+1].pos.y);
            //laserColor(in[i+1].col.r, in[i+1].col.g, in[i].col.b);
        }
        //laserOff();
    }

    // returns true if point is inside volume
    bool clip_point(const Vertex& in)
    {
        if ((in.pos.x < -in.pos.w ||
             in.pos.x > in.pos.w ||
             in.pos.y < -in.pos.w ||
             in.pos.y > in.pos.w ||
             in.pos.z < -in.pos.w ||
             in.pos.z > in.pos.w))
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    void clip_line_component(const Vertex& q1, const Vertex& q2,
                             const uint8_t index, const math::fixed32 factor,
                             Vertex& q1new, Vertex& q2new)
    {
        q1new = q1;
        q2new = q2;

        Vertex previousVertex = q2;
        math::fixed32 previousComponent = previousVertex.pos[index] * factor;
        bool previousInside = previousComponent <= previousVertex.pos.w;

        Vertex currentVertex = q1;
        math::fixed32 currentComponent = currentVertex.pos[index] * factor;
        bool currentInside = currentComponent <= currentVertex.pos.w;

        if((currentInside) && (!previousInside))
        {
            math::fixed32 lerpAmount = (previousVertex.pos.w - previousComponent) /
                               ((previousVertex.pos.w - previousComponent) -
                                (currentVertex.pos.w - currentComponent));
            q2new.pos = ffr::math::mix(previousVertex.pos, currentVertex.pos, lerpAmount);

            //fren::math::vec3 p2v3 = fren::math::mix(fren::math::vec3(previousVertex.pos), fren::math::vec3(currentVertex.pos), lerpAmount);
            //q2new.pos = fren::math::vec4(p2v3, previousVertex.pos.w);

            return;

        }
        else if((!currentInside) && (previousInside))
        {
            math::fixed32 lerpAmount = (currentVertex.pos.w - currentComponent) /
                               ((currentVertex.pos.w - currentComponent) -
                                (previousVertex.pos.w - previousComponent));
            q1new.pos = ffr::math::mix(currentVertex.pos, previousVertex.pos, lerpAmount);
            return;
        }
        return;
    }


};






    /*
def Render(p, phi, height, horizon, scale_height, distance, screen_width, screen_height):
    # precalculate viewing angle parameters
    var sinphi = math.sin(phi);
    var cosphi = math.cos(phi);

    # initialize visibility array. Y position for each column on screen
    ybuffer = np.zeros(screen_width)
    for i in range(0, screen_width):
        ybuffer[i] = screen_height

    # Draw from front to the back (low z coordinate to high z coordinate)
    dz = 1.
    z = 1.
    while z < distance
        # Find line on map. This calculation corresponds to a field of view of 90°
        pleft = Point(
            (-cosphi*z - sinphi*z) + p.x,
            ( sinphi*z - cosphi*z) + p.y)
        pright = Point(
            ( cosphi*z - sinphi*z) + p.x,
            (-sinphi*z - cosphi*z) + p.y)

        # segment the line
        dx = (pright.x - pleft.x) / screen_width
        dy = (pright.y - pleft.y) / screen_width

        # Raster line and draw a vertical line for each segment
        for i in range(0, screen_width):
            height_on_screen = (height - heightmap[pleft.x, pleft.y]) / z * scale_height. + horizon
            DrawVerticalLine(i, height_on_screen, ybuffer[i], colormap[pleft.x, pleft.y])
            if height_on_screen < ybuffer[i]:
                ybuffer[i] = height_on_screen
            pleft.x += dx
            pleft.y += dy

        # Go to next line and increase step size when you are far away
        z += dz
        dz += 0.2

# Call the render function with the camera parameters:
# position, viewing angle, height, horizon line position,
# scaling factor for the height, the largest distance,
# screen width and the screen height parameter
Render( Point(0, 0), 0, 50, 120, 120, 300, 800, 600 )

void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
    int16_t dx = std::abs(x1 - x0);
    int16_t dy = std::abs(y1 - y0);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;

    while (true)
    {
        plot(x0, y0);

        if (x0 == x1 && y0 == y1)
        {
            break;
        }

        int16_t e2 = 2 * err;

        if (e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }

        if (e2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}




*/

}
