#ifndef ENTITY_H
#define ENTITY_H

class Entity {
protected:
    // position
    float x;
    float y;
    
    float dir;
    bool active;

public:
    Entity(float startX, float startY);
    virtual ~Entity();
    
    virtual void update(float deltaTime);
    virtual void render(class Renderer& renderer) = 0;

    float getX() const;
    float getY() const;
};

#endif