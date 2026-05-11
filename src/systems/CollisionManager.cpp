#include "CollisionManager.hpp"
#include "Player.hpp"
#include "Obstacle.hpp"


bool CollisionManager::checkIntersection(
    const Entity::Rect& a, 
    const Entity::Rect& b
) 
{
    return (
        a.x < b.x + b.w &&
        a.x + a.w > b.x &&
        a.y < b.y + b.h &&
        a.y + a.h > b.y
    );
}


bool CollisionManager::handleCollisions(
    Player& player, 
    std::vector<std::unique_ptr<Obstacle>>& obstacles
) 
{
    if (player.isDead()) 
    {
        return false;
    }

    bool tookDamage = false;

    for (auto& obstacle : obstacles) 
    {
        if (!obstacle->isActive()) 
        {
            continue;
        }

        if (checkIntersection(player.getHitbox(), obstacle->getHitbox())) 
        {
            int hpBefore = player.getHealth();
            
            obstacle->onCollision(player);
            
            if (player.getHealth() < hpBefore) 
            {
                tookDamage = true;
            }
        }
    }

    return tookDamage;
}
