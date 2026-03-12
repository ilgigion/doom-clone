#include "Game.h"
#include <iostream>
#include <memory>
#include <algorithm>
#include <cmath>

Game::Game() : renderer(800, 600, "Doom Clone"), player(nullptr), map(nullptr), isRunning(false),  state(GameState::Menu) {
}

Game::~Game() {}


void Game::init() {
    map = std::make_unique<Map>();
    player = std::make_unique<Player>(1.5f, 1.5f);

    spawnEnemies();

    isRunning = true;
    state = GameState::Menu;
    menu.reset();

    renderer.loadWallTexture(1, "assets/textures/wall0.bmp");
    renderer.loadFloorTexture("assets/textures/floor0.bmp");
    renderer.loadCeilingTexture("assets/textures/roof0.bmp");
    renderer.loadGunTexture("assets/textures/gun.bmp");
    renderer.loadEnemyTexture(EnemyType::Melee, "assets/textures/enemy_melee.bmp");
    renderer.loadEnemyTexture(EnemyType::Ranged, "assets/textures/enemy_range.bmp");
    menu.loadTextures(renderer.getSDLRenderer());
}

void Game::run() {
    Uint64 lastTime = SDL_GetTicks64();

    while (isRunning) {
        Uint64 currentTime = SDL_GetTicks64();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        if (deltaTime > 0.1f) deltaTime = 0.1f;
        update(deltaTime);
        render(deltaTime);
    }
}

void Game::update(float deltaTime) {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            isRunning = false;
        }

        if (state == GameState::Menu) {
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER) {
                    state = GameState::Playing;
                }
                else if (e.key.keysym.sym == SDLK_ESCAPE) {
                    isRunning = false;
                }
            }

            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                int mouseX = e.button.x;
                int mouseY = e.button.y;

                if (menu.isStartClicked(mouseX, mouseY)) {
                    state = GameState::Playing;
                }
            }
        }
        else if (state == GameState::Playing) {
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                state = GameState::Menu;
                menu.reset();
            }
        }
    }

    if (state == GameState::Playing && player != nullptr && map != nullptr) {
        player->handleInput(SDL_GetKeyboardState(NULL));
        player->update(deltaTime, *map);

        for (auto& enemy : enemies) {
            enemy->update(*player, *map, deltaTime);
        }
    }
}

void Game::render(float deltaTime) {
    renderer.clear();

    if (state == GameState::Menu) {
        menu.render(renderer.getSDLRenderer());
    }
    else if (state == GameState::Playing) {
        renderer.render3D(*player, *map, deltaTime);

        std::vector<Enemy*> sortedEnemies;
        for (auto& enemy : enemies) {
            sortedEnemies.push_back(enemy.get());
        }

        std::sort(sortedEnemies.begin(), sortedEnemies.end(),
            [this](Enemy* a, Enemy* b) {
                float distA = std::hypot(a->getX() - player->getX(), a->getY() - player->getY());
                float distB = std::hypot(b->getX() - player->getX(), b->getY() - player->getY());
                return distA > distB;
            });

        for (auto* enemy : sortedEnemies) {
            renderer.drawEnemySprite(*enemy, *player);
        }

        renderer.renderGun();
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