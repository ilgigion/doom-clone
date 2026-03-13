#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.h"
#include "Map.h"
#include <SDL2/SDL.h>
#include "Weapon.h"
#include "Enemy.h"

class Player : public Entity {
private:
    float velocity;
    float maxSpeed;
    float acceleration;
    float deceleration;
    float rotSpeed;
    float fov;
    float radius;
    bool moveForward;
    bool moveBackward;
    bool turnLeft;
    bool turnRight;
    //*****GUN****
    Weapon shotgun;  //gun
    std::vector<std::unique_ptr<Projectile>> projectiles;  //active shots

    //******HP AND SHOOT FLAG****
    int hp; //player hp
    bool isShooting; //check if is shooting
    float shootCooldown; //cd of shot
    float shootAnimTimer;//timer of animation of the shot
    static constexpr float SHOOT_DELAY = 0.4f; //constant for shot delay not to write the num everytime

    //*****KILL COUNT FOR BETTER EXP FOR PLAYER***
    int killCount;
    
    // get damage effect
    float damageTimer;
public:
    static constexpr int MAX_HP = 100; //const for max player hp
    static constexpr float DAMAGE_FLASH_DURATION = 0.5f;
    Player(float startX, float startY);
    ~Player();

    // in future: moving player
    void update(float deltaTime, const Map& map); 
    // renderring
    void render(class Renderer& renderer) override;
    // keyboard input
    void handleInput(const uint8_t* keyState);

    //methods for shooting and damage
    void shoot(std::vector<std::unique_ptr<Enemy>>& enemies, const Map& map); //shoot
    void takeDamage(int amount); //take damage from enemies
    void updateProjectiles(float deltaTime, const Map& map, std::vector<std::unique_ptr<Enemy>>& enemies); //update the shot obstacles
    void heal(int amount); //heal after kill

    // field of view
    float getFov() const;
    float getDir() const;
    bool isMoving() const;
    float getVelocity() const;

    //*****GETTTERS FOR UI+WEAPON
    int getHP() const; //get player hp
    bool isAlive() const; //check if player is alive
    bool isShootingNow() const; //check the condition is shooting or not
    Weapon& getWeapon();


    //*******FUNCTIONS FOR KILL COUNT*****
    int getKillCount() const; //get kill count
    void incrementKillCount(); //increment on enemy death

    // timer for damage effect
    float getDamageTimer() const;
};

#endif