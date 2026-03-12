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
        radius = 0.25f;

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
            
            // collision regards with dirsction of movement
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

