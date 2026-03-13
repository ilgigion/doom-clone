#include "Player.h"
#include "Renderer.h"
#include <SDL2/SDL.h>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <Projectile.h>
Player::Player(float startX, float startY) : Entity(startX, startY), hp(MAX_HP), killCount(0), isShooting(false),
      shootCooldown(0.0f), shootAnimTimer(0.0f) {
    //parametrs
    velocity = 0.0f;
    maxSpeed = 0.04f;
    acceleration = 0.002f;
    deceleration = 0.002f;
    rotSpeed = 0.018f;
    radius = 0.25f;

    dir = 0.0f;
    fov = 60.0f * (3.14159f / 180.0f);
    active = true;

    // keys status
    moveForward = false;
    moveBackward = false;
    turnLeft = false;
    turnRight = false;

    // damage effect
    damageTimer = 0.0f;
}

Player::~Player() {}

void Player::handleInput(const uint8_t* keyState) {
    moveForward = keyState[SDL_SCANCODE_W];
    moveBackward = keyState[SDL_SCANCODE_S];
    turnLeft = keyState[SDL_SCANCODE_A];
    turnRight = keyState[SDL_SCANCODE_D];
}

//get damage
void Player::takeDamage(int amount) {
    hp = std::max(0, hp - amount); //lower hp but not less than 0
    damageTimer = DAMAGE_FLASH_DURATION;
}


//heal after kill
void Player::heal(int amount) {
    hp = std::min(MAX_HP, hp + amount); //higher hp but not greater than 100
}

//*******SHOOTINGWITH WEAPON CLASS****
void Player::shoot(std::vector<std::unique_ptr<Enemy>>& enemies, const Map& map) {
    //cooldown check
    if (shootCooldown > 0.0f) return;

    //start cooldown
    shootCooldown = SHOOT_DELAY;
    //start animation timer
    shootAnimTimer = SHOOT_DELAY;
    isShooting = true;

    //make shoots through weapon
    auto newProjectiles = shotgun.shoot(x, y, dir, map);
    //add active shots
    for (auto& p : newProjectiles) {
        projectiles.push_back(std::move(p));
    }
}

//******UPDATE OF PROJECTILES+HEAL FOR KILL****
void Player::updateProjectiles(float deltaTime, const Map& map,
                              std::vector<std::unique_ptr<Enemy>>& enemies) {
    //update every shot
    for (auto& proj : projectiles) {
        if (proj && proj->isActive()) {
            proj->update(deltaTime, map);
            //check if hit enemy
            for (auto& enemy : enemies) {
                if (enemy && enemy->isAlive()) {
                    //if dead - heal player
                    int hpBefore = enemy->getHP();
                    proj->checkEnemyHit(*enemy);
                    int hpAfter = enemy->getHP();

                    //if enemy dead
                    if (hpBefore > 0 && hpAfter <= 0) {
                        heal(10);
                        incrementKillCount();
                        std::cout << "Enemy killed! +10 HP (current: " << hp << ")\n";
                    }
                }
            }
        }
    }
    //delete unactive projectiles
    projectiles.erase(
        std::remove_if(projectiles.begin(), projectiles.end(),
            [](const std::unique_ptr<Projectile>& p) {
                return !p || !p->isActive();
            }),
        projectiles.end()
    );
}


void Player::update(float deltaTime, const Map& map) {

    //update timer for shot animation
    if (shootAnimTimer > 0.0f) {
        shootAnimTimer -= deltaTime;
        if (shootAnimTimer <= 0.0f) {
            shootAnimTimer = 0.0f;
            isShooting = false;
        }
    }

    //update shot cooldown
    if (shootCooldown > 0.0f) {
        shootCooldown -= deltaTime;
        if (shootCooldown < 0.0f) shootCooldown = 0.0f;
    }

    // 1. rotation
    if (turnLeft) {
        dir -= rotSpeed;
    }
    if (turnRight) {
        dir += rotSpeed;
    }

    // 2. Velocity control
    if (moveForward) {
        velocity += acceleration;
        if (velocity > maxSpeed) velocity = maxSpeed;
    } 
    else if (moveBackward) {
        velocity -= acceleration;
        if (velocity < -maxSpeed * 0.5f) velocity = -maxSpeed * 0.5f;
    } 
    else {
        // braking if no keys is pressed
        if (velocity > 0) {
            velocity -= deceleration;
            if (velocity < 0) velocity = 0;
        } else if (velocity < 0) {
            velocity += deceleration;
            if (velocity > 0) velocity = 0;
        }
    }

    // 3. moving player in dependence on velocity
    if (velocity != 0) {
        float moveStep = velocity;
        float newX = x + std::cos(dir) * moveStep;
        float newY = y + std::sin(dir) * moveStep;

        // collision regards with direction of movement
        float checkDir = (velocity > 0) ? dir : dir + M_PI;

        float checkX = newX + std::cos(checkDir) * radius;
        float checkY = newY + std::sin(checkDir) * radius;

        // collision by x
        if (!map.isWall((int)checkX, (int)y)) {
            x = newX;
        }
        // collision by y
        if (!map.isWall((int)x, (int)checkY)) {
            y = newY;
        }
    }

    // damage timer update
    if (damageTimer > 0.0f) {
        damageTimer -= deltaTime;
        if (damageTimer < 0.0f) damageTimer = 0.0f;
    }

}

void Player::render(Renderer& renderer) {
    // empty yet
}

float Player::getFov() const {
    return fov;
}

float Player::getDir() const {
    return dir;
}

bool Player::isMoving() const {
    return std::abs(velocity) > 0.001f;
}

float Player::getVelocity() const {
    return velocity;
}

//getters for hp and condition
int Player::getHP() const { return hp; }
bool Player::isAlive() const { return hp > 0; }
bool Player::isShootingNow() const { return isShooting; }

Weapon& Player::getWeapon() {
    return shotgun;
}

int Player::getKillCount() const {
    return killCount;
}

void Player::incrementKillCount() {
    killCount++;
}

float Player::getDamageTimer() const {
    return damageTimer;
}