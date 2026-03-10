    #include "Player.h"
    #include "Renderer.h"
    #include <SDL2/SDL.h>
    #include <cmath>
    #include <iostream>

    Player::Player(float startX, float startY) : Entity(startX, startY) {
        //parametrs
        velocity = 0.0f;
        maxSpeed = 0.1f;
        acceleration = 0.005f;
        deceleration = 0.005f;
        rotSpeed = 0.04f;
        
        dir = 0.0f;
        fov = 60.0f * (3.14159f / 180.0f);
        active = true;
        
        // keys status
        moveForward = false;
        moveBackward = false;
        turnLeft = false;
        turnRight = false;
    }

    Player::~Player() {}

    void Player::handleInput(const uint8_t* keyState) {
        moveForward = keyState[SDL_SCANCODE_W];
        moveBackward = keyState[SDL_SCANCODE_S];
        turnLeft = keyState[SDL_SCANCODE_A];
        turnRight = keyState[SDL_SCANCODE_D];
    }


    

    void Player::update(float deltaTime, const Map& map) {
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
            if (velocity < -maxSpeed * 0.5f) velocity = -maxSpeed * 0.5f; // Назад медленнее
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
            
            // collision
            if (!map.isWall((int)newX, (int)y)) {
                x = newX;
            }
            if (!map.isWall((int)x, (int)newY)) {
                y = newY;
            }
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