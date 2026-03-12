#ifndef GAME_H
#define GAME_H

#include "Renderer.h"
#include "Player.h"
#include "Map.h"
#include "Enemy.h"
#include <memory>

class Game {
private:
    Renderer renderer;
    std::unique_ptr<Player> player;
    std::unique_ptr<Map> map;
    std::vector<std::unique_ptr<Enemy>> enemies;
    bool isRunning;

public:
    Game();
    ~Game();

    void init();
    void run();
    void update(float deltaTime);
    void render(float deltaTime);
    void spawnEnemies();
};

#endif