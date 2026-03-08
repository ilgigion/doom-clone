#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>

class Renderer {
private:
    SDL_Window* window;
    SDL_Renderer* sdlRenderer;
    int width;
    int height;

public:
    Renderer(int w, int h, const char* title);
    ~Renderer();

    void clear();
    void present();
    bool isRunning() const;
    
    // for 3d rendering in future
    void drawWall(int x, int height, int textureId); 
    SDL_Renderer* getSDLRenderer();
};

#endif