#include "Game.h"
#include <iostream>

Game::Game() : renderer(800, 600, "Doom Clone"), player(nullptr), map(nullptr), isRunning(false) {
}

Game::~Game() {
    delete player;
    delete map;
}

void Game::init() {
    map = new Map();
    player = new Player(1.5f, 1.5f);
    isRunning = true;
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
        player->update(0.016f, *map);
    }
}

void Game::render() {
    renderer.clear();
    // empty yet
    renderer.present();
}