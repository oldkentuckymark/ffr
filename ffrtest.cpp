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




auto main(int argc, char *argv[]) -> int
{

    SDL_Init(SDL_INIT_VIDEO);


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



    }




    return 0;
}
