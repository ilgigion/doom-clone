#include "Renderer.h"
#include <cstdlib>
#include <iostream>
#include <limits>

int runEffectsTests() {
    try {
        Renderer renderer(64, 48, "Effects test");
        auto require = [](bool condition, const char* message) {
            if (!condition) throw std::runtime_error(message);
        };
        auto sample = [&](float alpha) {
            SDL_SetRenderDrawColor(renderer.getSDLRenderer(), 0, 0, 0, 255);
            SDL_RenderClear(renderer.getSDLRenderer());
            SDL_SetRenderDrawColor(renderer.getSDLRenderer(), 12, 34, 56, 78);
            SDL_SetRenderDrawBlendMode(renderer.getSDLRenderer(), SDL_BLENDMODE_ADD);
            renderer.renderDamageOverlay(alpha);
            SDL_BlendMode mode;
            SDL_GetRenderDrawBlendMode(renderer.getSDLRenderer(), &mode);
            require(mode == SDL_BLENDMODE_ADD, "Overlay restores blend mode");
            Uint8 r, g, b, a;
            SDL_GetRenderDrawColor(renderer.getSDLRenderer(), &r, &g, &b, &a);
            require(r == 12 && g == 34 && b == 56 && a == 78, "Overlay restores draw color");
            Uint32 pixel = 0;
            const SDL_Rect area{20, 20, 1, 1};
            require(SDL_RenderReadPixels(renderer.getSDLRenderer(), &area, SDL_PIXELFORMAT_ARGB8888,
                &pixel, sizeof(pixel)) == 0, "Read overlay pixel");
            require((pixel & 0xffffu) == 0, "Overlay remains red");
            return (pixel >> 16) & 255;
        };
        const auto full = sample(0.2f), half = sample(0.1f);
        require(full >= 49 && full <= 52 && half >= 24 && half <= 27, "Damage flash fades with alpha");
        require(sample(0.0f) == 0 && sample(-1.0f) == 0, "No overlay for nonpositive alpha");
        require(sample(std::numeric_limits<float>::quiet_NaN()) == 0, "Ignore invalid alpha");
        require(sample(2.0f) == 255, "Clamp excessive alpha");

        Player player(1.5f, 1.5f);
        Map map;
        std::vector<std::unique_ptr<Enemy>> enemies;
        player.shoot(enemies, map);
        std::srand(91);
        const int expected = std::rand();
        std::srand(91);
        for (int frame = 0; frame < 10; ++frame) renderer.renderHUD(player);
        require(std::rand() == expected, "Visual effects do not consume gameplay random numbers");
        std::cout << "Effects tests passed: alpha, SDL state, isolated randomness\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
