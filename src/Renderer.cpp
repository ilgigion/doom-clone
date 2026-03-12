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

    // bobbing
    bobPhase = 0.0f;
    bobAmplitude = 8.0f;
    bobFrequency = 1.3f;
    
    zBuffer.resize(width, 9999.0f);
    spriteZBuffer.resize(width, 9999.0f); 
}

Renderer::~Renderer() {
    for (auto& pair : wallTextures) {
        SDL_DestroyTexture(pair.second);
    }
    if (floorTexture) SDL_DestroyTexture(floorTexture);
    if (ceilingTexture) SDL_DestroyTexture(ceilingTexture);
    if (gunTexture) SDL_DestroyTexture(gunTexture);

    for (auto& pair : enemyTextures) {
        if (pair.second.texture) {
            SDL_DestroyTexture(pair.second.texture);
        }
    }
    
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
    SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 255, 255, 255));

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
    SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 255, 255, 255));
    if (surface == nullptr) {
        std::cout << "Unable to load ceiling texture: " << path << std::endl;
        return false;
    }
    
    ceilingTexture = SDL_CreateTextureFromSurface(sdlRenderer, surface);
    SDL_FreeSurface(surface);
    return ceilingTexture != nullptr;
}

// if texture has black backgtound
// SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 255, 255, 255));

bool Renderer::loadGunTexture(const std::string& path) {
    SDL_Surface* surface = SDL_LoadBMP(path.c_str());
    SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 255, 255, 255));
    if (surface == nullptr) {
        std::cout << "Unable to load gun texture: " << path << std::endl;
        return false;
    }
    
    gunTexture = SDL_CreateTextureFromSurface(sdlRenderer, surface);
    SDL_FreeSurface(surface);
    return gunTexture != nullptr;
}

bool Renderer::loadEnemyTexture(EnemyType type, const std::string& path) {
    SDL_Surface* surface = SDL_LoadBMP(path.c_str());
    if (surface == nullptr) {
        std::cout << "Unable to load enemy texture: " << path << " Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // set black as backgroud for textures
    SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 255, 255, 255));

    SDL_Texture* texture = SDL_CreateTextureFromSurface(sdlRenderer, surface);
    
    // get original size of texture
    int texW = 0, texH = 0;
    if (SDL_QueryTexture(texture, nullptr, nullptr, &texW, &texH) != 0) {
        std::cout << "Unable to query texture size: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
        return false;
    }

    SDL_FreeSurface(surface);
    
    if (texture == nullptr) {
        std::cout << "Unable to create texture: " << SDL_GetError() << std::endl;
        return false;
    }

    // save texture size
    enemyTextures[type] = TextureInfo(texture, texW, texH);
    
    std::cout << "Loaded enemy texture: " << path << " (" << texW << "x" << texH << ")" << std::endl;
    return true;
}

const TextureInfo* Renderer::getEnemyTextureInfo(EnemyType type) const {
    auto it = enemyTextures.find(type);
    if (it != enemyTextures.end()) {
        return &it->second;
    }
    return nullptr;
}

void Renderer::resetSpriteZBuffer() {
    std::fill(spriteZBuffer.begin(), spriteZBuffer.end(), 9999.0f);
}

void Renderer::drawVerticalLine(int x, int yStart, int yEnd, int r, int g, int b) {
    if (yStart < 0) yStart = 0;
    if (yEnd >= height) yEnd = height - 1;

    SDL_SetRenderDrawColor(sdlRenderer, r, g, b, 255);
    SDL_RenderDrawLine(sdlRenderer, x, yStart, x, yEnd);
}

void Renderer::render3D(const Player& player, const Map& map, float deltaTime) {
    // clean up
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
    SDL_RenderClear(sdlRenderer);

    // calc bobbing     
    if (player.isMoving()) {
        bobPhase += player.getVelocity() * bobFrequency; 
        bobPhase = std::fmod(bobPhase, 2.0f * M_PI);
    } else {
        float decay = std::exp(-3.0f * deltaTime);
        bobPhase *= decay;
        
        if (std::abs(bobPhase) < 0.05f) {
            bobPhase = 0.0f;
        }
    }

    
    // bobbing offset
    float bobOffset = std::sin(bobPhase) * bobAmplitude;

    int halfHeight = height / 2;
    int bobMargin = (int)bobAmplitude + 5; 
    // rendering roof
    if (ceilingTexture) {
        SDL_Rect ceilingRect = {0, (int)bobOffset - 30, width, height / 2 + 30};
        SDL_RenderCopy(sdlRenderer, ceilingTexture, nullptr, &ceilingRect);
    } else {
        SDL_SetRenderDrawColor(sdlRenderer, 50, 50, 80, 255);
        SDL_Rect ceilingRect = {0, (int)bobOffset, width, height / 2};
        SDL_RenderFillRect(sdlRenderer, &ceilingRect);
    }

    // rendering floor
    if (floorTexture) {
        SDL_Rect floorRect = {0, height / 2 + (int)bobOffset, width, height / 2 + 30};
        SDL_RenderCopy(sdlRenderer, floorTexture, nullptr, &floorRect);
    } else {
        SDL_SetRenderDrawColor(sdlRenderer, 100, 100, 100, 255);
        SDL_Rect floorRect = {0, height / 2 + (int)bobOffset, width, height / 2};
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
        zBuffer[i] = correctedDistance;
        
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

        int yStart = (height / 2) - (wallHeight / 2) + (int)bobOffset;
        int yEnd = (height / 2) + (wallHeight / 2) + (int)bobOffset;

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

    float gunBob = std::sin(bobPhase) * (bobAmplitude * 0.8f);
    // get size
    int texW, texH;
    SDL_QueryTexture(gunTexture, nullptr, nullptr, &texW, &texH);

    // position of gun
    int gunWidth = texW;
    int gunHeight = texH;
    int gunX = (width - gunWidth) / 2 + 100;
    int gunY = height - texH - (int)gunBob + 30; 

    SDL_Rect destRect = {gunX, gunY, gunWidth, gunHeight};
    SDL_RenderCopy(sdlRenderer, gunTexture, nullptr, &destRect);

}

void Renderer::drawEnemySprite(const Enemy& enemy, const Player& player) {
    const TextureInfo* texInfo = getEnemyTextureInfo(enemy.getType());
    if (!texInfo || !texInfo->texture) return;

    // calc vector to the enemy
    float dx = enemy.getX() - player.getX();
    float dy = enemy.getY() - player.getY();
    float distance = std::sqrt(dx * dx + dy * dy);
    
    if (distance < 0.3f || distance > 25.0f) return;

    // angle to enemy
    float angleToEnemy = std::atan2(dy, dx);
    float relativeAngle = angleToEnemy - player.getDir();
    while (relativeAngle > M_PI) relativeAngle -= 2.0f * M_PI;
    while (relativeAngle < -M_PI) relativeAngle += 2.0f * M_PI;

    // is enemy in fov
    float fov = player.getFov();
    if (std::abs(relativeAngle) > fov / 2.0f) return;
    
    // size of sprite
    float spriteScale = 0.8f;
    int spriteHeight = static_cast<int>((height / distance) * spriteScale);
    
    // saving original size
    float aspectRatio = static_cast<float>(texInfo->width) / static_cast<float>(texInfo->height);
    int spriteWidth = static_cast<int>(spriteHeight * aspectRatio);

    if (spriteHeight < 20) spriteHeight = 20;
    if (spriteWidth < 20) spriteWidth = 20;
    if (spriteHeight > 500) spriteHeight = 500;

    // calc position
    int screenX = static_cast<int>((relativeAngle + fov / 2.0f) / fov * width);
    
    // bobbing offset
    int bobOffset = static_cast<int>(std::sin(bobPhase) * bobAmplitude);
    
    int spriteX = screenX - spriteWidth / 2;
    int spriteY = height / 2 - spriteHeight / 2 + bobOffset;

    // rendering
    for (int stripe = spriteX; stripe < spriteX + spriteWidth; stripe++) {
        if (stripe < 0 || stripe >= width) continue;
        
        // the wall?
        if (zBuffer[stripe] < distance) {
            continue;
        }
        // the other enemy?
        if (spriteZBuffer[stripe] < distance) {
            continue;
        }

        // coords in original texture
        float texCoord = static_cast<float>(stripe - spriteX) / static_cast<float>(spriteWidth);
        int texX = static_cast<int>(texCoord * texInfo->width);
        
        if (texX < 0) texX = 0;
        if (texX >= texInfo->width) texX = texInfo->width - 1;

        SDL_Rect srcRect = {texX, 0, 1, texInfo->height};
        SDL_Rect dstRect = {stripe, spriteY, 1, spriteHeight};
        
        SDL_RenderCopy(sdlRenderer, texInfo->texture, &srcRect, &dstRect);
        
        // update sprite buffer
        spriteZBuffer[stripe] = distance;
    }
}