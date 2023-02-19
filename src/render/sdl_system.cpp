#include "sdl_system.hpp"

#include <SDL2/SDL.h>

namespace SDL_System
{

void exitHandler()
{
    if (SDL_WasInit(0))
        SDL_Quit();
}

int initSDL(uint32_t flags)
{
    if (!SDL_WasInit(flags))
    {
        atexit(&exitHandler);
        if (SDL_Init(flags) < 0)
            return 1;
    }
    return 0;
}
} // namespace SDL_System