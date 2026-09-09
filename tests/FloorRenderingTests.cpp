#include "Renderer.h"
#include "SDLWrappers.h"
#include <filesystem>
#include <iostream>
#include <numbers>

namespace {
void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(std::string(message) + " - " + SDL_GetError());
}

class FloorCamera : public Player {
public:
    FloorCamera(float px, float py, float angle) : Player(px, py) { dir = angle; }
    void pose(float px, float py, float angle) { x = px; y = py; dir = angle; }
};

std::filesystem::path outputPath(const std::string& name) {
    char* base = SDL_GetBasePath();
    require(base != nullptr, "Executable directory");
    const std::filesystem::path directory(base);
    SDL_free(base);
    return directory / name;
}

Uint32 color(int x, int y) {
    return 0xff000000u | (static_cast<Uint32>(20 + x * 31) << 16) |
           (static_cast<Uint32>(10 + y * 41) << 8) | static_cast<Uint32>(7 + (x + y) * 17);
}

std::string fixture(int w, int h, int bits, Uint32 solid = 0) {
    const Uint32 format = bits == 24 ? SDL_PIXELFORMAT_RGB24 : SDL_PIXELFORMAT_ARGB8888;
    SDLSurfacePtr image(SDL_CreateRGBSurfaceWithFormat(0, w, h, bits, format));
    require(image != nullptr, "Create BMP fixture");
    for (int y = 0; y < h; ++y) for (int x = 0; x < w; ++x) {
        const Uint32 pixel = solid ? solid : color(x, y);
        const SDL_Rect rect{x, y, 1, 1};
        // Zero alpha in the 32-bit input verifies that a floor remains opaque.
        require(SDL_FillRect(image.get(), &rect, SDL_MapRGBA(image->format,
            (pixel >> 16) & 255, (pixel >> 8) & 255, pixel & 255, 0)) == 0, "Fill fixture");
    }
    const auto path = outputPath("floor-fixture-" + std::to_string(w) + "x" +
        std::to_string(h) + "-" + std::to_string(bits) + "-" + std::to_string(solid) + ".bmp");
    require(SDL_SaveBMP(image.get(), path.string().c_str()) == 0, "Save BMP fixture");
    return path.string();
}

std::vector<Uint32> capture(Renderer& renderer, const Player& player, int w, int h, float dt = 0) {
    Map map;
    renderer.render3D(player, map, dt);
    std::vector<Uint32> pixels(static_cast<size_t>(w) * h);
    require(SDL_RenderReadPixels(renderer.getSDLRenderer(), nullptr, SDL_PIXELFORMAT_ARGB8888,
                                 pixels.data(), w * sizeof(Uint32)) == 0, "Read rendered frame");
    return pixels;
}

void saveFrame(const std::vector<Uint32>& pixels, int w, int h, const char* name) {
    SDLSurfacePtr image(SDL_CreateRGBSurfaceWithFormatFrom(const_cast<Uint32*>(pixels.data()),
        w, h, 32, w * sizeof(Uint32), SDL_PIXELFORMAT_ARGB8888));
    require(image != nullptr, "Create preview surface");
    require(SDL_SaveBMP(image.get(), outputPath(name).string().c_str()) == 0, "Save preview");
}

void checkWorldSamples(const std::vector<Uint32>& pixels, const FloorCamera& player,
                       int w, int h, int bob = 0) {
    // Independent analytical reference: camera basis D + tan(alpha)*R.
    // Samples are in the near, unobstructed part of the starting room.
    for (int y : {h - 1, h - h / 10}) for (int x : {w / 4, w / 2, 3 * w / 4}) {
        const double alpha = -player.getFov() / 2.0 + x * static_cast<double>(player.getFov()) / w;
        const double depth = h * 0.5 / (y + 0.5 - (h / 2 + bob));
        const double worldX = player.getX() + depth * (std::cos(player.getDir()) - std::tan(alpha) * std::sin(player.getDir()));
        const double worldY = player.getY() + depth * (std::sin(player.getDir()) + std::tan(alpha) * std::cos(player.getDir()));
        const int tx = static_cast<int>((worldX - std::floor(worldX)) * 7);
        const int ty = static_cast<int>((worldY - std::floor(worldY)) * 5);
        if (pixels[static_cast<size_t>(y) * w + x] != color(tx, ty)) {
            throw std::runtime_error("Floor world sample differs at " + std::to_string(x) + "," + std::to_string(y) +
                "; world=" + std::to_string(worldX) + "," + std::to_string(worldY) +
                "; bob=" + std::to_string(bob) + "; actual=" +
                std::to_string(pixels[static_cast<size_t>(y) * w + x]) + "; expected=" + std::to_string(color(tx, ty)));
        }
    }
}

void worldAnchoring() {
    for (auto size : {std::pair{64, 48}, std::pair{800, 600}, std::pair{801, 601}, std::pair{1280, 720}}) {
        Renderer renderer(size.first, size.second, "Floor coordinates");
        FloorCamera player(1.23f, 1.37f, 0);
        for (int bits : {24, 32}) {
            require(renderer.loadFloorTexture(fixture(7, 5, bits)).value_or(false), "Load odd-sized BMP");
            player.pose(1.23f, 1.37f, 0);
            auto original = capture(renderer, player, size.first, size.second);
            checkWorldSamples(original, player, size.first, size.second);
            player.pose(1.46f, 1.56f, 0.2f);
            auto moved = capture(renderer, player, size.first, size.second);
            checkWorldSamples(moved, player, size.first, size.second);
            require(original != moved, "Floor changes with camera pose");
            player.pose(1.23f, 1.37f, 0);
            require(capture(renderer, player, size.first, size.second) == original, "Returning camera restores frame");
            if (size.first == 800 && bits == 24) saveFrame(original, 800, 600, "floor-coordinate-preview.bmp");
        }
    }
}

void fallbackAndReplacement() {
    Renderer renderer(801, 601, "Floor fallback");
    FloorCamera player(1.23f, 1.37f, 0);
    auto plain = capture(renderer, player, 801, 601);
    require(plain.back() == 0xff646464u, "Fallback covers last row at odd resolution");
    require(!renderer.loadFloorTexture("__missing_floor_test__.bmp").value_or(false), "Missing BMP fails");
    require(capture(renderer, player, 801, 601) == plain, "Missing BMP keeps fallback");
    require(renderer.loadFloorTexture(fixture(1, 1, 24)).value_or(false), "Load one-pixel floor");
    const auto loaded = capture(renderer, player, 801, 601);
    require(loaded.back() == color(0, 0), "One-pixel texture repeats");
    require(!renderer.loadFloorTexture("README.md").value_or(false), "Invalid BMP fails");
    require(capture(renderer, player, 801, 601) == loaded, "Invalid replacement preserves pixels");
}

void wallsAndHorizon() {
    constexpr int w = 320, h = 240;
    constexpr Uint32 wall = 0xffd020d0u, floor = 0xff20b040u;
    Renderer renderer(w, h, "Floor junction");
    FloorCamera player(1.23f, 1.37f, 0.2f);
    require(renderer.loadWallTexture(1, fixture(1, 1, 24, wall)).value_or(false), "Load solid wall");
    const auto before = capture(renderer, player, w, h);
    require(renderer.loadFloorTexture(fixture(1, 1, 24, floor)).value_or(false), "Load solid floor");
    const auto after = capture(renderer, player, w, h);
    int floorCount = 0, wallCount = 0;
    for (size_t i = 0; i < before.size(); ++i) {
        if (before[i] != 0xff646464u) require(before[i] == after[i], "Walls and ceiling preserved");
        if (i >= w * (h / 2)) {
            require(after[i] == floor || after[i] == wall, "No gap at wall/floor junction or horizon");
            floorCount += after[i] == floor;
            wallCount += after[i] == wall;
        }
    }
    require(floorCount > 0 && wallCount > 0, "Scene contains visible floor and walls");
    saveFrame(after, w, h, "floor-junction-preview.bmp");
}

void bobbingAndBoundaries() {
    constexpr int w = 64, h = 49;
    Renderer renderer(w, h, "Floor bobbing");
    // Keep the sampled floor clear of the border wall even at maximum bobbing.
    FloorCamera player(1.23f, 2.3f, 0);
    Map map;
    require(renderer.loadFloorTexture(fixture(7, 5, 24)).value_or(false), "Load bobbing floor");
    Uint8 keys[SDL_NUM_SCANCODES]{};
    keys[SDL_SCANCODE_W] = 1;
    player.handleInput(keys);
    bool positive = false, negative = false;
    for (int frame = 0; frame < 110; ++frame) {
        player.update(0.016f, map);
        player.pose(1.23f, 2.3f, 0.0f); // Isolate bobbing from translation into walls.
        const auto pixels = capture(renderer, player, w, h, 0.016f);
        const int bob = static_cast<int>(renderer.calculateBobOffset(player, 0));
        positive |= bob > 0; negative |= bob < 0;
        checkWorldSamples(pixels, player, w, h, bob);
    }
    require(positive && negative, "Both directions of bobbing checked");
    // Invalid map positions must not cause out-of-bounds sampling or crashes.
    for (auto position : {std::pair{-0.25f, -0.25f}, std::pair{19.99f, 19.99f}, std::pair{25.0f, 25.0f}}) {
        FloorCamera outside(position.first, position.second, 0);
        const auto pixels = capture(renderer, outside, w, h);
        require(pixels.size() == w * h, "Boundary frame produced");
    }
}

void spriteDepthAndFloor() {
    constexpr int w = 320, h = 240;
    constexpr Uint32 spriteColor = 0xffe03020u;
    Renderer renderer(w, h, "Sprite projection");
    FloorCamera player(1.5f, 1.5f, 0);
    require(renderer.loadEnemyTexture(EnemyType::Melee, fixture(1, 1, 24, spriteColor)).value_or(false), "Load sprite fixture");
    require(renderer.loadDeadEnemyTexture(fixture(1, 1, 24, spriteColor)), "Load corpse fixture");
    auto read = [&]() {
        std::vector<Uint32> pixels(w * h);
        require(SDL_RenderReadPixels(renderer.getSDLRenderer(), nullptr, SDL_PIXELFORMAT_ARGB8888,
            pixels.data(), w * sizeof(Uint32)) == 0, "Read sprite frame");
        return pixels;
    };
    auto background = capture(renderer, player, w, h);
    renderer.resetSpriteZBuffer();
    // Just in front of the wall at x=7: radial distance exceeds wall depth,
    // while forward depth is smaller. The old mixed-metric test hid this enemy.
    Enemy enemy(6.8f, 3.74f, EnemyType::Melee);
    renderer.drawEnemySprite(enemy, player);
    auto pixels = read();
    const double angle = std::atan2(2.24, 5.3);
    const int centre = static_cast<int>((angle + player.getFov() / 2) / player.getFov() * w);
    const int bottom = static_cast<int>(h / 2 + h * 0.5 / 5.3);
    require(pixels[(bottom - 1) * w + centre] == spriteColor, "Visible enemy feet reach floor at perpendicular depth");
    require(pixels[bottom * w + centre] == background[bottom * w + centre], "Sprite does not extend below floor");
    renderer.drawEnemyHPBar(enemy.getX(), enemy.getY(), enemy.getHP(), Enemy::MAX_HP, player, {255, 0, 0, 255});
    require(read() != pixels, "Visible enemy receives HP bar");

    background = capture(renderer, player, w, h);
    renderer.resetSpriteZBuffer();
    Enemy hidden(8.5f, 4.5f, EnemyType::Melee);
    renderer.drawEnemySprite(hidden, player);
    renderer.drawEnemyHPBar(hidden.getX(), hidden.getY(), hidden.getHP(), Enemy::MAX_HP, player, {255, 0, 0, 255});
    require(read() == background, "Wall hides both enemy and HP bar");

    player.pose(1.5f, 1.5f, 0.3f);
    capture(renderer, player, w, h);
    renderer.resetSpriteZBuffer();
    Enemy partial(5.0f, 4.4f, EnemyType::Melee);
    renderer.drawEnemySprite(partial, player);
    pixels = read();
    int visibleColumns = 0;
    for (int x = 0; x < w; ++x) {
        bool visible = false;
        for (int y = 0; y < h; ++y) visible |= pixels[y * w + x] == spriteColor;
        visibleColumns += visible;
    }
    const float partialDepth = 3.5f * std::cos(0.3f) + 2.9f * std::sin(0.3f);
    const int fullWidth = static_cast<int>(h * 0.8f / partialDepth);
    require(visibleColumns > 0 && visibleColumns < fullWidth, "Corner partially occludes sprite");
    player.pose(1.5f, 1.5f, 0);
    background = capture(renderer, player, w, h);
    renderer.resetSpriteZBuffer();

    enemy.takeDamage(Enemy::MAX_HP);
    renderer.drawDeadEnemySprite(enemy, player);
    pixels = read();
    require(pixels[(bottom - 1) * w + centre] == spriteColor, "Corpse rests on same floor");
    require(pixels[bottom * w + centre] == background[bottom * w + centre], "Corpse does not extend below floor");

    capture(renderer, player, w, h);
    renderer.resetSpriteZBuffer();
    Enemy edge(1.5f + 2 * std::cos(0.55f), 1.5f + 2 * std::sin(0.55f), EnemyType::Melee);
    renderer.drawEnemySprite(edge, player);
    pixels = read();
    require(std::find(pixels.begin(), pixels.end(), spriteColor) != pixels.end(), "Partly visible sprite survives centre outside FOV");
    saveFrame(pixels, w, h, "sprite-edge-preview.bmp");
}

void realAssetsPreview() {
    Renderer renderer(800, 600, "Floor preview");
    FloorCamera player(1.5f, 1.5f, 0);
    require(renderer.loadWallTexture(1, "assets/textures/wall0.bmp").value_or(false), "Load game wall");
    require(renderer.loadCeilingTexture("assets/textures/roof0.bmp").value_or(false), "Load game ceiling");
    require(renderer.loadFloorTexture("assets/textures/floor0.bmp").value_or(false), "Load game floor");
    saveFrame(capture(renderer, player, 800, 600), 800, 600, "floor-game-preview.bmp");
    player.pose(2.1f, 1.8f, 0.35f);
    saveFrame(capture(renderer, player, 800, 600), 800, 600, "floor-game-turned-preview.bmp");
    require(renderer.loadEnemyTexture(EnemyType::Melee, "assets/textures/enemy_melee.bmp").value_or(false), "Load game sprite");
    require(renderer.loadDeadEnemyTexture("assets/textures/dead_enemy.bmp"), "Load game corpse");
    player.pose(1.5f, 1.5f, 0);
    capture(renderer, player, 800, 600);
    renderer.resetSpriteZBuffer();
    Enemy alive(4.0f, 1.6f, EnemyType::Melee), dead(3.6f, 2.1f, EnemyType::Melee);
    dead.takeDamage(Enemy::MAX_HP);
    renderer.drawEnemySprite(alive, player);
    renderer.drawDeadEnemySprite(dead, player);
    renderer.drawEnemyHPBar(alive.getX(), alive.getY(), alive.getHP(), Enemy::MAX_HP, player, {255, 0, 0, 255});
    std::vector<Uint32> pixels(800 * 600);
    require(SDL_RenderReadPixels(renderer.getSDLRenderer(), nullptr, SDL_PIXELFORMAT_ARGB8888,
        pixels.data(), 800 * sizeof(Uint32)) == 0, "Read sprite preview");
    saveFrame(pixels, 800, 600, "sprite-game-preview.bmp");
}

}

int runWallTextureTests() {
    try {
        const auto pattern = fixture(7, 5, 24);
        const auto small = fixture(1, 1, 32, 0xffff0000u);
        Player player(1.5f, 1.5f);
        Map map;
        Renderer single(320, 240, "One wall texture");
        single.loadWallTexture(1, pattern);
        const auto expected = capture(single, player, 320, 240);
        Renderer multiple(320, 240, "Mixed wall dimensions");
        multiple.loadWallTexture(2, small);
        multiple.loadWallTexture(1, pattern);
        require(capture(multiple, player, 320, 240) == expected, "Wall dimensions independent of first load");
        multiple.loadWallTexture(1, small);
        single.loadWallTexture(1, small);
        require(capture(multiple, player, 320, 240) == capture(single, player, 320, 240), "Replacement updates dimensions");
        Renderer fallback(320, 240, "Fallback wall texture");
        fallback.loadWallTexture(3, small);
        fallback.loadWallTexture(2, pattern);
        require(capture(fallback, player, 320, 240) == expected, "Fallback uses its own dimensions");
        std::cout << "Wall texture dimensions passed\n";
        return 0;
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}

int runSpriteRenderingTests() {
    try { spriteDepthAndFloor(); realAssetsPreview(); return 0; }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}

int runFloorRenderingTests() {
    int failures = 0;
    for (const auto& test : {std::pair<const char*, void(*)()>{"worldAnchoring", worldAnchoring},
            {"fallbackAndReplacement", fallbackAndReplacement}, {"wallsAndHorizon", wallsAndHorizon},
            {"bobbingAndBoundaries", bobbingAndBoundaries}, {"realAssetsPreview", realAssetsPreview}}) {
        try { test.second(); std::cout << test.first << ": passed\n"; }
        catch (const std::exception& error) { ++failures; std::cerr << test.first << ": " << error.what() << '\n'; }
    }
    return failures == 0 ? 0 : 1;
}
