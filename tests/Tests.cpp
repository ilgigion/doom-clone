#define SDL_MAIN_HANDLED

#include <iostream>
#include <cassert>
#include <cmath>
#include <SDL2/SDL.h>
#include "../include/Entity.h"
#include "../include/Player.h"

int passTest = 0; //counter for passed
int failTest = 0; //counter for failed

//*****DEFINE THE FUNCTION WITH THE NAME OF THE TEST*****
#define TEST(name) void name()

//******FOR START TEST AND DROP EXCEPTION IF NEEDED
#define RUN_TEST(name) do { \
    std::cout << "working " << #name << "... "; \
    try { \
        name(); \
        std::cout << "all good" << std::endl; \
        passTest++; \
    } catch (const std::exception& ex) { \
        std::cout << "all bad (" << ex.what() << ")" << std::endl; \
        failTest++; \
    } \
} while(0)
//check if numbers are equal
#define ASS_EQ(a, b) if ((a) != (b)) throw std::runtime_error("Expected " + std::to_string(b) + " but got " + std::to_string(a))
//check if numbers are nearby at some distance
#define ASS_NEAR(a, b, eps) if (std::abs((a) - (b)) > (eps)) throw std::runtime_error("Values not close enough")



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

//*****OUTPUT AND WORK OF THE TESTS******

int main() {
    std::cout<<"RUNNING TEST....."<<std::endl;
    std::cout<<"***********************"<<std::endl;
    //*******TESTS FOR ENTITY*******
    RUN_TEST(testEntityCreation);
    RUN_TEST(testEntityPosition);

    //check if tests failed
    if (failTest > 0) {
        std::cout << "NOT ALL TESTS PASSED!" << std::endl;
        return 1;
    } else {
        std::cout << "ALL GOOD!!!" << std::endl;
        std::cout << std::endl;
        std::cout << "DOOM FROM VANKA MOROZ SANKA AND MARK IS STARTING..." << std::endl;
        return 0;
    }
}