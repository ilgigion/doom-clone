#ifndef WEAPON_H
#define WEAPON_H

#include <SDL2/SDL.h>
#include <vector>
#include <memory>

class Projectile;
class Map;

class Weapon {
private:
    //******WEAPON PARAMETERS****
    int pelletCount;//num of pellets in one shot
    float spreadAngle;//angle of damage
    float maxEffectiveDist;//max damage dist
    float minDamageDist;//full damage dist
    int baseDamage;//damage per pellet at small range

    SDL_Texture* gunTexture;//normal gun sprite
    SDL_Texture* gunFireTexture;//firing animation sprite

public:
    Weapon();
    ~Weapon();

    //set textures
    void setTextures(SDL_Texture* normal, SDL_Texture* fire);

    //create projectiles
    std::vector<std::unique_ptr<Projectile>> shoot(
        float playerX, float playerY, float playerDir,
        const Map& map
    );

    //getter for rendering
    SDL_Texture* getCurrentGunTexture() const;
    
    //constant for infinite ammo
    static constexpr int MAX_AMMO = -1;//-1 = infinite
};

#endif