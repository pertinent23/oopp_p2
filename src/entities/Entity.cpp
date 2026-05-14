#include "Entity.hpp"


Entity::Entity(
    float x, 
    float y, 
    float w, 
    float h
) : position(x, y), size(w, h), velocity(0.0f, 0.0f) 
{}


const Vector2D& Entity::getPosition() const 
{
    return position;
}


const Vector2D& Entity::getSize() const 
{
    return size;
}


const Vector2D& Entity::getVelocity() const 
{
    return velocity;
}


Entity::Rect Entity::getHitbox() const 
{
    return {position.x, position.y, size.x, size.y};
}
