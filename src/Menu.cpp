#include "Menu.h"

Menu::Menu() : selectedItem(0)
{
}

void Menu::handleEvent(const SDL_Event& e)
{
    if (e.type != SDL_KEYDOWN)
        return;

    if (e.key.keysym.sym == SDLK_w || e.key.keysym.sym == SDLK_UP)
    {
        selectedItem--;
        if (selectedItem < 0)
            selectedItem = 1;
    }

    if (e.key.keysym.sym == SDLK_s || e.key.keysym.sym == SDLK_DOWN)
    {
        selectedItem++;
        if (selectedItem > 1)
            selectedItem = 0;
    }
}

void Menu::render(SDL_Renderer* renderer)
{
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderClear(renderer);

    SDL_Rect titleRect = {250, 80, 300, 80};
    SDL_SetRenderDrawColor(renderer, 120, 20, 20, 255);
    SDL_RenderFillRect(renderer, &titleRect);

    SDL_Rect startRect = {300, 220, 200, 60};
    SDL_Rect quitRect  = {300, 320, 200, 60};

    if (selectedItem == 0)
        SDL_SetRenderDrawColor(renderer, 200, 200, 60, 255);
    else
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderFillRect(renderer, &startRect);

    if (selectedItem == 1)
        SDL_SetRenderDrawColor(renderer, 200, 200, 60, 255);
    else
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderFillRect(renderer, &quitRect);
}

void Menu::reset()
{
    selectedItem = 0;
}

MenuResult Menu::activateSelected() const
{
    if (selectedItem == 0)
        return MenuResult::StartGame;

    if (selectedItem == 1)
        return MenuResult::Quit;

    return MenuResult::None;
}