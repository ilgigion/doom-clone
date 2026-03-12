#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <SDL2/SDL.h>

class Enemy;
class Map;

class Projectile {
private:
    float x, y; //curent pos
    float direction; //where it is moving to check if missed or not
    float speed;//movespeed
    int damage;//base damage
    float maxDist;//max distance when you make damage
    float minDamageDist;//distance when you make max damage
    float traveledDist;//distance that was travelled
    bool active;//can our projectile still kill
    
public:
    Projectile(float startX, float startY, float dir, 
               int dmg, float maxD, float minD);
    
    //update pos and check colisions
    void update(float deltaTime, const Map& map);
    
    //check if it hit enemy
    bool checkEnemyHit(Enemy& enemy);
    
    //a few getters to check everything
    bool isActive() const;
    float getX() const;
    float getY() const;
    
    //calculate damage based on dist
    int getActualDamage() const;
};

#endif