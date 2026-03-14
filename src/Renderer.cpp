#include "Renderer.h"
#include "Player.h"
#include "Map.h"
#include "Enemy.h"
#include <SDL2/SDL.h>
#include <cmath>
#include <iostream>
#include  <Game.h>
#include <optional>

//made to draw numbers on the screen for kill count without libs
void Renderer::drawDigit(int x, int y, int digit, int r, int g, int b) {
    // Pattern for digits 0-9 (3 width, 5 height)
    // 1 = pixel on, 0 = pixel off
    const int patterns[10][5] = {
        {0b110, 0b101, 0b101, 0b101, 0b110}, // 0
        {0b010, 0b110, 0b010, 0b010, 0b111}, // 1
        {0b110, 0b001, 0b110, 0b100, 0b111}, // 2
        {0b110, 0b001, 0b110, 0b001, 0b110}, // 3
        {0b101, 0b101, 0b111, 0b001, 0b001}, // 4
        {0b111, 0b100, 0b110, 0b001, 0b110}, // 5
        {0b110, 0b100, 0b110, 0b101, 0b110}, // 6
        {0b111, 0b001, 0b010, 0b100, 0b100}, // 7
        {0b110, 0b101, 0b110, 0b101, 0b110}, // 8
        {0b110, 0b101, 0b111, 0b001, 0b110}  // 9
    };

    if (digit < 0 || digit > 9) return;

    SDL_SetRenderDrawColor(sdlRenderer, r, g, b, 255);

    //scale factor (make pixels bigger)
    int scale = 4;

    for (int row = 0; row < 5; row++) {
        int pattern = patterns[digit][row];
        for (int col = 0; col < 3; col++) {
            if ((pattern >> (2 - col)) & 1) {
                SDL_Rect pixel = {x + col * scale, y + row * scale, scale, scale};
                SDL_RenderFillRect(sdlRenderer, &pixel);
            }
        }
    }
}

//COSTIL to draw a full number
void Renderer::drawNumber(int x, int y, int number, int r, int g, int b) {
    if (number == 0) {
        drawDigit(x, y, 0, r, g, b);
        return;
    }
    //convert number to string to iterate digits
    std::string numStr = std::to_string(number);
    int digitWidth = 3 * 4; //3 columns * scale 4
    int gap = 4; //space between digits

    //center the number roughly or draw from left
    //let's draw from left to right
    for (char c : numStr) {
        int digit = c - '0';
        drawDigit(x, y, digit, r, g, b);
        x += digitWidth + gap;
    }
}

Renderer::Renderer(int w, int h, const char* title) {
    width = w;
    height = h;
    floorTexture = nullptr;
    ceilingTexture = nullptr;
    gunTexture = nullptr;
    //firing animation and dead enemy texture
    gunFireTexture = nullptr;
    deadEnemyTexture = nullptr;

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

    //clean up new textures
    if (gunFireTexture) SDL_DestroyTexture(gunFireTexture);
    if (deadEnemyTexture) SDL_DestroyTexture(deadEnemyTexture);

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

std::optional<bool> Renderer::loadWallTexture(int id, const std::string& path) {
    SDL_Surface* surface = SDL_LoadBMP(path.c_str());
    if (surface == nullptr) {
        throw ResourceLoadException("Wall texture: " + path + " - " + SDL_GetError());
    }

    SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 255, 255, 255));
    SDL_Texture* texture = SDL_CreateTextureFromSurface(sdlRenderer, surface);
    SDL_FreeSurface(surface);

    if (texture == nullptr) {
        throw ResourceLoadException("Failed to create wall texture: " + path);
    }

    wallTextures[id] = texture;

    if (wallTextures.size() == 1) {
        SDL_QueryTexture(texture, nullptr, nullptr, &textureWidth, &textureHeight);
    }
    return true;
}

std::optional<bool> Renderer::loadFloorTexture(const std::string &path) {
    SDL_Surface* surface = SDL_LoadBMP(path.c_str());
    if (surface == nullptr) {
        std::cout << "Unable to load floor texture: " << path << std::endl;
        return false;
    }

    floorTexture = SDL_CreateTextureFromSurface(sdlRenderer, surface);
    SDL_FreeSurface(surface);
    return floorTexture != nullptr;
}

std::optional<bool> Renderer::loadCeilingTexture(const std::string &path) {
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

std::optional<bool> Renderer::loadGunTexture(const std::string &path) {
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

//******LOAD FIRING ANIMATION TEXTURE***
bool Renderer::loadGunFireTexture(const std::string& path) {
    SDL_Surface* surface = SDL_LoadBMP(path.c_str());
    if (surface == nullptr) {
        std::cout << "Unable to load gun_fire texture: " << path << std::endl;
        return false;
    }
    SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 255, 255, 255));
    gunFireTexture = SDL_CreateTextureFromSurface(sdlRenderer, surface);
    SDL_FreeSurface(surface);
    return gunFireTexture != nullptr;
}

//******LOAD DEAD ENEMY TEXTURE*****
bool Renderer::loadDeadEnemyTexture(const std::string& path) {
    SDL_Surface* surface = SDL_LoadBMP(path.c_str());
    if (surface == nullptr) {
        std::cout << "Unable to load dead_enemy texture: " << path << std::endl;
        return false;
    }
    SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 255, 255, 255));
    deadEnemyTexture = SDL_CreateTextureFromSurface(sdlRenderer, surface);
    SDL_FreeSurface(surface);
    return deadEnemyTexture != nullptr;
}

std::optional<bool> Renderer::loadEnemyTexture(EnemyType type, const std::string &path) {
    SDL_Surface* surface = SDL_LoadBMP(path.c_str());
    if (surface == nullptr) {
        std::cout << "Unable to load enemy texture: " << path << " Error: " << SDL_GetError() << std::endl;
        return false;
    }
    SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 255, 255, 255));

    SDL_Texture* texture = SDL_CreateTextureFromSurface(sdlRenderer, surface);
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
        float stepSize = 0.0075f;

        float testX = mapX;
        float testY = mapY;
        int wallTileX = 0, wallTileY = 0;
        // the side of wall
        bool hitVerticalSide = false;

        while (!hitWall && distanceToWall < 30.0f) {
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

void Renderer::renderGun(const Player& player) {
    //chosing texture based on shooting or not
    SDL_Texture* currentGunTex = player.isShootingNow() && gunFireTexture
        ? gunFireTexture
        : gunTexture;

    if (!currentGunTex) return;

    float gunBob = std::sin(bobPhase) * (bobAmplitude * 0.8f);
    // get size
    int texW, texH;
    SDL_QueryTexture(currentGunTex, nullptr, nullptr, &texW, &texH);

    // position of gun
    int gunWidth = texW;
    int gunHeight = texH;
    int gunX = (width - gunWidth) / 2 + 100;
    int gunY = height - texH - (int)gunBob + 30;

    SDL_Rect destRect = {gunX, gunY, gunWidth, gunHeight};
    SDL_RenderCopy(sdlRenderer, currentGunTex, nullptr, &destRect);
}

void Renderer::drawEnemySprite(const Enemy& enemy, const Player& player) {
    //only render alive enemies
    if (!enemy.isAlive()) return;

    const TextureInfo* texInfo = getEnemyTextureInfo(enemy.getType());
    if (!texInfo || !texInfo->texture) return;

    //calc vector to the enemy
    float dx = enemy.getX() - player.getX();
    float dy = enemy.getY() - player.getY();
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance < 0.3f || distance > 25.0f) return;

    //angle to enemy
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

//*******RENDER OF DEAD ENEMY*****
void Renderer::drawDeadEnemySprite(const Enemy& enemy, const Player& player) {
    if (enemy.isAlive()) return;
    if (enemy.getDeathTimer() <= 0.0f) return;
    if (!deadEnemyTexture) return;

    float dx = enemy.getX() - player.getX();
    float dy = enemy.getY() - player.getY();
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance < 0.1f || distance > 30.0f) return;

    float angleToEnemy = std::atan2(dy, dx);
    float relativeAngle = angleToEnemy - player.getDir();
    while (relativeAngle > M_PI) relativeAngle -= 2.0f * M_PI;
    while (relativeAngle < -M_PI) relativeAngle += 2.0f * M_PI;

    float fov = player.getFov();
    if (std::abs(relativeAngle) > fov / 2.0f + 0.3f) return;

    float screenX = (relativeAngle + fov / 2.0f) / fov * width;
    int spriteSize = static_cast<int>(500 / distance);
    if (spriteSize < 12) spriteSize = 12;
    if (spriteSize > 200) spriteSize = 200;

    //texture dimensions getr
    int texW = 0, texH = 0;
    SDL_QueryTexture(deadEnemyTexture, nullptr, nullptr, &texW, &texH);

    //dead enemy 2 times lower that original
    int spriteWidth = spriteSize;
    int spriteHeight = spriteSize / 2;

    //bobbing offset
    int bobOffset = static_cast<int>(std::sin(bobPhase) * bobAmplitude);

    int spriteX = static_cast<int>(screenX - spriteWidth / 2);
    int spriteY = height / 2 - spriteHeight / 2 + bobOffset;

    for (int stripe = spriteX; stripe < spriteX + spriteWidth; stripe++) {
        if (stripe < 0 || stripe >= width) continue;
        //wall occlusion
        if (zBuffer[stripe] < distance) continue;
        //other sprite occlusion
        if (spriteZBuffer[stripe] < distance) continue;

        //texture coordinate for this stripe
        float texCoord = static_cast<float>(stripe - spriteX) / static_cast<float>(spriteWidth);
        int texX = static_cast<int>(texCoord * texW);
        if (texX < 0) texX = 0;
        if (texX >= texW) texX = texW - 1;

        SDL_Rect srcRect = {texX, 0, 1, texH};
        SDL_Rect dstRect = {stripe, spriteY, 1, spriteHeight};

        SDL_RenderCopy(sdlRenderer, deadEnemyTexture, &srcRect, &dstRect);

        //update sprite for this column
        spriteZBuffer[stripe] = distance;
    }
}

//*****ENEMY HP BAR*****
void Renderer::drawEnemyHPBar(float worldX, float worldY, int currentHP, int maxHP,
                              const Player& player, SDL_Color /*barColor*/) {
    float dx = worldX - player.getX();
    float dy = worldY - player.getY();
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance < 0.5f || distance > 15.0f) return;

    float angleToEnemy = std::atan2(dy, dx);
    float relativeAngle = angleToEnemy - player.getDir();

    while (relativeAngle > M_PI) relativeAngle -= 2.0f * M_PI;
    while (relativeAngle < -M_PI) relativeAngle += 2.0f * M_PI;

    float fov = player.getFov();
    if (std::abs(relativeAngle) > fov / 2.0f + 0.2f) return;

    float screenX = (relativeAngle + fov / 2.0f) / fov * width;
    int spriteSize = static_cast<int>(600 / distance);

    int barWidth = std::max(30, static_cast<int>(spriteSize * 0.8f));
    int barHeight = 6;
    int barX = static_cast<int>(screenX - barWidth / 2);

    //bobbing offset
    int bobOffset = static_cast<int>(std::sin(bobPhase) * bobAmplitude);
    int barY = height / 2 - spriteSize / 2 - barHeight - 10 + bobOffset;  // + bobOffset

    SDL_SetRenderDrawColor(sdlRenderer, 30, 30, 30, 200);
    SDL_Rect bgRect = {barX - 1, barY - 1, barWidth + 2, barHeight + 2};
    SDL_RenderFillRect(sdlRenderer, &bgRect);

    float hpRatio = static_cast<float>(currentHP) / maxHP;
    if (hpRatio < 0.0f) hpRatio = 0.0f;
    if (hpRatio > 1.0f) hpRatio = 1.0f;
    Uint8 r = 255;
    Uint8 g = static_cast<Uint8>(200 * hpRatio);
    Uint8 b = 0;

    int fillWidth = static_cast<int>(barWidth * hpRatio);

    SDL_SetRenderDrawColor(sdlRenderer, r, g, b, 255);
    SDL_Rect fillRect = {barX, barY, fillWidth, barHeight};
    SDL_RenderFillRect(sdlRenderer, &fillRect);

    SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(sdlRenderer, &bgRect);
}

//******PLAYER HEALTH****
void Renderer::renderHUD(const Player& player) {
    int hp = player.getHP();
    int maxHp = Player::MAX_HP;
    float ratio = static_cast<float>(hp) / maxHp;
    if (ratio < 0.0f) ratio = 0.0f;
    if (ratio > 1.0f) ratio = 1.0f;

    //red border
    SDL_SetRenderDrawColor(sdlRenderer, 200, 50, 50, 255);
    SDL_Rect healthFrame = {10, 10, 120, 25};
    SDL_RenderDrawRect(sdlRenderer, &healthFrame);

    //greeen hp
    SDL_SetRenderDrawColor(sdlRenderer, 0, 200, 0, 255);
    SDL_Rect healthFill = {12, 12, static_cast<int>(116 * ratio), 21};
    SDL_RenderFillRect(sdlRenderer, &healthFill);

    //kill counter
    int killCount = player.getKillCount();

    //location
    int labelX = 145;
    int labelY = 14; //center vertically in the 25px height bar


    //draw the actual kill count numbers next to the icon
    drawNumber(labelX + 12, labelY, killCount, 255, 0, 0);

    //draw the scope
    if (player.isShootingNow()) {
        int cx = width / 2;
        int cy = height / 2;
        SDL_SetRenderDrawColor(sdlRenderer, 255, 200, 0, 255);
        for (int i = 0; i < 16; i++) {
            float angle = i * M_PI / 8;
            int dist = 20 + rand() % 5;
            int px = cx + static_cast<int>(cos(angle) * dist);
            int py = cy + static_cast<int>(sin(angle) * dist);
            SDL_Rect spike = {px - 2, py - 2, 4, 4};
            SDL_RenderFillRect(sdlRenderer, &spike);
        }
    }

}

float Renderer::calculateBobOffset(const Player& player, float deltaTime) {
    return std::sin(bobPhase) * bobAmplitude;
}

void Renderer::renderDamageOverlay(float alpha) {
    if (!sdlRenderer) return;
    alpha = 0.2;
    Uint8 alphaValue = static_cast<Uint8>(alpha * 255.0f);
    // switch on blendering colors
    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);
    // save current render color
    Uint8 r, g, b, a;
    SDL_GetRenderDrawColor(sdlRenderer, &r, &g, &b, &a);
    // put red with alpha
    SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, alphaValue);
    // draw red
    SDL_Rect overlay = {0, 0, width, height};
    SDL_RenderFillRect(sdlRenderer, &overlay);

    // recover original color
    SDL_SetRenderDrawColor(sdlRenderer, r, g, b, a);
    // switch off blendering colors
    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_NONE);
}
//check commit




//nature is good