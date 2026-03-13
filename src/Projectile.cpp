#include "Projectile.h"
#include "Enemy.h"
#include "Map.h"
#include <cmath>

Projectile::Projectile(float startX, float startY, float dir, 
                       int dmg, float maxD, float minD)
    : x(startX), y(startY), direction(dir), speed(15.0f),
      damage(dmg), maxDist(maxD), minDamageDist(minD),
      traveledDist(0.0f), active(true) {}

void Projectile::update(float deltaTime, const Map& map) {
    if (!active) return;
    
    //move projectile
    float dx = std::cos(direction) * speed * deltaTime;
    float dy = std::sin(direction) * speed * deltaTime;
    x += dx;
    y += dy;
    traveledDist += std::sqrt(dx*dx + dy*dy);
    
    //check wall collision
    if (map.isWall(static_cast<int>(x), static_cast<int>(y))) {
        active = false;  // Hit wall, deactivate
        return;
    }
    
    //check max distance
    if (traveledDist >= maxDist * 1.5f) {
        active = false;  // Too far, deactivate
    }
}

bool Projectile::checkEnemyHit(Enemy& enemy) {
    if (!active || !enemy.isAlive()) return false;
    
    //distance to enemy
    float dx = enemy.getX() - x;
    float dy = enemy.getY() - y;
    float dist = std::sqrt(dx*dx + dy*dy);
    
    //check if within enemy radius
    if (dist <= enemy.radius + 0.15f) {
        //check angle alignment to hit enemy
        float angleToEnemy = std::atan2(dy, dx);
        float angleDiff = std::abs(angleToEnemy - direction);
        //normalize angle
        while (angleDiff > M_PI) angleDiff -= 2*M_PI;
        while (angleDiff < -M_PI) angleDiff += 2*M_PI;
        
        if (std::abs(angleDiff) < 0.5f) {  // ~28 degree help
            int actualDmg = getActualDamage();
            if (actualDmg > 0) {
                enemy.takeDamage(actualDmg);
            }
            active = false;//pellet hit enemy, deactivate
            return true;
        }
    }
    return false;
}

bool Projectile::isActive() const { return active; }
float Projectile::getX() const { return x; }
float Projectile::getY() const { return y; }

int Projectile::getActualDamage() const {
    if (traveledDist <= minDamageDist) {
        return damage;//full damage at close range
    } else if (traveledDist <= maxDist) {
        //linear falloff from 100% to 30% damage
        float t = (traveledDist - minDamageDist) / (maxDist - minDamageDist);
        return static_cast<int>(damage * (1.0f - t * 0.7f));
    }
    return 0;//no damage beyond maxDist
}

//for tests
void Projectile::setTraveledDist(float dist) {
    traveledDist = dist;
}