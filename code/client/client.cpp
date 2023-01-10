
#define SDL_MAIN_HANDLED
#include <iostream>
#include "SDL.h"
#include <iostream>

SDL_Window *window = NULL; // the window we render to

void close()
{
    // Destroy window
    SDL_DestroyWindow(window);
    window = NULL;

    // Quit SDL subsystems
    SDL_Quit();
}

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "Failed to initialize the SDL2 library\n";
        return -1;
    }

    window = SDL_CreateWindow("SDL2 Window",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          680, 480,
                                          0);

    if (!window)
    {
        std::cerr << "Failed to create window\n";
        return -1;
    }

    SDL_Surface *window_surface = SDL_GetWindowSurface(window);

    if (!window_surface)
    {
        std::cerr << "Failed to get the surface from the window\n";
        return -1;
    }

    SDL_PumpEvents();
    SDL_PumpEvents();
    SDL_PumpEvents();

    bool quit{false};
    // Event handler
    SDL_Event e;

    while (!quit)
    {

        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                quit = true;
            }
        }
        SDL_UpdateWindowSurface(window);
    }
    close();
}
