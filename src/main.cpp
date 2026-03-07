#include <iostream>
#include <SDL2/SDL.h>

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not initialize! Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    std::cout << "Project initialized successfully!" << std::endl;

    SDL_Quit();
    return 0;
}
