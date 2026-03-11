#include "Game.h"
#include <iostream>
#include <memory>

Game::Game() : renderer(800, 600, "Doom Clone"), player(nullptr), map(nullptr), isRunning(false) {
}

Game::~Game() {}


void Game::init() {
    map = std::make_unique<Map>();
    player = std::make_unique<Player>(1.5f, 1.5f);

    spawnEnemies();

    isRunning = true;


    renderer.loadWallTexture(1, "assets/textures/wall0.bmp");
    renderer.loadFloorTexture("assets/textures/floor0.bmp");
    renderer.loadCeilingTexture("assets/textures/roof0.bmp");
    renderer.loadGunTexture("assets/textures/gun.bmp");
    renderer.loadEnemyTexture("assets/textures/enemy.bmp");
}

void Game::run() {
    while (isRunning) {
        update();
        render();
    }
}

void Game::update() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            isRunning = false;
        }
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
            isRunning = false;
        }
    }

    if (player != nullptr && map != nullptr) {
        player->handleInput(SDL_GetKeyboardState(NULL));
        player->update(0.016f, *map);
    }

    for (auto& enemy : enemies)
    {
        enemy->update(*player, *map, 0.016f);
    }
}

void Game::render() {
    renderer.clear();

    renderer.render3D(*player, *map);
    renderer.renderGun();
    for (auto& enemy : enemies)
    {
        renderer.drawEnemySprite(*enemy, *player);
    }

    renderer.present();
}

void Game::spawnEnemies()
{
    enemies.clear();

    enemies.push_back(std::make_unique<Enemy>(5.5f, 5.5f, EnemyType::Melee));
    enemies.push_back(std::make_unique<Enemy>(8.5f, 3.5f, EnemyType::Melee));
    enemies.push_back(std::make_unique<Enemy>(12.5f, 10.5f, EnemyType::Ranged));

}