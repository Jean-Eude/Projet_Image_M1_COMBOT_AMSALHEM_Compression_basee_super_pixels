#pragma once

#include <SDL2/SDL.h>
#include <cstdio>
#include <iostream>

class Window {
    public:
        Window(const char* title, int width, int height);
        Window() : title(nullptr), width(0), height(0), window(nullptr), renderer(nullptr), frameBuffer(nullptr), done(false) {}
        void destroyWindow();


        int getWidth();
        int getHeight();

        SDL_Window* getWindow();
        void setWindowSize(int width, int height);
        SDL_Renderer* getRenderer();
        SDL_Texture* getTexture();

        bool getDone();
        void setDone(bool newdone);

    private:
        int width, height;
        const char* title;

        SDL_Window* window;
        SDL_Renderer* renderer;
        SDL_Texture* frameBuffer;
        bool done = false;
};