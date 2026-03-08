#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.h"
#include "Map.h"
#include <SDL2/SDL.h>

class Player : public Entity {
private:
    float speed;
    float rotSpeed;

public:
    Player(float startX, float startY);
    ~Player();

    // in future: moving player
    void update(float deltaTime, const Map& map); 
    // renderring
    void render(class Renderer& renderer) override;
    // keyboard input
    void handleInput(const uint8_t* keyState);
};

#endif