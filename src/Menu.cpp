#include "Menu.h"
#include <iostream>

Menu::Menu()
    : backgroundTexture(nullptr),
      startTexture(nullptr)
{
}

Menu::~Menu()
{
    if (backgroundTexture) SDL_DestroyTexture(backgroundTexture);
    if (startTexture) SDL_DestroyTexture(startTexture);
}

bool Menu::loadTextures(SDL_Renderer* renderer)
{
    auto loadOne = [&](const char* path) -> SDL_Texture*
    {
        SDL_Surface* surface = SDL_LoadBMP(path);
        if (!surface)
        {
            std::cout << "Unable to load menu texture: " << path
                      << " Error: " << SDL_GetError() << std::endl;
            return nullptr;
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        if (!texture)
        {
            std::cout << "Unable to create menu texture: " << path
                      << " Error: " << SDL_GetError() << std::endl;
        }

        return texture;
    };

    backgroundTexture = loadOne("assets/textures/menu_background.bmp");
    startTexture = loadOne("assets/textures/button_start.bmp");

    return backgroundTexture && startTexture;
}

void Menu::handleEvent(const SDL_Event& e)
{
    // пока ничего не нужно
}

void Menu::render(SDL_Renderer* renderer)
{
    int screenWidth, screenHeight;
    SDL_GetRendererOutputSize(renderer, &screenWidth, &screenHeight);

    if (backgroundTexture)
    {
        SDL_Rect bgRect = {0, 0, screenWidth, screenHeight};
        SDL_RenderCopy(renderer, backgroundTexture, nullptr, &bgRect);
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        SDL_RenderClear(renderer);
    }

    SDL_Rect startRect = {250, 260, 300, 80};

    if (startTexture)
    {
        SDL_RenderCopy(renderer, startTexture, nullptr, &startRect);
    }
}

void Menu::reset()
{
}

MenuResult Menu::activateSelected() const
{
    return MenuResult::StartGame;
}

bool Menu::isStartClicked(int mouseX, int mouseY) const
{
    SDL_Rect startRect = {250, 260, 300, 80};

    return mouseX >= startRect.x &&
           mouseX <= startRect.x + startRect.w &&
           mouseY >= startRect.y &&
           mouseY <= startRect.y + startRect.h;
}