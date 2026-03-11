#include "Renderer.h"
#include "Player.h"
#include "Map.h"
#include <SDL2/SDL.h>
#include <cmath>
#include <iostream>

Renderer::Renderer(int w, int h, const char* title) {
    width = w;
    height = h;
    floorTexture = nullptr;
    ceilingTexture = nullptr;
    gunTexture = nullptr;
    enemyTexture = nullptr;
    textureWidth = 64;
    textureHeight = 64;
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
    for (auto& pair : wallTextures) {
        SDL_DestroyTexture(pair.second);
    }
    if (floorTexture) SDL_DestroyTexture(floorTexture);
    if (ceilingTexture) SDL_DestroyTexture(ceilingTexture);
    if (gunTexture) SDL_DestroyTexture(gunTexture);
    if (enemyTexture) SDL_DestroyTexture(enemyTexture);
    
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

bool Renderer::loadWallTexture(int id, const std::string& path) {
    SDL_Surface* surface = SDL_LoadBMP(path.c_str());
    if (surface == nullptr) {
        std::cout << "Unable to load wall texture: " << path << " Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // set black as empty
    SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 0, 0, 0));

    SDL_Texture* texture = SDL_CreateTextureFromSurface(sdlRenderer, surface);
    SDL_FreeSurface(surface);
    
    if (texture == nullptr) {
        std::cout << "Unable to create texture from surface: " << SDL_GetError() << std::endl;
        return false;
    }

    wallTextures[id] = texture;
    
    // size of first texture
    if (wallTextures.size() == 1) {
        SDL_QueryTexture(texture, nullptr, nullptr, &textureWidth, &textureHeight);
    }
    
    return true;
}

bool Renderer::loadFloorTexture(const std::string& path) {
    SDL_Surface* surface = SDL_LoadBMP(path.c_str());
    if (surface == nullptr) {
        std::cout << "Unable to load floor texture: " << path << std::endl;
        return false;
    }
    
    floorTexture = SDL_CreateTextureFromSurface(sdlRenderer, surface);
    SDL_FreeSurface(surface);
    return floorTexture != nullptr;
}

bool Renderer::loadCeilingTexture(const std::string& path) {
    SDL_Surface* surface = SDL_LoadBMP(path.c_str());
    if (surface == nullptr) {
        std::cout << "Unable to load ceiling texture: " << path << std::endl;
        return false;
    }
    
    ceilingTexture = SDL_CreateTextureFromSurface(sdlRenderer, surface);
    SDL_FreeSurface(surface);
    return ceilingTexture != nullptr;
}

// if texture has black backgtound
// SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 0, 0, 0));

bool Renderer::loadGunTexture(const std::string& path) {
    SDL_Surface* surface = SDL_LoadBMP(path.c_str());
    if (surface == nullptr) {
        std::cout << "Unable to load gun texture: " << path << std::endl;
        return false;
    }
    
    gunTexture = SDL_CreateTextureFromSurface(sdlRenderer, surface);
    SDL_FreeSurface(surface);
    return gunTexture != nullptr;
}

bool Renderer::loadEnemyTexture(const std::string& path) {
    SDL_Surface* surface = SDL_LoadBMP(path.c_str());
    if (surface == nullptr) {
        std::cout << "Unable to load enemy texture: " << path << std::endl;
        return false;
    }
    
    enemyTexture = SDL_CreateTextureFromSurface(sdlRenderer, surface);
    SDL_FreeSurface(surface);
    return enemyTexture != nullptr;
}

void Renderer::drawVerticalLine(int x, int yStart, int yEnd, int r, int g, int b) {
    if (yStart < 0) yStart = 0;
    if (yEnd >= height) yEnd = height - 1;

    SDL_SetRenderDrawColor(sdlRenderer, r, g, b, 255);
    SDL_RenderDrawLine(sdlRenderer, x, yStart, x, yEnd);
}

void Renderer::render3D(const Player& player, const Map& map) {
    // clean up
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
    SDL_RenderClear(sdlRenderer);

    // rendering roof
    if (ceilingTexture) {
        SDL_Rect ceilingRect = {0, 0, width, height / 2};
        SDL_RenderCopy(sdlRenderer, ceilingTexture, nullptr, &ceilingRect);
    } else {
        SDL_SetRenderDrawColor(sdlRenderer, 50, 50, 80, 255);
        SDL_Rect ceilingRect = {0, 0, width, height / 2};
        SDL_RenderFillRect(sdlRenderer, &ceilingRect);
    }

    // rendering floor
    if (floorTexture) {
        SDL_Rect floorRect = {0, height / 2, width, height / 2};
        SDL_RenderCopy(sdlRenderer, floorTexture, nullptr, &floorRect);
    } else {
        SDL_SetRenderDrawColor(sdlRenderer, 100, 100, 100, 255);
        SDL_Rect floorRect = {0, height / 2, width, height / 2};
        SDL_RenderFillRect(sdlRenderer, &floorRect);
    }

    // rays parametrs 
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
        float stepSize = 0.05f;

        float testX = mapX;
        float testY = mapY;
        int wallTileX = 0, wallTileY = 0;
        // the side of wall
        bool hitVerticalSide = false;

        while (!hitWall && distanceToWall < 20.0f) {
            distanceToWall += stepSize;
            testX = mapX + rayDirX * distanceToWall;
            testY = mapY + rayDirY * distanceToWall;

            if (map.isWall((int)testX, (int)testY)) {
                hitWall = true;
                wallTileX = (int)testX;
                wallTileY = (int)testY;
                
                // checking direction of ray
                float prevX = mapX + rayDirX * (distanceToWall - stepSize);
                float prevY = mapY + rayDirY * (distanceToWall - stepSize);
                
                if ((int)prevX != (int)testX) {
                    hitVerticalSide = true;
                }

                else if ((int)prevY != (int)testY) {
                    hitVerticalSide = false;
                }
            }
        }

        if (!hitWall) {
            currentAngle += angleStep;
            continue;
        }

        float correctedDistance = distanceToWall * cos(currentAngle - player.getDir());

        int wallHeight = (int)(height / correctedDistance);

        // calc of coords of the wall
        float wallX;
        if (hitVerticalSide) {
            wallX = testY - floor(testY);
        } else {
            wallX = testX - floor(testX);
        }

        if (hitVerticalSide && rayDirX < 0) {
            wallX = 1.0f - wallX;
        } else if (!hitVerticalSide && rayDirY < 0) {
            wallX = 1.0f - wallX;
        }

        int texX = (int)(wallX * textureWidth);
        if (texX >= textureWidth) texX = textureWidth - 1;
        if (texX < 0) texX = 0;
        
        // type of wall
        int tileType = map.getTile(wallTileX, wallTileY);
        SDL_Texture* currentWallTexture = nullptr;
        
        if (wallTextures.find(tileType) != wallTextures.end()) {
            currentWallTexture = wallTextures[tileType];
        } else if (!wallTextures.empty()) {
            currentWallTexture = wallTextures.begin()->second;
        }

        int yStart = (height / 2) - (wallHeight / 2);
        int yEnd = (height / 2) + (wallHeight / 2);

        if (currentWallTexture) {
            SDL_Rect srcRect;
            srcRect.x = texX;
            srcRect.y = 0;
            srcRect.w = 1;
            srcRect.h = textureHeight;

            SDL_Rect destRect;
            destRect.x = i;
            destRect.y = yStart;
            destRect.w = 1;
            destRect.h = wallHeight;

            SDL_RenderCopy(sdlRenderer, currentWallTexture, &srcRect, &destRect);
        } else {
            int colorVal = 255 - (int)(correctedDistance * 10);
            if (colorVal < 0) colorVal = 0;
            drawVerticalLine(i, yStart, yEnd, colorVal, colorVal, colorVal);
        }

        currentAngle += angleStep;      
    }
}

void Renderer::renderGun() {
    if (!gunTexture) return;

    // get size
    int texW, texH;
    SDL_QueryTexture(gunTexture, nullptr, nullptr, &texW, &texH);

    // position of gun
    int gunWidth = texW;
    int gunHeight = texH;
    int gunX = (width - gunWidth) / 2 + 100;
    int gunY = height - gunHeight;

    SDL_Rect destRect = {gunX, gunY, gunWidth, gunHeight};
    SDL_RenderCopy(sdlRenderer, gunTexture, nullptr, &destRect);
}

void Renderer::drawEnemy2D(float x, float y, int tileSize)
{
    int size = 12;

    int screenX = static_cast<int>(x * tileSize) - size / 2;
    int screenY = static_cast<int>(y * tileSize) - size / 2;

    SDL_Rect rect = { screenX, screenY, size, size };

    SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 255);
    SDL_RenderFillRect(sdlRenderer, &rect);
}