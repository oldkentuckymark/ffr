#include "ffr.hpp"
#include <SDL2/SDL.h>


ffr::math::fixed32 par[6] =
{
    0.25_fx,0.25_fx, 0.25_fx,0.5_fx, 0.5_fx,0.5_fx
};

uint16_t car[6] =
{
    UINT16_MAX,UINT16_MAX,UINT16_MAX,UINT16_MAX,UINT16_MAX,UINT16_MAX
};



class SDL_Context : public ffr::Context<64>
{
public:
    SDL_Context()
    {
        SDL_CreateWindowAndRenderer(240,160,0,&win,&ren);
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
        SDL_SetRenderDrawColor(ren,255,255,255,255);
        SDL_RenderDrawPoint(ren,x,y);
    }

private:
    SDL_Window* win;
    SDL_Renderer* ren;
};


auto main(int argc, char *argv[]) -> int
{

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Context c;

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

        c.triangle(20,23,100,20,100,100, 0);

        c.present();


    }




    return 0;
}
