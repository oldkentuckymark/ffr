#include "ffr.hpp"
#include <SDL2/SDL.h>


class SDLDRAWER : public ffr::Context
{
    SDL_Window* win{};
    SDL_Renderer* ren{};
public:
    SDLDRAWER()
    {
        SDL_CreateWindowAndRenderer(240*4,160*4,0,&win,&ren);
        SDL_RenderSetLogicalSize(ren, 240, 160);
    }

    auto virtual plot(uint16_t x, uint16_t y, uint16_t c) -> void override
    {
        auto col = ffr::Convert555to888(c);
        SDL_SetRenderDrawColor(ren,col[0],col[1],col[2],col[3]);
        //SDL_SetRenderDrawColor(ren,255,0,0,255);
        SDL_RenderDrawPoint(ren,x,y);
    }

    //auto virtual line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) -> void override
    //{
        //auto col = fren::Convert555to888(color);
        //SDL_SetRenderDrawColor(ren,col[0],col[1],col[2],col[3]);
        //SDL_RenderDrawLine(ren, x1,y1,x2,y2);

    //}

    auto clear() -> void override
    {
        SDL_SetRenderDrawColor(ren,0,0,0,255);
        SDL_RenderClear(ren);
    }

    auto present() -> void override
    {
        SDL_RenderPresent(ren);
    }
};

class VertexShader : public ffr::VertexFunction
{
public:
    VertexShader()
    {
        //mv = glm::mat4(1.0f);
        //pj = glm::ortho(0.0f,1.0f,1.0f,0.0f);
        //pj = glm::perspective(45.0f, 1.0f, 0.2f, 1000.0f);

    }

    auto operator() (const ffr::math::vec4& in) -> ffr::math::vec4 override
    {
        //fren::math::vec4 r;
        //r.x = in.x / in.z;
        //r.y = in.y / in.z;
        //r.z = in.z;
        //r.w = in.w;
        //return r;
        //return in;
        return pj * mv * in;
    }

    ffr::math::mat4 mv, pj;
};



ffr::math::fixed32 par[6] =
{
    0.25_fx,0.25_fx, 0.25_fx,0.5_fx, 0.5_fx,0.5_fx
};

uint16_t car[6] =
{
    UINT16_MAX,UINT16_MAX,UINT16_MAX,UINT16_MAX,UINT16_MAX,UINT16_MAX
};




auto main(int argc, char *argv[]) -> int
{

    SDL_Init(SDL_INIT_VIDEO);

    ffr::math::vec3 pos;
    pos.x = 0.0_fx;
    pos.y = 0.0_fx;
    pos.z = 0.0_fx;
    ffr::math::mat4 m;
    ffr::math::mat4 p = ffr::math::mat4::perspective(65.0_fx,1.6_fx, 0.1_fx, 1000.0_fx);
    VertexShader vs;
    vs.pj = p;



    SDLDRAWER r;
    r.setViewPort(160,128);
    r.setVertexFunction(&vs);

    bool running = true;
    while (running)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if(e.type == SDL_QUIT)
            {
                running = false;
            }
            else if(e.type == SDL_MOUSEMOTION)
            {
                //cam0ref.yaw_ -= (float)e.motion.xrel / 200.f;

            }
        }
        uint8_t const * const keys = SDL_GetKeyboardState(nullptr);
        if (keys[SDL_SCANCODE_ESCAPE])
        {
            running = false;
        }
        if (keys[SDL_SCANCODE_W])
        {

        }

        const auto sintable = ffr::math::makeTable< int,30,std::sinf >;

        pos.z = pos.z - 0.00007_fx;
        m = ffr::math::mat4::translation(pos);
        vs.mv = m;

        r.clear();

        r.VertexPointer(2, par);
        r.ColorPointer(car);

        r.DrawArray(ffr::DrawType::Line_Loop, 0, 3);

        r.present();



    }




    return 0;
}
