#include "Renderer.h"
#include "Player.h"
#include "Map.h"
#include <SDL2/SDL.h>
#include <cmath>
#include <iostream>

Renderer::Renderer(int w, int h, const char* title) {
    width = w;
    height = h;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL init error: " << SDL_GetError() << std::endl;
        window = nullptr;
        sdlRenderer = nullptr;
        return;
    }

    window = SDL_CreateWindow(title,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width, height, SDL_WINDOW_SHOWN);

    if (window == nullptr) {
        std::cout << "Window create error: " << SDL_GetError() << std::endl;
        return;
    }

    sdlRenderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (sdlRenderer == nullptr) {
        std::cout << "Renderer create error: " << SDL_GetError() << std::endl;
        return;
    }
}

Renderer::~Renderer() {
    if (sdlRenderer) SDL_DestroyRenderer(sdlRenderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}

void Renderer::clear() {
    if (sdlRenderer) {
        SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
        SDL_RenderClear(sdlRenderer);
    }
}

void Renderer::present() {
    if (sdlRenderer) {
        SDL_RenderPresent(sdlRenderer);
    }
}

bool Renderer::isRunning() const {
    return (window != nullptr && sdlRenderer != nullptr);
}

SDL_Renderer* Renderer::getSDLRenderer() {
    return sdlRenderer;
}

void Renderer::drawVerticalLine(int x, int yStart, int yEnd, int r, int g, int b) {
    if (yStart < 0) yStart = 0;
    if (yEnd >= height) yEnd = height - 1;

    SDL_SetRenderDrawColor(sdlRenderer, r, g, b, 255);
    SDL_RenderDrawLine(sdlRenderer, x, yStart, x, yEnd);
}

void Renderer::render3D(const Player& player, const Map& map) {
    // display clean, draw cell
    SDL_SetRenderDrawColor(sdlRenderer, 50, 50, 50, 255);
    SDL_RenderClear(sdlRenderer);

    SDL_Rect floorRect;
    floorRect.x = 0;
    floorRect.y = height / 2;
    floorRect.w = width;
    floorRect.h = height / 2;

    SDL_SetRenderDrawColor(sdlRenderer, 100, 100, 100, 255);
    SDL_RenderFillRect(sdlRenderer, &floorRect);

    // rays variabls
    float fov = player.getFov();
    int numRays = width;
    float angleStep = fov / numRays;
    float currentAngle = player.getDir() - fov / 2.0f;

    for (int i = 0; i < numRays; i++) {
        float rayDirX = cos(currentAngle);
        float rayDirY = sin(currentAngle);

        float mapX = player.getX();
        float mapY = player.getY();

        float distanceToWall = 0.0f;
        bool hitWall = false;
        float stepSize = 0.02f;

        float testX = mapX;
        float testY = mapY;

        while (!hitWall && distanceToWall < 20.0f) {
            distanceToWall += stepSize;
            testX = mapX + rayDirX * distanceToWall;
            testY = mapY + rayDirY * distanceToWall;

            if (map.isWall((int)testX, (int)testY)) {
                hitWall = true;
            }
        }

        float correctedDistance = distanceToWall * cos(currentAngle - player.getDir());

        // height calc
        int wallHeight = (int)(height / correctedDistance);

        // color and darkness
        int colorVal = 255 - (int)(correctedDistance * 12);
        if (colorVal < 0) colorVal = 0;

        int yStart = (height / 2) - (wallHeight / 2);
        int yEnd = (height / 2) + (wallHeight / 2);

        drawVerticalLine(i, yStart, yEnd, colorVal, colorVal, colorVal);

        currentAngle += angleStep;
    }
}