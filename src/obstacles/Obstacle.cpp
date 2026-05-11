#include "Obstacle.hpp"


Obstacle::Obstacle(
    float x, 
    float y, 
    float w, 
    float h, 
    float speedX
) : Entity(x, y, w, h), active(true) 
{
    velocity.x = -speedX;
}


void Obstacle::update(float deltaTime) 
{
    position += velocity * deltaTime;
    
    if (position.x + size.x < 0) 
    {
        deactivate();
    }
}


bool Obstacle::isActive() const 
{
    return active;
}


void Obstacle::deactivate() 
{
    active = false;
}
