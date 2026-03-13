#ifndef ENEMY_H
#define ENEMY_H

#include "Entity.h"

class Map;
class Player;
class Renderer;

enum class EnemyType
{
    Melee,
    Ranged
};

class Enemy : public Entity
{
public:
    EnemyType type;
    float velocity;
    float radius;

    //added hp system
    int hp;
    float attackTimer;

    //******NEW FIELDS FOR RESPAWN****
    float deathTimer;
    bool markedForRespawn;
    float respawnX;
    float respawnY;
    EnemyType respawnType;

    //******NEW CONSTANTS*****
    static constexpr int MAX_HP = 100; //max health
    static constexpr int MELEE_DAMAGE = 7; //mili damage
    static constexpr int RANGED_DAMAGE = 5; //damage range
    static constexpr float ATTACK_RANGE = 0.8f; //attack dist
    static constexpr float RANGED_ATTACK_RANGE = 5.0f; //range of attack of range enemy
    static constexpr float ATTACK_COOLDOWN_MILI = 1.5f; //delay mili
    static constexpr float ATTACK_COOLDOWN_RANGE = 2.0f; //delay range
    static constexpr float DEATH_TIMEOUT = 1.0f; //time before corpse disappears
    static constexpr float RESPAWN_DELAY = 10.0f; //time before respawn

    constexpr float getAttackRangeForType(EnemyType type) {
        return (type == EnemyType::Melee) ? ATTACK_RANGE : RANGED_ATTACK_RANGE;
    }

    constexpr float getCooldownForType(EnemyType type) {
        return (type == EnemyType::Melee) ? ATTACK_COOLDOWN_MILI : ATTACK_COOLDOWN_RANGE;
    }

    Enemy(float startX, float startY, EnemyType enemyType);

    void update(Player& player, const Map& map, float deltaTime);

    void render(Renderer& renderer) override;

    //getting damage
    void takeDamage(int amount);


    //check if enemy is alive
    bool isAlive() const;

    //health getter
    int getHP() const;

    //*******NEW METHODS FOR RESPAWN
    void updateDeathTimer(float deltaTime);
    bool shouldRespawn() const;
    void respawn();
    bool shouldRemove() const;
    EnemyType getType() const;
    float getDeathTimer() const { return deathTimer; }

    //Check if player or enemy is visible for attack(no wall)
    bool hasLineOfSight(const Player& player, const Map& map) const;
};

#endif