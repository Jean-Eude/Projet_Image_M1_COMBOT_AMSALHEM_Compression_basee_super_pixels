#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <iostream>


int main()
{
    int width = 1280, height = 720;
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        std::cout << "Failed to initialize the SDL2 library\n";
        std::cout << "SDL2 Error: " << SDL_GetError() << "\n";
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow("Projet Image - Compressions basée superpixels",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          width, height,
                                          0);

    if(!window)
    {
        std::cout << "Failed to create window\n";
        std::cout << "SDL2 Error: " << SDL_GetError() << "\n";
        return -1;
    }

    SDL_Surface *window_surface = SDL_GetWindowSurface(window);
    uint32_t *buffer = static_cast<uint32_t*>(window_surface->pixels);


    if(!window_surface)
    {
        std::cout << "Failed to get the surface from the window\n";
        std::cout << "SDL2 Error: " << SDL_GetError() << "\n";
        return -1;
    }

    bool keep_window_open = true;
    while(keep_window_open)
    {
        SDL_Event e;
        while(SDL_PollEvent(&e) > 0)
        {
            switch(e.type)
            {
                case SDL_QUIT:
                    keep_window_open = false;
                    break;
            }

    
            SDL_UpdateWindowSurface(window);
        }
    }
    
    SDL_DestroyWindow(window);
    SDL_Quit();
}

