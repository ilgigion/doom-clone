#include "Game.h"
#include <iostream>

Game::Game() : renderer(800, 600, "Doom Clone"), player(nullptr), map(nullptr), isRunning(false) {
}

Game::~Game() {}

void Game::init() {
    map = std::make_unique<Map>();
    player = std::make_unique<Player>(1.5f, 1.5f);
    isRunning = true;

    renderer.loadWallTexture(1, "assets/textures/wall.bmp");
    renderer.loadFloorTexture("assets/textures/floor.bmp");
    renderer.loadCeilingTexture("assets/textures/roof.bmp");
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
}

void Game::render() {
    renderer.clear();

    renderer.render3D(*player, *map);
    renderer.renderGun();
    
    renderer.present();
}