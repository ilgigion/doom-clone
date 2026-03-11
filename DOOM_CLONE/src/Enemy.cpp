#include "Enemy.h"
#include "Player.h"
#include "Map.h"
#include <cmath>

Enemy::Enemy(float startX, float startY, EnemyType enemyType)
    : Entity(startX, startY)
{
    speed = 2.0f;
    radius = 0.2f;
    type = enemyType;
}

void Enemy::update(const Player& player, const Map& map, float deltaTime)
{
    float dx = player.getX() - x;
    float dy = player.getY() - y;
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance <= 0.001f)
        return;

    if (type == EnemyType::Melee)
    {
        if (distance > 0.7f)
        {
            dx /= distance;
            dy /= distance;

            float newX = x + dx * speed * deltaTime;
            float newY = y + dy * speed * deltaTime;

            if (!map.isWall((int)(newX + dx * radius), (int)y))
                x = newX;

            if (!map.isWall((int)x, (int)(newY + dy * radius)))
                y = newY;
        }
    }
    else if (type == EnemyType::Ranged)
    {
        if (distance > 3.0f)
        {
            dx /= distance;
            dy /= distance;

            float newX = x + dx * speed * deltaTime;
            float newY = y + dy * speed * deltaTime;

            if (!map.isWall((int)(newX + dx * radius), (int)y))
                x = newX;

            if (!map.isWall((int)x, (int)(newY + dy * radius)))
                y = newY;
        }
    }
}