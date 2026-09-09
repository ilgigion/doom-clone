#include "Renderer.h"
#include "Player.h"
#include "Map.h"
#include "Enemy.h"
#include <SDL2/SDL.h>
#include <cmath>
#include <iostream>
#include  <Game.h>
#include <optional>
#include "SDLWrappers.h"
#include <utility>
#include "FrameProjection.h"
#include <cstring>

namespace {
SDLTexturePtr loadTexture(SDL_Renderer* renderer, const std::string& path, bool transparent) {
    SDLSurfacePtr surface(SDL_LoadBMP(path.c_str()));
    if (!surface) {
        std::cerr << "Unable to load texture: " << path << " - " << SDL_GetError() << std::endl;
        return {};
    }
    if (transparent && SDL_SetColorKey(surface.get(), SDL_TRUE,
            SDL_MapRGB(surface->format, 255, 255, 255)) != 0) {
        std::cerr << "Unable to set texture color key: " << path << " - " << SDL_GetError() << std::endl;
        return {};
    }
    SDLTexturePtr texture(SDL_CreateTextureFromSurface(renderer, surface.get()));
    if (!texture) {
        std::cerr << "Unable to create texture: " << path << " - " << SDL_GetError() << std::endl;
    }
    return texture;
}

bool replaceTexture(SDL_Renderer* renderer, SDL_Texture*& target,
                    const std::string& path, bool transparent) {
    auto replacement = loadTexture(renderer, path, transparent);
    if (!replacement) return false;
    SDLTexturePtr previous(std::exchange(target, replacement.release()));
    return true;
}
}

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

Renderer::Renderer(int w, int h, const char* title)
    : window(nullptr), sdlRenderer(nullptr), width(w), height(h),
      floorTexture(nullptr), ceilingTexture(nullptr), gunTexture(nullptr),
      bobPhase(0.0f),
      bobAmplitude(8.0f), bobFrequency(0.65f),
      gunFireTexture(nullptr), deadEnemyTexture(nullptr) {
    if (width <= 0 || height <= 0) {
        throw InitializationException("Renderer dimensions must be positive");
    }
    zBuffer.resize(width, 9999.0f);
    spriteZBuffer.resize(width, 9999.0f);
    floorRays.resize(width);
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
        throw InitializationException(std::string("SDL video: ") + SDL_GetError());
    }
    try {
        SDLWindowPtr newWindow(SDL_CreateWindow(title,
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            width, height, SDL_WINDOW_SHOWN));
        if (!newWindow) {
            throw InitializationException(std::string("Window: ") + SDL_GetError());
        }
        SDLRendererPtr newRenderer(SDL_CreateRenderer(newWindow.get(), -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));
        if (!newRenderer) {
            throw InitializationException(std::string("Renderer: ") + SDL_GetError());
        }
        window = newWindow.release();
        sdlRenderer = newRenderer.release();
    } catch (...) {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        throw;
    }
}

Renderer::~Renderer() {
    for (auto& pair : wallTextures) {
        SDL_DestroyTexture(pair.second.texture);
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
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
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
    auto texture = loadTexture(sdlRenderer, path, true);
    if (!texture) throw ResourceLoadException("Wall texture: " + path);
    int texW = 0, texH = 0;
    if (SDL_QueryTexture(texture.get(), nullptr, nullptr, &texW, &texH) != 0) {
        throw ResourceLoadException("Wall texture dimensions: " + path);
    }
    auto& target = wallTextures[id];
    SDLTexturePtr previous(std::exchange(target.texture, texture.release()));
    target.width = texW;
    target.height = texH;
    return true;
}

std::optional<bool> Renderer::loadFloorTexture(const std::string& path) {
    SDLSurfacePtr source(SDL_LoadBMP(path.c_str()));
    if (!source) {
        std::cerr << "Unable to load floor: " << path << " - " << SDL_GetError() << std::endl;
        return false;
    }
    SDLSurfacePtr converted(SDL_ConvertSurfaceFormat(source.get(), SDL_PIXELFORMAT_ARGB8888, 0));
    if (!converted || converted->w <= 0 || converted->h <= 0) {
        std::cerr << "Unable to convert floor: " << path << " - " << SDL_GetError() << std::endl;
        return false;
    }
    std::vector<Uint32> pixels(static_cast<size_t>(converted->w) * converted->h);
    if (SDL_LockSurface(converted.get()) != 0) {
        std::cerr << "Unable to read floor: " << SDL_GetError() << std::endl;
        return false;
    }
    for (int y = 0; y < converted->h; ++y) {
        const auto* row = static_cast<const Uint8*>(converted->pixels) + static_cast<size_t>(y) * converted->pitch;
        std::memcpy(pixels.data() + static_cast<size_t>(y) * converted->w,
                    row, static_cast<size_t>(converted->w) * sizeof(Uint32));
    }
    SDL_UnlockSurface(converted.get());
    for (auto& pixel : pixels) pixel |= 0xff000000u; // The floor is opaque.

    SDLTexturePtr frame(SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888,
                                          SDL_TEXTUREACCESS_STREAMING, width, height));
    if (!frame || SDL_SetTextureBlendMode(frame.get(), SDL_BLENDMODE_NONE) != 0) {
        std::cerr << "Unable to create floor frame: " << SDL_GetError() << std::endl;
        return false;
    }
    // Commit only after both the CPU texture and streaming frame are ready.
    SDLTexturePtr previous(std::exchange(floorTexture, frame.release()));
    floorPixels.swap(pixels);
    floorWidth = converted->w;
    floorHeight = converted->h;
    return true;
}

std::optional<bool> Renderer::loadCeilingTexture(const std::string& path) {
    return replaceTexture(sdlRenderer, ceilingTexture, path, true);
}

std::optional<bool> Renderer::loadGunTexture(const std::string& path) {
    return replaceTexture(sdlRenderer, gunTexture, path, true);
}

bool Renderer::loadGunFireTexture(const std::string& path) {
    return replaceTexture(sdlRenderer, gunFireTexture, path, true);
}

bool Renderer::loadDeadEnemyTexture(const std::string& path) {
    return replaceTexture(sdlRenderer, deadEnemyTexture, path, true);
}

std::optional<bool> Renderer::loadEnemyTexture(EnemyType type, const std::string& path) {
    auto texture = loadTexture(sdlRenderer, path, true);
    if (!texture) return false;
    int texW = 0, texH = 0;
    if (SDL_QueryTexture(texture.get(), nullptr, nullptr, &texW, &texH) != 0) {
        std::cerr << "Unable to query texture size: " << SDL_GetError() << std::endl;
        return false;
    }
    auto& target = enemyTextures[type];
    SDLTexturePtr previous(std::exchange(target.texture, texture.release()));
    target.width = texW;
    target.height = texH;
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

void Renderer::renderFloor(const Player& player, const Map& map, const FrameProjection& projection) {
    const int firstRow = std::clamp(projection.horizon(), 0, height);
    if (firstRow == height) return;
    const SDL_Rect area{0, firstRow, width, height - firstRow};
    if (!floorTexture) {
        SDL_SetRenderDrawColor(sdlRenderer, 100, 100, 100, 255);
        SDL_RenderFillRect(sdlRenderer, &area);
        return;
    }

    float angle = projection.firstRayAngle();
    for (auto& ray : floorRays) {
        ray = projection.floorRay(angle);
        angle += projection.angleStep();
    }
    void* pixels = nullptr;
    int pitch = 0;
    if (SDL_LockTexture(floorTexture, &area, &pixels, &pitch) != 0) {
        throw ResourceLoadException(std::string("Floor frame lock: ") + SDL_GetError());
    }
    constexpr Uint32 fallback = 0xff646464u;
    constexpr float maxDepth = 30.0f;
    for (int y = firstRow; y < height; ++y) {
        auto* row = reinterpret_cast<Uint32*>(static_cast<Uint8*>(pixels) +
                    static_cast<size_t>(y - firstRow) * pitch);
        std::fill_n(row, width, fallback);
        const auto depth = projection.floorDepth(y + 0.5f);
        if (!depth || *depth > maxDepth) continue;
        for (int x = 0; x < width; ++x) {
            const auto& ray = floorRays[x];
            if (!ray) continue;
            const double worldX = player.getX() + *depth * ray->x;
            const double worldY = player.getY() + *depth * ray->y;
            // Check bounds before converting coordinates to integer texture indices.
            if (!std::isfinite(worldX) || !std::isfinite(worldY) ||
                worldX < 0 || worldY < 0 || worldX >= map.getWidth() || worldY >= map.getHeight()) continue;
            const auto tx = FrameProjection::textureCoordinate(static_cast<float>(worldX), floorWidth);
            const auto ty = FrameProjection::textureCoordinate(static_cast<float>(worldY), floorHeight);
            if (tx && ty) row[x] = floorPixels[static_cast<size_t>(*ty) * floorWidth + *tx];
        }
    }
    SDL_UnlockTexture(floorTexture);
    // Only the initialized region is copied; the ceiling remains untouched.
    if (SDL_RenderCopy(sdlRenderer, floorTexture, &area, &area) != 0) {
        throw ResourceLoadException(std::string("Floor frame copy: ") + SDL_GetError());
    }
}

void Renderer::render3D(const Player& player, const Map& map, float deltaTime) {
    // clean up
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
    SDL_RenderClear(sdlRenderer);

    // calc bobbing
    if (player.isMoving()) {
        bobPhase += player.getVelocity() * bobFrequency * deltaTime;
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
    const FrameProjection projection(width, height, player.getDir(), player.getFov(),
                                     static_cast<int>(bobOffset));
    // rendering roof
    if (ceilingTexture) {
        SDL_Rect ceilingRect = {0, (int)bobOffset - 30, width, height / 2 + 30};
        SDL_RenderCopy(sdlRenderer, ceilingTexture, nullptr, &ceilingRect);
    } else {
        SDL_SetRenderDrawColor(sdlRenderer, 50, 50, 80, 255);
        SDL_Rect ceilingRect = {0, (int)bobOffset, width, height / 2};
        SDL_RenderFillRect(sdlRenderer, &ceilingRect);
    }

    renderFloor(player, map, projection);

    // rays parametrs
    int numRays = projection.width();
    float angleStep = projection.angleStep();
    float currentAngle = projection.firstRayAngle();

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

        float correctedDistance = projection.perpendicularDepth(distanceToWall, currentAngle);
        zBuffer[i] = correctedDistance;

        int wallHeight = static_cast<int>(projection.wallHeight(correctedDistance));

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

        // type of wall
        int tileType = map.getTile(wallTileX, wallTileY);
        const TextureInfo* currentWallTexture = nullptr;

        if (wallTextures.find(tileType) != wallTextures.end()) {
            currentWallTexture = &wallTextures.at(tileType);
        } else if (!wallTextures.empty()) {
            currentWallTexture = &wallTextures.begin()->second;
        }

        int yStart = projection.horizon() - (wallHeight / 2);
        int yEnd = projection.horizon() + (wallHeight / 2);

        if (currentWallTexture) {
            SDL_Rect srcRect;
            srcRect.x = std::clamp(static_cast<int>(wallX * currentWallTexture->width), 0, currentWallTexture->width - 1);
            srcRect.y = 0;
            srcRect.w = 1;
            srcRect.h = currentWallTexture->height;

            SDL_Rect destRect;
            destRect.x = i;
            destRect.y = yStart;
            destRect.w = 1;
            destRect.h = wallHeight;

            SDL_RenderCopy(sdlRenderer, currentWallTexture->texture, &srcRect, &destRect);
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

std::optional<Renderer::SpriteProjection> Renderer::projectSprite(
    float worldX, float worldY, const Player& player, float worldHeight, float aspect, float maxDistance) const {
    const float dx = worldX - player.getX(), dy = worldY - player.getY();
    const float distance = std::hypot(dx, dy);
    if (!std::isfinite(distance) || distance > maxDistance || distance < 0.1f) return std::nullopt;
    const float angle = std::atan2(dy, dx);
    const FrameProjection projection(width, height, player.getDir(), player.getFov(),
        static_cast<int>(std::sin(bobPhase) * bobAmplitude));
    const float depth = projection.perpendicularDepth(distance, angle);
    if (depth < 0.1f) return std::nullopt;
    const float relativeAngle = std::remainder(angle - player.getDir(), 2.0f * static_cast<float>(M_PI));
    const float unitHeight = projection.wallHeight(depth);
    const int spriteHeight = std::max(1, static_cast<int>(unitHeight * worldHeight));
    const int spriteWidth = std::max(1, static_cast<int>(spriteHeight * aspect));
    const int centre = static_cast<int>((relativeAngle + player.getFov() / 2) / player.getFov() * width);
    const int bottom = static_cast<int>(projection.horizon() + unitHeight * FrameProjection::cameraHeight);
    const SDL_Rect rect{centre - spriteWidth / 2, bottom - spriteHeight, spriteWidth, spriteHeight};
    if (rect.x >= width || rect.x + rect.w <= 0 || rect.y >= height || rect.y + rect.h <= 0) return std::nullopt;
    return SpriteProjection{rect, depth};
}

void Renderer::drawSpriteColumns(SDL_Texture* texture, int texWidth, int texHeight,
                                  const SpriteProjection& sprite) {
    const auto& rect = sprite.rect;
    for (int stripe = std::max(0, rect.x); stripe < std::min(width, rect.x + rect.w); ++stripe) {
        if (zBuffer[stripe] < sprite.depth || spriteZBuffer[stripe] < sprite.depth) continue;
        const float u = static_cast<float>(stripe - rect.x) / rect.w;
        const int texX = std::clamp(static_cast<int>(u * texWidth), 0, texWidth - 1);
        const SDL_Rect src{texX, 0, 1, texHeight};
        const SDL_Rect dst{stripe, rect.y, 1, rect.h};
        SDL_RenderCopy(sdlRenderer, texture, &src, &dst);
        spriteZBuffer[stripe] = sprite.depth;
    }
}

void Renderer::drawEnemySprite(const Enemy& enemy, const Player& player) {
    if (!enemy.isAlive()) return;
    const auto* info = getEnemyTextureInfo(enemy.getType());
    if (!info || !info->texture || info->width <= 0 || info->height <= 0) return;
    const auto sprite = projectSprite(enemy.getX(), enemy.getY(), player, 0.8f,
                                      static_cast<float>(info->width) / info->height, 25.0f);
    if (sprite) drawSpriteColumns(info->texture, info->width, info->height, *sprite);
}

void Renderer::drawDeadEnemySprite(const Enemy& enemy, const Player& player) {
    if (enemy.isAlive() || enemy.getDeathTimer() <= 0.0f || !deadEnemyTexture) return;
    int texWidth = 0, texHeight = 0;
    if (SDL_QueryTexture(deadEnemyTexture, nullptr, nullptr, &texWidth, &texHeight) != 0 ||
        texWidth <= 0 || texHeight <= 0) return;
    // A flattened billboard, 0.8 cells wide and 0.2 cells high, resting on the floor.
    const auto sprite = projectSprite(enemy.getX(), enemy.getY(), player, 0.2f, 4.0f, 30.0f);
    if (sprite) drawSpriteColumns(deadEnemyTexture, texWidth, texHeight, *sprite);
}

void Renderer::drawEnemyHPBar(float worldX, float worldY, int currentHP, int maxHP,
                              const Player& player, SDL_Color /*barColor*/) {
    if (maxHP <= 0) return;
    const auto sprite = projectSprite(worldX, worldY, player, 0.8f, 1.0f, 15.0f);
    if (!sprite) return;
    const int barWidth = std::max(12, static_cast<int>(sprite->rect.h * 0.8f));
    const int barHeight = 6;
    const int centre = sprite->rect.x + sprite->rect.w / 2;
    const int barX = centre - barWidth / 2;
    const int barY = sprite->rect.y - barHeight - 10;
    const float hpRatio = std::clamp(static_cast<float>(currentHP) / maxHP, 0.0f, 1.0f);
    const int fillWidth = static_cast<int>(barWidth * hpRatio);
    // Called after every sprite: only columns belonging to this visible enemy
    // may receive its HP bar. A hidden enemy cannot reveal itself through a wall.
    for (int x = std::max(0, barX); x < std::min(width, barX + barWidth); ++x) {
        if (zBuffer[x] < sprite->depth || std::abs(spriteZBuffer[x] - sprite->depth) > 0.0001f) continue;
        SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 255);
        SDL_RenderDrawPoint(sdlRenderer, x, barY - 1);
        SDL_RenderDrawPoint(sdlRenderer, x, barY + barHeight);
        if (x - barX < fillWidth) SDL_SetRenderDrawColor(sdlRenderer, 255, static_cast<Uint8>(200 * hpRatio), 0, 255);
        else SDL_SetRenderDrawColor(sdlRenderer, 30, 30, 30, 255);
        SDL_RenderDrawLine(sdlRenderer, x, barY, x, barY + barHeight - 1);
    }
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
            int dist = 20 + effectRandom() % 5;
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
    if (!std::isfinite(alpha) || alpha <= 0.0f) return;
    alpha = std::clamp(alpha, 0.0f, 1.0f);
    Uint8 alphaValue = static_cast<Uint8>(alpha * 255.0f);
    // switch on blendering colors
    SDL_BlendMode previousBlend;
    SDL_GetRenderDrawBlendMode(sdlRenderer, &previousBlend);
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
    SDL_SetRenderDrawBlendMode(sdlRenderer, previousBlend);
}
//check commit




//nature is good
