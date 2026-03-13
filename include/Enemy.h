#ifndef ENEMY_H
#define ENEMY_H

#include "Entity.h"

class Map;
class Player;
class Renderer;

//types of enemies
enum class EnemyType { Melee, Ranged };

class Enemy : public Entity {
public:
    EnemyType type; //type mili or range
    float velocity; //speed of movement
    float radius; //range of enemy
    int hp; //current hp
    float attackTimer; //attack timer

    // NEW: поля для системы респавна
    float deathTimer;
    bool markedForRespawn;
    float respawnX;
    float respawnY;
    EnemyType respawnType;

    //balance constants
    static constexpr int MAX_HP = 100; //max health
    static constexpr int MELEE_DAMAGE = 7; //mili damage
    static constexpr int RANGED_DAMAGE = 5; //FIXED: урон дальнего боя (было 15)
    static constexpr float ATTACK_RANGE = 0.8f; //attack dist
    static constexpr float RANGED_ATTACK_RANGE = 5.0f; //дистанция атаки дальнего боя
    static constexpr float ATTACK_COOLDOWN_MILI = 1.5f; //delay
    static constexpr float ATTACK_COOLDOWN_RANGE = 2.0f; //delay
    static constexpr float DEATH_TIMEOUT = 5.0f; //time before corpse disappears
    static constexpr float RESPAWN_DELAY = 20.0f; //time before respawn

    Enemy(float startX, float startY, EnemyType enemyType);

    //update of movement and attack
    void update(Player& player, const Map& map, float deltaTime);

    //render - signature MUST match Entity (only Renderer&)
    void render(Renderer& renderer) override;

    //getting damage
    void takeDamage(int amount);

    //check if enemy is alive
    bool isAlive() const;

    //health getter
    int getHP() const;

    // NEW: методы системы респавна
    void updateDeathTimer(float deltaTime);
    bool shouldRespawn() const;
    void respawn();
    bool shouldRemove() const;

    // NEW: проверка линии видимости (для дальних атак)
    bool hasLineOfSight(const Player& player, const Map& map) const;
};

#endif