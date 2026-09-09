#ifndef GAME_H
#define GAME_H
#include <stdexcept>
#include <string>

//сustom exceptions for specific error types
class ResourceLoadException : public std::runtime_error {
public:
    explicit ResourceLoadException(const std::string& resource)
        : std::runtime_error("Failed to load resource: " + resource) {}
};

class InitializationException : public std::runtime_error {
public:
    explicit InitializationException(const std::string& msg)
        : std::runtime_error("Initialization failed: " + msg) {}
};

#include "Renderer.h"
#include "Player.h"
#include "Map.h"
#include "Enemy.h"
#include "Menu.h"
#include <memory>
#include <vector>
#include <SDL2/SDL_mixer.h>

enum class GameState
{
    Menu,
    Playing
};

class Game {
private:
    Renderer renderer;
    std::unique_ptr<Player> player;
    std::unique_ptr<Map> map;
    std::vector<std::unique_ptr<Enemy>> enemies;
    bool isRunning;

    Menu menu;
    GameState state;
    double simulationAccumulator = 0.0;
    static constexpr double SIMULATION_STEP = 1.0 / 120.0;
    void simulate(float deltaTime);

    //****TIMERS FOR RESPAWN AND SPAWN OF ENEMIES***
    float enemySpawnTimer;        // timer fornext spawn
    float enemyRespawnCheckTimer; // timer of respawn check
    static constexpr float SPAWN_INTERVAL = 8.0f;    // const for spawning new enemies
    static constexpr float RESPAWN_CHECK_INTERVAL = 1.0f; // check respawn every sec
    static constexpr int MAX_ENEMIES = 50;            // max amount of enemies

    //*******MUSIC*****
    Mix_Music* backgroundMusic;
    bool musicEnabled;
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
    const Player& getPlayer() const { return *player; } // Valid after init().
    const Map& getMap() const { return *map; }
    size_t getEnemyCount() const { return enemies.size(); }
    const Enemy& getEnemy(size_t index) const { return *enemies.at(index); }

    //*****MUSIC CONTROLING******
    void initMusic();
    void loadMusic(const std::string& path);
    void playMusic(bool loop = true);
    void setMusicVolume(int volume);
    void cleanupMusic();
};

#endif
