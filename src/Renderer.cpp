#include "Renderer.h"
#include <iostream>

Renderer::Renderer(int w, int h, const char* title) : width(w), height(h) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not initialize! Error: " << SDL_GetError() << std::endl;
        window = nullptr;
        sdlRenderer = nullptr;
        return;
    }

    window = SDL_CreateWindow(title,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        width, height,
        SDL_WINDOW_SHOWN);

    if (window == nullptr) {
        std::cout << "Window could not be created! Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return;
    }

    sdlRenderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (sdlRenderer == nullptr) {
        std::cout << "Renderer could not be created! Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }
}

Renderer::~Renderer() {
    if (sdlRenderer != nullptr) {
        SDL_DestroyRenderer(sdlRenderer);
    }
    if (window != nullptr) {
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
}

void Renderer::clear() {
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
    SDL_RenderClear(sdlRenderer);
}

void Renderer::present() {
    SDL_RenderPresent(sdlRenderer);
}

bool Renderer::isRunning() const {
    return window != nullptr && sdlRenderer != nullptr;
}

SDL_Renderer* Renderer::getSDLRenderer() {
    return sdlRenderer;
}

void Renderer::drawWall(int x, int height, int textureId) {
    // empty yet
}