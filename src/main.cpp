#include "Game.h"
#include "Player.h"
#include "Map.h"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        Game game;
        game.init();
        game.run();
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Unable to start game: " << error.what() << std::endl;
        return 1;
    }
}
