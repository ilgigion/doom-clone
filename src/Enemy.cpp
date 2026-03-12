#include "Enemy.h"
#include "Player.h"
#include "Map.h"
#include <cmath>

//added parameters
Enemy::Enemy(float startX, float startY, EnemyType enemyType)
    : Entity(startX, startY), hp(MAX_HP), attackTimer(0.0f),
      deathTimer(0.0f), markedForRespawn(false),
      respawnX(startX), respawnY(startY), respawnType(enemyType) {
    velocity = 2.0f; //movespeed
    radius = 0.2f; //hitbox range
    type = enemyType; //mili or range
}

//take damage
void Enemy::takeDamage(int amount) {
    //make damage but not lower than 0
    hp = std::max(0, hp - amount);

    //if dead and not marked(for respawn) set up timer
    if (hp <= 0 && !markedForRespawn) {
        deathTimer = DEATH_TIMEOUT;//5 seconds of dead enemy
        markedForRespawn = true;//make a mark for a future respawn
    }
}

//check if alive
bool Enemy::isAlive() const {
    return hp > 0;
}

//getter of curr health
int Enemy::getHP() const {
    return hp;
}

//update of respawn/death timer
void Enemy::updateDeathTimer(float deltaTime) {
    if (!isAlive() && markedForRespawn) {
        deathTimer -= deltaTime;
        // Cap the timer so it isnt negative
        if (deathTimer <= -RESPAWN_DELAY + DEATH_TIMEOUT) {
            deathTimer = -RESPAWN_DELAY + DEATH_TIMEOUT;
        }
    }
}

//check if we can respawn enemy
bool Enemy::shouldRespawn() const {
    //20 secs after death respawn
    return !isAlive() && markedForRespawn && deathTimer <= -RESPAWN_DELAY + DEATH_TIMEOUT;
}

//update for respawn
void Enemy::respawn() {
    //return to the position of spawn
    x = respawnX;
    y = respawnY;
    type = respawnType;

    //drop health and timer(so it respawns with full health)
    hp = MAX_HP;
    attackTimer = 0.0f;
    deathTimer = 0.0f;
    markedForRespawn = false;

    //make it active
    active = true;
}


//check if we can remove enemy(no, cause all of them respawn)
bool Enemy::shouldRemove() const {
    return false;
}

//check if is visible(true if no walls and we can attack)
bool Enemy::hasLineOfSight(const Player& player, const Map& map) const {
    float dx = player.getX() - x;
    float dy = player.getY() - y;
    float distance = std::sqrt(dx * dx + dy * dy);
    if (distance <= 0.001f) return true; //too near

    //normalize vector of direction
    dx /= distance;
    dy /= distance;

    //make a way to player
    float step = 0.1f;//step of check(smaller - more percise)
    float dist = 0.0f;

    while (dist < distance) {
        float testX = x + dx * dist;
        float testY = y + dy * dist;
        //if we hit the wall then cant see
        if (map.isWall(static_cast<int>(testX), static_cast<int>(testY))) {
            return false;
        }
        dist += step;
    }
    //in other case return true - all good
    return true;
}

void Enemy::update(Player& player, const Map& map, float deltaTime)
{
    //if dead update respawn timer
    if (!isAlive()) {
        deathTimer += deltaTime;
        return;
    }

    float dx = player.getX() - x;
    float dy = player.getY() - y;
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance <= 0.001f)
        return;



    //melee range
    if (type == EnemyType::Melee && distance < ATTACK_RANGE) {
        //if ready for attack
        if (attackTimer <= 0.0f) {
            player.takeDamage(MELEE_DAMAGE);
            //drop timer for cooldown
            attackTimer = ATTACK_COOLDOWN_MILI;
        } else {
            attackTimer -= deltaTime;
        }
        return;
    }

    //ranged range
    if (type == EnemyType::Ranged && distance < RANGED_ATTACK_RANGE) {
        //check that there is no wall
        if (hasLineOfSight(player, map)) {
            //if ready for attack
            if (attackTimer <= 0.0f) {
                player.takeDamage(RANGED_DAMAGE);
                //drop timer for cooldown
                attackTimer = ATTACK_COOLDOWN_RANGE;
            } else {
                attackTimer -= deltaTime;
            }
        }
        //range enemies don't move when attack
        return;
    }

    //if out of range drop timer
    if (distance >= ATTACK_RANGE * 1.2f && distance >= RANGED_ATTACK_RANGE * 1.2f) {
        attackTimer = 0.0f;
    }


    if (type == EnemyType::Melee)
    {
        if (distance > 0.7f)
        {
            dx /= distance;
            dy /= distance;

            float newX = x + dx * velocity * deltaTime;
            float newY = y + dy * velocity * deltaTime;

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

            float newX = x + dx * velocity * deltaTime;
            float newY = y + dy * velocity * deltaTime;

            if (!map.isWall((int)(newX + dx * radius), (int)y))
                x = newX;

            if (!map.isWall((int)x, (int)(newY + dy * radius)))
                y = newY;
        }
    }
}

void Enemy::render(Renderer& renderer){}

EnemyType Enemy::getType() const {
    return type;
}