#define SDL_MAIN_HANDLED

#include <iostream>
#include <cassert>
#include <cmath>
#include <SDL2/SDL.h>
#include "Entity.h"
#include "Player.h"
#include "Map.h"

int passTest = 0; //counter for passed
int failTest = 0; //counter for failed

//*****DEFINE THE FUNCTION WITH THE NAME OF THE TEST*****
#define TEST(name) void name()

//******FOR START TEST AND DROP EXCEPTION IF NEEDED
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
#define ASS_EQ(a, b) if ((a) != (b)) throw std::runtime_error("Expected " + std::to_string(b) + " but got " + std::to_string(a))
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
    float oldDir = player.getDir();
    const Uint8* keyState = SDL_GetKeyboardState(NULL);
    player.handleInput(keyState);
    for (int i = 0; i < 10; i++) {
        player.update(0.016f, map);
    }
    ASS_TRUE(true);
}


//********MAP TESTS******

//Creation of map and check of its size
TEST(testMapCreation) {
    Map map;
    ASS_EQ(map.getWidth(), 20);
    ASS_EQ(map.getHeight(), 20);
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
    ASS_FALSE(map.isWall(1, 1));
    ASS_FALSE(map.isWall(5, 5));
}

//Check of the processing of coordinates out from the border
TEST(testMapOutOfBounds) {
    Map map;
    ASS_TRUE(map.isWall(-1, 5));
    ASS_TRUE(map.isWall(25, 5));
    ASS_TRUE(map.isWall(5, -1));
    ASS_TRUE(map.isWall(5, 25));
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


    //******TESTS FOR MAP*****
    RUN_TEST(testMapCreation);
    RUN_TEST(testMapWalls);
    RUN_TEST(testMapEmptySpace);
    RUN_TEST(testMapOutOfBounds);


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