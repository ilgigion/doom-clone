#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>

enum class MenuResult
{
    None,
    StartGame
};

class Menu
{
private:
    SDL_Texture* backgroundTexture;
    SDL_Texture* startTexture;

public:
    Menu();
    ~Menu();

    bool loadTextures(SDL_Renderer* renderer);
    void handleEvent(const SDL_Event& e);
    void render(SDL_Renderer* renderer);
    void reset();
    MenuResult activateSelected() const;
    bool isStartClicked(int mouseX, int mouseY) const;
};

#endif