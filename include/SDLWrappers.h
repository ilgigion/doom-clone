#ifndef SDL_WRAPPERS_H
#define SDL_WRAPPERS_H
#include <SDL2/SDL.h>
#include <memory>

struct SDLDeleter {
    void operator()(SDL_Window* w) const { SDL_DestroyWindow(w); }
    void operator()(SDL_Renderer* r) const { SDL_DestroyRenderer(r); }
    void operator()(SDL_Surface* s) const { SDL_FreeSurface(s); }
    void operator()(SDL_Texture* t) const { SDL_DestroyTexture(t); }
};

using SDLWindowPtr = std::unique_ptr<SDL_Window, SDLDeleter>;
using SDLRendererPtr = std::unique_ptr<SDL_Renderer, SDLDeleter>;
using SDLSurfacePtr = std::unique_ptr<SDL_Surface, SDLDeleter>;
using SDLTexturePtr = std::unique_ptr<SDL_Texture, SDLDeleter>;

#endif