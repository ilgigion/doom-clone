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

    //****TIMERS FOR RESPAWN AND SPAWN OF ENEMIES***
    float enemySpawnTimer;        // timer fornext spawn
    float enemyRespawnCheckTimer; // timer of respawn check
    static constexpr float SPAWN_INTERVAL = 30.0f;    // const for spawning new enemies
    static constexpr float RESPAWN_CHECK_INTERVAL = 1.0f; // check respawn every sec
    static constexpr int MAX_ENEMIES = 10;            // max amount of enemies

public:
    Game();
    ~Game();

    void init();
    void run();
    void update(float deltaTime);
    void render(float deltaTime);
    void spawnEnemies();
    void spawnAdditionalEnemy();  // spawn of new enemy
    void checkRespawns();         // check and make of respawn
};

#endif