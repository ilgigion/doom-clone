#include "Game.h"
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>

namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
void near(float actual, float expected, const char* message) {
    require(std::abs(actual - expected) < 0.0002f, message);
}
void playerTime() {
    Map map;
    for (int fps : {30, 60, 144}) {
        Player player(1.5f, 1.5f);
        std::array<Uint8, SDL_NUM_SCANCODES> keys{};
        keys[SDL_SCANCODE_W] = 1;
        player.handleInput(keys.data());
        for (int frame = 0; frame < fps; ++frame) player.update(1.0f / fps, map);
        near(player.getX(), 5.9f, "One second acceleration and travel");
        near(player.getVelocity(), 4.8f, "Speed in cells per second");
        keys.fill(0);
        player.handleInput(keys.data());
        for (int frame = 0; frame < fps; ++frame) player.update(1.0f / fps, map);
        near(player.getX(), 6.3f, "Braking distance");
        near(player.getVelocity(), 0.0f, "Braking completes");
        keys[SDL_SCANCODE_D] = 1;
        player.handleInput(keys.data());
        for (int frame = 0; frame < fps; ++frame) player.update(1.0f / fps, map);
        near(player.getDir(), 2.16f, "Rotation per second");
        player.takeDamage(1);
        player.update(0.15f, map);
        near(player.getDamageTimer(), 0.15f, "Damage timer uses elapsed time once");
        const float angle = player.getDir();
        player.update(0.0f, map);
        player.update(std::numeric_limits<float>::quiet_NaN(), map);
        near(player.getDir(), angle, "Invalid delta does not change state");
    }
    Enemy dead(2.0f, 2.0f, EnemyType::Melee);
    Player target(1.5f, 1.5f);
    dead.takeDamage(Enemy::MAX_HP);
    const float timer = dead.getDeathTimer();
    dead.update(target, map, 0.5f);
    near(dead.getDeathTimer(), timer - 0.5f, "Dead update counts down");
}
void collision() {
    Map map;
    require(map.canOccupy(1.5f, 1.5f, 0.25f), "Free spawn");
    require(!map.canOccupy(10.0f, 7.0f, 0.2f), "Old spawn in wall");
    require(!map.canOccupy(13.0f, 10.0f, 0.2f), "Second old spawn in wall");
    require(!map.canOccupy(1.1f, 1.5f, 0.25f), "Radius overlaps border wall");
    require(map.canOccupy(1.25f, 1.5f, 0.25f), "Tangency is allowed");
    require(!map.canOccupy(-0.1f, 1.5f, 0.25f), "Outside map");
    require(!map.canOccupy(std::numeric_limits<float>::infinity(), 1.5f, 0.25f), "Nonfinite position");
    Player player(1.5f, 1.5f);
    std::array<Uint8, SDL_NUM_SCANCODES> keys{};
    keys[SDL_SCANCODE_S] = 1;
    player.handleInput(keys.data());
    player.update(2.0f, map);
    require(player.getX() >= 1.0f + player.getRadius() && player.getX() < 1.5f, "Large update cannot cross border");
    require(map.canOccupy(player.getX(), player.getY(), player.getRadius()), "Whole player remains clear");
}
void key(Game& game, SDL_Keycode code) {
    SDL_Event event{};
    event.type = SDL_KEYDOWN;
    event.key.keysym.sym = code;
    require(SDL_PushEvent(&event) == 1, "Push menu event");
    game.update(0.0f);
}
std::vector<float> gameTime(int fps) {
    Game game;
    game.init();
    std::srand(12345);
    const float initialX = game.getEnemy(0).getX();
    for (int frame = 0; frame < fps; ++frame) game.update(1.0f / fps);
    near(game.getEnemy(0).getX(), initialX, "Menu pauses simulation");
    key(game, SDLK_RETURN);
    for (int frame = 0; frame < fps * 9; ++frame) game.update(1.0f / fps);
    require(game.getEnemyCount() == 5, "Spawn after eight seconds");
    std::vector<float> result{static_cast<float>(game.getPlayer().getHP())};
    for (size_t i = 0; i < game.getEnemyCount(); ++i) {
        result.push_back(game.getEnemy(i).getX());
        result.push_back(game.getEnemy(i).getY());
    }
    key(game, SDLK_ESCAPE);
    for (int frame = 0; frame < fps; ++frame) game.update(1.0f / fps);
    near(game.getEnemy(0).getX(), result[1], "Escape pauses simulation");
    return result;
}
void spawns() {
    Game game;
    game.init();
    for (int attempt = 0; attempt < 70; ++attempt) game.spawnAdditionalEnemy();
    require(game.getEnemyCount() == 50, "Population limit");
    for (size_t i = 0; i < game.getEnemyCount(); ++i) {
        const Enemy& enemy = game.getEnemy(i);
        require(game.getMap().canOccupy(enemy.getX(), enemy.getY(), 0.2f), "Spawn radius is clear");
    }

}
}

int runSimulationTests() {
    try {
        playerTime();
        collision();
        const auto baseline = gameTime(30);
        for (int fps : {60, 144}) {
            const auto actual = gameTime(fps);
            require(actual.size() == baseline.size(), "Same population at different FPS");
            for (size_t i = 0; i < actual.size(); ++i) near(actual[i], baseline[i], "Same simulation at different FPS");
        }
        spawns();
        std::cout << "Simulation tests passed: time, collision, FPS, pause, spawns\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Simulation test failed: " << error.what() << '\n';
        return 1;
    }
}
