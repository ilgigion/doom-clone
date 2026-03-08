#include "Entity.h"

Entity::Entity(float startX, float startY) : x(startX), y(startY), dir(0.0f), active(true) {
}

Entity::~Entity() {
}

void Entity::update(float deltaTime) {
    // base is empty now
}

float Entity::getX() const {
    return x;
}

float Entity::getY() const {
    return y;
}