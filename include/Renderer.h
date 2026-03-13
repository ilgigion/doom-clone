#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include "Player.h"
#include "Enemy.h"
#include <map>
#include <string>
#include <unordered_map>
#include <optional>

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

    //*****NEW TEXTURES FOR SHOOTING GUN AND DEAD ENEMY****
    SDL_Texture* gunFireTexture;//firing animation
    SDL_Texture* deadEnemyTexture;//dead enemy sprite
    void drawDigit(int x, int y, int digit, int colorR, int colorG, int colorB);
    void drawNumber(int x, int y, int number, int colorR, int colorG, int colorB);
public:
    Renderer(int w, int h, const char* title);
    ~Renderer();

    void clear();
    void present();
    bool isRunning() const;

    //changed to optional
    std::optional<bool> loadWallTexture(int id, const std::string& path);
    std::optional<bool> loadFloorTexture(const std::string& path);
    std::optional<bool> loadCeilingTexture(const std::string& path);
    std::optional<bool> loadGunTexture(const std::string& path);
    std::optional<bool> loadEnemyTexture(EnemyType type, const std::string& path);


    void drawEnemySprite(const Enemy& enemy, const Player& player);
    void drawDeadEnemySprite(const Enemy& enemy, const Player& player);//added for the player to see if he killed enemy

    //***ADDED NEW TEXTURES FOR THE SHOOTING AND DEAD
    bool loadGunFireTexture(const std::string& path);
    bool loadDeadEnemyTexture(const std::string& path);

    //****DRAW ENEMY HP FOR PLAYER TO UNDERSTAND HOW MUCH DAMAGE DID HE MAKE*****
    void drawEnemyHPBar(float worldX, float worldY, int currentHP, int maxHP,
                        const Player& player, SDL_Color barColor = {255, 50, 50, 255});

    void render3D(const Player& player, const Map& map, float deltaTime);
    void renderGun(const Player& player); //added the parameter to make animation
    void SDL_RenderDrawCircle(SDL_Renderer * sdl_renderer, int cx, int cy, int i);

    void renderHUD(const Player& player); //interface as in DOOM
    void drawVerticalLine(int x, int yStart, int yEnd, int colorR, int colorG, int colorB);
    float calculateBobOffset(const Player& player, float deltaTime);

    const TextureInfo* getEnemyTextureInfo(EnemyType type) const;
    void resetSpriteZBuffer();

    void renderDamageOverlay(float alpha); // get damage effect

    SDL_Renderer* getSDLRenderer();
};

#endif