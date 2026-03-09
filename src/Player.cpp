#include "Player.h"
#include "Renderer.h"
#include <SDL2/SDL.h>
#include <cmath>

Player::Player(float startX, float startY) : Entity(startX, startY) {
    speed = 0.08f;
    rotSpeed = 0.04f;
    dir = 0.0f;
    fov = 60.0f * (3.14159f / 180.0f);
    active = true;
}

Player::~Player() {
}

void Player::update(float deltaTime, const Map& map) {
    handleInput(SDL_GetKeyboardState(NULL));

    float newX = x + std::cos(dir) * speed;
    float newY = y + std::sin(dir) * speed;

    // collision
    if (!map.isWall((int)newX, (int)y)) {
        x = newX;
    }
    if (!map.isWall((int)x, (int)newY)) {
        y = newY;
    }
}

void Player::render(Renderer& renderer) {
    // empty yet
}

// controll
void Player::handleInput(const uint8_t* keyState) {
    if (keyState[SDL_SCANCODE_LEFT]) {
        dir -= rotSpeed;
    }
    if (keyState[SDL_SCANCODE_RIGHT]) {
        dir += rotSpeed;
    }
    if (keyState[SDL_SCANCODE_W]) {
        x += std::cos(dir) * speed;
    }
    if (keyState[SDL_SCANCODE_S]) {
        x -= std::cos(dir) * speed;
    }
}

float Player::getFov() const {
    return fov;
}

float Player::getDir() const {
    return dir;
}