#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>

enum class MenuResult
{
    None,
    StartGame,
    Quit
};

class Menu
{
private:
    int selectedItem;

public:
    Menu();

    void handleEvent(const SDL_Event& e);
    void render(SDL_Renderer* renderer);

    void reset();
    MenuResult activateSelected() const;
};

#endif