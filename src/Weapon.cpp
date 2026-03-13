#include "Weapon.h"
#include "Projectile.h"
#include "Map.h"
#include <cmath>
#include <cstdlib>

Weapon::Weapon() 
    : pelletCount(8),
      spreadAngle(0.25f),
      maxEffectiveDist(4.0f),
      minDamageDist(1.0f),
      baseDamage(35),
      gunTexture(nullptr),
      gunFireTexture(nullptr) {}

Weapon::~Weapon() {
}

void Weapon::setTextures(SDL_Texture* normal, SDL_Texture* fire) {
    gunTexture = normal;
    gunFireTexture = fire;
}

//raycast helper for wall collision
static bool raycastHit(float startX, float startY, float angle,
                      float maxDist, const Map& map) {
    float dx = std::cos(angle), dy = std::sin(angle);
    float step = 0.05f, dist = 0.0f;
    while (dist < maxDist) {
        float tx = startX + dx * dist;
        float ty = startY + dy * dist;
        if (map.isWall(static_cast<int>(tx), static_cast<int>(ty))) {
            return true;
        }
        dist += step;
    }
    return false;
}

std::vector<std::unique_ptr<Projectile>> Weapon::shoot(
    float playerX, float playerY, float playerDir,
    const Map& map) {

    std::vector<std::unique_ptr<Projectile>> newProjectiles;


    //create pellets
    for (int i = 0; i < pelletCount; ++i) {
        //random spread within cone
        float spread = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * spreadAngle;
        float pelletAngle = playerDir + spread;

        //check if pellet hits a wall immediately
        if (raycastHit(playerX, playerY, pelletAngle, 0.3f, map)) {
            continue;  // Pellet hits wall too close, skip
        }

        //calculate damage based on future distance
        int damage = baseDamage;

        //create projectile
        auto pellet = std::make_unique<Projectile>(
            playerX, playerY, pelletAngle,
            damage, maxEffectiveDist, minDamageDist
        );
        newProjectiles.push_back(std::move(pellet));
    }

    return newProjectiles;
}

SDL_Texture* Weapon::getCurrentGunTexture() const {
    return gunFireTexture ? gunFireTexture : gunTexture;
}