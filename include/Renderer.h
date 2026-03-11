#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include "Player.h"
#include <map>
#include <string>

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
    SDL_Texture* enemyTexture;
    
    int textureWidth;
    int textureHeight;

    // bobbing parametrs
    float bobPhase;           
    float bobAmplitude;       
    float bobFrequency;  

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
    
    void render3D(const Player& player, const Map& map);
    void renderGun();
    void drawVerticalLine(int x, int yStart, int yEnd, int colorR, int colorG, int colorB);
    float calculateBobOffset(const Player& player, float deltaTime);

    SDL_Renderer* getSDLRenderer();
    
};

#endif