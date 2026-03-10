#include "Game.h"
#include "Player.h"
#include "Map.h"

int main(int argc, char* argv[]) {
    Game game;
    game.init();
    game.run();
    return 0;
}