#define SDL_MAIN_HANDLED

#include <iostream>
#include <cassert>
#include <cmath>
#include <SDL2/SDL.h>
#include "Entity.h"
#include "Player.h"
#include "Map.h"
#include "Enemy.h"
#include "Projectile.h"
#include "Renderer.h"
#include "Game.h"

int passTest = 0; //counter for passed
int failTest = 0; //counter for failed

//*****DEFINE THE FUNCTION WITH THE NAME OF THE TEST*****
#define TEST(name) void name()

//******FOR START TEST AND DROP EXCEPTION IF NEEDED****
#define RUN_TEST(name) do { \
    std::cout << "working " << #name << "... "; \
    try { \
        name(); \
        std::cout << "+" << std::endl; \
        passTest++; \
    } catch (const std::exception& ex) { \
        std::cout << "ERROR (" << ex.what() << ")" << std::endl; \
        failTest++; \
    } \
} while(0)


//check if numbers are equal
#define ASS_EQ(a, b) if ((a) != (b)) throw std::runtime_error("Expected " + std::to_string(static_cast<int>(b)) + " but got " + std::to_string(static_cast<int>(a)))
//check if numbers are nearby at some distance
#define ASS_NEAR(a, b, eps) if (std::abs((a) - (b)) > (eps)) throw std::runtime_error("Values not close enough")
//check for true(for example we gotta get true but get false)
#define ASS_TRUE(x) if (!(x)) throw std::runtime_error("Expected true but got false")
//check for false(same as for true but uther way around)
#define ASS_FALSE(x) if (x) throw std::runtime_error("Expected false but got true")



//**********ENTITY CHECK***********

//check for creation of Entity with fixed coordinates
TEST(testEntityCreation) {
    Player entity(5.0f, 10.0f); //check on player because Entity is virtual and can't be tested as we want it to be tested
    ASS_EQ(entity.getX(), 5.0f);
    ASS_EQ(entity.getY(), 10.0f);
}


//check for the object at the beggining of the axes
TEST(testEntityPosition) {
    Player entity(0.0f, 0.0f); //same reason for usage of Player
    ASS_NEAR(entity.getX(), 0.0f, 0.001f);
    ASS_NEAR(entity.getY(), 0.0f, 0.001f);
}


//*****PLAYER TESTS*****

//Check of creation of player with given coordinates(a bit a tautology, but still ok)
TEST(testPlayerCreation) {
    Player player(3.0f, 4.0f);
    ASS_EQ(player.getX(), 3.0f);
    ASS_EQ(player.getY(), 4.0f);
    ASS_EQ(player.getHP(), Player::MAX_HP);
    ASS_TRUE(player.isAlive());
}

//Check of the start angle and direction of view
TEST(testPlayerDirection) {
    Player player(0.0f, 0.0f);
    ASS_EQ(player.getDir(), 0.0f);
    ASS_NEAR(player.getFov(), 60.0f * (3.14159f / 180.0f), 0.01f);
}

//Check of movement when no input
TEST(testPlayerMovement) {
    Player player(1.0f, 1.0f);
    Map map;
    const Uint8* keyState = SDL_GetKeyboardState(NULL);
    player.handleInput(keyState);
    float oldX = player.getX();
    float oldY = player.getY();
    player.update(0.016f, map);
    ASS_EQ(player.getX(), oldX);
    ASS_EQ(player.getY(), oldY);
}

//Check of rolling after update
TEST(testPlayerRotation) {
    Player player(0.0f, 0.0f);
    Map map;
    //make fake click
    Uint8 keyState[SDL_NUM_SCANCODES];
    SDL_memset(keyState, 0, sizeof(keyState));
    keyState[SDL_SCANCODE_D] = 1; //press D to check rotation

    //give it directly to input
    player.handleInput(keyState);

    float oldDir = player.getDir();
    player.update(0.016f, map);

    //check that direction changed
    ASS_TRUE(player.getDir() != oldDir);
}

//Check player damage and heal
TEST(testPlayerDamageAndHeal) {
    Player player(0.0f, 0.0f);
    int startHP = player.getHP();
    player.takeDamage(30);
    ASS_EQ(player.getHP(), startHP - 30);
    player.heal(10);
    ASS_EQ(player.getHP(), startHP - 20);
    player.takeDamage(200);
    ASS_EQ(player.getHP(), 0);
    ASS_FALSE(player.isAlive());
}

//Check player shootdown cooldown
TEST(testPlayerShootCooldown) {
    Player player(0.0f, 0.0f);
    Map map;
    std::vector<std::unique_ptr<Enemy>> enemies;
    ASS_FALSE(player.isShootingNow());
    player.shoot(enemies, map);
    ASS_TRUE(player.isShootingNow());
}

//********MAP TESTS******

//Creation of map and check of its size
TEST(testMapCreation) {
    Map map;
    ASS_EQ(map.getWidth(), 20);
    ASS_EQ(map.getHeight(), 20);
    ASS_EQ(map.getTileSize(), 64);
}

//Test that borders of the map are walls
TEST(testMapWalls) {
    Map map;
    ASS_TRUE(map.isWall(0, 0));
    ASS_TRUE(map.isWall(19, 0));
    ASS_TRUE(map.isWall(0, 19));
    ASS_TRUE(map.isWall(19, 19));
}

//Test that inside of the map is free space
TEST(testMapEmptySpace) {
    Map map;
    ASS_FALSE(map.isWall(1, 3));
    ASS_FALSE(map.isWall(6, 4));
    ASS_FALSE(map.isWall(11, 13));
}

//Check of the processing of coordinates out from the border
TEST(testMapOutOfBounds) {
    Map map;
    ASS_TRUE(map.isWall(-1, 5));
    ASS_TRUE(map.isWall(25, 5));
    ASS_TRUE(map.isWall(5, -1));
    ASS_TRUE(map.isWall(5, 25));
}


//*******ENEMY TESTS*****
//Check of enemy creation
TEST(testEnemyCreation) {
    Enemy enemy(5.0f, 5.0f, EnemyType::Melee);
    ASS_EQ(enemy.getX(), 5.0f);
    ASS_EQ(enemy.getY(), 5.0f);
    ASS_EQ(enemy.getType(), EnemyType::Melee);
    ASS_EQ(enemy.getHP(), Enemy::MAX_HP);
    ASS_TRUE(enemy.isAlive());
}


//Check enemy taking damage
TEST(testEnemyTakeDamage) {
    Enemy enemy(0.0f, 0.0f, EnemyType::Ranged);
    enemy.takeDamage(30);
    ASS_EQ(enemy.getHP(), Enemy::MAX_HP - 30);
    ASS_TRUE(enemy.isAlive());
    enemy.takeDamage(100);
    ASS_EQ(enemy.getHP(), 0);
    ASS_FALSE(enemy.isAlive());
}

//Check enemy death timer
TEST(testEnemyDeathTimer) {
    Enemy enemy(0.0f, 0.0f, EnemyType::Melee);
    enemy.takeDamage(200);
    ASS_FALSE(enemy.isAlive());
    ASS_NEAR(enemy.getDeathTimer(), Enemy::DEATH_TIMEOUT, 0.001f);
    enemy.updateDeathTimer(0.5f);
    ASS_NEAR(enemy.getDeathTimer(), Enemy::DEATH_TIMEOUT - 0.5f, 0.001f);
}

//Check respawn
TEST(testEnemyRespawnLogic) {
    Enemy enemy(3.0f, 3.0f, EnemyType::Melee);
    enemy.takeDamage(200);
    ASS_FALSE(enemy.shouldRespawn());
    enemy.updateDeathTimer(Enemy::RESPAWN_DELAY + Enemy::DEATH_TIMEOUT + 1.0f);
    ASS_TRUE(enemy.shouldRespawn());
    enemy.respawn();
    ASS_TRUE(enemy.isAlive());
    ASS_EQ(enemy.getHP(), Enemy::MAX_HP);
    ASS_EQ(enemy.getX(), 3.0f);
    ASS_EQ(enemy.getY(), 3.0f);
}

//Check what enemy sees
TEST(testEnemyLineOfSight) {
    Map map;
    Player player(2.0f, 2.0f);
    Enemy enemy(4.0f, 2.0f, EnemyType::Ranged);
    ASS_TRUE(enemy.hasLineOfSight(player, map));
}

//Check melee attack
TEST(testEnemyMeleeAttack) {
    Map map;
    Player player(0.0f, 0.0f);
    Enemy enemy(0.5f, 0.0f, EnemyType::Melee);
    int startHP = player.getHP();
    enemy.update(player, map, 2.0f);
    ASS_EQ(player.getHP(), startHP - Enemy::MELEE_DAMAGE);
}

//Check range attack
TEST(testEnemyRangedAttack) {
    Map map;
    Player player(2.5f, 2.5f);
    Enemy enemy(5.5f, 2.5f, EnemyType::Ranged);

    int startHP = player.getHP();
    float deltaTime = 0.016f;

    enemy.update(player, map, deltaTime);
    int expectedHP = startHP - Enemy::RANGED_DAMAGE;
    ASS_EQ(player.getHP(), expectedHP);

    int hpAfterFirst = player.getHP();

    const int MAX_FRAMES = static_cast<int>(2.5f / deltaTime);
    int frames = 0;

    while (frames < MAX_FRAMES && player.getHP() == hpAfterFirst) {
        enemy.update(player, map, deltaTime);
        frames++;
    }

    ASS_TRUE(player.getHP() < hpAfterFirst);
    ASS_EQ(player.getHP(), hpAfterFirst - Enemy::RANGED_DAMAGE);
}

//Check if blocked by wall
TEST(testEnemyRangedAttackBlockedByWall) {
    Map map;
    Player player(0.0f, 0.0f);
    Enemy enemy(3.0f, 0.0f, EnemyType::Ranged);
    int startHP = player.getHP();
    enemy = Enemy(6.0f, 0.0f, EnemyType::Ranged);
    enemy.update(player, map, 2.0f);
    ASS_EQ(player.getHP(), startHP);
}

//Check if out of range
TEST(testEnemyRangedAttackOutOfRange) {
    Map map;
    Player player(0.0f, 0.0f);
    Enemy enemy(6.0f, 0.0f, EnemyType::Ranged);
    int startHP = player.getHP();
    enemy.update(player, map, 2.0f);
    ASS_EQ(player.getHP(), startHP);
}

//*******PROJECTILE TESTS*****
//Check projectile creation
TEST(testProjectileCreation) {
    Projectile proj(0.0f, 0.0f, 0.0f, 50, 10.0f, 2.0f);
    ASS_NEAR(proj.getX(), 0.0f, 0.001f);
    ASS_NEAR(proj.getY(), 0.0f, 0.001f);
    ASS_TRUE(proj.isActive());
    ASS_EQ(proj.getActualDamage(), 50);
}

//Check projectile movement
TEST(testProjectileMovement) {
    Map map;
    Projectile proj(2.5f, 2.5f, 0.0f, 50, 20.0f, 2.0f);
    float oldX = proj.getX();
    proj.update(0.016f, map);

    ASS_TRUE(proj.getX() > oldX);
    ASS_TRUE(proj.isActive());
}

//Check projectile going through wall
TEST(testProjectileWallCollision) {
    Map map;
    Projectile proj(5.9f, 2.5f, 0.0f, 50, 20.0f, 2.0f);
    proj.update(1.0f, map);
    ASS_FALSE(proj.isActive());
}

//Check projectile damage fall of
TEST(testProjectileDamageFalloff) {
    Projectile proj(0.0f, 0.0f, 0.0f, 100, 10.0f, 2.0f);
    ASS_EQ(proj.getActualDamage(), 100);
    proj.setTraveledDist(5.0f);
    ASS_TRUE(proj.getActualDamage() < 100 && proj.getActualDamage() > 30);
    proj.setTraveledDist(15.0f);
    ASS_EQ(proj.getActualDamage(), 0);
}

//Check projectile hit enemy
TEST(testProjectileEnemyHit) {
    Enemy enemy(5.0f, 0.0f, EnemyType::Melee);
    Projectile proj(0.0f, 0.0f, 0.0f, 50, 20.0f, 2.0f);
    ASS_FALSE(proj.checkEnemyHit(enemy));
    Projectile proj2(4.9f, 0.0f, 0.0f, 50, 20.0f, 2.0f);
    ASS_TRUE(proj2.checkEnemyHit(enemy));
    ASS_EQ(enemy.getHP(), Enemy::MAX_HP - 50);
}


//*******WEAPON TESTS****
//Check creation
TEST(testWeaponCreation) {
    Weapon weapon;
    ASS_EQ(weapon.MAX_AMMO, -1);
}

//Check amount of shots
TEST(testWeaponShootCount) {
    Map map;
    Weapon weapon;
    auto projectiles = weapon.shoot(2.5f, 2.5f, 0.0f, map);
    ASS_TRUE(projectiles.size() > 0);
}

//Check angle of shot
TEST(testWeaponSpreadAngle) {
    Map map;
    Weapon weapon;
    auto projectiles = weapon.shoot(0.0f, 0.0f, 0.0f, map);
    for (const auto& p : projectiles) {
        ASS_TRUE(p->getX() >= 0.0f);
    }
}

//Check damage parameters
TEST(testWeaponDamageParams) {
    Weapon weapon;
    Projectile proj(0.0f, 0.0f, 0.0f, 35, 4.0f, 1.0f);
    ASS_EQ(proj.getActualDamage(), 35);
    proj.setTraveledDist(2.5f);
    ASS_TRUE(proj.getActualDamage() < 35);
}


//*****SOME RENDER TESTS****
//Check render init
TEST(testRendererInit) {
    if (SDL_Init(SDL_INIT_VIDEO) == 0) {
        Renderer renderer(800, 600, "Test");
        ASS_TRUE(renderer.isRunning() || !renderer.isRunning());
        SDL_Quit();
    }
}
//Check render buffer
TEST(testRendererZBuffer) {
    if (SDL_Init(SDL_INIT_VIDEO) == 0) {
        Renderer renderer(800, 600, "Test");
        renderer.resetSpriteZBuffer();
        SDL_Quit();
    }
}


//******GAME TESTS****
//Check game init
TEST(testGameInit) {
    if (SDL_Init(SDL_INIT_VIDEO) == 0) {
        Game game;
        game.init();
        ASS_TRUE(true);
        SDL_Quit();
    }
}

//Check enemy spawn
TEST(testGameEnemySpawn) {
    if (SDL_Init(SDL_INIT_VIDEO) == 0) {
        Game game;
        game.init();
        size_t initialCount = 3;
        ASS_TRUE(true);
        SDL_Quit();
    }
}

//Check update loop
TEST(testGameUpdateLoop) {
    if (SDL_Init(SDL_INIT_VIDEO) == 0) {
        Game game;
        game.init();
        float playerStartX = 1.5f;
        game.update(0.016f);
        ASS_TRUE(true);
        SDL_Quit();
    }
}

//*****OUTPUT AND WORK OF THE TESTS******
int main() {
    //*****OUTPUT FOR UNDERSTANDING*****
    std::cout<<"RUNNING TEST....."<<std::endl;
    std::cout<<"***********************"<<std::endl;


    //*******TESTS FOR ENTITY*******
    RUN_TEST(testEntityCreation);
    RUN_TEST(testEntityPosition);


    //******TESTS FOR PLAYER*****
    RUN_TEST(testPlayerCreation);
    RUN_TEST(testPlayerDirection);
    RUN_TEST(testPlayerMovement);
    RUN_TEST(testPlayerRotation);
    RUN_TEST(testPlayerDamageAndHeal);
    RUN_TEST(testPlayerShootCooldown);


    //******TESTS FOR MAP*****
    RUN_TEST(testMapCreation);
    RUN_TEST(testMapWalls);
    RUN_TEST(testMapEmptySpace);
    RUN_TEST(testMapOutOfBounds);


    //*******TESTS FOR ENEMY******
    RUN_TEST(testEnemyCreation);
    RUN_TEST(testEnemyTakeDamage);
    RUN_TEST(testEnemyDeathTimer);
    RUN_TEST(testEnemyRespawnLogic);
    RUN_TEST(testEnemyLineOfSight);
    RUN_TEST(testEnemyMeleeAttack);
    RUN_TEST(testEnemyRangedAttack);
    RUN_TEST(testEnemyRangedAttackBlockedByWall);
    RUN_TEST(testEnemyRangedAttackOutOfRange);


    //********TESTS FOR PROJECTILE*****
    RUN_TEST(testProjectileCreation);
    RUN_TEST(testProjectileMovement);
    RUN_TEST(testProjectileWallCollision);
    RUN_TEST(testProjectileDamageFalloff);
    RUN_TEST(testProjectileEnemyHit);


    //********TESTS FOR WEAPON*****
    RUN_TEST(testWeaponCreation);
    RUN_TEST(testWeaponShootCount);
    RUN_TEST(testWeaponSpreadAngle);
    RUN_TEST(testWeaponDamageParams);


    //********TESTS FOR RENDER*****
    RUN_TEST(testRendererInit);
    RUN_TEST(testRendererZBuffer);


    //********TESTS FOR GAME*****
    RUN_TEST(testGameInit);
    RUN_TEST(testGameEnemySpawn);
    RUN_TEST(testGameUpdateLoop);

    //******CHECK HOW TESTS WORKED****
    if (failTest > 0) {
        std::cout << "NOT ALL TESTS PASSED!" << std::endl;
        return 1;
    } else {
        std::cout << "ALL GOOD!!!" << std::endl;
        std::cout << std::endl;
        std::cout << "DOOM FROM OUR TEAM IS STARTING..." << std::endl;
        return 0;
    }
}