#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include "Player.h"
#include "Enemy.h"
#include <map>
#include <string>
#include <unordered_map>

struct TextureInfo {
    SDL_Texture* texture;
    int width;
    int height;
    
    TextureInfo() : texture(nullptr), width(0), height(0) {}
    TextureInfo(SDL_Texture* tex, int w, int h) : texture(tex), width(w), height(h) {}
};

class Renderer {
private:
    SDL_Window* window;
    SDL_Renderer* sdlRenderer;
    int width;
    int height;

    std::map<int, SDL_Texture*> wallTextures;
    SDL_Texture* floorTexture;
    SDL_Texture* ceilingTexture;
    SDL_Texture* gunTexture;                  
    std::unordered_map<EnemyType, TextureInfo> enemyTextures;
    
    int textureWidth;
    int textureHeight;

    // bobbing parametrs
    float bobPhase;           
    float bobAmplitude;       
    float bobFrequency;  

    // buffer for containing dist to the walls
    std::vector<float> zBuffer;
    // buffer for containing dist to the enemy
    std::vector<float> spriteZBuffer;
public:
    Renderer(int w, int h, const char* title);
    ~Renderer();

    void clear();
    void present();
    bool isRunning() const;
    
    // instead drawWall
    bool loadWallTexture(int id, const std::string& path);
    bool loadFloorTexture(const std::string& path);
    bool loadCeilingTexture(const std::string& path);
    bool loadGunTexture(const std::string& path);
    bool loadEnemyTexture(const std::string& path);
    void drawEnemySprite(const Enemy& enemy, const Player& player);
    
    void render3D(const Player& player, const Map& map, float deltaTime);
    void renderGun();
    void drawVerticalLine(int x, int yStart, int yEnd, int colorR, int colorG, int colorB);
    float calculateBobOffset(const Player& player, float deltaTime);

    bool loadEnemyTexture(EnemyType type, const std::string& path);
    const TextureInfo* getEnemyTextureInfo(EnemyType type) const;
    void resetSpriteZBuffer();

    SDL_Renderer* getSDLRenderer();
};

#endif