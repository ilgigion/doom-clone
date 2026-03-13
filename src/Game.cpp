#include "Game.h"
#include <iostream>
#include <memory>
#include <algorithm>
#include <cmath>

Game::Game() : renderer(800, 600, "Doom Clone"),
    player(nullptr), 
    map(nullptr), 
    isRunning(false),  
    state(GameState::Menu), 
    enemySpawnTimer(0.0f), 
    enemyRespawnCheckTimer(0.0f),
    backgroundMusic(nullptr),
    musicEnabled(true) {
}

Game::~Game() {
    cleanupMusic();
}


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

    //*****LOAD FIRE AND DEAD TEXTURE*****
    renderer.loadGunFireTexture("assets/textures/gun_fire.bmp");
    renderer.loadDeadEnemyTexture("assets/textures/dead_enemy.bmp");

    //***MUSIC***
    initMusic();
    loadMusic("assets/music/RIPandTEAR.ogg");
    setMusicVolume(64);
    playMusic(true);
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

        if (player && map) {
            player->handleInput(SDL_GetKeyboardState(NULL));
            // Shoot on SPACE
            if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_SPACE]) {
                player->shoot(enemies, *map);
            }
            player->update(0.016f, *map);
            //update players projectiles
            player->updateProjectiles(0.016f, *map, enemies);
        }

        //timer for spawning additional enemies
        enemySpawnTimer += 0.016f;
        if (enemySpawnTimer >= SPAWN_INTERVAL) {
            enemySpawnTimer = 0.0f;
            spawnAdditionalEnemy();
        }

        //timer for checking respawns
        enemyRespawnCheckTimer += 0.016f;
        if (enemyRespawnCheckTimer >= RESPAWN_CHECK_INTERVAL) {
            enemyRespawnCheckTimer = 0.0f;
            checkRespawns();
        }

        //update enemies
        for (auto& enemy : enemies) {
            if (enemy && enemy->isAlive()) {
                enemy->update(*player, *map, 0.016f);
            } else if (enemy) {
                //dead enemies update respawn timer
                enemy->updateDeathTimer(0.016f);
            }
        }

        //check if player is dead
        if (player && !player->isAlive()) {
            std::cout << "GAME OVER\n";
            isRunning = false;
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

        renderer.resetSpriteZBuffer();

        // sort enemies
        std::vector<Enemy*> sortedEnemies;
        for (auto& enemy : enemies) {
            if (enemy) {
                sortedEnemies.push_back(enemy.get());
            }
        }

        std::sort(sortedEnemies.begin(), sortedEnemies.end(),
            [this](Enemy* a, Enemy* b) {
                float distA = std::hypot(a->getX() - player->getX(), a->getY() - player->getY());
                float distB = std::hypot(b->getX() - player->getX(), b->getY() - player->getY());
                return distA > distB;
            });

        // drawing in order
        for (auto* enemy : sortedEnemies) {
            if (enemy->isAlive()) {
                renderer.drawEnemySprite(*enemy, *player);
                renderer.drawEnemyHPBar(enemy->getX(), enemy->getY(),
                                    enemy->getHP(), Enemy::MAX_HP, *player, {255, 50, 50, 255});
            } else {
                // dead enemies
                if (enemy->getDeathTimer() > 0.0f) {
                    renderer.drawDeadEnemySprite(*enemy, *player);
                }
            }
        }

        renderer.renderGun(*player);
        if (player->getDamageTimer() > 0.0f) {
            // alpha: from 0.2 to 0.0 as it fades
            float flashAlpha = 0.2f * (player->getDamageTimer() / Player::DAMAGE_FLASH_DURATION);
            renderer.renderDamageOverlay(flashAlpha);
        }
        renderer.renderHUD(*player);
    }

    renderer.present();
}

void Game::spawnEnemies()
{
    enemies.clear();

    enemies.push_back(std::make_unique<Enemy>(10.0f, 7.0f, EnemyType::Melee));
    enemies.push_back(std::make_unique<Enemy>(5.0f, 5.0f, EnemyType::Melee));
    enemies.push_back(std::make_unique<Enemy>(8.0f, 3.0f, EnemyType::Melee));
    enemies.push_back(std::make_unique<Enemy>(13.0f, 10.0f, EnemyType::Ranged));

}

//*****SPAWN ENEMIES AFTER 30 SECONDS****
void Game::spawnAdditionalEnemy() {
    if (static_cast<int>(enemies.size()) >= MAX_ENEMIES) {
        std::cout << "Max enemies reached (" << MAX_ENEMIES << ")\n";
        return;
    }

    //random position on map
    float spawnX, spawnY;
    int attempts = 0;
    do {
        spawnX = 2.0f + static_cast<float>(rand()) / RAND_MAX * (map->getWidth() - 4.0f);
        spawnY = 2.0f + static_cast<float>(rand()) / RAND_MAX * (map->getHeight() - 4.0f);
        attempts++;
    } while (map->isWall(static_cast<int>(spawnX), static_cast<int>(spawnY)) && attempts < 10);

    //random type of enemy
    EnemyType type = (rand() % 2 == 0) ? EnemyType::Melee : EnemyType::Ranged;

    enemies.push_back(std::make_unique<Enemy>(spawnX, spawnY, type));
    std::cout << "Additional enemy spawned! Total: " << enemies.size() << "\n";
}

//check and perform respawns of killed enemies
void Game::checkRespawns() {
    for (auto& enemy : enemies) {
        if (enemy && enemy->shouldRespawn()) {
            enemy->respawn();
            std::cout << "Enemy respawned at (" << enemy->getX() << ", " << enemy->getY() << ")\n";
        }
    }
}
//*******MUSIC METHODS*******
void Game::initMusic() {
    // init mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cout << "SDL_mixer could not initialize! Error: " << Mix_GetError() << std::endl;
        musicEnabled = false;
    }
}

void Game::loadMusic(const std::string& path) {
    if (!musicEnabled) return;
    if (backgroundMusic) {
        Mix_FreeMusic(backgroundMusic);
    }
    backgroundMusic = Mix_LoadMUS(path.c_str());
    if (!backgroundMusic) {
        std::cout << "Failed to load music: " << path << " Error: " << Mix_GetError() << std::endl;
        return;
    }
}

void Game::playMusic(bool loop) {
    if (!musicEnabled || !backgroundMusic) return;
    Mix_PlayMusic(backgroundMusic, loop ? -1 : 1);
}

void Game::setMusicVolume(int volume) {
    Mix_VolumeMusic(volume);
}

void Game::cleanupMusic() {
    if (backgroundMusic) {
        Mix_FreeMusic(backgroundMusic);
        backgroundMusic = nullptr;
    }
    Mix_CloseAudio();
}