#include "Map.h"
#include <algorithm>
#include <cmath>

Map::Map() {
    width = 20;
    height = 20;
    tileSize = 64;

    // 1 - wall, 0 - space
    grid = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1},
        {1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
        {1, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    };
}

int Map::getTile(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return 1;
    }
    return grid[x][y];
}

bool Map::isWall(int x, int y) const {
    return getTile(x, y) == 1;
}

int Map::getWidth() const {
    return width;
}

int Map::getHeight() const {
    return height;
}

int Map::getTileSize() const {
    return tileSize;
}

bool Map::canOccupy(float x, float y, float radius) const {
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(radius) || radius < 0 ||
        x - radius < 0 || y - radius < 0 || x + radius >= width || y + radius >= height) return false;
    if (radius == 0) return !isWall(static_cast<int>(std::floor(x)), static_cast<int>(std::floor(y)));
    for (int cellX = static_cast<int>(std::floor(x - radius)); cellX <= static_cast<int>(std::floor(x + radius)); ++cellX) {
        for (int cellY = static_cast<int>(std::floor(y - radius)); cellY <= static_cast<int>(std::floor(y + radius)); ++cellY) {
            if (!isWall(cellX, cellY)) continue;
            const float dx = x - std::clamp(x, static_cast<float>(cellX), cellX + 1.0f);
            const float dy = y - std::clamp(y, static_cast<float>(cellY), cellY + 1.0f);
            if (dx * dx + dy * dy < radius * radius) return false;
        }
    }
    return true;
}
