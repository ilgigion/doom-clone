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
    maxSpeed = 4.8f;
    acceleration = 28.8f;
    deceleration = 28.8f;
    rotSpeed = 2.16f;
    radius = 0.3f;

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
    if (!std::isfinite(deltaTime) || deltaTime <= 0.0f) return;

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

    if (turnLeft) dir -= rotSpeed * deltaTime;
    if (turnRight) dir += rotSpeed * deltaTime;
    dir = std::remainder(dir, 2.0f * static_cast<float>(M_PI));

    // Integrate acceleration up to the target speed, then constant velocity.
    const float target = moveForward ? maxSpeed : (moveBackward ? -maxSpeed * 0.5f : 0.0f);
    const float rate = (moveForward || moveBackward) ? acceleration : deceleration;
    const float sign = target >= velocity ? 1.0f : -1.0f;
    const float rampTime = std::min(deltaTime, std::abs(target - velocity) / rate);
    const float distance = velocity * rampTime + 0.5f * sign * rate * rampTime * rampTime
                         + target * (deltaTime - rampTime);
    velocity = rampTime < deltaTime ? target : velocity + sign * rate * rampTime;

    // Small collision steps prevent crossing a wall during a long update.
    const int steps = std::max(1, static_cast<int>(std::ceil(std::abs(distance) / (radius * 0.5f))));
    const float stepX = std::cos(dir) * distance / steps;
    const float stepY = std::sin(dir) * distance / steps;
    for (int step = 0; step < steps; ++step) {
        if (map.canOccupy(x + stepX, y, radius)) x += stepX;
        if (map.canOccupy(x, y + stepY, radius)) y += stepY;
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