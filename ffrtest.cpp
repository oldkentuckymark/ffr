#include "ffr.hpp"
#include <SDL2/SDL.h>


ffr::math::fixed32 par[18] =
{
    0.25_fx,0.25_fx, -0.2_fx, 0.25_fx,0.5_fx,-0.2_fx, 0.5_fx,0.5_fx,-0.2_fx,   2.25_fx,2.25_fx, -0.2_fx, 2.25_fx,2.5_fx,-0.2_fx, 2.5_fx,2.5_fx,-0.2_fx
};

uint16_t car[2] =
{
    UINT16_MAX,60000// 31744
};



class SDL_Context : public ffr::Context<128>
{
public:
    SDL_Context()
    {
        SDL_CreateWindowAndRenderer(240*4,160*4,0,&win,&ren);
        SDL_RenderSetScale(ren, 4.0,4.0);
    }

    ~SDL_Context()
    {

    }

    void clear() override
    {
        SDL_SetRenderDrawColor(ren,0,0,0,255);
        SDL_RenderClear(ren);
    }

    void present() override
    {
        SDL_RenderPresent(ren);
    }

    void plot(uint16_t x, uint16_t y, uint16_t c) override
    {
        auto cc = ffr::Convert555to888(c);
        SDL_SetRenderDrawColor(ren,cc[0],cc[1],cc[2],255);
        SDL_RenderDrawPoint(ren,x,y);
    }

private:
    SDL_Window* win;
    SDL_Renderer* ren;
};


class VF : public ffr::VertexFunction
{
public:

    ffr::math::mat4 mv, pj;

    auto operator()(ffr::math::vec4& in) -> void override
    {

        in = pj * mv * in;

        //in.w = in.z * 2.0_fx;
    }

};


auto main(int argc, char *argv[]) -> int
{

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Context c;
    c.setViewPort(240,160);
    c.setVertexPointer(reinterpret_cast<ffr::math::vec3*>(par));
    c.setColorPointer(car);

    VF vf;
    ffr::math::fixed32 g = 0.0_fx;



    c.setVertexFunction(&vf);


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

        //const auto sintable = ffr::math::makeTable< int,30,std::sinf >;
        c.clear();

        c.drawArray(ffr::DrawType::Triangles, 0, 6);

        //c.triangle(20,20,50,25,30,80,UINT16_MAX);

        c.present();


        vf.mv = ffr::math::mat4::translation(ffr::math::vec3{0.0_fx,0.0_fx,g});
        g = g - 0.001_fx;
        vf.pj = ffr::math::mat4::perspective(90.0_fx,0.6666_fx,1.0_fx, 1000.0_fx);


    }




    return 0;
}
