#ifndef ENEMY_H
#define ENEMY_H

#include "Entity.h"

class Map;
class Player;

enum class EnemyType
{
    Melee,
    Ranged
};

class Enemy : public Entity
{
public:
    EnemyType type;
    float speed;
    float radius;

    Enemy(float startX, float startY, EnemyType enemyType);

    void update(const Player& player, const Map& map, float deltaTime);
};

#endif