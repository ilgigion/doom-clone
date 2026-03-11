#ifndef MAP_H
#define MAP_H

#include <vector>

class Map {
private:
    std::vector<std::vector<int>> grid;
    int width;
    int height;
    // size of one tile (quanity of pixels for one block of map)
    int tileSize;

public:
    Map();
    
    int getTile(int x, int y) const;
    bool isWall(int x, int y) const;
    int getWidth() const;
    int getHeight() const;
    int getTileSize() const;
};

#endif