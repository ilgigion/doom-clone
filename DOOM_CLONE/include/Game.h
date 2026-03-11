#ifndef GAME_H
#define GAME_H

#include "Renderer.h"
#include "Player.h"
#include "Map.h"

class Game {
private:
    Renderer renderer;
    Player* player;
    Map* map;
    bool isRunning;

public:
    Game();
    ~Game();

    void init();
    void run();
    void update();
    void render();
};

#endif