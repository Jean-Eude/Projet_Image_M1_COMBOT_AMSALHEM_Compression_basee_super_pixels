#include <Window.h>

Window::Window(const char* title, int width, int height) {
    this->title = title;
    this->width = width;
    this->height = height;


    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return;
    }

    this->window = SDL_CreateWindow(this->title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, this->width, this->height, SDL_WINDOW_SHOWN);
    this->renderer = SDL_CreateRenderer(this->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    this->frameBuffer = SDL_CreateTexture(this->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, this->width, this->height);
}

void Window::destroyWindow() {
    SDL_DestroyTexture(this->frameBuffer);
    SDL_DestroyRenderer(this->renderer);
    SDL_DestroyWindow(this->window);
    SDL_Quit();
}

int Window::getWidth() {
    return this->width;
}

int Window::getHeight() {
    return this->height;
}


SDL_Window* Window::getWindow() {
    return this->window;
}

void Window::setWindowSize(int width, int height) {
    SDL_SetWindowSize(this->window, width, height);
    this->width = width;
    this->height = height;
}

SDL_Renderer* Window::getRenderer() {
    return this->renderer;
}

SDL_Texture* Window::getTexture() {
    return this->frameBuffer;
}


bool Window::getDone() {
    return this->done;
}

void Window::setDone(bool newdone) {
    this->done = newdone;
}

void Window::destroyTexture() {
    SDL_DestroyTexture(frameBuffer);
    frameBuffer = nullptr;
}

void Window::createTexture(Uint32 format, int access, int textureWidth, int textureHeight) {
    if (frameBuffer != nullptr) {
        SDL_DestroyTexture(frameBuffer);
        frameBuffer = nullptr;
    }

    // Créer une nouvelle texture avec les spécifications données
    frameBuffer = SDL_CreateTexture(renderer, format, access, textureWidth, textureHeight);
    if (frameBuffer == nullptr) {
        printf("Erreur lors de la création de la texture : %s\n", SDL_GetError());
    }
}
